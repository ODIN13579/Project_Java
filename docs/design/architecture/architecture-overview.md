# SwiftletCare — Architecture Overview

> **Loại tài liệu:** Đặc tả kiến trúc thành phần và ranh giới hệ thống.  
> Đề xuất công nghệ và nội dung chưa chốt được ghi là **[ĐỀ XUẤT]/[TBD]**; không tự biến thành yêu cầu chính thức.

## 1. Bức tranh tổng thể

SwiftletCare gồm **thiết bị quan trắc/điều khiển tại nhà yến**, **hệ thống giao tiếp và xử lý nghiệp vụ tập trung**, **giao diện giám sát và dịch vụ thông báo**. Hệ thống dùng **Java Spring Boot làm Backend nghiệp vụ trung tâm**, trong khi các thuật toán AI chạy ở dịch vụ riêng và AUTO điều khiển môi trường có thể hoạt động cục bộ trên ESP32 khi mất mạng.

![System Architecture](system-architecture.png)

### 1.1. Nguyên tắc phân tách

| Phạm vi | Chạy ở đâu | Trách nhiệm cốt lõi | Không chịu trách nhiệm |
|---|---|---|---|
| **ESP32/Wokwi** | Tại nhà yến / mô phỏng | Đọc cảm biến, áp dụng AUTO/MANUAL, điều khiển relay, Buffer/Retry MQTT | Login Web/JWT, lưu dữ liệu SQL, YOLO/ByteTrack |
| **MQTT Broker** | Server/MQTT infrastructure | Chuyển bản tin, xác thực client/ACL, LWT nếu cấu hình | Quyết định nghiệp vụ, tự lưu JPA, bảo đảm Backend đã commit |
| **Java Spring Boot** | Server | REST, JWT/RBAC, MQTT client, validate, JPA, báo cáo, cảnh báo, WebSocket | Đọc GPIO trực tiếp; chạy AI trên ESP32 |
| **AI Service** | Máy edge riêng hoặc nguồn xử lý phù hợp | YOLOv8/ByteTrack, đếm chim, bất thường và snapshot | Điều khiển ESP32 trực tiếp |
| **React Web** | Trình duyệt | Hiển thị dashboard, REST, realtime WebSocket, gửi yêu cầu điều khiển | Kết nối thẳng DB, tạo telemetry tin cậy, truy cập MQTT broker như nguồn sự thật |
| **DB** | Server | Lưu dữ liệu nghiệp vụ và lịch sử | Lưu video/ảnh dung lượng lớn trong các cột ảnh nếu dùng object storage |

## 2. Khối A — Device / Edge tại nhà yến

### 2.1. Cảm biến môi trường

**[SƠ ĐỒ]** Đo **nhiệt độ, độ ẩm, CO₂, ánh sáng**. Thông qua giao tiếp cảm biến đến ESP32. Sơ đồ tổng thể **không quy định** model cảm biến, chân GPIO, công suất hoặc điện áp; đặc tả kết nối thuộc `iot-hardware-spec.md`.

Giá trị không hợp lệ phải có trạng thái lỗi tương ứng; `PARTIAL_ERROR` nghĩa là một số phép đo không sử dụng được, không buộc mọi điều khiển hợp lệ phải dừng. Trạng thái an toàn của từng actuator được mô tả trong đặc tả IoT Control/Offline Recovery.

### 2.2. ESP32 / mô phỏng Wokwi

**[SƠ ĐỒ]** Có AUTO/MANUAL, ngưỡng điều khiển cục bộ, MQTT, Buffer và Reconnect. Điều khiển relay qua GPIO sau kiểm tra an toàn.

**[ĐÃ CHỐT trong hợp đồng MQTT/Backend]**:

- AUTO/MANUAL là **ControlMode theo Actuator**; `ONLINE/OFFLINE/RECOVERING` là trạng thái kết nối riêng. Có thể đồng thời `AUTO + OFFLINE`.
- Gửi Telemetry mặc định **5 giây**, dải cấu hình **1–60 giây**; bản tin Telemetry định danh bằng `event_id`.
- Gửi Heartbeat **2 giây/lần**; Backend kiểm tra Offline sau **6 giây**, với Scheduler **1 giây/lần**.
- Telemetry chưa được ứng dụng Backend xác nhận `STORED` vẫn ở Buffer; retry giữ nguyên `event_id` và thời điểm đo.
- `SET_STATE` khi còn AUTO bị từ chối; chuyển mode là thao tác tách biệt `SET_MODE` (theo thiết kế giao tiếp hiện hành, vẫn cần duyệt mọi trường payload/QoS).

**[SƠ ĐỒ/NFR]** Mục tiêu AUTO offline **≥24 giờ** khi ESP32 vẫn có nguồn, cảm biến và bộ chấp hành đáp ứng. Mục tiêu lưu bù **≥17.280 bản ghi/24 giờ** ở chu kỳ 5s. Đây là **mục tiêu phải đo**, không phải khả năng đã chứng minh.

**[TBD]** phần cứng thực tế, lưu trữ Buffer bền vững, mức kích relay, xử lý Buffer đầy, safe state từng thiết bị, timeout Telemetry ACK.

### 2.3. Relay / thiết bị chấp hành

**[SƠ ĐỒ]** Máy phun sương, quạt, sưởi và loa/ampli. GPIO/relay là đầu ra điều khiển; **bản tin ACK xác nhận trạng thái GPIO/relay được báo**, không tự chứng minh quạt/máy thực chạy cơ học. Cần phản hồi phần cứng bổ sung nếu FR yêu cầu xác minh tải vật lý.

### 2.4. Camera / Video và AI Edge

**[SƠ ĐỒ]** Camera hoặc video mô phỏng cung cấp khung hình qua LAN/RTSP cho một **AI Service riêng**, dùng **YOLOv8 + ByteTrack** để tạo **count, anomaly, snapshot**. Sơ đồ ghi rõ **không chạy AI trên ESP32**. AI Service gửi sự kiện và đường dẫn ảnh đến Backend qua **HTTPS REST** theo giao thức AI–Backend.

**[TBD]** Máy edge thực tế, cách lấy frame, tốc độ khung hình, yêu cầu bảo vệ luồng camera; Live View chỉ xuất hiện trên sơ đồ bằng **nét đứt/đề xuất**.

## 3. Khối B — Communication / Backend / Data

### 3.1. MQTT Broker

**[SƠ ĐỒ]** Broker trung gian trao đổi Telemetry, Heartbeat, Command và ACK; có xác thực thiết bị và ACL. **Mosquitto (*)** là lựa chọn đề xuất, chưa tự coi là công nghệ đã chọn cuối cùng. Kênh bảo vệ là **MQTTS** ở thiết kế mục tiêu; cấu hình Wokwi/demo có thể khác nhưng phải nêu rõ giới hạn bảo mật.

**[ĐÃ GHI trong MQTT Contract]**: Telemetry và Heartbeat đang dùng **QoS 0**, Availability/LWT **QoS 1 + Retain**, Command/ACK/Configuration có QoS/Retain đề xuất cần duyệt. **MQTT QoS/PUBACK không thay thế Application ACK** sau khi Database commit.

### 3.2. Spring Boot Backend — Modular Monolith (*)

**[SƠ ĐỒ]** Backend trung tâm có REST API + MQTT client, token/RBAC, kiểm tra schema/chống trùng, module User/House/Zone/Device/Telemetry/AI Events/Report/Alert/Control/Notification và WebSocket endpoint.

**[ĐỀ XUẤT cụ thể cho Spring Boot]** cấu trúc nội bộ:

```text
Spring Boot application
├── api/controller/        # HTTP REST Controller
├── application/service/   # business rule + transaction
├── domain/                 # Entity, enum, DTO mapping
├── persistence/repository/ # JPA/SQL
├── integration/mqtt/      # subscribe/publish + ACK
├── integration/ai/        # HTTP internal endpoints
├── notification/          # alert/notification channel orchestration
└── realtime/              # WebSocket event publisher
```

Đây là **khung minh họa**, không bắt buộc thay đổi cách tổ chức package của ứng dụng. Các loại HTTP/path, DTO và status cụ thể lấy từ **`openapi.yaml`**, không trùng lặp quy định ở tài liệu kiến trúc tổng thể.

**Nguyên tắc giao dịch:** xác thực/kiểm tra quyền trước khi xử lý; đọc/ghi qua Service/Repository; chỉ gửi `telemetry_ack(STORED)` khi đã commit hoặc xác nhận bản trùng hợp lệ; realtime thông báo trạng thái **sau khi dữ liệu nghiệp vụ được lưu thành công**.

### 3.3. Database

**[SƠ ĐỒ]** Database **PostgreSQL (*)** là đề xuất, kết nối Backend qua SQL/JDBC/JPA, chứa User/House/Zone/Device, lịch sử môi trường, AI events, alerts và báo cáo. **[TBD]** lựa chọn cuối PostgreSQL hoặc MySQL (nếu chưa xác định lựa chọn cuối), version DB, migration tool và lưu trữ dữ liệu dài hạn.

**[Đối chiếu đặc tả ERD]** Baseline ERD có **17 bảng**; không tự thêm các bảng Heartbeat, TelemetryAck, Camera, Session video vào baseline nếu không có quyết định thay đổi. `devices.last_seen_at`, `online` biểu diễn trạng thái; `telemetry.event_id` + `device_id` cần UNIQUE để chống ghi trùng.

### 3.4. Kho ảnh bằng chứng

**[SƠ ĐỒ]** `MinIO / S3 theo FR (*)` để lưu ảnh riêng, Database chỉ lưu metadata/path như `AnomalyEvent.snapshotPath`. **[TBD]** có sử dụng trong triển khai hay không, cách tạo presigned URL/ủy quyền, TTL, tên bucket, phương án local filesystem nếu chỉ demo. Không công khai bucket ngoài quyền truy cập.

### 3.5. WebSocket Endpoint

**[SƠ ĐỒ]** Push dữ liệu **Telemetry / ACK / Alert** đến Dashboard. REST lấy snapshot ban đầu và sau reconnect; WebSocket gửi sự kiện cập nhật. WebSocket **không thay thế Database** và không bảo đảm browser nhận được mọi sự kiện khi mất mạng.

## 4. Khối C — Presentation / Push

### 4.1. React Web Dashboard

**[SƠ ĐỒ]** Login, quản lý, điều khiển, thống kê, count, cảnh báo; dữ liệu realtime cần Backend. Dùng **HTTPS REST** để truy vấn/yêu cầu thay đổi, **WSS** cho trạng thái mới.

**Không được** dùng browser trực tiếp truy cập database hoặc tin rằng HTTP `202 Accepted` cho Command nghĩa relay đã bật. UI nên đọc trạng thái cuối từ `ControlCommand`/Actuator cập nhật sau Command ACK.

### 4.2. Notification Service (*)

**[SƠ ĐỒ]** Web Push + Mobile Push, đề xuất FCM; có retry giới hạn và chống trùng. `NotificationDelivery` theo dõi trạng thái WEB/PUSH; `SENT`, `DELIVERED`, `FAILED` không đồng nghĩa **đã đọc**. Mobile Push/FCM và nhà cung cấp cụ thể phải được duyệt (dấu *).

### 4.3. Media Gateway (*)

**[SƠ ĐỒ]** Chỉ cho Live View khi được yêu cầu; luồng video nét đứt đi qua gateway từ nguồn LAN/RTSP đến browser qua **HTTPS/HLS** (phương án minh họa). Trình duyệt thông thường không dùng trực tiếp luồng RTSP; chuyển giao thức và bảo vệ session tùy quyết định. **Media Gateway là thành phần tùy chọn** và chỉ có trong kiến trúc thực thi khi chức năng Live View được hỗ trợ.

## 5. Phân quyền và các kênh giao tiếp

| Kênh | Bên gửi → nhận | Định dạng / mục đích | An ninh / nguồn contract |
|---|---|---|---|
| MQTTS | ESP32 ↔ Broker ↔ Spring Boot | Telemetry, Command, Heartbeat, ACK, Config | MQTT client credential, ACL, đặc tả MQTT |
| HTTPS REST | React → Spring Boot | Login/CRUD/query/commands | Token/JWT, HouseRole/RBAC, đặc tả REST API |
| HTTPS REST nội bộ | AI Service → Spring Boot | CountLog/AnomalyEvent và metadata snapshot | Service authentication **[TBD]**, đặc tả AI Service và REST API |
| SQL/JDBC | Spring Boot ↔ Database | Transaction, query lịch sử | Quyền DB riêng **[ĐỀ XUẤT]**, đặc tả ERD |
| WSS | Spring Boot → React | Telemetry/Device/Command/Alert change | Auth connection, scoped subscription **[ĐỀ XUẤT]**, đặc tả REST API |
| HTTPS API/Push | Backend → Notification Service | Web/Mobile push | Provider auth/config **[TBD]** |
| LAN / RTSP | Camera → AI Service | Video/frame | Chặn truy cập trái phép, đặc tả AI Service |
| HTTPS/HLS (*) | Gateway → Browser | Live View (nếu chốt) | Session authorize trước khi phát video |

## 6. Dữ liệu nghiệp vụ theo Class/ERD

| Dòng dữ liệu | Entity/Bảng chuẩn | Khóa/nguyên tắc |
|---|---|---|
| Cảm biến | `Telemetry` / `telemetry` | MQTT `event_id` → Java `eventId` → SQL `event_id`; UNIQUE `(device_id,event_id)` |
| Thiết bị | `Device` / `devices` | REST `id BIGINT` khác MQTT `device_id` dạng chuỗi |
| Chế độ | `Actuator` / `actuators` | `ControlMode.AUTO/MANUAL`; trạng thái kết nối không phải mode |
| Lệnh | `ControlCommand` / `control_commands` | `SET_MODE`/`SET_STATE`; commandId, action, targetMode/targetState, ACK |
| AI count | `CountLog` / `count_logs` | Tổng hợp IN/OUT theo kỳ, cần cần quy tắc chống trùng |
| AI alert | `AnomalyEvent` / `anomaly_events` | Confidence, detectedAt, snapshotPath |
| Cảnh báo | `Alert`, `AlertHistory` | `NEW / IN_PROGRESS / RESOLVED` + audit |
| Gửi thông báo | `NotificationDelivery` | `WEB / PUSH` trong baseline |
| Báo cáo, năng suất | `Report`, `HarvestLog` | Báo cáo metadata, HarvestLog theo Zone/ngày |

## 7. NFR thể hiện trong sơ đồ và giới hạn chứng minh

| Chỉ tiêu được ghi trong hình/contract | Phạm vi đo cần định nghĩa | Trạng thái hiện tại |
|---|---|---|
| Broker → Dashboard **≤2s** | Từ Backend/Broker nhận bản tin hợp lệ đến UI hiển thị; test đường truyền cụ thể | **Chưa có bằng chứng PASS** |
| AI phát sự kiện → mobile **≤5s** | Cần chốt mốc bắt đầu/kết thúc (AI output → nhà cung cấp push → mobile nhận) | **Chưa có bằng chứng PASS** |
| Giảm băng thông **≥90% /1 giờ** | So với baseline stream video nào, đo upload ở AI edge / server trong một giờ | **Thiếu baseline/phương pháp đo đã phê duyệt** |
| Dashboard hiện Offline **≤10s** | Từ lúc mất kết nối thực tế đến UI hiển thị; cấu hình heartbeat 2s/timeout 6s/checker 1s | **Cần test end-to-end** |
| ESP32 AUTO offline **≥24 giờ** | Thiết bị vẫn có nguồn; sensor/actuator hoạt động | **Chưa kiểm thử đủ 24h** |
| Buffer **≥17.280 mẫu/24h tại chu kỳ 5s** | Theo byte thực tế, flash/SD và tình huống reset | **Cần xác nhận storage** |

Các con số trong sơ đồ/NFR là **mục tiêu**, không suy ra hệ thống đã đạt.


## 8. Các tùy chọn kiến trúc chưa xác định

1. **[TBD]** Broker chọn Mosquitto hay EMQX; môi trường MQTTS/ACL của demo.
2. **[TBD]** Chọn PostgreSQL hay MySQL và migration tool/version.
3. **[TBD]** Có tích hợp MinIO/S3, Media Gateway, FCM trong triển khai không.
4. **[TBD]** Payload QoS Command/Config/Telemetry ACK, Timeout Telemetry ACK và Buffer đầy.
5. **[TBD]** Chính sách safe state/manual override cho từng actuator.
6. **[TBD]** Cơ chế xác thực AI Service và idempotency cho AI Event.
7. **[TBD]** Chỉ tiêu giảm băng thông ≥90% và AI→Mobile ≤5s đo như thế nào.
8. **[TBD]** Quy tắc phân quyền `RoleName` và `HouseRole` cuối cùng.

Xem `architecture-decisions.md` để biết các tùy chọn kỹ thuật và tác động đến kiến trúc.
