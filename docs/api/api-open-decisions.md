# SwiftletCare — DSN-09 Open Decisions / Design Gaps

**Phân loại:** `CONFIRMED` = quyết định đã được người dùng/thiết kế gần nhất chốt; `PROPOSED` = nhóm cần duyệt; `BLOCKER` = cần chốt trước khi hai bên lập trình tích hợp.

| ID | Nội dung | Tình trạng | Hướng hiện tại | Chủ trì / Review |
|---|---|---|---|---|
| API-D01 | MQTT Telemetry tên `event_id`, Java `eventId` | CONFIRMED | REST DTO `eventId`, DB `event_id` | P1/P4 |
| API-D02 | Heartbeat 2s, timeout 6s, Scheduler 1s | CONFIRMED | `Device.online`, `lastSeenAt`; không public Heartbeat POST | P1/P4 |
| API-D03 | AUTO/MANUAL theo `Actuator` | CONFIRMED trong bản tích hợp hiện tại | REST Command nhắm Actuator | P1/P4 |
| API-D04 | `SET_MODE`/`SET_STATE` + XOR target | PROPOSED cần sign-off schema | API command body dùng action, targetMode/targetState | P1/P4 |
| API-D05 | P3 Class Diagram còn thiếu `action: CommandAction` | BLOCKER sơ đồ | Sửa UML và xác nhận trước JPA/DTO | P3/P5 |
| API-D06 | ERD có duplicate `target_state` khi vẽ | BLOCKER sơ đồ | Giữ chỉ `target_state NULL` + `target_mode NULL` + `action` | P5 |
| API-D07 | Chọn MySQL hay PostgreSQL, chiến lược migration | PROPOSED | Flyway/JPA, enum string; chọn 1 DB | P4/P5 |
| API-D08 | Mức quyền House ADMIN và quyền global User | BLOCKER RBAC | Security matrix đang là đề xuất | P3/P4 |
| API-D09 | POST House với UML House 1..* Zone | PROPOSED | `initialZone` tạo cùng transaction hoặc đổi multiplicity | P3/P4/P5 |
| API-D10 | JWT refresh/logout và DTO | PROPOSED theo DSN-05 | Chốt endpoint/refresh token storage | P4 |
| API-D11 | Trang list `page=0,size=20,max=100` | PROPOSED | Cùng wrapper `{items,page}` cho toàn bộ | P4/P5 |
| API-D12 | QoS Command/ACK, Telemetry ACK timeout | BLOCKER MQTT liên nhóm | Không thay cấu hình P1 trước khi P4 ký duyệt | P1/P4 |
| API-D13 | MQTT config payload và biết ESP32 áp dụng xong chưa | BLOCKER config sync | PUT response nói DB saved, không khẳng định applied | P1/P4 |
| API-D14 | Command khi ESP32 offline và HTTP 202/broker lỗi | BLOCKER control | 409 DEVICE_OFFLINE hoặc 503 BROKER_UNAVAILABLE; outbox TBD | P1/P4 |
| API-D15 | `AlertHistory.actor_type` có cần không | PROPOSED | Trong v1 dùng `changedBy=null` cho hệ thống, không tự thêm enum | P3/P4/P5 |
| API-D16 | Idempotency của AI Service (CountLog/AnomalyEvent) | BLOCKER khi AI retry | P2 cung cấp ID nguồn hoặc unique key; hiện ERD chưa có | P2/P4/P5 |
| API-D17 | API report correlation: thuật toán, time buckets, sample minimum | BLOCKER riêng endpoint | Giữ trong contract đề xuất; triển khai sau phê duyệt | P3/P4/P5 |
| API-D18 | REST path `{deviceId}` số vs `Device.deviceId` mã chuỗi | PROPOSED trong API, cần đồng bộ UI | Sử dụng số nội bộ path, MQTT dùng chuỗi | P4/P5 |
| API-D19 | AI internal credential (X-Service-Key) | PROPOSED | Prototype service key bằng env, không public | P2/P4 |
| API-D20 | File report tạo đồng bộ/async | PROPOSED | 201 synchronous; nếu async phải thêm lifecycle Job | P4/P5 |
| API-D21 | Error code dictionary/validation 400 vs 422 | PROPOSED | Dùng 400 với lỗi input, 409 state conflict | P3/P4 |
| API-D22 | URL/filePath snapshot/report ACL | PROPOSED | Không trả object-storage secret; signed URL theo quyền nếu cần | P2/P4 |
| API-D23 | Mã FR ID chi tiết từ danh sách FR v1.0 | BLOCKER traceability cuối | Điền vào `fr-api-mapping.md` theo FR thật, không bịa ID | P3 |
| API-D24 | WebSocket endpoint, protocol, event naming | PROPOSED | Có tài liệu riêng, P4/P5 ký duyệt | P4/P5 |