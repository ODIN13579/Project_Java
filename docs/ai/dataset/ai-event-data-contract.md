# AI Event Data Contract — SwiftletCare

## 1. Mục đích và nguyên tắc

Đề xuất hợp đồng dữ liệu giữa AI Service/Edge Node và Backend. Đây là **draft để P2 và P4 thống nhất**, chưa phải API chính thức của TASK-05. Chi tiết endpoint, authentication và transport do Backend chốt riêng.

**Ngoài payload ingest BUILD1-04:** các raw event/cameraId/zoneId bên dưới vẫn là thiết kế cho giai đoạn sau. Backend MVP chỉ nhận aggregate CountLog với `deviceId` số, periodStart/periodEnd, counts, aggregationType và processedAt; xem [contract đang triển khai](../../api/build104-mvp.md). Không POST JSON của tài liệu này vào CountLog.

Nguyên tắc:
- Mỗi event có ID ổn định để chống xử lý trùng.
- Timestamp ISO 8601 có timezone; khuyến nghị UTC (`Z`) khi truyền.
- `cameraId` và `zoneId` là ID đã đăng ký với Backend.
- Ảnh evidence dùng URI tham chiếu có kiểm soát quyền truy cập; không nhúng ảnh/video base64 vào event.
- Track ID chỉ có ý nghĩa trong tracking session.
- Payload cần có version schema.

## 2. Event type đề xuất

- `PREDATOR_DETECTED`
- `BIRD_ENTRY`
- `BIRD_EXIT`
- Có thể bổ sung `BIRD_COUNT_SNAPSHOT` khi Backend cần thống kê theo cửa sổ; không dùng snapshot thay cho event từng lượt.

## 3. Schema JSON minh họa

```json
{
  "schemaVersion": "1.0",
  "eventId": "0199-example-unique-id",
  "eventType": "PREDATOR_DETECTED",
  "timestamp": "2026-10-15T10:20:30.123Z",
  "producer": {
    "service": "swiftletcare-ai",
    "modelVersion": "model-version",
    "trackingSessionId": "session-20261015-01"
  },
  "cameraId": "camera-01",
  "zoneId": "zone-01",
  "object": {
    "className": "snake",
    "confidence": 0.91,
    "trackId": "track-17",
    "bbox": {
      "format": "xyxy",
      "x1": 120,
      "y1": 80,
      "x2": 220,
      "y2": 190,
      "imageWidth": 1280,
      "imageHeight": 720
    }
  },
  "crossing": null,
  "evidence": {
    "evidenceId": "evidence-0199-example",
    "uri": "https://storage.example.invalid/evidence/file.jpg",
    "mediaType": "image/jpeg",
    "capturedAt": "2026-10-15T10:20:30.123Z",
    "sha256": "replace-with-real-sha256"
  },
  "deduplicationKey": "camera-01:session-20261015-01:track-17:PREDATOR_DETECTED",
  "traceId": "trace-example"
}
```

URI và ID ở trên chỉ là placeholder, không phải endpoint hay file thật.

## 4. Quy tắc trường

| Trường | Bắt buộc | Quy tắc |
|---|---|---|
| `schemaVersion` | Có | Version schema để tương thích ngược |
| `eventId` | Có | ID duy nhất cho một event; tạo trước khi publish và giữ nguyên khi retry |
| `eventType` | Có | Enum đã thống nhất |
| `timestamp` | Có | ISO 8601 có timezone; thống nhất timestamp của crossing/detection |
| `cameraId`, `zoneId` | Có | ID đã đăng ký; không dùng tên hiển thị tùy ý |
| `producer.service` | Có | Tên service tạo event |
| `producer.modelVersion` | Có với AI event | Version checkpoint/model hoặc `unknown` khi chưa có |
| `producer.trackingSessionId` | Có cho Entry/Exit | Định danh phiên tracking |
| `object.className`, `confidence` | Có cho object event | Class trong class map; confidence nằm trong [0,1] |
| `object.trackId` | Có cho Entry/Exit; nếu có cho detection | Chỉ duy nhất trong session, không phải ID chim vĩnh viễn |
| `object.bbox` | Có nếu có detection box | Hệ tọa độ `xyxy`, pixel trên ảnh gốc; tọa độ và kích thước ảnh phải cùng hệ |
| `crossing.direction` | Có cho Entry/Exit | `ENTRY` hoặc `EXIT` |
| `crossing.lineId` | Có cho Entry/Exit | ID đường đếm đã cấu hình |
| `evidence.uri` | Theo chính sách event | URI nội bộ hoặc URL ký hạn; phải có kiểm soát truy cập/retention |
| `deduplicationKey` | Khuyến nghị | Giúp chống tạo event nghiệp vụ trùng; quy tắc cụ thể cần P4 chốt |
| `traceId` | Khuyến nghị | Theo dõi event qua AI và Backend |

## 5. Ví dụ Entry/Exit

```json
{
  "schemaVersion": "1.0",
  "eventId": "0199-entry-example",
  "eventType": "BIRD_ENTRY",
  "timestamp": "2026-10-15T10:20:31.500Z",
  "producer": {
    "service": "swiftletcare-ai",
    "modelVersion": "yolo-version+tracker-version",
    "trackingSessionId": "session-20261015-01"
  },
  "cameraId": "camera-01",
  "zoneId": "zone-01",
  "object": {
    "className": "swiftlet",
    "confidence": 0.88,
    "trackId": "track-42",
    "bbox": {
      "format": "xyxy",
      "x1": 310,
      "y1": 160,
      "x2": 350,
      "y2": 205,
      "imageWidth": 1280,
      "imageHeight": 720
    }
  },
  "crossing": {
    "direction": "ENTRY",
    "lineId": "entrance-line-01",
    "fromSide": "OUTSIDE",
    "toSide": "INSIDE",
    "crossingTimestamp": "2026-10-15T10:20:31.500Z"
  },
  "evidence": {
    "evidenceId": "evidence-entry-example",
    "uri": "https://storage.example.invalid/evidence/entry.jpg",
    "mediaType": "image/jpeg",
    "capturedAt": "2026-10-15T10:20:31.500Z"
  },
  "deduplicationKey": "camera-01:session-20261015-01:track-42:entrance-line-01:ENTRY"
}
```

## 6. Chống trùng và retry

- Producer tạo `eventId` trước lần gửi đầu tiên; retry cùng event phải dùng lại `eventId`.
- Backend nên xử lý idempotent theo `eventId`; uniqueness scope và retention do P4 chốt.
- `deduplicationKey` hỗ trợ phát hiện cùng track qua cùng line/hướng trong một session, nhưng không thay thế `eventId`.
- Nếu track ID bị reset sau mất kết nối/restart, session mới phải có `trackingSessionId` mới.
- Không gộp hai lượt chim khác nhau chỉ vì chúng đi qua cùng camera trong thời gian gần nhau.
- Evidence upload thất bại không được âm thầm làm mất event; chính sách retry/evidence-missing phải được chốt.

## 7. Bảo mật và lưu trữ

- Không gửi API key, mật khẩu hoặc thông tin nhạy cảm trong payload.
- Evidence URI phải được bảo vệ, thời hạn truy cập và thời gian lưu do P4/chủ trang trại thống nhất.
- Backend xác thực camera/service và kiểm tra enum, timestamp, confidence, kích thước bbox, schema version.
- Thay đổi breaking schema phải tăng major version hoặc được thống nhất tương thích.
