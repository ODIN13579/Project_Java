# QUY ƯỚC DỮ LIỆU SỰ KIỆN AI GỬI BACKEND — SWIFTLETCARE

**Nhiệm vụ:** TASK-11  
**Phối hợp:** P2 — AI và P4 — Backend  
**Liên quan:** TASK-04 — Kiến trúc AI; TASK-05 — Backend

## 1. Quy tắc chung

- Mỗi sự kiện logic có một `eventId` duy nhất. Khi gửi lại do lỗi mạng, giữ nguyên `eventId` để Backend loại bỏ dữ liệu trùng.
- Thời gian dùng định dạng ISO 8601 và phải có múi giờ. Ví dụ bên dưới dùng UTC, ký hiệu `Z`.
- `cameraId` và `zoneId` phải trỏ đến camera/khu vực đã được khai báo trong Backend.
- `confidence` là số trong khoảng 0–1 nếu được cung cấp.
- `evidenceUrl` là đường dẫn tham chiếu đến ảnh/clip bằng chứng đã lưu, không phải dữ liệu ảnh nhúng trực tiếp. P4 cần thống nhất quyền truy cập, thời hạn lưu và thời điểm hết hạn.
- `trackId` chỉ có ý nghĩa trong một phiên theo dõi, không phải mã định danh vĩnh viễn của chim.
- Đây là quy ước đề xuất để rà soát, chưa phải lược đồ API chính thức. P4 cần xác nhận tên trường, giao thức truyền, xác thực, lưu trữ và cơ chế gửi lại.

## 2. Ví dụ sự kiện phát hiện động vật săn mồi

```json
{
  "eventId": "example-event-id",
  "eventType": "PREDATOR_DETECTED",
  "className": "snake",
  "confidence": 0.91,
  "timestamp": "2026-10-15T10:20:30Z",
  "cameraId": "camera-01",
  "zoneId": "zone-01",
  "evidenceUrl": "example-evidence-reference"
}
```

## 3. Ví dụ sự kiện chim bay vào/ra

```json
{
  "eventId": "bird-event-001",
  "eventType": "ENTRY",
  "className": "swiftlet",
  "trackId": "track-125",
  "trackingSessionId": "session-20261015-01",
  "direction": "OUTSIDE_TO_INSIDE",
  "timestamp": "2026-10-15T10:20:30Z",
  "cameraId": "camera-01",
  "zoneId": "zone-01",
  "countingLineId": "entrance-line-01",
  "confidence": 0.88,
  "evidenceUrl": "example-frame-reference"
}
```

Giá trị `eventType` cho sự kiện vượt đường đếm của chim: `ENTRY`, `EXIT`.
- `ENTRY` tương ứng `OUTSIDE_TO_INSIDE` — từ ngoài vào trong.
- `EXIT` tương ứng `INSIDE_TO_OUTSIDE` — từ trong ra ngoài.

Giá trị `confidence` trong ví dụ chỉ để minh họa. Nhóm cần xác định rõ đó là độ tin cậy phát hiện, độ tin cậy theo dõi/vượt đường hay độ tin cậy sự kiện đã hiệu chỉnh.

## 4. Gửi lại và chống ghi nhận trùng

Khi gặp lỗi tạm thời, bên gửi có thể gửi lại sự kiện với cùng `eventId`. Backend cần xem các lần nhận cùng `eventId` là một sự kiện logic, không cộng lượt nhiều lần. Không tạo mã sự kiện mới cho mỗi lần gửi lại.

## 5. Các nội dung P2/P4 cần chốt

- Tên trường cuối cùng và trường bắt buộc/tùy chọn.
- API endpoint hoặc MQTT topic và cấu trúc gói tin.
- Xác thực, phân quyền và bảo mật truyền dữ liệu.
- Nguồn thời gian, quy ước UTC và cách hiển thị giờ địa phương.
- Nơi lưu bằng chứng, quyền truy cập, thời hạn lưu và hết hạn.
- Cách tạo `eventId` và bên chịu trách nhiệm chống trùng.
- Cơ chế gửi lại, thời gian chờ và lưu tạm khi mất mạng.
- Gửi dữ liệu phát hiện/track thô hay chỉ gửi sự kiện tổng hợp.
