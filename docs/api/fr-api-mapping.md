# SwiftletCare — FR / Use Case → API Traceability

> Bản đối chiếu ở **mức nhóm FR** do trong các kiến trúc hiện có chưa có đầy đủ bảng FR ID cuối cùng. **Không tự đặt FR-xxx-001** hay tự tuyên bố coverage 100%. P3 phải dán mã FR chính thức vào cột “ID FR final” trước nghiệm thu.

| Nhóm FR | Nghiệp vụ | API trong `openapi.yaml` | Cơ chế không phải REST | ID FR final | Review |
|---|---|---|---|---|---|
| USER/RBAC | JWT, hồ sơ, quyền | `/auth/*`, `/users/me` | BCrypt, Spring Security | TBD | P3/P4 |
| FARM/RBAC | House, thành viên | `/houses`, `/houses/{houseId}`, `/houses/{houseId}/members` | Service HouseMembership | TBD | P3/P4 |
| ZONE | Zone CRUD | `/houses/{houseId}/zones`, `/zones/{zoneId}` | Domain/JPA | TBD | P3/P5 |
| DEVICE | Danh sách/trạng thái | `/zones/{zoneId}/devices`, `/devices/{deviceId}` | MQTT LWT, Heartbeat | TBD | P1/P4 |
| IOT | Sensor hiện tại/lịch sử | `/devices/{deviceId}/telemetry*` | ESP32 MQTT telemetry + telemetry_ack | TBD | P1/P4 |
| CONFIG | Ngưỡng và chu kỳ | `/zones/{zoneId}/config` | MQTT configuration (schema TBD) | TBD | P1/P4 |
| IOT/CONTROL | AUTO/MANUAL, ON/OFF | `/actuators/{actuatorId}/commands`, `/commands/{commandId}` | MQTT command/command_ack | TBD | P1/P4 |
| AI | Đếm chim, sự kiện bất thường | `/devices/{deviceId}/count-logs`, `/zones/{zoneId}/anomalies`, `/internal/ai/*` | Camera/YOLO/ByteTrack Python | TBD | P2/P4 |
| ALT | Cảnh báo và lịch sử | `/alerts`, `/alerts/{alertId}/status`, `/alerts/{alertId}/history` | Rule engine + AlertHistory transaction | TBD | P3/P4 |
| NOTI | Tra cứu thông báo | `/notifications` | WebSocket/Push, NotificationDelivery | TBD | P4/P5 |
| REP | Môi trường, chim, thu hoạch | `/reports/environment`, `/reports/bird-count`, `/zones/{zoneId}/harvest-logs` | Database aggregation | TBD | P3/P5 |
| REP/CORRELATION | Tương quan năng suất–môi trường | `/reports/productivity-correlation` | Statistical analytics (algorithm TBD) | TBD | P3/P4 |
| REP export | Lưu file báo cáo | `/reports`, `/reports/{reportId}/download` | Report metadata + file storage | TBD | P4/P5 |
| UI Dashboard | Tổng quan, cập nhật realtime | `/dashboard/summary` | WebSocket after DB commit | TBD | P5/P4 |

## Nghiệp vụ cố ý **không** tạo REST API cho Web

- ESP32 gửi dữ liệu theo chu kỳ: MQTT `telemetry`, không có `POST /telemetry` public.
- Heartbeat 2s / offline timeout 6s / scheduler 1s: xử lý IoT + Spring Boot, không có API Web để “gửi heartbeat thay ESP32”.
- Telemetry ACK/Command ACK: MQTT private, không có endpoint React gửi ACK giả.
- Camera RTSP/video: có thể truyền luồng/video URL riêng, không mặc định là REST endpoint phát video của Spring Boot.
- Phân tích YOLO/ByteTrack: Python AI Service riêng; Backend nhận **kết quả** bằng API nội bộ.

## Cách hoàn tất traceability

P3 lấy FR v1.0 cuối cùng, gán `FR-ID → operationId` cụ thể (không chỉ tên module), đánh dấu mỗi yêu cầu là `REST`, `MQTT`, `WebSocket`, `Service-only` hoặc `Out-of-scope approved`; P4/P5 xác minh. Mã FR chưa thể xác minh giữ nguyên `TBD`.
