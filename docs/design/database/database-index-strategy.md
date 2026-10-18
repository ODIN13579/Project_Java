# SwiftletCare — Database Index Strategy (đồng bộ ERD mới)

> **Nguyên tắc:** Chỉ tạo các index thật sự phục vụ demo Java Spring Boot trong BUILD. Các index bổ sung phải dựa trên câu SQL/API thực tế và kết quả `EXPLAIN`.

## 1. Nhóm index bắt buộc / quan trọng

| Bảng | Index / Unique | Trạng thái | Nghiệp vụ |
|---|---|---|---|
| users | `UNIQUE(email)` | Đề xuất | Đăng nhập |
| devices | `UNIQUE(device_id)` | Đã có UQ trên ERD | Lookup thiết bị từ MQTT payload `device_id` |
| house_memberships | `UNIQUE(user_id,house_id)` | Đề xuất nếu một membership/cặp | RBAC theo House |
| zone_configs | `UNIQUE(zone_id)` | Đã có UQ trên ERD | Một cấu hình đang có hiệu lực |
| telemetry | `UNIQUE(device_id,event_id)` | **Cần bổ sung vào ERD và DDL** | MQTT replay/dedup/application ACK |
| control_commands | `UNIQUE(command_id)` | **Cần chỉ rõ UQ** | Chống command trùng |
| telemetry | `(device_id, timestamp DESC)` | Nên thêm | Truy vấn bản đo gần nhất / lịch sử |
| control_commands | `(actuator_id, requested_at DESC)` | Nên thêm | Tra cứu lệnh theo actuator |
| alerts | `(zone_id,status,created_at DESC)` | Nên thêm | Lấy cảnh báo chưa xử lý theo Zone |

## 2. Index bổ sung theo truy vấn thực tế

| Bảng | Index đề xuất | Khi nào thêm |
|---|---|---|
| zones | `(house_id,active)` | Dashboard liệt kê Zone trong House |
| devices | `(zone_id,enabled)` | Liệt kê Device theo Zone |
| sensors, actuators | `(device_id)` | JOIN metadata phần cứng |
| house_memberships | `(house_id,active)` | Danh sách thành viên theo House |
| count_logs | `(device_id,period_start)` | Thống kê Entry/Exit theo kỳ |
| anomaly_events | `(device_id,detected_at DESC)` | Lịch sử AI camera |
| alerts | `(status,created_at DESC)` | Danh sách cảnh báo gần nhất |
| alert_histories | `(alert_id,change_at DESC)` | Timeline cảnh báo |
| notification_deliveries | `(user_id,status,created_at DESC)` | Inbox thông báo |
| reports | `(created_by,created_at DESC)` | Danh sách báo cáo của User |
| harvest_logs | `(zone_id,date)` | Sản lượng theo khu vực/ngày |

## 3. Chú ý khi tích hợp với Java Spring Data JPA

- `Device.deviceId` trong Java map sang `devices.device_id` String; `telemetry.device_id` là FK BIGINT nên phải lookup `Device` trước khi lưu.
- `Telemetry.eventId` map sang `telemetry.event_id`. Không cần thêm một index riêng chỉ trên `event_id` nếu mọi tra cứu định danh theo cặp `(device_id,event_id)`; cân nhắc thêm sau khi đo query.
- `ControlCommand.commandId` map sang `command_id`, là idempotency key của MQTT Command.
- `@Enumerated(EnumType.STRING)` dùng `VARCHAR` thay vì ordinal; DB có thể thêm CHECK tùy hệ quản trị.
- PostgreSQL và MySQL khác nhau về tự tạo index cho FK và timestamp handling; đối chiếu migration thật, tránh tạo index thừa.
- Tối ưu index có thể để sau khi luồng `ESP32 → Broker → Spring Boot → DB → WebSocket` chạy được và đã có sample query.
- Nếu hiệu năng Telemetry giảm vì nhiều index, giữ chỉ mục unique chống trùng và một composite theo thời gian là ưu tiên.
