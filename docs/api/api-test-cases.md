# SwiftletCare — DSN-09 API Test Cases / Acceptance Examples

> Đây là các test case **thiết kế** để triển khai trong BUILD; tất cả trạng thái mặc định `NOT RUN`, không khẳng định đã có server hoặc test đã PASS.

| ID | Tình huống / HTTP | Kỳ vọng | Phụ trách | Trạng thái |
|---|---|---|---|---|
| API-T01 | POST login đúng thông tin | 200, JWT, không lộ passwordHash | P4 | NOT RUN |
| API-T02 | GET `/users/me` thiếu JWT | 401 | P4 | NOT RUN |
| API-T03 | Viewer House A GET Device House B | 403/404 nhất quán, không rò dữ liệu | P4 | NOT RUN |
| API-T04 | POST House với `initialZone` | House và Zone được tạo atomic hoặc rollback | P3/P4 | NOT RUN |
| API-T05 | GET Telemetry latest sau Offline replay muộn | Lấy mẫu timestamp đo mới nhất, không lấy replay cũ | P1/P4 | NOT RUN |
| API-T06 | GET Telemetry `from > to` | 400 VALIDATION_ERROR | P4 | NOT RUN |
| API-T07 | GET Telemetry khi CO₂ lỗi | 200, `co2:null`, `PARTIAL_ERROR` | P1/P4 | NOT RUN |
| API-T08 | PUT ZoneConfig version cũ | 409 VERSION_CONFLICT | P4 | NOT RUN |
| API-T09 | PUT ZoneConfig thành công nhưng chưa gửi MQTT | Không tuyên bố ESP32 đã áp dụng | P1/P4 | NOT RUN |
| API-T10 | POST Command `SET_MODE MANUAL` | 202, `PENDING`/SENT, có commandId | P1/P4 | NOT RUN |
| API-T11 | POST `SET_STATE=true` khi AUTO | 409, không publish lệnh hoặc GPIO đổi | P1/P4 | NOT RUN |
| API-T12 | POST SET_MODE vừa truyền targetMode vừa targetState | 400 cross-field validation | P4 | NOT RUN |
| API-T13 | POST SET_STATE không có targetState | 400 cross-field validation | P4 | NOT RUN |
| API-T14 | POST command khi ESP32 Offline | Trả 409/503 theo policy đã duyệt, không báo thực thi | P1/P4 | NOT RUN |
| API-T15 | Command ACK thành công | GET command `ACKNOWLEDGED`, UI cập nhật | P1/P4/P5 | NOT RUN |
| API-T16 | Command ACK timeout | Trạng thái lỗi rõ, không báo thành công | P1/P4 | NOT RUN |
| API-T17 | P2 POST AI CountLog hợp lệ | 201, count_log được lưu, CAMERA hợp lệ | P2/P4 | NOT RUN |
| API-T18 | Browser JWT gọi `/internal/ai/anomalies` | 401/403 nếu thiếu service credential | P2/P4 | NOT RUN |
| API-T19 | AI post duplicate từ P2 | Theo idempotency đã chốt; chưa có phương án final | P2/P4 | NOT RUN |
| API-T20 | PATCH Alert status | Alert và AlertHistory cùng transaction | P3/P4 | NOT RUN |
| API-T21 | P5 GET Notification | `DELIVERED` không biến thành `READ` | P4/P5 | NOT RUN |
| API-T22 | GET report environment | Trả aggregation theo interval và quyền Zone | P4/P5 | NOT RUN |
| API-T23 | GET correlation thiếu mẫu | `pearsonR=null`, có thông báo, không suy nhân quả | P3/P4 | NOT RUN |
| API-T24 | GET Report download House không có quyền | 403/404, không lộ filePath gốc | P4 | NOT RUN |
| API-T25 | P1 ngừng Heartbeat | Sau timeout 6s scheduler 1s cập nhật Device; UI đo ≤10s | P1/P4/P5 | NOT RUN |
| API-T26 | MQTT Telemetry duplicate event_id | DB unique, REST không có dòng trùng, ACK ứng dụng hợp lệ | P1/P4 | NOT RUN |
| API-T27 | React WebSocket mất kết nối | GET REST khôi phục và loại trùng theo eventId | P4/P5 | NOT RUN |

## Quy trình kiểm thử

- P4 khởi động Spring Boot và database có seed `User, House, Zone, Device, Actuator` và token hợp lệ.
- P1 chạy Wokwi/MQTT Broker cho telemetry/command/heartbeat.
- P2 gọi AI internal API bằng service credential.
- P5 đối chiếu JSON với UI, thử user role và không có quyền.
- Ghi kết quả `PASS/FAIL/NOT RUN`, timestamp, request/response và ảnh/log vào báo cáo BUILD; không đánh dấu PASS trước khi chạy.
