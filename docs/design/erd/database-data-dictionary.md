# SwiftletCare — Database Data Dictionary (ERD 17 bảng)

> **Baseline:** `Class Diagram(1).png` + `database-erd(1).png`. Kiểu VARCHAR/DOUBLE/TIMESTAMP và độ dài trong bảng dưới là **đề xuất để triển khai**, chưa phải DDL được chạy.  
> **Quyết định IoT:** MQTT Telemetry `event_id`, Heartbeat 2s, timeout 6s, Scheduler 1s.  
> **Quy tắc:** `OPEN` = phần chưa được nhóm xác nhận; `NULL` = nullable được đề xuất hoặc nhìn thấy trên ERD (xem mô tả cụ thể).

## 0. Quy ước chung

- `id BIGINT` là PK nội bộ; `devices.device_id VARCHAR` là **mã bên ngoài** do MQTT mang theo.
- Java dùng camelCase (`eventId`, `lastSeenAt`); SQL dùng snake_case (`event_id`, `last_seen_at`).
- Timestamp truyền MQTT theo ISO-8601 UTC; lưu thời gian theo UTC, ánh xạ `Instant` trong Spring Boot (cụ thể kiểu DB tùy MySQL/PostgreSQL).
- Đối với enum trong Java JPA, ưu tiên `EnumType.STRING` thay vì ordinal.
- **Cột có thể NULL** không có nghĩa MQTT bắt buộc bỏ trường: Telemetry mang trường đo với `null` khi lỗi để Backend không ghi số 0 giả.
- Danh sách cột **theo sơ đồ**, chưa thêm bảng/chức năng ngoài phạm vi. Trường có ký hiệu `OPEN` cần quyết định trước khi khóa migration.

## 1. `users` → `User`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK, NOT NULL | Khóa chính |
| `email` | `VARCHAR(255)` | UQ, NOT NULL | Email đăng nhập |
| `password_hash` | `VARCHAR(255)` | NOT NULL | Mật khẩu BCrypt hash |
| `full_name` | `VARCHAR(150)` | NULL/OPEN | Tên đầy đủ |
| `phone` | `VARCHAR(30)` | NULL/OPEN | Số điện thoại |
| `role` | `VARCHAR(30)` | NOT NULL | RoleName: OWNER/ADMIN |
| `active` | `BOOLEAN` | NOT NULL | Đang hoạt động |

**Lưu ý:** Vai trò hệ thống `RoleName` khác `HouseRole` trong bảng membership. `password_hash` luôn là hash, không là password nguyên văn.

## 2. `houses` → `House`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `name` | `VARCHAR(150)` | NOT NULL | Tên nhà yến |
| `address` | `TEXT` | OPEN | Địa chỉ |
| `description` | `TEXT` | NULL | Mô tả |
| `active` | `BOOLEAN` | NOT NULL | Trạng thái hoạt động |

**Lưu ý:** Trong UML House–Zone `1..*` nhưng Database có thể tạo House trước Zone; quy tắc bắt buộc ít nhất một Zone khi kích hoạt nên thực hiện ở tầng nghiệp vụ.

## 3. `house_memberships` → `HouseMembership`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `user_id` | `BIGINT` | FK users.id, NOT NULL | Người tham gia |
| `house_id` | `BIGINT` | FK houses.id, NOT NULL | Nhà yến |
| `role` | `VARCHAR(30)` | NOT NULL | HouseRole: OWNER/ADMIN/VIEWER |
| `active` | `BOOLEAN` | NOT NULL | Membership còn hiệu lực |
| `joined_at` | `TIMESTAMP` | NOT NULL | Thời điểm gia nhập |

**Lưu ý:** `UNIQUE(user_id,house_id)` nếu mỗi cặp chỉ có một membership hiện tại; quản lý lịch sử nhiều giai đoạn cần bảng/vòng đời khác.

## 4. `zones` → `Zone`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `house_id` | `BIGINT` | FK houses.id, NOT NULL | Nhà chứa Zone |
| `name` | `VARCHAR(150)` | NOT NULL | Tên khu vực |
| `floor_label` | `VARCHAR(50)` | NULL | Nhãn tầng |
| `area_m2` | `DOUBLE` | NULL | Diện tích m² |
| `active` | `BOOLEAN` | NOT NULL | Trạng thái |

**Lưu ý:** Trong v1 Zone thuộc một House và có thể chứa nhiều Device; không hỗ trợ di chuyển ESP32 giữa Zone.

## 5. `devices` → `Device`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `zone_id` | `BIGINT` | FK zones.id, NOT NULL v1 | Zone cố định (v1) |
| `device_id` | `VARCHAR(100)` | UQ, NOT NULL | Mã thiết bị dùng trong MQTT |
| `name` | `VARCHAR(150)` | NOT NULL | Tên thiết bị |
| `type` | `VARCHAR(20)` | NOT NULL | DeviceType: ESP32/CAMERA |
| `enabled` | `BOOLEAN` | NOT NULL | Cho phép hoạt động |
| `online` | `BOOLEAN` | NOT NULL | Trạng thái Backend xác định |
| `last_seen_at` | `TIMESTAMP` | NULL | Thời điểm nhận Heartbeat gần nhất |

**Lưu ý:** Không suy ra `online=true` nghĩa là ESP32 đang ở MANUAL; `online` được cập nhật từ Heartbeat/LWT. Timeout Checker chạy định kỳ **độc lập** với việc nhận Heartbeat.

## 6. `zone_configs` → `ZoneConfig`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `zone_id` | `BIGINT` | FK zones.id, UNIQUE, NOT NULL | Một cấu hình hiện hành / Zone |
| `humidity_on` | `DOUBLE` | NULL/OPEN | Ngưỡng bật phun sương |
| `humidity_off` | `DOUBLE` | NULL/OPEN | Ngưỡng tắt phun sương |
| `temp_min` | `DOUBLE` | NULL/OPEN | Ngưỡng thấp nhiệt độ |
| `temp_max` | `DOUBLE` | NULL/OPEN | Ngưỡng cao nhiệt độ |
| `temp_hysteresis` | `DOUBLE` | NULL/OPEN | Độ trễ nhiệt độ |
| `co2_min` | `DOUBLE` | NULL/OPEN | CO₂ tối thiểu |
| `co2_max` | `DOUBLE` | NULL/OPEN | CO₂ tối đa |
| `light_min` | `DOUBLE` | NULL/OPEN | Lux tối thiểu |
| `light_max` | `DOUBLE` | NULL/OPEN | Lux tối đa |
| `telemetry_interval_sec` | `INT` | NOT NULL | Mặc định 5s; cấu hình 1–60s |
| `heartbeat_interval_sec` | `INT` | NOT NULL | Mặc định 2s theo quyết định mới |
| `version` | `INT` | NOT NULL | Phiên bản cấu hình |
| `updated_at` | `TIMESTAMP` | NOT NULL | Lần cập nhật |

**Lưu ý:** ERD dùng `zone_id` UNIQUE. Class Diagram có hai hộp ZoneConfig (đầy đủ/rút gọn) nhưng chỉ tương ứng **một** bảng. Heartbeat mặc định 2s, telemetry mặc định 5s.

## 7. `sensors` → `Sensor`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `device_id` | `BIGINT` | FK devices.id, NOT NULL | Thiết bị chứa Sensor |
| `kind` | `VARCHAR(80)` | NOT NULL | Loại cảm biến: String |
| `unit` | `VARCHAR(30)` | NULL | Đơn vị đo |

**Lưu ý:** `kind` là String trên UML, không tự chuyển thành enum nếu chưa thống nhất.

## 8. `actuators` → `Actuator`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `device_id` | `BIGINT` | FK devices.id, NOT NULL | ESP32 điều khiển |
| `kind` | `VARCHAR(20)` | NOT NULL | ActuatorKind |
| `mode` | `VARCHAR(20)` | NOT NULL | AUTO/MANUAL |
| `on` | `BOOLEAN` | NOT NULL | Trạng thái được báo cáo, theo contract |
| `manual_until` | `TIMESTAMP` | NULL | Hạn lệnh MANUAL, nếu có |

**Lưu ý:** `on` là trạng thái đầu ra do ESP32 báo; không chứng minh tải vật lý thực sự chạy nếu không có phần cứng feedback. `mode` theo từng Actuator.

## 9. `telemetry` → `Telemetry`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `device_id` | `BIGINT` | FK devices.id, NOT NULL | FK đến ESP32; MQTT device_id là mã String khác với FK này |
| `event_id` | `VARCHAR(150)` | NOT NULL; UNIQUE kết hợp device_id | ID bản đo MQTT, giữ nguyên khi retry |
| `timestamp` | `TIMESTAMP` | NOT NULL | Thời điểm đo tại nguồn, UTC |
| `received_at` | `TIMESTAMP` | NOT NULL | Thời điểm Backend nhận, UTC |
| `temperature` | `DOUBLE` | NULL | °C; null khi lỗi |
| `humidity` | `DOUBLE` | NULL | %RH; null khi lỗi |
| `co2` | `DOUBLE` | NULL | ppm; null khi lỗi |
| `light` | `DOUBLE` | NULL | lux; null khi lỗi |
| `sensor_status` | `VARCHAR(30)` | NOT NULL | SensorStatus: OK/PARTIAL_ERROR/ERROR |

**Lưu ý:** **Phải bổ sung UNIQUE(device_id,event_id) trên ERD và migration**. Backend trả `telemetry_ack(STORED)` sau commit, kể cả nhận lại sự kiện trùng giống hệt. Hai payload cùng ID nhưng khác nội dung phải báo xung đột.

## 10. `control_commands` → `ControlCommand`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `actuator_id` | `BIGINT` | FK actuators.id, NOT NULL | Đích thực hiện |
| `requested_by` | `BIGINT` | FK users.id, NOT NULL trên ảnh; OPEN cho system command | Người tạo yêu cầu |
| `command_id` | `VARCHAR(150)` | UQ, NOT NULL | ID lệnh để chống lặp |
| `action` | `VARCHAR(20)` | NOT NULL; **cần giữ một dòng** | CommandAction SET_MODE/SET_STATE |
| `target_state` | `BOOLEAN` | NULL; **xóa dòng trùng NOT NULL trên ERD** | Chỉ dùng khi SET_STATE |
| `target_mode` | `VARCHAR(20)` | NULL | Chỉ dùng khi SET_MODE (AUTO/MANUAL) |
| `requested_at` | `TIMESTAMP` | NOT NULL | Thời điểm tạo yêu cầu |
| `status` | `VARCHAR(30)` | NOT NULL | CommandStatus |
| `sent_at` | `TIMESTAMP` | NULL | Đã gửi MQTT lúc |
| `ack_at` | `TIMESTAMP` | NULL | Thiết bị ACK lúc |
| `failure_reason` | `TEXT` | NULL | Lỗi, TIMEOUT, REJECTED |
| `expires_at` | `TIMESTAMP` | NOT NULL trong Command DTO; ERD chưa đánh NOT NULL | Thời hạn lệnh |

**Lưu ý:** **LỖI SƠ ĐỒ:** ERD hiện vẽ `target_state` **hai lần**, một NOT NULL và một NULL. Bản đồng bộ này chỉ giữ `target_state BOOLEAN NULL`. `SET_STATE` cần `target_state`; `SET_MODE` cần `target_mode`. Enum `CommandAction` có trong UML nhưng P3 còn phải đưa `action` vào lớp `ControlCommand`.

## 11. `count_logs` → `CountLog`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `device_id` | `BIGINT` | FK devices.id, NOT NULL | Camera nguồn |
| `period_start` | `TIMESTAMP` | NOT NULL | Đầu kỳ |
| `period_end` | `TIMESTAMP` | NOT NULL | Cuối kỳ |
| `entry_count` | `INT` | NOT NULL | Số lượt vào |
| `exit_count` | `INT` | NOT NULL | Số lượt ra |
| `detect_count` | `INT` | NOT NULL | Số detection, không mặc định là đàn chim |
| `aggregation_type` | `VARCHAR(20)` | NOT NULL | AggregationType: HOURLY/DAILY |
| `processed_at` | `TIMESTAMP` | NOT NULL | Thời gian xử lý |

**Lưu ý:** P2 phải chốt nghĩa của `detect_count` và khóa chống đếm trùng; bản hiện tại chỉ lưu thống kê theo kỳ.

## 12. `anomaly_events` → `AnomalyEvent`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `device_id` | `BIGINT` | FK devices.id, NOT NULL | Camera phát hiện |
| `species` | `VARCHAR(100)` | NULL/OPEN | Loài/nhãn phân loại |
| `confidence` | `DOUBLE` | NOT NULL/OPEN | 0–1 |
| `detected_at` | `TIMESTAMP` | NOT NULL | Thời điểm phát hiện |
| `snapshot_path` | `TEXT` | NULL | Khóa ảnh bằng chứng bên ngoài DB |

**Lưu ý:** Sơ đồ chưa có event_id nguồn AI. P2/P4 phải chốt chống trùng bằng ID nguồn hoặc composite key phù hợp trước BUILD.

## 13. `alerts` → `Alert`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `zone_id` | `BIGINT` | FK zones.id, NOT NULL theo ERD | Zone liên quan |
| `anomaly_event_id` | `BIGINT` | FK anomaly_events.id, NULL | Event AI nếu có |
| `type` | `VARCHAR(80)` | NOT NULL | Loại cảnh báo, String |
| `level` | `VARCHAR(20)` | NOT NULL | WARNING/CRITICAL |
| `status` | `VARCHAR(20)` | NOT NULL | NEW/IN_PROGRESS/RESOLVED |
| `message` | `TEXT` | NOT NULL | Nội dung cảnh báo |
| `created_at` | `TIMESTAMP` | NOT NULL | Thời điểm tạo |
| `handled_at` | `TIMESTAMP` | NULL | Thời điểm xử lý |

**Lưu ý:** `anomaly_event_id` nullable là thiết kế đúng vì cảnh báo môi trường/thiết bị không cần sự kiện AI. Nếu cần Alert cấp House không thuộc Zone, phạm vi phải được mở rộng có chủ đích.

## 14. `alert_histories` → `AlertHistory`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `alert_id` | `BIGINT` | FK alerts.id, NOT NULL | Alert thay đổi |
| `changed_by` | `BIGINT` | FK users.id, NULL | NULL khi thao tác tự động; chưa có actor_type trên sơ đồ |
| `from_status` | `VARCHAR(20)` | NULL | Trạng thái cũ, NULL khi tạo |
| `to_status` | `VARCHAR(20)` | NOT NULL | Trạng thái mới |
| `change_at` | `TIMESTAMP` | NOT NULL | Tên cột hiện có trên ERD; Java dùng changeAt |
| `note` | `TEXT` | NULL | Ghi chú |

**Lưu ý:** `changed_by` cho SYSTEM có thể NULL. Nếu cần phân biệt SYSTEM/USER tường minh, cần thêm `actor_type` vào cả UML và ERD sau khi nhóm chốt.

## 15. `notification_deliveries` → `NotificationDelivery`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `alert_id` | `BIGINT` | FK alerts.id, NOT NULL | Alert nguồn |
| `user_id` | `BIGINT` | FK users.id, NOT NULL | Người nhận |
| `channel` | `VARCHAR(15)` | NOT NULL | WEB/PUSH |
| `status` | `VARCHAR(20)` | NOT NULL | PENDING/SENT/DELIVERED/FAILED |
| `created_at` | `TIMESTAMP` | NOT NULL | Thời điểm tạo |
| `sent_at` | `TIMESTAMP` | NULL | Thời điểm gửi |
| `delivered_at` | `TIMESTAMP` | NULL | Thời điểm có xác nhận delivery |
| `failure_reason` | `TEXT` | NULL | Lý do lỗi |

**Lưu ý:** `DELIVERED` chỉ khi kênh có xác nhận, khác với người dùng đã đọc; không tự thêm SMS/Zalo.

## 16. `reports` → `Report`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `created_by` | `BIGINT` | FK users.id, NOT NULL theo ERD | Người tạo báo cáo |
| `type` | `VARCHAR(80)` | NOT NULL | Loại báo cáo, String |
| `range_from` | `TIMESTAMP` | NOT NULL | Bắt đầu kỳ |
| `range_to` | `TIMESTAMP` | NOT NULL | Kết thúc kỳ |
| `file_path` | `TEXT` | NULL | Tham chiếu file lưu |
| `created_at` | `TIMESTAMP` | NOT NULL | Thời điểm tạo |

**Lưu ý:** Bảng lưu metadata file báo cáo, không lưu hết dữ liệu tổng hợp. `created_by` FK hiện có trên ERD.

## 17. `harvest_logs` → `HarvestLog`

| Cột SQL | Kiểu đề xuất | Khóa / NULL | Ý nghĩa |
|---|---|---|---|
| `id` | `BIGINT` | PK | Khóa chính |
| `zone_id` | `BIGINT` | FK zones.id, NOT NULL | Khu vực thu hoạch |
| `date` | `DATE` | NOT NULL | Ngày thu hoạch |
| `yield_kg` | `DOUBLE` | NOT NULL | Sản lượng kg |
| `note` | `TEXT` | NULL | Ghi chú |

**Lưu ý:** Sản lượng không âm. Báo cáo correlation phải dùng telemetry và harvest theo cùng phạm vi Zone và kỳ thời gian.

## 18. Enum thống nhất với Class Diagram mới

| Enum | Giá trị | Bảng/cột |
|---|---|---|
| `RoleName` | OWNER, ADMIN | users.role |
| `HouseRole` | OWNER, ADMIN, VIEWER | house_memberships.role |
| `DeviceType` | ESP32, CAMERA | devices.type |
| `ActuatorKind` | MIST, FAN, HEATER, AMPLIFIER | actuators.kind |
| `ControlMode` | AUTO, MANUAL | actuators.mode; control_commands.target_mode |
| `CommandAction` | SET_MODE, SET_STATE | control_commands.action |
| `CommandStatus` | PENDING, SENT, ACKNOWLEDGED, FAILED, REJECTED, EXPIRED | control_commands.status |
| `SensorStatus` | OK, PARTIAL_ERROR, ERROR | telemetry.sensor_status |
| `AggregationType` | HOURLY, DAILY | count_logs.aggregation_type |
| `AlertLevel` | WARNING, CRITICAL | alerts.level |
| `AlertStatus` | NEW, IN_PROGRESS, RESOLVED | alerts.status; alert_histories |
| `NotificationChannel` | WEB, PUSH | notification_deliveries.channel |
| `DeliveryStatus` | PENDING, SENT, DELIVERED, FAILED | notification_deliveries.status |

## 19. Checklist mapping Java ↔ SQL quan trọng

| Java field / DTO | SQL column | Ghi chú |
|---|---|---|
| `Telemetry.eventId`, MQTT `event_id` | `telemetry.event_id` | Không tạo thêm `messageId` |
| MQTT `device_id` dạng String | `devices.device_id` | Query Device, rồi lấy `devices.id` để ghi FK `telemetry.device_id` BIGINT |
| `Device.lastSeenAt` | `devices.last_seen_at` | 2s heartbeat; 6s timeout; scheduler 1s |
| `ControlCommand.commandId` | `control_commands.command_id` | UNIQUE |
| `ControlCommand.action` (cần bổ sung UML) | `control_commands.action` | SET_MODE / SET_STATE |
| `ControlCommand.targetState` | `control_commands.target_state` | Nullable; chỉ áp dụng SET_STATE |
| `ControlCommand.targetMode` | `control_commands.target_mode` | Nullable; chỉ áp dụng SET_MODE |
| `AlertHistory.changeAt` | `alert_histories.change_at` | Giữ cách viết hiện có trên ERD cho đến khi nhóm chính thức đổi |


