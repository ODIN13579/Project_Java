## 1. Mục tiêu và cách sử dụng

Tài liệu này giải thích các lớp nghiệp vụ chính, trách nhiệm của từng lớp, kiểu dữ liệu, các quan hệ và các quy tắc cần tuân thủ khi triển khai SwiftletCare. Đây là **Domain/Entity Class Diagram**; không phải sơ đồ triển khai chi tiết các `Controller`, `Service`, `Repository` của Spring Boot, không phải ERD, và không mô tả thuật toán bên trong firmware ESP32 hay YOLO/ByteTrack.

**Nguyên tắc nguồn sự thật:** sơ đồ mới là nguồn cho các thuộc tính *đang hiển thị*. Các quyết định tích hợp P1–P4 đã xác nhận (ví dụ `event_id`, heartbeat) được ghi riêng. Khi sơ đồ còn thiếu thuộc tính hoặc thông tin nullable, tài liệu đánh dấu **CẦN SỬA SƠ ĐỒ**, không giả định là đã sửa xong.

## 2. Tổng quan các module

| Khối trên sơ đồ | Các lớp chính | Trách nhiệm |
|---|---|---|
| **Quản lý** | `User`, `HouseMembership`, `House`, `Zone`, `Device` | Người dùng, phân quyền theo nhà yến, tổ chức khu vực và thiết bị |
| **IoT** | `ZoneConfig`, `Sensor`, `Actuator`, `Telemetry`, `ControlCommand` | Cấu hình môi trường, đo đạc, điều khiển, lưu dữ liệu và lệnh |
| **AI** | `CountLog`, `AnomalyEvent` | Kết quả đếm chim và các sự kiện phát hiện bất thường |
| **Cảnh báo** | `Alert`, `AlertHistory`, `NotificationDelivery` | Cảnh báo, lịch sử xử lý và các lần phân phối thông báo |
| **Thống kê và báo cáo** | `HarvestLog`, `Report` | Nhật ký sản lượng và báo cáo đã tạo |

Sơ đồ thể hiện **17 lớp nghiệp vụ phân biệt**. `ZoneConfig` được vẽ **hai lần** (một ô gần `Zone` và một ô trong khu vực IoT); trong tài liệu này chúng được coi là **cùng một lớp**, không phải hai entity khác nhau.

## 3. Danh mục lớp và trách nhiệm

### 3.1. Quản lý

#### `User`
- **Trách nhiệm:** biểu diễn tài khoản người dùng, trạng thái kích hoạt và vai trò toàn hệ thống.
- **Thuộc tính trên sơ đồ:** `id: Long`, `email: String`, `passwordHash: String`, `fullName: String`, `phone: String`, `role: RoleName`, `active: boolean`.
- **Quan hệ:** một `User` có thể có nhiều `HouseMembership`; sơ đồ cũng nối `User` tới các đối tượng phát sinh từ hành động người dùng (báo cáo, thao tác cảnh báo, yêu cầu điều khiển và thông báo).
- **Lưu ý triển khai:** chỉ lưu **hash mật khẩu**, không lưu mật khẩu rõ; phân biệt quyền tổng thể `RoleName` với vai trò theo nhà `HouseMembership.role`.

#### `HouseMembership`
- **Trách nhiệm:** nối `User` và `House` để biểu diễn thành viên/quyền truy cập nhà yến.
- **Thuộc tính:** `id: Long`, `role: HouseRole`, `active: boolean`, `joinedAt: Instant`.
- **Quan hệ:** một bản ghi thuộc **1 User** và **1 House**; mỗi User/House có thể có **0..*** bản ghi Membership.
- **Quy tắc cần hiện thực:** tránh nhiều membership đang hoạt động trùng cho cùng cặp User–House; chỉ cho phép thao tác trong phạm vi quyền được cấp. `HouseRole` **đã được vẽ** trong ảnh mới với `OWNER`, `ADMIN`, `VIEWER`; khác phạm vi với `User.role: RoleName`.

#### `House`
- **Trách nhiệm:** thông tin một nhà yến.
- **Thuộc tính:** `id: Long`, `name: String`, `address: String`, `description: String`, `active: boolean`.
- **Quan hệ:** liên kết với `HouseMembership` và chứa các `Zone`; hình vẽ cho `House 1` — `Zone 1..*` với ký hiệu composition.
- **Lưu ý:** quan hệ `1..*` nghĩa là theo mô hình đã vẽ, House phải có ít nhất một Zone; quy trình tạo House cần thỏa điều kiện này hoặc nhóm xác nhận ngoại lệ ở giai đoạn khởi tạo.

#### `Zone`
- **Trách nhiệm:** một khu vực (tầng/vùng) thuộc nhà yến; phạm vi để gắn thiết bị, cấu hình và thống kê/cảnh báo.
- **Thuộc tính:** `id: Long`, `name: String`, `floorLabel: String`, `areaM2: Double`, `active: boolean`.
- **Quan hệ:** thuộc **1 House**; có quan hệ **1–0..1** với ô `ZoneConfig` cạnh Zone trên sơ đồ mới; chứa **0..*** `Device`; được dùng làm phạm vi của cảnh báo và nhật ký sản lượng.
- **Lưu ý:** sơ đồ hiện tại thể hiện một Device đang thuộc một Zone, không mô tả lịch sử chuyển Zone.

#### `Device`
- **Trách nhiệm:** đại diện thiết bị vật lý đã đăng ký (ESP32 hoặc camera), trạng thái hoạt động và thời điểm hoạt động gần nhất.
- **Thuộc tính:** `id: Long`, `deviceId: String`, `name: String`, `type: DeviceType`, `enabled: boolean`, `online: boolean`, `lastSeenAt: Instant`.
- **Quan hệ:** thuộc Zone; có thể gắn `Sensor`, `Actuator`; phát sinh `Telemetry`, `CountLog`, `AnomalyEvent` theo loại thiết bị. Các quan hệ phải bảo đảm thiết bị CAMERA không bị gán hành vi chỉ có ở ESP32 và ngược lại.
- **Lưu ý:** `online` là trạng thái được Backend tính/cập nhật theo Heartbeat và timeout; `enabled` là việc cho phép thiết bị hoạt động, hai thuộc tính không đồng nghĩa.

### 3.2. IoT

#### `ZoneConfig`
- **Trách nhiệm:** cấu hình các ngưỡng môi trường và chu kỳ telemetry/heartbeat cho Zone; hỗ trợ đồng bộ cấu hình với ESP32.
- **Thuộc tính đọc được từ bản thể hiện đầy đủ:** `id: Long`, `humidityOn: Double`, `humidityOff: Double`, `tempMin: Double`, `tempMax: Double`, `tempHysteresis: Double`, `co2Min: Double`, `co2Max: Double`, `lightMin: Double`, `lightMax: Double`, `telemetryIntervalSec: int`, `heartbeatIntervalSec: int`, `version: int`, `updatedAt: Instant`.
- **Quan hệ:** trên liên kết với ô gần Zone, `Zone 1` — `ZoneConfig 0..1`; trong khối IoT vẫn còn một ô `ZoneConfig` lặp và liên kết khác. Cần giữ một lớp thống nhất, một multiplicity chính thức.
- **Lưu ý:** bản vẽ lặp lại của `ZoneConfig` bên trong vùng IoT có **ít thuộc tính hơn**. Khi thực hiện ERD/code, sử dụng **một định nghĩa thống nhất** được nhóm xác nhận; không tạo hai bảng `ZoneConfig` từ hai ô vẽ.

#### `Sensor`
- **Trách nhiệm:** danh mục cảm biến/loại phép đo được gắn với thiết bị IoT.
- **Thuộc tính:** `id: Long`, `kind: String`, `unit: String`.
- **Quan hệ:** `Device 1` — `Sensor 0..*`.
- **Lưu ý:** đơn vị đo (°C, %RH, ppm, lux...) cần thống nhất giữa thiết bị, MQTT, Backend và UI; `kind` đang là `String`, không phải enum trong bản chốt.

#### `Actuator`
- **Trách nhiệm:** mô hình thiết bị chấp hành: phun sương, quạt, máy sưởi, ampli.
- **Thuộc tính:** `id: Long`, `kind: ActuatorKind`, `mode: ControlMode`, `on: boolean`, `manualUntil: Instant`.
- **Quan hệ:** `Device 1` — `Actuator 0..*`; `Actuator 1` — `ControlCommand 0..*`.
- **Lưu ý:** `on` chỉ có một cờ; khi triển khai cần ghi rõ đó là trạng thái được yêu cầu hay trạng thái nhận từ ESP32, vì phản hồi GPIO không đảm bảo thiết bị vật lý thực sự hoạt động.

#### `Telemetry`
- **Trách nhiệm:** lưu bản đo môi trường từ thiết bị và trạng thái chất lượng dữ liệu.
- **Thuộc tính:** `id: Long`, `eventId: String`, `timestamp: Instant`, `receivedAt: Instant`, `temperature: Double`, `humidity: Double`, `co2: Double`, `light: Double`, `sensorStatus: SensorStatus`.
- **Quan hệ:** `Device 1` — `Telemetry 0..*`.
- **Quy tắc:** tên thuộc tính Java `eventId` ánh xạ JSON MQTT `event_id` và SQL `event_id` (quyết định đã chốt); chống trùng qua `UNIQUE(device_id, event_id)` ở DB; retry giữ nguyên `event_id` và thời điểm đo. `timestamp` khác `receivedAt`. Các phép đo lỗi cần cho phép `null` trong DTO/DB; không dùng số 0 giả làm dữ liệu hợp lệ.

#### `ControlCommand`
- **Trách nhiệm:** theo dõi lệnh điều khiển gửi tới actuator và vòng đời xử lý lệnh.
- **Thuộc tính THỰC SỰ đang vẽ:** `id: Long`, `commandId: String`, `targetState: boolean`, `targetMode: ControlMode`, `requestedAt: Instant`, `status: CommandStatus`, `sentAt: Instant`, `ackAt: Instant`, `failureReason: String`, `expiresAt: Instant`.
- **Sai lệch còn lại:** dù `CommandAction {SET_MODE, SET_STATE}` đã vẽ trong nhóm Enum, `ControlCommand` **chưa có** `action: CommandAction`. Hai thuộc tính `targetState` và `targetMode` cũng chưa thể hiện nullable.
- **Cấu trúc đề xuất cần P3 sửa trên sơ đồ để khớp ERD/P1:** `action: CommandAction` (bắt buộc), `targetState: Boolean [0..1]`, `targetMode: ControlMode [0..1]`. Quy tắc XOR: `SET_STATE` chỉ dùng `targetState`, `SET_MODE` chỉ dùng `targetMode`.
- **Quan hệ:** `Actuator 1` — `ControlCommand 0..*`; sơ đồ còn nối các lệnh với `User` ở phía yêu cầu thao tác.
- **Quy tắc:** `PENDING`/`SENT` chưa đồng nghĩa thi hành thành công; lệnh quá hạn hoặc bị từ chối không được áp dụng; `commandId` không đổi khi retry.

### 3.3. AI

#### `CountLog`
- **Trách nhiệm:** kết quả đếm chim theo khoảng thời gian và loại tổng hợp.
- **Thuộc tính:** `id: Long`, `periodStart: Instant`, `periodEnd: Instant`, `entryCount: int`, `exitCount: int`, `detectCount: int`, `aggregationType: AggregationType`, `processedAt: Instant`.
- **Quan hệ:** gắn với `Device` kiểu CAMERA.
- **Lưu ý:** `entryCount`/`exitCount` là số lượt vào/ra; `detectCount` cần có định nghĩa đo lường do P2 thống nhất. `AggregationType` đã xuất hiện như một ô enum riêng trong ảnh mới: `HOURLY`, `DAILY`.

#### `AnomalyEvent`
- **Trách nhiệm:** lưu sự kiện bất thường do AI phát hiện cùng độ tin cậy và ảnh bằng chứng.
- **Thuộc tính:** `id: Long`, `species: String`, `confidence: Double`, `detectedAt: Instant`, `snapshotPath: String`.
- **Quan hệ:** phát sinh từ Device kiểu CAMERA; sơ đồ có liên kết với Alert.
- **Lưu ý:** kiểm tra `confidence` trong `[0,1]`; quy định `snapshotPath` là đường dẫn/khóa lưu trữ nào và chính sách lưu bằng chứng.

### 3.4. Cảnh báo và thông báo

#### `Alert`
- **Trách nhiệm:** cảnh báo nghiệp vụ cần theo dõi/xử lý.
- **Thuộc tính:** `id: Long`, `type: String`, `level: AlertLevel`, `status: AlertStatus`, `message: String`, `createdAt: Instant`, `handledAt: Instant`.
- **Quan hệ:** phát sinh trong ngữ cảnh Zone/AI; có nhiều `AlertHistory` và `NotificationDelivery`.
- **Quy tắc:** trạng thái theo `NEW`, `IN_PROGRESS`, `RESOLVED`; không coi việc đã gửi thông báo là đã xử lý cảnh báo.

#### `AlertHistory`
- **Trách nhiệm:** lưu lịch sử chuyển trạng thái của cảnh báo, phục vụ tra cứu/audit.
- **Thuộc tính:** `id: Long`, `fromStatus: AlertStatus`, `toStatus: AlertStatus`, `changeAt: Instant`, `note: String`.
- **Quan hệ:** thuộc đúng **1 Alert**; mỗi Alert có **0..*** lịch sử. Sơ đồ có liên kết với User cho người thực hiện.
- **Quy tắc:** cập nhật `Alert.status` và tạo bản ghi lịch sử tương ứng trong cùng transaction ở Backend. Hành động tự động cần nhận diện được tác nhân hệ thống; cách mã hóa chưa được chỉ rõ trên sơ đồ.

#### `NotificationDelivery`
- **Trách nhiệm:** theo dõi từng lần phân phối thông báo liên quan đến cảnh báo.
- **Thuộc tính:** `id: Long`, `channel: NotificationChannel`, `status: DeliveryStatus`, `createdAt: Instant`, `sentAt: Instant`, `deliveredAt: Instant`, `failureReason: String`.
- **Quan hệ:** thuộc **1 Alert**; mỗi Alert có **0..*** lượt phân phối; sơ đồ có liên kết với User nhận thông báo.
- **Quy tắc:** `SENT` khác `DELIVERED` và khác *đã đọc*; chỉ đặt `DELIVERED` nếu có bằng chứng từ kênh cung cấp. Các kênh đã thể hiện là Web/Push.

### 3.5. Thống kê và báo cáo

#### `HarvestLog`
- **Trách nhiệm:** ghi nhận sản lượng thu hoạch và ghi chú theo ngày.
- **Thuộc tính:** `id: Long`, `date: LocalDate`, `yieldKg: Double`, `note: String`.
- **Quan hệ:** sơ đồ liên kết HarvestLog với Zone.
- **Quy tắc:** `yieldKg >= 0`; dữ liệu đầu vào dùng trong báo cáo môi trường/năng suất cần thống nhất phạm vi Zone và kỳ thời gian.

#### `Report`
- **Trách nhiệm:** bản ghi metadata của báo cáo được tạo/lưu.
- **Thuộc tính:** `id: Long`, `type: String`, `rangeFrom: Instant`, `rangeTo: Instant`, `filePath: String`, `createdAt: Instant`.
- **Quan hệ:** sơ đồ liên kết Report với User.
- **Quy tắc:** `rangeFrom < rangeTo`; `filePath` là tham chiếu tệp kết quả, không phải dữ liệu báo cáo; P3 cần thống nhất loại báo cáo và phạm vi dữ liệu với FR-REP.

## 4. Enumeration trong phiên bản Class Diagram mới

Ảnh mới **đã vẽ đủ 13 enum** sau. Không đánh dấu `HouseRole`, `AggregationType` hay `CommandAction` là thiếu ô khai báo nữa.

| Enum trên sơ đồ | Giá trị | Đang sử dụng ở |
|---|---|---|
| `RoleName` | `OWNER`, `ADMIN` | `User.role` |
| `HouseRole` | `OWNER`, `ADMIN`, `VIEWER` | `HouseMembership.role` |
| `DeviceType` | `ESP32`, `CAMERA` | `Device.type` |
| `ActuatorKind` | `MIST`, `FAN`, `HEATER`, `AMPLIFIER` | `Actuator.kind` |
| `ControlMode` | `AUTO`, `MANUAL` | `Actuator.mode`, `ControlCommand.targetMode` |
| `CommandAction` | `SET_MODE`, `SET_STATE` | **Chưa được tham chiếu trong thuộc tính ControlCommand** — cần thêm `action` |
| `CommandStatus` | `PENDING`, `SENT`, `ACKNOWLEDGED`, `FAILED`, `REJECTED`, `EXPIRED` | `ControlCommand.status` |
| `AlertLevel` | `WARNING`, `CRITICAL` | `Alert.level` |
| `AlertStatus` | `NEW`, `IN_PROGRESS`, `RESOLVED` | `Alert.status`, `AlertHistory.fromStatus/toStatus` |
| `NotificationChannel` | `WEB`, `PUSH` | `NotificationDelivery.channel` |
| `DeliveryStatus` | `PENDING`, `SENT`, `DELIVERED`, `FAILED` | `NotificationDelivery.status` |
| `SensorStatus` | `OK`, `PARTIAL_ERROR`, `ERROR` | `Telemetry.sensorStatus` |
| `AggregationType` | `HOURLY`, `DAILY` | `CountLog.aggregationType` |

`Sensor.kind`, `Alert.type`, `Report.type`, `AnomalyEvent.species` là `String` trong sơ đồ. **`OFFLINE` không thuộc `ControlMode`**; đây là trạng thái kết nối thiết bị được Backend suy ra từ heartbeat.

## 5. Quan hệ nghiệp vụ trọng tâm

| Liên kết | Multiplicity đọc được từ sơ đồ | Diễn giải |
|---|---|---|
| `User` — `HouseMembership` | User **1** : Membership **0..*** | Một người dùng có thể có nhiều quyền tham gia nhà |
| `House` — `HouseMembership` | House **1** : Membership **0..*** | Một nhà có nhiều thành viên |
| `House` — `Zone` | House **1** : Zone **1..***, composition | Một nhà chứa ít nhất một khu vực theo sơ đồ |
| `Zone` — `ZoneConfig` | **Zone 1 : Config 0..1** ở ô gần Zone | Mỗi Zone có tối đa một cấu hình hiện hành; ô trùng trong vùng IoT cần gỡ/hợp nhất |
| `Zone` — `Device` | Zone **1** : Device **0..*** | Mỗi Device thuộc một Zone trong baseline |
| `Device` — `Sensor` | Device **1** : Sensor **0..*** | Thiết bị có nhiều cảm biến |
| `Device` — `Actuator` | Device **1** : Actuator **0..*** | Thiết bị có nhiều bộ chấp hành |
| `Device` — `Telemetry` | Device **1** : Telemetry **0..*** | Một thiết bị phát sinh nhiều mẫu đo |
| `Actuator` — `ControlCommand` | Actuator **1** : Command **0..*** | Nhiều lệnh theo thời gian |
| `Device` — `CountLog` | Device **1** : CountLog **0..*** | Camera tạo nhiều bản tổng hợp số đếm |
| `Device` — `AnomalyEvent` | Device **1** : AnomalyEvent **0..*** | Camera phát hiện sự kiện bất thường |
| `Alert` — `AlertHistory` | Alert **1** : History **0..***, composition | Alert có lịch sử thay đổi |
| `Alert` — `NotificationDelivery` | Alert **1** : Delivery **0..*** | Alert được gửi qua nhiều lượt/kênh |
| `Zone` — `HarvestLog` | Zone **1** : HarvestLog **0..*** | Lưu nhiều nhật ký theo khu vực |

Các liên kết **User ↔ Report**, **User ↔ AlertHistory**, **User ↔ ControlCommand**, **User ↔ NotificationDelivery**, **Zone ↔ Alert**, **Alert ↔ AnomalyEvent** đã có trên ảnh nhưng một số đoạn đi qua nhiều module; khi chuyển sang ERD, P4 cần đọc trực tiếp sơ đồ nguồn `.drawio` để xác nhận chính xác multiplicity và tính tùy chọn, không suy đoán từ đường giao nhau trong ảnh PNG.

### 5.1. Vấn đề nghiệp vụ về chuyển thiết bị giữa Zone

Baseline cho biết một thiết bị thuộc **một Zone tại một thời điểm**, nhưng **không chứa** `DeviceZoneHistory` hoặc thao tác `reassignZone`. Vì vậy tài liệu này **không khẳng định** nhóm đã có chức năng chuyển thiết bị giữa các Zone. Nếu FR chính thức có chức năng đó, cần một quyết định bổ sung để bảo toàn lịch sử Telemetry và thời điểm hiệu lực cấu hình; xem `review-checklist.md`.

## 6. Quy tắc nghiệp vụ cần bảo đảm khi triển khai

> Các quy tắc dưới đây giải thích ý nghĩa của các lớp/trường trong baseline; các quy tắc chưa được FR hoặc nhóm phê duyệt cần xác minh trước khi coi là tiêu chí nghiệm thu chính thức.

**BR-01 — Phân quyền:** quyền toàn hệ thống (`User.role`) và quyền từng nhà (`HouseMembership.role`) có hai phạm vi khác nhau; yêu cầu API phải kiểm tra đúng phạm vi đối tượng House/Zone/Device.

**BR-02 — Tổ chức dữ liệu:** Zone thuộc một House; Device thuộc một Zone ở thời điểm hiện tại; không sửa ngược dữ liệu lịch sử khi thay đổi thiết bị/cấu hình.

**BR-03 — Cấu hình:** kiểm tra `humidityOn < humidityOff`, `tempMin < tempMax`, `co2Min <= co2Max`, `lightMin <= lightMax` và các giới hạn/chủng loại cảm biến hợp lệ; cấu hình mới cần tăng `version` và không ghi đè cấu hình đã xác minh khi bản mới lỗi.

**BR-04 — AUTO/MANUAL:** việc mất mạng không đồng nghĩa chuyển AUTO thành MANUAL; trong phạm vi thiết kế IoT của P1, ESP32 phải có chính sách điều khiển cục bộ và safe state được chốt.

**BR-05 — Telemetry:** `eventId` không thay đổi khi retry; Backend chống trùng theo thiết bị và sự kiện, phân biệt `timestamp` với `receivedAt`, không dùng số đo không hợp lệ để ra lệnh AUTO.

**BR-06 — Command:** `SET_MODE` đổi AUTO/MANUAL; `SET_STATE` đặt ON/OFF chỉ khi actuator MANUAL và không vi phạm an toàn. Cần bổ sung `action` ở lớp (hiện sơ đồ chưa có); trường đích không dùng cho action tương ứng phải để `NULL`. Xác thực quyền, kiểm tra hạn lệnh, chống trùng `commandId`; `SENT` không có nghĩa relay đã thực thi. Command ACK phải phân biệt với Telemetry ACK và MQTT PUBACK.

**BR-07 — Bird count:** `entryCount` và `exitCount` là số lượt; `detectCount` không được mặc định là quy mô đàn chim. `periodStart < periodEnd`.

**BR-08 — AI evidence:** `confidence` thuộc `[0,1]`; sự kiện phát hiện có thời điểm, nguồn camera và bằng chứng hợp lệ theo thiết kế lưu trữ.

**BR-09 — Alert workflow:** cập nhật trạng thái Alert kèm bản ghi AlertHistory nhất quán; `NEW → IN_PROGRESS → RESOLVED` là luồng thông dụng nhưng các chuyển trạng thái thực sự được phép phải đối chiếu FR.

**BR-10 — Notification:** `NotificationDelivery` là kết quả gửi theo kênh; không suy ra trạng thái đã đọc từ `SENT` hoặc `DELIVERED`.

**BR-11 — Harvest/Report:** sản lượng không âm; khoảng thời gian báo cáo hợp lệ; báo cáo tương quan môi trường/năng suất cần dùng dữ liệu cùng phạm vi và thời kỳ, tránh suy luận quan hệ nhân quả.

**BR-12 — Khả năng truy xuất:** các dữ liệu lịch sử (`Telemetry`, `CountLog`, `AnomalyEvent`, `AlertHistory`, `HarvestLog`, `Report`) cần chính sách lưu giữ/khóa ngoại phù hợp; không xóa cascade bừa bãi khi vô hiệu hóa House/Zone/Device.

## 7. Ranh giới giữa Class Diagram và các thiết kế khác

| Nội dung | Nơi chốt chi tiết | Phối hợp |
|---|---|---|
| GPIO, cảm biến, hysteresis, ESP32 offline và Buffer | IoT Architecture / MQTT Topic Specification | P1 + P4 |
| Kiểu dữ liệu MQTT `event_id` ↔ Java `eventId`, ACK, `commandId` | MQTT Spec / REST API Spec | P1 + P3 + P4 |
| Định nghĩa đếm Entry/Exit, `detectCount`, AI event | AI Architecture / Dataset Spec | P2 + P3 |
| Ràng buộc `UNIQUE`, FK, nullable, index, cascade | ERD và Database Schema | P5 + P4 |
| API Controller/Service/Repository, auth, transaction | Backend Architecture | P4 |
| Web/Push và trạng thái gửi/đọc | Notification design / UI | P5 + P4 |
| Công thức thống kê, phân tích tương quan, xuất file | FR-REP / Report design | P3 + P4 |

Không thể suy ra một thiết kế chi tiết chỉ từ đường nối UML; các tài liệu khác phải viện dẫn và thống nhất với mô hình lớp.

## 8. Ghi chú rà soát bản chốt — không tự thay đổi baseline

Các điểm sau là **vấn đề tài liệu/triển khai cần xác minh**, không phải yêu cầu bắt buộc bổ sung tính năng:

1. **`ZoneConfig` vẫn được vẽ hai lần với danh sách thuộc tính khác nhau:** dùng bản có đầy đủ các ngưỡng và chu kỳ; P3 cần hợp nhất thành một class/định nghĩa và giữ quan hệ Zone–ZoneConfig `1 : 0..1` theo ô gần Zone, trừ khi nhóm duyệt thay đổi.
2. **`HouseRole`, `AggregationType`, `CommandAction` đã có ô Enum trong bản mới.** Tuy nhiên `CommandAction` chưa xuất hiện trong thuộc tính `ControlCommand` (thiếu `action: CommandAction`).
3. **`+method(): Type` xuất hiện trong nhiều lớp:** đây có vẻ là placeholder của công cụ vẽ; không xem là phương thức nghiệp vụ thực tế.
4. **Chưa thể hiện rõ nullable:** quan trọng nhất là `ControlCommand.targetState/targetMode` (chỉ được dùng một trong hai tùy action), cùng `ackAt`, `sentAt`, `handledAt`, `deliveredAt`, `manualUntil`, `failureReason`, `filePath`; P3 cần ghi trên sơ đồ/đặc tả, P5 và P4 thực hiện trong ERD/JPA.
5. **Vai trò toàn cục và vai trò House:** sơ đồ có cả `RoleName` và `HouseRole`, cần bảng quyền rõ ràng để không phân quyền sai.
6. **`Device.online` phụ thuộc heartbeat/timeout:** nhóm đã chốt P1 gửi heartbeat **2 giây**; P4 timeout **6 giây**, scheduler **1 giây**. `online` là trạng thái Backend ghi nhận, không thay thế `ControlMode`.
7. **Không thấy `DeviceZoneHistory`:** bản chốt chưa cam kết lưu lịch sử chuyển Zone; chỉ xử lý nếu FR xác nhận tính năng.
8. **Sơ đồ dùng `String` cho `Sensor.kind`, `Alert.type`, `Report.type`:** đây là lựa chọn của baseline; Backend có thể validate theo danh mục nhưng không tự coi các trường là Enum.
9. **Đường nối liên module dày:** kiểm tra cardinality trên file nguồn `.drawio` khi xuất ERD để tránh hiểu sai.
10. **Chưa biểu diễn riêng luồng audio đo âm lượng, trạng thái đọc thông báo, thiết bị đang chờ gán Zone:** không tự thêm Entity nếu FR không yêu cầu.
11. **`ControlCommand` còn thiếu `action` và hai target chưa đánh dấu nullable:** buộc sửa trước khi chốt migration cho `SET_MODE`/`SET_STATE`.
12. **`AlertHistory` không có `actorType` trên class mới:** ERD có thể dùng `changed_by` nullable để biểu diễn System, nhưng nếu cần phân biệt tường minh USER/SYSTEM thì phải được nhóm chốt và sửa đồng bộ cả hai sơ đồ.
13. **Các `+method(): Type` vẫn còn trong hình mới:** không được xem là API Java thật, nên loại khỏi bản bàn giao cuối.

## 9. Hồ sơ bàn giao DSN-06

Các tệp nên đi cùng một phiên bản thiết kế:

```text
 docs/design/class-diagram/
 ├── system-class-diagram.drawio  # nguồn chỉnh sửa được
 ├── system-class-diagram.png     # ảnh từ đúng bản drawio
 ├── overview.md                 # tài liệu này
 └── review-checklist.md         # bảng kiểm nghiệm thu
```

**Điều kiện nhất quán:** tên lớp/trường/enum trong ERD, API payload và các tài liệu của P1/P2/P4/P5 phải cùng ngữ nghĩa với bản class diagram đã chốt. Nếu phát hiện khác biệt, ghi rõ issue và người xác nhận; không âm thầm sửa baseline.

## 10. Bảng đồng bộ P3 — P5 — P1/P4 trước BUILD

| Ý nghĩa | Class Diagram P3 | MQTT P1 / Java P4 | ERD P5 | Trạng thái |
|---|---|---|---|---|
| Telemetry ID | `Telemetry.eventId` | JSON `event_id`, Java `eventId` | `telemetry.event_id` + UNIQUE `(device_id, event_id)` | Đã chốt tên; xác minh constraint trong migration |
| Device ID | `Device.deviceId` | JSON `device_id` là mã chuỗi | `devices.device_id` unique; `telemetry.device_id` là FK BIGINT | Cần mapping DTO → Device.id |
| Đổi chế độ | Enum `CommandAction.SET_MODE` | MQTT command `SET_MODE` | `control_commands.action`, `target_mode` | **Thiếu `action` trong class** |
| Bật/tắt | Enum `CommandAction.SET_STATE` | MQTT command `SET_STATE` | `control_commands.action`, `target_state` | **Hai target phải nullable** |
| Command ACK | `status: CommandStatus`, `ackAt` | `command_ack` từ ESP32 | `status`, `ack_at`, `failure_reason` | P1/P4 xác nhận ngữ nghĩa ACK |
| Heartbeat | `Device.online`, `lastSeenAt` | 2s / timeout 6s / check 1s | `devices.online`, `last_seen_at` | Đã chốt quy trình, chưa chứng minh NFR |
| Sensor lỗi | `SensorStatus` | Phép đo lỗi là `null` | Cột đo nullable | Đồng bộ logic; cần test |
| ZoneConfig | Một class logic, vẽ trùng 2 vị trí | Cấu hình theo Zone | `zone_configs.zone_id` UNIQUE | **Phải hợp nhất ô trùng ở sơ đồ** |
