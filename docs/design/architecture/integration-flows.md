# SwiftletCare — Integration Flows

> **Phạm vi:** Mô tả các luồng tích hợp giữa thiết bị, MQTT, Spring Boot, AI, dữ liệu và Web.  
> **Quy tắc:** Phân biệt *luồng trên hình* với *cơ chế chi tiết đề xuất trong tài liệu giao tiếp*. Sơ đồ tổng thể không thay thế đặc tả MQTT, OpenAPI hoặc Sequence Diagram chi tiết.

## 0. Các nguyên tắc dùng chung

- Không chuyển Telemetry/Heartbeat của ESP32 thành API POST công khai cho React. **ESP32 ↔ Backend dùng MQTT**.
- **Spring Boot là trung tâm xử lý nghiệp vụ**, không phải MQTT Broker.
- **REST** để tải dữ liệu/thao tác; **WebSocket** cho cập nhật; **Database** là nguồn dữ liệu lâu dài; **MQTT** cho thiết bị; **AI internal REST** nhận kết quả AI.
- Các bước liên quan ACK/commit được chốt theo MQTT Contract; tên đầy đủ topic, payload và QoS chưa duyệt được định nghĩa trong đặc tả MQTT.
- Chuẩn định danh: MQTT `device_id` chuỗi, `event_id` chuỗi; Java `Device.id` là PK số, `Telemetry.eventId` là field Java; SQL `telemetry.event_id`.

## FLOW-01 — Sensor Telemetry → Spring Boot → Database → React

**Thành phần:** ESP32/Wokwi, MQTT Broker, Spring Boot, Database, React.  
**Mục tiêu:** Telemetry được lưu một lần và hiện trên Dashboard; ESP32 nhận ACK tầng ứng dụng sau khi DB đã lưu.

```text
1. ESP32 lấy mẫu, validate: temperature/humidity/co2/light, sensor_status.
2. Tạo event_id + timestamp và đưa Telemetry vào Buffer (theo chính sách đã chốt).
3. MQTT publish telemetry → Broker → Spring Boot MQTT Handler.
4. Backend validate schema, device đã đăng ký, topic/ownership.
5. Backend chống trùng (device_id, event_id) và lưu dữ liệu trong transaction.
6. Sau DB commit, Backend publish telemetry_ack(status=STORED, event_id).
7. ESP32 nhận ACK đúng event_id rồi xóa bản ghi đã xác nhận khỏi Buffer.
8. Backend phát sự kiện WebSocket và/hoặc REST cung cấp dữ liệu mới nhất/lịch sử cho React.
```

**Ngoại lệ cần có:** cảm biến lỗi biểu diễn `null` + `PARTIAL_ERROR`/`ERROR`, không thay số 0; DB lỗi không ACK `STORED`; ACK mất sau DB commit → ESP32 retry → Backend không INSERT trùng nhưng ACK lại cho bản trùng cùng nội dung; cùng ID khác nội dung phải từ chối an toàn.

**Điều kiện kiểm thử:** 1 Telemetry tạo 1 dòng DB, retry cùng ID không tạo dòng mới; `event_id` khớp từ MQTT sang Database.

**Tài liệu chi tiết:** Sensor/Telemetry Flow; MQTT Contract (`telemetry`, `telemetry_ack`); Backend Ingestion; OpenAPI Telemetry.

## FLOW-02 — React → Spring Boot → MQTT → Actuator → Command ACK

**Thành phần:** React, Spring Boot, MQTT Broker, ESP32/Relay.

```text
1. User thực hiện thao tác Web: SET_MODE(AUTO/MANUAL) hoặc SET_STATE(ON/OFF).
2. React gửi HTTP POST theo OpenAPI (Authorization bearer).
3. Backend xác thực JWT, kiểm tra HouseMembership/role và ownership của Actuator.
4. Service validate action, mode, thời hạn, tạo ControlCommand(PENDING) trong DB.
5. Backend publish MQTT command → Broker → ESP32.
6. ESP32 kiểm tra commandId, expiresAt, mode, safety policy; điều khiển GPIO hoặc REJECT.
7. ESP32 publish command_ack(commandId, result, reportedState).
8. Backend cập nhật ControlCommand, trạng thái được báo của Actuator; WebSocket đến Web.
9. React GET trạng thái cuối khi cần (đặc biệt sau reconnect).
```

**Quy tắc bắt buộc của thiết kế hiện tại:** `SET_STATE` không tự chuyển từ AUTO sang MANUAL; mode thuộc actuator; `HTTP 202` chỉ là **đã tiếp nhận yêu cầu**, không chứng minh relay đã đổi. `ACKNOWLEDGED` xác nhận kết quả xử lý theo hợp đồng nhưng không chứng minh tải vật lý thật vận hành.

**Thời hạn từ đặc tả Command/Backend:** ACK Command timeout 10s, command expiry 15s kể từ khi tạo; **không dùng các mốc này cho Telemetry ACK** (mốc ấy chưa chốt).

**Ngoại lệ:** không đủ quyền, sai Zone, offline, lệnh trùng, hết hạn, AUTO không cho SET_STATE, safety reject, không ACK. Trạng thái `SENT/ACKNOWLEDGED/REJECTED/FAILED/EXPIRED` phải được thống nhất. Nếu Backend publish thất bại sau khi lưu PENDING, xử lý queue/retry/error theo chính sách điều khiển đã thống nhất; không hiển thị thiết bị đã bật.

**Tài liệu chi tiết:** AUTO/MANUAL Flow; MQTT Command/ACK; Class/ERD `ControlCommand` và `Actuator`; OpenAPI Commands.

## FLOW-03 — Heartbeat → Online/Offline → Dashboard

**Thành phần:** ESP32, MQTT, Spring Boot, Database/Device, WebSocket/React. **Phải thể hiện hai tiến trình độc lập.**

**Luồng A — Nhận Heartbeat**:

```text
ESP32 timer mỗi 2s → MQTT heartbeat → Backend validate
→ cập nhật Device.lastSeenAt (thời điểm Backend nhận hợp lệ).
```

**Luồng B — Backend kiểm tra Offline**:

```text
Spring Boot @Scheduled mỗi 1s → đọc lastSeenAt
→ nếu now - lastSeenAt > 6s → Device.online=false
→ nếu có trạng thái thay đổi → phát WebSocket → React cập nhật UI.
```

Nếu device gửi lại Heartbeat hợp lệ và được chấp nhận, Backend có thể chuyển `online=true` và phát sự kiện thay đổi. MQTT **LWT availability** là tín hiệu phụ trợ theo chính sách, không thay thế Timeout Checker. Cần kiểm thử chống false-offline khi mạng chập chờn.

**Mục tiêu:** UI hiển thị Offline **≤10s** kể từ mất kết nối, **phải đo thời gian thực tế**. Chế độ `AUTO` cục bộ của ESP32 vẫn có thể hoạt động khi Backend nhìn thấy `Device.online=false`.

**Tài liệu chi tiết:** Heartbeat Flow, DeviceStatusScheduler, MQTT Availability/LWT, REST/WebSocket Contract.

## FLOW-04 — Camera → AI Edge → Java Backend → AI Data/Alert

**Thành phần:** Camera/video mô phỏng, LAN/RTSP, AI Service, Spring Boot, DB/kho ảnh đề xuất.

```text
Camera/nguồn video → frame qua LAN/RTSP → AI Edge Service
→ YOLOv8 + ByteTrack → CountLog / AnomalyEvent / snapshot
→ HTTPS REST nội bộ (service auth) → Spring Boot AI Controller/Service
→ validate, chống trùng theo key được duyệt → lưu CountLog/AnomalyEvent
→ nếu vượt quy tắc nghiệp vụ: tạo Alert / lưu tham chiếu snapshot.
```

**[TBD]** endpoint/schema chính thức xem OpenAPI, cơ chế chống trùng nguồn AI, nhận dạng loài, cadence count, lưu snapshot local hay MinIO/S3, token dịch vụ và retry. Không chuyển nhiệm vụ YOLO/ByteTrack sang ESP32 hoặc Backend Java.

**Tài liệu chi tiết:** AI Architecture, AI Event Processing, Class/ERD `CountLog`/`AnomalyEvent`, Internal AI REST.

## FLOW-05 — Alert → WebSocket / WEB, PUSH Notification

**Thành phần:** Spring Boot AlertService, DB, React, Notification Service (*) và thiết bị người dùng.

```text
Nguồn sự kiện môi trường / lỗi thiết bị / AI
→ Backend xác định mức cảnh báo và điều kiện chống tạo trùng
→ ghi Alert (NEW) vào DB → thông báo thay đổi qua WebSocket
→ nếu kênh được duyệt: tạo NotificationDelivery(PENDING)
→ Notification Service gửi WEB/PUSH (provider/FCM đề xuất)
→ cập nhật SENT/DELIVERED/FAILED khi có bằng chứng tương ứng.
```

Khi người dùng xử lý, `PATCH Alert` cập nhật trạng thái (`NEW`, `IN_PROGRESS`, `RESOLVED`) và lưu `AlertHistory` cùng giao dịch. Không đồng nhất `DELIVERED` với đã đọc.

**[TBD]** phương án push provider, TTL/dedup/retry cụ thể, mobile Push/FCM, cách auth subscription và điều kiện tạo cảnh báo. Nếu không có nhà cung cấp Push, vẫn có thể demo WEB + WebSocket theo phạm vi được duyệt, không tự tuyên bố đáp ứng Mobile Push.

**Tài liệu chi tiết:** Alert/Notification Architecture, Class/ERD `Alert`/`NotificationDelivery`, REST/Realtime.

## FLOW-06 — Mất Internet/Server/Broker → Local AUTO → Reconnect/Replay

**Thành phần:** ESP32, MQTT Broker, Spring Boot, Database.

```text
1. Mất Wi-Fi/MQTT/Backend: MQTT Manager báo ConnectionState=OFFLINE.
2. ESP32 tiếp tục đọc sensor và điều khiển AUTO tại chỗ nếu sensor/actuator còn an toàn.
3. Telemetry chưa có ACK từ ứng dụng vẫn được giữ trong Buffer; retry không chặn AUTO.
4. MQTT Manager reconnect theo backoff; khôi phục subscribe và publish.
5. ConnectionState=RECOVERING; retry các bản chưa ACK giữ nguyên event_id/timestamp.
6. Backend dedup và gửi ACK STORED sau khi đã có bản ghi hợp lệ/commit.
7. ESP32 xác nhận đúng event_id, xóa record, tiếp tục trạng thái bình thường.
```

**Không được suy diễn:** Không có mạng thì người dùng vẫn xem realtime Web hoặc gửi MANUAL qua Internet; thực tế các tính năng này bị gián đoạn cho tới khi kết nối phục hồi.

**[TBD]** buffer persistent medium, policy khi Buffer đầy, thang backoff tối đa, Telemetry ACK timeout, hồi phục cấu hình sau reset, ngưỡng nguy hiểm/safe state từng actuator. NFR AUTO 24h và 17.280 bản ghi chỉ PASS khi được đo.

**Tài liệu chi tiết:** IoT Offline Recovery, MQTT Reliability, Backend Ingestion/Recovery.

## FLOW-07 — Báo cáo / thống kê / tương quan môi trường – năng suất

**Thành phần:** React → Spring Boot REST → Database → React/download nếu được duyệt.

```text
React yêu cầu thống kê theo House/Zone/khoảng thời gian
→ REST + JWT + HouseRole check
→ Report/Statistics Service tổng hợp Telemetry, CountLog, HarvestLog
→ trả JSON thống kê/báo cáo cho React
→ nếu có chức năng export: tạo file + Report metadata/path.
```

Phân tích tương quan **không đồng nghĩa** quan hệ nhân quả. Nếu số mẫu không đủ, phải báo giới hạn kết luận. **[TBD]** công thức tương quan, đơn vị thời gian, cách gộp thiếu dữ liệu, định dạng file và bộ lọc quyền. Sơ đồ có tên “báo cáo” nhưng chưa đặc tả thuật toán.

**Tài liệu chi tiết:** Class/ERD `Report`/`HarvestLog`, REST Report Endpoints và quy tắc thống kê.

## FLOW-08 — Live View qua Media Gateway (*), chỉ khi duyệt

**Thành phần:** Camera/RTSP, Media Gateway đề xuất, React, Backend bảo vệ phiên.

```text
Camera / nguồn RTSP (LAN) → Media Gateway (*) chuyển sang HTTPS/HLS (hoặc giải pháp được duyệt)
→ người dùng yêu cầu Live View, Backend kiểm tra quyền House/Zone/camera
→ Browser phát luồng đã cấp quyền (theo session/token được chốt).
```

**[TBD]** Live View có nằm trong triển khai hay không, giải pháp chuyển mã/đóng gói, bảo mật URL, độ trễ, giới hạn quyền truy cập. **Không yêu cầu trình duyệt kết nối RTSP trực tiếp.**

**Tài liệu chi tiết:** AI/Video Architecture, UI Live View và sơ đồ tổng thể nét đứt.

## 9. Ma trận giao thức theo luồng

| Flow | Giao thức / Công nghệ | Dữ liệu chính | Điều kiện kỹ thuật |
|---|---|---|---|
| FLOW-01 Telemetry | MQTT + JPA/SQL + REST/WSS | `event_id`, số đo, trạng thái cảm biến | Lưu chống trùng và Application ACK sau commit |
| FLOW-02 Control | HTTPS REST + MQTT | `commandId`, `SET_MODE`/`SET_STATE`, Command ACK | Kiểm tra quyền, an toàn và thời hạn |
| FLOW-03 Heartbeat | MQTT + Scheduler + WSS | Heartbeat, `lastSeenAt`, `online` | Timeout độc lập với việc nhận Heartbeat |
| FLOW-04 AI | LAN/RTSP + HTTPS REST | CountLog, AnomalyEvent, snapshot metadata | Xác thực service và kiểm tra dữ liệu |
| FLOW-05 Alert/Push | SQL/JPA + WSS + Push (*) | Alert, AlertHistory, NotificationDelivery | Trạng thái gửi khác trạng thái xử lý |
| FLOW-06 Recovery | Local GPIO control + MQTT | Buffer, retry, `event_id` | AUTO không bị chặn bởi reconnect |
| FLOW-07 Report | HTTPS REST + SQL/JPA | Telemetry, CountLog, HarvestLog | Lọc theo quyền và thời gian |
| FLOW-08 Live View (*) | RTSP → Gateway → HTTPS/HLS | Video trực tiếp | Phân quyền truy cập stream |

## 10. Nguyên tắc xử lý lỗi xuyên luồng

- **Không bao giờ** phát `telemetry_ack(STORED)` khi Database chưa commit; bản trùng đã lưu hợp lệ là ngoại lệ có thể ACK lại.
- **Không bao giờ** tin rằng HTTP `202` hay MQTT PUBACK có nghĩa actuator đã thực thi.
- Mọi lệnh/alert/report phải kiểm tra **phạm vi House/Zone** theo RBAC đã duyệt.
- Reconnect WebSocket: tải snapshot REST, không giả định đã nhận lại mọi sự kiện trong thời gian mất kết nối.
- Error/status code cụ thể theo OpenAPI; MQTT topics/payload/QoS theo MQTT Contract; NFR test threshold thuộc NFR đã chốt.
