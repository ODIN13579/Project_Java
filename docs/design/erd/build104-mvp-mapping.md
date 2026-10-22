# BUILD1-04 — mapping Entity/database đã triển khai

Đây là mapping của backend MVP, không thay thế [ERD 17 bảng](README.md) hoặc [Class Diagram toàn hệ thống](../class/class-diagram-overview.md). Các lựa chọn triển khai còn cần cross-review; không tự coi là sign-off mọi quyết định thiết kế.

## Migration và quan hệ

V1 chỉ tạo schema `swiftletcare`, giữ nguyên. V2 tạo 7 bảng bên dưới. Flyway history nằm ở `public.flyway_schema_history`; Hibernate `ddl-auto=validate`, không tạo/sửa DDL. ID dùng PostgreSQL BIGINT IDENTITY; thời gian TIMESTAMPTZ ↔ Java Instant, JVM/JDBC UTC. Thay schema sau này phải thêm V3 trở đi, không sửa V1/V2 đã áp dụng.

| Java / package | Bảng swiftletcare | Quan hệ và ràng buộc trong V2 |
|---|---|---|
| `management.User` / UserRepository | users | PK id; UNIQUE email; password_hash là BCrypt; role OWNER/ADMIN, active |
| `management.House` / HouseRepository | houses | PK id, name/active; address và description là TEXT |
| Chưa có entity/repository Membership; `security.Access` dùng JDBC | house_memberships | FK user_id → users, house_id → houses; UNIQUE(user_id,house_id); role OWNER/ADMIN/VIEWER; active/joined_at |
| `management.Zone` / ZoneRepository | zones | FK house_id → houses; area_m2 nullable, CHECK >=0 |
| `management.Device` / DeviceRepository | devices | FK zone_id → zones; UNIQUE device_id (mã MQTT); type ESP32/CAMERA, enabled/online, last_seen_at nullable |
| `iot.Telemetry` / TelemetryRepository | telemetry | FK device_id → devices.id; UNIQUE(device_id,event_id); timestamp/received_at; measurements nullable; sensor_status OK/PARTIAL_ERROR/ERROR |
| `ai.CountLog` / CountLogRepository | count_logs | FK device_id → devices.id; UNIQUE(device_id,aggregation_type,period_start,period_end); counts >=0; start < end; HOURLY/DAILY |

FK không có ON DELETE CASCADE; PostgreSQL mặc định NO ACTION ngăn xóa cha còn được tham chiếu. House có thể tồn tại chưa có Zone ở mức DDL; quy tắc kích hoạt House tối thiểu một Zone vẫn thuộc thiết kế chưa có API CRUD trong MVP.

Java hiện lưu FK bằng scalar Long (`houseId`, `zoneId`, `deviceId`), không dùng @ManyToOne/@OneToMany hoặc cascade ORM. Giá trị enum trong UML được lưu bằng String trong entity và kiểm soát bằng CHECK/validation; không phải Java enum @Enumerated. Domain relation vẫn được bảo vệ bằng FK thật. Có 6 entity JPA và 7 bảng; không phải đã triển khai đủ 17 entity thiết kế.

## Index / validation / transaction

- Unique key tự tạo unique index. Thêm `telemetry_device_time(device_id,timestamp DESC,received_at DESC,id DESC)` cho latest/history và partial index `devices_online_seen(last_seen_at) WHERE online` cho scheduler.
- V2 CHECK humidity 0..100, co2/light >=0. **NFR-IOT-004 đầy đủ được kiểm tra ở Ingestion.validate trước transaction**: temperature -10..60, humidity 0..100, co2 0..5000, light 0..100000; tất cả hữu hạn, biên inclusive. Không sửa DDL hoặc dữ liệu cũ trong đợt đồng bộ này; không tuyên bố DB CHECK tự bảo đảm toàn bộ NFR.
- Null đại diện phép đo lỗi; 4 số → OK, 1..3 số → PARTIAL_ERROR, 0 số → ERROR. Dải NFR áp dụng ingest mới/retry qua service; bản ghi lịch sử đã lưu không được tự sửa/xóa.
- Telemetry dùng JDBC upsert trong TransactionTemplate, unique xử lý concurrent insert, so sánh toàn bộ nội dung khi trùng. ACK/realtime sau khi transaction trả về đã commit; không có bảng Heartbeat/ACK hoặc outbox.
- CountLog CAMERA/type/active, periodEnd <= processedAt và processedAt không quá 5 phút tương lai được kiểm tra ở service/controller. Aggregate cùng key phải giữ nguyên counts và processedAt khi retry; không cập nhật/cộng dồn âm thầm.

PostgreSQL/Mosquitto đã được chọn cho BUILD1-04; các mục OPEN ở tài liệu thiết kế chung vẫn giữ lịch sử và sign-off rộng hơn. Chi tiết giao tiếp: [contract MVP](../../api/build104-mvp.md), [MQTT](../../mqtt/topics.md).

## Ngoài BUILD1-04

ZoneConfig, Sensor, Actuator, ControlCommand, AnomalyEvent, Alert, AlertHistory, NotificationDelivery, HarvestLog và Report vẫn là thiết kế giai đoạn sau. Chưa có API House/Zone CRUD/list, GET CountLog, refresh/logout JWT hay CRUD membership. Không tạo các bảng/API này chỉ để làm đầy sơ đồ.
