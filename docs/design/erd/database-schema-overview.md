# SwiftletCare — Database Schema Overview (đồng bộ bản ERD mới)

> BUILD1-04 implementation: [mapping Entity/database MVP](build104-mvp-mapping.md). Baseline 17 tables and open decisions below remain the full-system design; implemented keys/types/indexes are listed separately.
  
> **Trạng thái:** Ready for review; tài liệu mô tả **17 bảng** và ghi rõ những chỗ ERD vẫn phải sửa.

## 1. Nhóm bảng (17 bảng)

| Module | Bảng SQL đề xuất | Class P3 | Trách nhiệm |
|---|---|---|---|
| Quản lý | `users` | `User` | Đăng nhập, hồ sơ, vai trò toàn hệ thống |
| Quản lý | `houses` | `House` | Thông tin nhà yến |
| Quản lý | `house_memberships` | `HouseMembership` | Vai trò trong từng House |
| Quản lý | `zones` | `Zone` | Khu vực thuộc một House |
| Quản lý / IoT | `devices` | `Device` | ESP32/camera, kết nối và `last_seen_at` |
| IoT | `zone_configs` | `ZoneConfig` | Cấu hình môi trường và chu kỳ theo Zone |
| IoT | `sensors` | `Sensor` | Metadata cảm biến gắn Device |
| IoT | `actuators` | `Actuator` | Chấp hành, AUTO/MANUAL, trạng thái đầu ra |
| IoT | `telemetry` | `Telemetry` | Mẫu đo và chất lượng dữ liệu |
| IoT | `control_commands` | `ControlCommand` | `SET_MODE`/`SET_STATE`, trạng thái lệnh, ACK |
| AI | `count_logs` | `CountLog` | Đếm chim theo kỳ |
| AI | `anomaly_events` | `AnomalyEvent` | Bất thường/predator và tham chiếu snapshot |
| Cảnh báo | `alerts` | `Alert` | Cảnh báo môi trường/thiết bị/AI |
| Cảnh báo | `alert_histories` | `AlertHistory` | Lịch sử thay đổi cảnh báo |
| Thông báo | `notification_deliveries` | `NotificationDelivery` | Gửi Web/Push đến người nhận |
| Báo cáo | `reports` | `Report` | Metadata báo cáo được tạo |
| Báo cáo | `harvest_logs` | `HarvestLog` | Sản lượng theo Zone/ngày |

Không có Entity `Heartbeat`, `TelemetryAck`, `Camera` riêng trong baseline. Heartbeat cập nhật `devices.last_seen_at`; ACK Telemetry là bản tin MQTT, không nhất thiết phải tạo bảng.

## 2. Các liên kết dữ liệu

| Quan hệ | FK thực hiện | Bội số / ý nghĩa |
|---|---|---|
| User → HouseMembership | `house_memberships.user_id` | `User 1 → 0..* Membership` |
| House → HouseMembership | `house_memberships.house_id` | `House 1 → 0..* Membership` |
| House → Zone | `zones.house_id` | `House 1 → 0..* Zone` ở mức lưu trữ; UML hiển thị `1..*` theo nghiệp vụ khi House hoạt động |
| Zone → Device | `devices.zone_id` | Mỗi thiết bị được gán **một** Zone tại thời điểm hiện tại (v1) |
| Zone → ZoneConfig | `zone_configs.zone_id` | Zone tối đa một cấu hình hiện hành; `UNIQUE(zone_id)` |
| Device → Sensor/Actuator | `sensors.device_id`, `actuators.device_id` | ESP32 có nhiều thành phần |
| Device → Telemetry | `telemetry.device_id` | Một thiết bị tạo nhiều mẫu đo |
| Actuator → ControlCommand | `control_commands.actuator_id` | Một actuator nhận nhiều lệnh |
| User → ControlCommand | `control_commands.requested_by` | Người yêu cầu (trường hợp hệ thống tự tạo lệnh cần làm rõ) |
| Camera Device → CountLog/AnomalyEvent | `count_logs.device_id`, `anomaly_events.device_id` | Device loại `CAMERA` |
| Zone → Alert | `alerts.zone_id` | Alert có phạm vi Zone trong ERD hiện tại |
| AnomalyEvent → Alert | `alerts.anomaly_event_id` | **Nullable:** cảnh báo môi trường không phải sự kiện AI |
| Alert → AlertHistory/NotificationDelivery | `alert_histories.alert_id`, `notification_deliveries.alert_id` | Nhiều lần xử lý/gửi |
| User → AlertHistory/NotificationDelivery/Report | `changed_by`, `user_id`, `created_by` | Actor, người nhận, người tạo |
| Zone → HarvestLog | `harvest_logs.zone_id` | Sản lượng theo khu vực |

## 3. Đồng bộ Class Diagram ↔ ERD ↔ MQTT/Java

| Hợp đồng / miền | Java Domain / DTO | SQL ERD | Quy tắc |
|---|---|---|---|
| Telemetry ID | `Telemetry.eventId` | `telemetry.event_id` | **MQTT JSON `event_id`**; UNIQUE `(device_id, event_id)` |
| ID thiết bị | `Device.deviceId` | `devices.device_id` | Payload MQTT `device_id` là chuỗi; FK `telemetry.device_id` là `BIGINT` đến `devices.id` |
| Cảm biến lỗi | `Telemetry.sensorStatus` | `telemetry.sensor_status` | `OK`, `PARTIAL_ERROR`, `ERROR`; phép đo hỏng có thể `NULL`, không dùng số 0 giả |
| Telemetry ACK | Không bắt buộc một Entity riêng | Không thêm bảng | Backend lưu thành công/commit trước khi ACK `event_id` |
| Command | `ControlCommand.commandId` | `control_commands.command_id` | Unique, giữ nguyên ID khi lặp bản tin |
| Command action | `CommandAction` enum đã vẽ | `control_commands.action` | `SET_MODE` hoặc `SET_STATE` |
| Command target | `targetState`, `targetMode` | `target_state`, `target_mode` | Một trong hai giá trị được dùng theo `action` |
| Heartbeat | `Device.lastSeenAt`, `online` | `devices.last_seen_at`, `online` | ESP32 gửi 2s; Backend timeout 6s; scheduler 1s |
| Chế độ điều khiển | `Actuator.mode: ControlMode` | `actuators.mode` | AUTO/MANUAL; OFFLINE là trạng thái kết nối, không phải `ControlMode` |
| Báo cáo | `Report` | `reports` | Lưu metadata báo cáo; không tự thêm bảng tổng hợp |

**Lưu ý:** UML mới có enum `CommandAction` nhưng hộp `ControlCommand` **chưa hiển thị thuộc tính `action`**; ERD có `action` ở cuối bảng. P3 cần thêm `action: CommandAction` vào Class để đồng bộ.

## 4. Các điểm đã chốt dùng cho tuần BUILD

1. Giữ **17 bảng** của ERD; chưa mở rộng tùy tiện thêm Camera, Heartbeat hay lịch sử chuyển Zone.
2. ESP32 cố định trong một Zone ở v1; `devices.zone_id` là FK đến Zone.
3. `event_id` là tên chính thức trên MQTT; Java có thể dùng `eventId` (camelCase).
4. Telemetry chưa ACK được giữ trong ESP32 Buffer. Sau Backend commit và ACK hợp lệ, ESP32 mới xóa bản tin. Database chống ghi trùng bằng `(device_id, event_id)`.
5. Heartbeat: **2s**; Backend xác định Offline khi quá **6s**, kiểm tra mỗi **1s**; kết quả hiệu năng thực tế vẫn phải kiểm thử.
6. Điều khiển: `SET_MODE(AUTO|MANUAL)` và `SET_STATE(ON|OFF)` được tài liệu P1–P4 sử dụng; `SET_STATE` chỉ được thực thi khi actuator ở MANUAL và không vi phạm safety policy.
7. `zone_configs.telemetry_interval_sec` mặc định 5s, `heartbeat_interval_sec` mặc định 2s; giá trị mặc định chỉ là đề xuất triển khai cho SQL khi nhóm chưa có migration.

## 5. Các điểm **cần sửa trước khi tạo migration**

- **ERD ControlCommand có hai dòng `target_state`**: một `BOOLEAN` không có NULL và một `BOOLEAN NULL`. Chỉ giữ **một** trường `target_state BOOLEAN NULL`. Giữ `action CommandAction` và `target_mode ControlMode NULL`.
- **Class Diagram ControlCommand chưa có `action: CommandAction`**, dù enum đã được vẽ. Thêm thuộc tính này.
- **Class Diagram ZoneConfig được vẽ hai lần**: một ô mô tả đầy đủ và một ô rút gọn. Chỉ có **một** Entity/bảng `ZoneConfig`; nếu giữ hai ô để trình bày thì ghi rõ một ô là tham chiếu (`ref`), không phải định nghĩa khác.
- **AlertHistory**: ERD `changed_by` nullable; có thể biểu thị SYSTEM bằng `NULL`, nhưng chưa có `actor_type` ở cả hai sơ đồ. Nếu cần phân biệt rõ `USER`/`SYSTEM`, đây là thay đổi cần nhóm phê duyệt.
- **`AnomalyEvent` chưa có `event_id` nguồn AI**: nếu P2/API yêu cầu chống trùng sự kiện, cần thêm định danh hoặc một key khác đã xác định, không tự giả định PK DB ngăn được gửi trùng.
- **`Telemetry.event_id` trên ERD chưa đánh dấu UQ trong hộp**: phải bổ sung `UNIQUE(device_id, event_id)` trong Database Constraints/migration.
- **`ControlCommand.action`, target nullable và mode** phải khớp DTO MQTT `SET_MODE/SET_STATE`; tên trường/thời hạn lệnh vẫn do MQTT Contract làm nguồn sự thật.
- **Enum `HouseRole`, `AggregationType`** nay đã xuất hiện trong Class Diagram: lần lượt `OWNER/ADMIN/VIEWER`, `HOURLY/DAILY`; sửa lại tài liệu cũ ghi "chưa có".

## 6. Ranh giới công việc

- **P3:** sửa UML & Class Catalog (`action`, bản sao `ZoneConfig`, nullable, enum); review nghiệp vụ.
- **P5:** sửa ERD, constraint, dictionary/index/decisions và xuất ảnh; phối hợp migration.
- **P4:** chọn MySQL/PostgreSQL, implement Entity/JPA/DTO/Service, transaction + ACK.
- **P1:** xác nhận payload `event_id`, ACK, AUTO/MANUAL, status và Heartbeat.
- **P2:** chốt key idempotency cho AI, `detectCount` và kỳ thống kê.


