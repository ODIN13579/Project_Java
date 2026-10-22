# SwiftletCare — Architecture Decisions

> **Loại tài liệu:** Tập hợp quyết định và tùy chọn kỹ thuật của kiến trúc tổng thể.  
> **Phiên bản:** 1.0 — 18/10/2026  
> **Nguồn đối chiếu:** sơ đồ tổng thể, cấu trúc IoT, Backend, Class Diagram, ERD, MQTT Contract, OpenAPI.  
> Tài liệu không chứng nhận sản phẩm đã triển khai hoặc đã đạt NFR.

## 1. Quy ước

**Ghi chú triển khai BUILD1-04:** PostgreSQL 17/Flyway V1–V2, Mosquitto 2, JWT access token và plain WebSocket đã được chọn cho MVP. Đây là trạng thái implementation, không tự đổi trạng thái sign-off thiết kế tổng thể trong bảng dưới. Mapping cụ thể: [Entity/database MVP](../erd/build104-mvp-mapping.md); phạm vi giao tiếp: [contract MVP](../../api/build104-mvp.md).

- `CONFIRMED`: quyết định thiết kế đã được xác nhận trong kiến trúc/hợp đồng giao tiếp hiện có.
- `PROPOSED`: công nghệ hoặc cách tổ chức được thể hiện dưới dạng phương án, đặc biệt khi có dấu `(*)`.
- `OPEN`: thông số hoặc phạm vi chưa được quy định đủ.
- `TEST PENDING`: có mục tiêu kỹ thuật, chưa có phép đo đủ để kết luận.

## 2. Các quyết định kiến trúc

| ID | Chủ đề | Quyết định hoặc phương án | Trạng thái | Tác động kỹ thuật |
|---|---|---|---|---|
| ADR-01 | Backend | Java Spring Boot theo kiểu Modular Monolith (*) | PROPOSED | Dùng module nghiệp vụ chung tiến trình, REST và MQTT Client |
| ADR-02 | Kết nối IoT | ESP32 ↔ MQTT Broker ↔ Spring Boot; không dùng Web POST để nạp Telemetry | CONFIRMED | Backend subscribe và xử lý dữ liệu thiết bị |
| ADR-03 | MQTT Broker | Mosquitto (*) hoặc broker MQTT tương thích | PROPOSED | Cần cấu hình Topic/ACL/LWT, client và môi trường mạng |
| ADR-04 | Giao tiếp Web | React ↔ Spring Boot qua REST + WebSocket | CONFIRMED | REST truy vấn/ghi lệnh; WSS cập nhật trạng thái |
| ADR-05 | Database | PostgreSQL (*) trên sơ đồ; chưa loại trừ MySQL | PROPOSED | Kiểu SQL, migration, JDBC/JPA và index phụ thuộc DB |
| ADR-06 | AI | Service riêng dùng YOLOv8 + ByteTrack, nhận video qua LAN/RTSP | CONFIRMED | AI không chạy trên ESP32 |
| ADR-07 | AI Integration | AI Service gửi CountLog/AnomalyEvent qua REST nội bộ | CONFIRMED | Backend xác thực service, validate và lưu sự kiện |
| ADR-08 | Lưu ảnh bằng chứng | MinIO/S3 (*) và metadata/path trong Database | PROPOSED | Phân quyền, đường dẫn ảnh và thời hạn lưu cần định nghĩa |
| ADR-09 | Live View | Media Gateway (*) cung cấp stream phù hợp với Browser | PROPOSED | Cần HTTPS/HLS hoặc giao thức browser hỗ trợ |
| ADR-10 | Notification | Web Push/Mobile Push; FCM (*) là một provider đề xuất | PROPOSED | WebSocket không thay thế Mobile Push |
| ADR-11 | AUTO offline | ESP32 điều khiển AUTO cục bộ khi mất mạng nếu cấu hình/phần cứng an toàn | CONFIRMED | Không phụ thuộc Spring Boot để thực thi ngưỡng cơ bản |
| ADR-12 | Telemetry ID | MQTT `event_id` ↔ Java `eventId` ↔ SQL `event_id` | CONFIRMED | Unique theo `(device_id, event_id)` để chống trùng |
| ADR-13 | Reliability | Lưu Buffer trước Publish, giữ tới Application ACK `STORED` sau DB commit | CONFIRMED | Retry có thể tạo bản nhận trùng; Backend xử lý idempotent |
| ADR-14 | Heartbeat | Gửi 2s, timeout 6s, kiểm tra định kỳ 1s | CONFIRMED; NFR TEST PENDING | Backend cập nhật `lastSeenAt`, `online` và WebSocket |
| ADR-15 | Điều khiển | `SET_MODE` và `SET_STATE`; `AUTO`/`MANUAL` theo Actuator | CONFIRMED thiết kế cơ bản | Lệnh `SET_STATE` khi AUTO không tự đổi chế độ |
| ADR-16 | Realtime | REST snapshot kết hợp WebSocket events | CONFIRMED | Khi reconnect UI lấy lại snapshot đáng tin cậy |
| ADR-17 | Bảo mật | JWT/RBAC House/Zone ở Backend; MQTT credentials + ACL tại Broker | CONFIRMED nguyên tắc | Chính sách phân quyền và cấu hình credential cần chi tiết hóa |
| ADR-18 | Lấy mẫu IoT | Telemetry mặc định 5s, cấu hình 1–60s | CONFIRMED target | Dung lượng Buffer thực tế và hiệu năng cần thử nghiệm |
| ADR-19 | Mô hình dữ liệu | Baseline Class Diagram/ERD gồm 17 Entity/bảng | CONFIRMED baseline | Không tự sinh thêm bảng Heartbeat/ACK khi chưa có quyết định |
| ADR-20 | Thống kê báo cáo | Tổng hợp Telemetry, CountLog, HarvestLog, lưu metadata Report khi cần | CONFIRMED phạm vi tổng quát | Phép tương quan môi trường–năng suất cần mô tả riêng |

## 3. Các tùy chọn còn mở

| ID | Vấn đề kỹ thuật chưa xác định | Hệ quả nếu chưa chốt |
|---|---|---|
| OPEN-01 | Chọn Mosquitto/EMQX và vị trí Broker (local/cloud) | Chưa thể cố định kết nối và cấu hình TLS/ACL |
| OPEN-02 | Chọn PostgreSQL/MySQL, version và công cụ migration | SQL migration và kiểu dữ liệu có thể khác nhau |
| OPEN-03 | Có triển khai MinIO/S3 và chính sách đường dẫn ảnh hay không | Truy xuất snapshot phải được bảo vệ riêng |
| OPEN-04 | Có dùng Media Gateway/Live View hay không | Nhánh video nét đứt chưa phải luồng bắt buộc |
| OPEN-05 | Nhà cung cấp Web/Mobile Push và cơ chế FCM | Không khẳng định đạt AI → mobile ≤5s khi chưa chọn provider |
| OPEN-06 | QoS/Retain/TLS cuối cùng cho Command/Config/Telemetry ACK | Ảnh hưởng retry, subscribe và tính an toàn giao thức |
| OPEN-07 | Telemetry ACK timeout, backoff, Buffer full, bộ nhớ lưu bền vững | Chưa thể khẳng định khả năng lưu bù 24 giờ |
| OPEN-08 | Manual timeout, safe state từng actuator, relay active level | Quyết định điều khiển khi lỗi cần rõ ràng |
| OPEN-09 | Xác thực AI Service và khóa chống trùng AI Event | Ảnh hưởng kiểm soát nguồn và tính đúng dữ liệu AI |
| OPEN-10 | Baseline, phép đo cho giảm băng thông ≥90%/giờ | Chưa thể kết luận KPI giảm lưu lượng |
| OPEN-11 | Mốc đo Broker→Dashboard ≤2s và AI→Mobile ≤5s | Cần xác định timestamps, sample size và đường đo |
| OPEN-12 | Phạm vi phân quyền RoleName/HouseRole; API authorization | Ảnh hưởng kiểm soát House/Zone/Device |
| OPEN-13 | Schema `ControlCommand` và nullable của targetState/targetMode | Cần thống nhất Class Diagram ↔ ERD ↔ OpenAPI/MQTT |

## 4. Ràng buộc tích hợp không được vi phạm

1. **Application ACK** được phát sau khi bản Telemetry đã commit thành công hoặc đã được lưu trước đó với đúng nội dung; MQTT PUBACK không có ý nghĩa tương đương.
2. `event_id` và timestamp phải giữ nguyên khi ESP32 retry; Backend chống trùng theo thiết bị và ID.
3. Backend chỉ báo lệnh được tiếp nhận khi HTTP `202`; kết quả thực thi cập nhật từ Command ACK và trạng thái được báo.
4. `AUTO/MANUAL` không được gộp với `ONLINE/OFFLINE/RECOVERING` thành một enum điều khiển.
5. Luồng nhận Heartbeat và tác vụ kiểm tra Offline phải độc lập.
6. AI chạy trong AI Service riêng; dữ liệu từ Camera không buộc phải đi qua ESP32.
7. WebSocket và Web/Mobile Push là hai cơ chế riêng; mất mạng thì không còn realtime liên tục.
8. Các mục tiêu NFR được ghi là mục tiêu cần kiểm chứng, không được xem là đã đạt chỉ vì kiến trúc có thể hỗ trợ chúng.

## 5. Mục tiêu NFR gắn với kiến trúc

| Mục tiêu | Phạm vi kiến trúc | Tình trạng kiểm chứng |
|---|---|---|
| Broker → Dashboard ≤2 giây | MQTT → Spring Boot → WSS/React | TEST PENDING |
| AI event → Mobile ≤5 giây | AI → Backend → Push Provider → Mobile | TEST PENDING |
| Giảm băng thông ≥90% / 1 giờ | Camera/AI edge và truyền dữ liệu đã rút gọn | TEST PENDING; baseline OPEN |
| Thiết bị hiển thị Offline ≤10 giây | Heartbeat 2s / timeout 6s / Scheduler 1s / WSS | TEST PENDING |
| ESP32 AUTO offline ≥24 giờ | Local control, cảm biến, actuator, nguồn | TEST PENDING |
| Buffer ≥17.280 records/24h tại 5s | Local storage, replay và ACK | TEST PENDING |
