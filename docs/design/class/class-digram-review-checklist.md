## 1. Kiểm tra phạm vi và tính nhất quán của sơ đồ

- [ ] **CD-G01** — Có đủ 5 nhóm nghiệp vụ: Quản lý, IoT, AI, Cảnh báo, Thống kê/Báo cáo.
- [ ] **CD-G02** — Có 17 lớp phân biệt trên sơ đồ: `User`, `HouseMembership`, `House`, `Zone`, `Device`, `ZoneConfig`, `Sensor`, `Actuator`, `Telemetry`, `ControlCommand`, `CountLog`, `AnomalyEvent`, `Alert`, `AlertHistory`, `NotificationDelivery`, `HarvestLog`, `Report`.
- [ ] **CD-G03** — Hợp nhất 2 ô `ZoneConfig` còn trùng trên ảnh mới; chỉ giữ 1 định nghĩa đầy đủ và quan hệ chính thức với Zone.
- [ ] **CD-G04** — Tên lớp và thuộc tính nhất quán với Class Catalog, ERD, MQTT/API; không có hai cách viết chỉ cùng một khái niệm.
- [ ] **CD-G05** — Xóa các `+method(): Type` placeholder còn hiện trên ảnh mới trước khi bàn giao (hoặc thay bằng method nghiệp vụ cụ thể).
- [ ] **CD-G06** — Sơ đồ gốc mở/chỉnh sửa được; bản PNG rõ nhãn quan hệ, enum và multiplicity.

## 2. Kiểm tra Entity và kiểu dữ liệu

### 2.1. Quản lý và phân quyền

- [ ] **CD-M01** — `User` có `passwordHash`, `role: RoleName`, `active`; không thiết kế lưu plain-text password.
- [ ] **CD-M02** — `HouseMembership` thuộc một `User` và một `House`; xác định cách ngăn bản ghi thành viên đang hiệu lực bị trùng.
- [ ] **CD-M03** — `RoleName` (`OWNER`, `ADMIN`) và `HouseRole` (`OWNER`, `ADMIN`, `VIEWER`) đã được khai báo riêng; xác nhận ma trận phân quyền toàn hệ thống/theo House.
- [ ] **CD-M04** — `House → Zone` đang là composition với Zone `1..*`; quy trình tạo nhà yến hợp lệ với ràng buộc này hoặc đã ghi quyết định xử lý ngoại lệ.
- [ ] **CD-M05** — `Zone` chứa Device `0..*` và một Device thuộc một Zone trong mô hình hiện tại; các trường hợp thiết bị chưa gán/chuyển Zone được phân định rõ **in scope/out of scope**.
- [ ] **CD-M06** — `Device.enabled`, `Device.online`, `Device.lastSeenAt` có định nghĩa thống nhất; online/offline phù hợp quy tắc Heartbeat.

### 2.2. IoT

- [ ] **CD-I01** — `ZoneConfig` có bản thuộc tính chuẩn gồm humidity, temperature, CO₂, light, telemetry interval, heartbeat interval, version và updatedAt theo baseline.
- [ ] **CD-I02** — Các ngưỡng đã có quy tắc kiểm tra hợp lệ, đơn vị đo, nguồn cấu hình và cách triển khai đồng bộ xuống ESP32.
- [ ] **CD-I03** — `Sensor.kind` và `Sensor.unit` dùng tên và đơn vị thống nhất với IoT Hardware Spec/MQTT Spec.
- [ ] **CD-I04** — `Actuator.kind`, `mode`, `on`, `manualUntil` có định nghĩa rõ; không đồng nhất cờ trạng thái mong muốn với xác nhận thiết bị vật lý đã hoạt động.
- [ ] **CD-I05** — `Telemetry.eventId` ↔ MQTT/SQL `event_id`; thời điểm đo `timestamp` khác `receivedAt`; trạng thái `sensorStatus` đúng MQTT Schema.
- [ ] **CD-I06** — DB có UNIQUE `(device_id, event_id)`; retry giữ nguyên `event_id` và timestamp; Java xử lý duplicate và Telemetry ACK sau commit.
- [ ] **CD-I07** — Quy tắc `SensorStatus.PARTIAL_ERROR` làm rõ trường hợp chỉ một cảm biến lỗi; các trường giá trị lỗi không bị ép thành số 0 hợp lệ.
- [ ] **CD-I08** — P3 thêm `ControlCommand.action: CommandAction` (enum đã có nhưng thuộc tính còn thiếu); đồng bộ `commandId`, `status`, `ackAt`, `expiresAt` với MQTT.
- [ ] **CD-I09** — `targetState: Boolean?` và `targetMode: ControlMode?` được thể hiện nullable; có điều kiện XOR theo `SET_STATE`/`SET_MODE`; `ACKNOWLEDGED` không nhầm với MQTT PUBACK.
- [ ] **CD-I10** — AUTO/MANUAL theo Actuator, an toàn khi Offline, safe state và hysteresis không mâu thuẫn P1; quyền thay đổi mode do Java xác thực.

### 2.3. AI

- [ ] **CD-A01** — `CountLog` ghi rõ ý nghĩa `entryCount`, `exitCount`, `detectCount` và đơn vị tổng hợp.
- [ ] **CD-A02** — `periodStart < periodEnd`, chu kỳ `aggregationType` và quy tắc không đếm trùng được P2 xác nhận.
- [ ] **CD-A03** — `AnomalyEvent.species`, `confidence`, `detectedAt`, `snapshotPath` khớp dữ liệu từ AI Service.
- [ ] **CD-A04** — `confidence` nằm trong `[0,1]`; `snapshotPath` trỏ đến bằng chứng có thể truy xuất theo giải pháp lưu trữ đã chốt.
- [ ] **CD-A05** — Quan hệ `Device(CAMERA) → CountLog/AnomalyEvent` không cho nhầm kết quả AI sang ESP32.
- [ ] **CD-A06** — Cơ chế chuyển AI event thành Alert có chính sách tránh tạo cảnh báo lặp và được thể hiện trong tài liệu luồng xử lý.

### 2.4. Cảnh báo và thông báo

- [ ] **CD-N01** — `Alert.level` và `Alert.status` dùng đúng Enum; người dùng thấy trạng thái đồng nhất với UI.
- [ ] **CD-N02** — `AlertHistory` lưu `fromStatus`, `toStatus`, `changeAt`, `note` và quan hệ đúng với Alert.
- [ ] **CD-N03** — Xác định cách ghi người thực hiện và thao tác tự động; cập nhật Alert và lưu lịch sử đồng bộ trong transaction.
- [ ] **CD-N04** — `NotificationDelivery` gắn một Alert và có người nhận phù hợp quyền truy cập House/Zone.
- [ ] **CD-N05** — `NotificationChannel` chỉ gồm `WEB`, `PUSH` theo baseline; không tự thêm SMS/Zalo nếu FR chưa đổi.
- [ ] **CD-N06** — `DeliveryStatus` phân biệt `PENDING`, `SENT`, `DELIVERED`, `FAILED`; không coi `DELIVERED` là đã đọc.
- [ ] **CD-N07** — Xác định kênh nào có bằng chứng delivered; các trường thời gian và `failureReason` có nullable phù hợp.

### 2.5. Thống kê và báo cáo

- [ ] **CD-R01** — `HarvestLog` có dữ liệu `date`, `yieldKg`, `note` và phạm vi Zone đúng với nghiệp vụ.
- [ ] **CD-R02** — `yieldKg >= 0`; số liệu thu hoạch thống nhất về đơn vị và chu kỳ.
- [ ] **CD-R03** — `Report.type`, `rangeFrom`, `rangeTo`, `filePath`, `createdAt` có nghĩa cụ thể; `rangeFrom < rangeTo`.
- [ ] **CD-R04** — Xác định rõ Report gắn người tạo và phạm vi House/Zone (nếu yêu cầu) để lọc dữ liệu đúng quyền.
- [ ] **CD-R05** — Báo cáo môi trường, Entry/Exit, thu hoạch và phân tích tương quan được đối chiếu FR; phân tích tương quan không bị diễn đạt thành quan hệ nhân quả.

## 3. Kiểm tra Enumeration — theo hình mới

**Đã thấy trên hình mới 13 Enum:** `RoleName`, `HouseRole`, `DeviceType`, `ActuatorKind`, `ControlMode`, `CommandAction`, `CommandStatus`, `AlertLevel`, `AlertStatus`, `NotificationChannel`, `DeliveryStatus`, `SensorStatus`, `AggregationType`.

- [ ] **CD-E01** — Kiểm tra đủ **13 enum** có đúng giá trị và chính tả như Class Diagram mới.
- [ ] **CD-E02** — `HouseRole` = `OWNER`, `ADMIN`, `VIEWER`; tách phạm vi `RoleName` = `OWNER`, `ADMIN`.
- [ ] **CD-E03** — `AggregationType` = `HOURLY`, `DAILY`; khớp `CountLog.aggregationType`.
- [ ] **CD-E04** — `CommandAction` = `SET_MODE`, `SET_STATE` **và** có trường `ControlCommand.action` (ảnh hiện chưa có trường).
- [ ] **CD-E05** — `Sensor.kind`, `Alert.type`, `Report.type`, `AnomalyEvent.species` vẫn là `String`, không tự thêm enum mới.
- [ ] **CD-E06** — Không thêm `OFFLINE` vào `ControlMode`; trạng thái mạng khác chế độ điều khiển.
- [ ] **CD-E07** — Backend dùng `@Enumerated(EnumType.STRING)` hoặc cơ chế lưu chuỗi tương thích; không dùng enum ordinal.

## 4. Kiểm tra quan hệ UML và chuyển sang ERD

- [ ] **CD-U01** — Đã kiểm tra cardinality `User–HouseMembership–House` và chính sách unique membership.
- [ ] **CD-U02** — Đã kiểm tra composition `House–Zone` và chính sách xóa/vô hiệu hóa, bảo vệ lịch sử.
- [ ] **CD-U03** — Kiểm tra ô ZoneConfig gần Zone là `Zone 1 : ZoneConfig 0..1`; ERD giữ `zone_configs.zone_id` FK+UNIQUE; gỡ class vẽ trùng.
- [ ] **CD-U04** — Đã kiểm tra `Zone–Device`, `Device–Sensor`, `Device–Actuator`, `Device–Telemetry`.
- [ ] **CD-U05** — Đã kiểm tra `Actuator–ControlCommand`, `Device–CountLog`, `Device–AnomalyEvent`.
- [ ] **CD-U06** — Đã kiểm tra `Alert–AlertHistory`, `Alert–NotificationDelivery`, `Zone–HarvestLog`.
- [ ] **CD-U07** — P5 và P4 xác nhận FK/cardinality của liên kết dài (`User–Report`, `User–AlertHistory`, `User–ControlCommand`, `User–NotificationDelivery`, `Zone–Alert`, `Alert–AnomalyEvent`) trên nguồn Draw.io.
- [ ] **CD-U08** — Xác định nullable của `targetState`, `targetMode`, `sentAt`, `ackAt`, `handledAt`, `deliveredAt`, `manualUntil`; đồng bộ ERD/JPA.
- [ ] **CD-U09** — Xác định chỉ mục/unique tối thiểu: tài khoản, `deviceId`, `eventId` theo Device, `commandId` và các FK cần truy vấn lịch sử.
- [ ] **CD-U10** — Không xóa cascade gây mất Telemetry/Alert/CountLog/HarvestLog lịch sử khi vô hiệu hóa một đối tượng quản lý.

## 5. Kiểm tra truy vết FR → Class và sản phẩm liên quan

- [ ] **CD-T01** — FR-USER/FARM/ZONE/DEVICE/RBAC → `User`, `HouseMembership`, `House`, `Zone`, `Device`.
- [ ] **CD-T02** — FR-IOT/CONFIG → `ZoneConfig`, `Sensor`, `Actuator`, `Telemetry`, `ControlCommand`.
- [ ] **CD-T03** — FR-AI → `CountLog`, `AnomalyEvent`; P2 xác nhận các FR thống kê chim và phát hiện bất thường.
- [ ] **CD-T04** — FR-ALT/NOTI → `Alert`, `AlertHistory`, `NotificationDelivery`.
- [ ] **CD-T05** — FR-REP → `HarvestLog`, `Report` và nguồn số liệu môi trường/chim.
- [ ] **CD-T06** — MQTT payload ↔ các trường `Telemetry`/`ControlCommand`; REST DTO ↔ domain; ERD ↔ association và định danh.
- [ ] **CD-T07** — Những FR không được biểu diễn bằng Entity đã được ghi rõ là xử lý ở Service/Sequence/Architecture, không phải bị bỏ quên.

## 6. Kịch bản review nhanh với nhóm

| Kịch bản | Câu hỏi cần trả lời | Các lớp liên quan |
|---|---|---|
| 01. Thêm người quản lý nhà | Có phân quyền theo House không? Membership được tạo ra sao? | User, HouseMembership, House |
| 02. Tạo Zone mới | Cấu hình `0..1` có thể chưa tồn tại; tạo Zone rồi tạo ZoneConfig ra sao? | House, Zone, ZoneConfig |
| 03. ESP32 mất mạng | MQTT `event_id` có giữ nguyên khi retry và Backend ACK sau commit chưa? | Device, Telemetry |
| 04. Cảm biến độ ẩm lỗi | Có đánh dấu PARTIAL_ERROR và tránh dùng giá trị lỗi để AUTO không? | Sensor, Telemetry, Actuator |
| 05. Gửi lệnh bật quạt | Có `SET_MODE(MANUAL)` rồi `SET_STATE(ON)`? `action` và các target nullable đã khớp không? | User, Actuator, ControlCommand, CommandAction |
| 06. Camera đếm chim | Entry/Exit có bị nhầm với số chim hiện có trong đàn không? | Device, CountLog |
| 07. AI phát hiện bất thường | Có snapshot, confidence và liên kết cảnh báo không? | AnomalyEvent, Alert |
| 08. Nhân viên xử lý cảnh báo | Có bản lịch sử cũ/mới và người xử lý không? | User, Alert, AlertHistory |
| 09. Gửi Web/Push | SENT và DELIVERED có được phân biệt và lưu trạng thái không? | Alert, NotificationDelivery |
| 10. Xuất báo cáo | Có đúng phạm vi, thời gian và quyền truy cập dữ liệu không? | HarvestLog, Report, User |
| 11. Chuyển Device giữa Zone | Đây có phải tính năng của Sprint/MVP hay chưa? Lịch sử được giữ thế nào? | Zone, Device, Telemetry |






## 7. Chặn lỗi tích hợp trước khi đóng DSN-06 (ảnh mới)

| Ưu tiên | Vấn đề còn nhìn thấy | Owner | Điều kiện xử lý |
|---|---|---|---|
| P0 | `ControlCommand` thiếu `action: CommandAction` dù enum đã vẽ | P3 | Sửa Draw.io, cập nhật PNG và ERD/JPA mapping |
| P0 | `targetState` và `targetMode` chưa được mô tả nullable/XOR | P3 + P5 | Class nullable; ERD `target_state`/`target_mode` nullable; Java validate theo `action` |
| P1 | `ZoneConfig` vẽ hai vị trí với thuộc tính khác nhau | P3 | Một class chính thức; không tạo hai Entity Java |
| P1 | `Zone–ZoneConfig` đang có mô tả quan hệ không nhất quán giữa hai hình của cùng class | P3 + P5 | Chốt `Zone 1 : ZoneConfig 0..1` hoặc quyết định khác được ghi nhận |
| P1 | Placeholder `method(): Type` vẫn có | P3 | Loại bỏ khỏi sơ đồ bản cuối |
| P1 | `AlertHistory` chưa phân biệt tường minh USER/SYSTEM | P3 + P5 | Chọn `changed_by` nullable hoặc thêm actorType cùng sửa cả hai sơ đồ |
| P1 | Kiểu nullable các phép đo lỗi chưa ghi rõ UML | P3 + P1 + P5 | `temperature/humidity/co2/light` nullable ở Java/DB; giá trị lỗi = `null` |
| P1 | Chưa có bằng chứng review quan hệ dài | P3 + P5 + P4 | Kiểm tra multiplicity bằng `.drawio` nguồn |


