# AI Event Specification — SwiftletCare

> Thiết kế raw-event/anomaly cho giai đoạn sau, ngoài BUILD1-04. Backend MVP chưa có endpoint nhận payload snake_case bên dưới; chỉ nhận aggregate CountLog theo [contract MVP](../../api/build104-mvp.md). Giữ thiết kế này để review, không dùng làm request CountLog.

## 1. Lớp đối tượng và ngưỡng confidence

| Đối tượng | `class_name` | Ngưỡng confidence |
|---|---|---:|
| Rắn | `snake` | 0.80 |
| Chuột | `rat` | 0.80 |
| Cú | `owl` | 0.80 |

Các ngưỡng trên là giá trị đề xuất ban đầu, cần được kiểm thử và hiệu chỉnh bằng video thực tế.

## 2. Điều kiện tạo cảnh báo

Tạo sự kiện `PREDATOR_DETECTED` khi đối tượng thuộc lớp hỗ trợ, đạt ngưỡng confidence và được phát hiện ổn định qua số frame xác nhận đã cấu hình.

## 3. Chống cảnh báo trùng

- Theo dõi đối tượng bằng `tracking_id`.
- Một đối tượng chỉ tạo một cảnh báo trong cùng một lần xuất hiện liên tục.
- Chỉ tạo cảnh báo mới khi đối tượng đã biến mất quá thời gian quy định rồi xuất hiện lại.
- Nếu mất tracking và không đủ bằng chứng xác định đối tượng mới, không tự tạo cảnh báo mới.

## 4. Payload gửi Backend

```json
{
  "event_id": "ai-event-001",
  "camera_id": "CAM-HOUSE01-01",
  "zone_id": "ZONE01",
  "event_type": "PREDATOR_DETECTED",
  "class_name": "snake",
  "confidence": 0.91,
  "tracking_id": "track-125",
  "event_timestamp": "2026-10-12T08:00:00Z",
  "snapshot_ref": "evidence/ai-event-001.jpg",
  "status": "DETECTED"
}
```

| Trường | Ý nghĩa |
|---|---|
| `event_id` | Mã sự kiện duy nhất |
| `camera_id` | Mã camera phát hiện |
| `zone_id` | Mã khu vực |
| `event_type` | Loại sự kiện cảnh báo |
| `class_name` | Lớp đối tượng được phát hiện |
| `confidence` | Độ tin cậy của kết quả AI |
| `tracking_id` | ID theo dõi đối tượng |
| `event_timestamp` | Thời điểm phát hiện, theo UTC |
| `snapshot_ref` | Tham chiếu ảnh bằng chứng |
| `status` | Trạng thái xử lý sự kiện |

## 5. Lưu ảnh bằng chứng

- Lưu ảnh hoặc clip bằng chứng riêng, không nhúng dữ liệu ảnh trực tiếp vào JSON.
- `snapshot_ref` chứa đường dẫn hoặc mã tham chiếu để Backend truy xuất ảnh.
- Mỗi sự kiện phải liên kết đúng ảnh bằng chứng và camera tương ứng.

## 6. Trạng thái sự kiện

| Trạng thái | Ý nghĩa |
|---|---|
| `DETECTED` | AI đã phát hiện đối tượng |
| `RECEIVED` | Backend đã tiếp nhận sự kiện |
| `CONFIRMED` | Backend đã xác nhận và lưu sự kiện |
| `REJECTED` | Backend từ chối sự kiện không hợp lệ |

Backend sử dụng `event_id` để chống ghi nhận trùng. AI chỉ gửi kết quả phát hiện; trạng thái xác nhận và lưu trữ do Backend quản lý.

