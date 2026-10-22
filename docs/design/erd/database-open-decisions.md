# SwiftletCare — Database Decisions & Open Issues (sau đối chiếu Class/ERD mới)

> BUILD1-04 implementation: [mapping Entity/database MVP](build104-mvp-mapping.md). Baseline 17 tables and open decisions below remain the full-system design; implemented keys/types/indexes are listed separately.

> Các quyết định ở đây phân biệt **đã chốt**, **lỗi cần sửa**, **chưa chốt**. Chỉ thay đổi trạng thái khi người phụ trách xác nhận và đã cập nhật sơ đồ `.drawio`.

## 1. Các quyết định đã chốt trong thiết kế IoT/đang có trên hình

| ID | Quyết định | Trạng thái | Tác động |
|---|---|---|---|
| DD-01 | MQTT Telemetry dùng `event_id`; Java dùng `eventId`; SQL dùng `event_id` | Đã chốt | UNIQUE theo device/event |
| DD-02 | Heartbeat 2s; timeout 6s; Backend Scheduler mỗi 1s | Đã chốt | `devices.last_seen_at`, `online`; không cần bảng heartbeat |
| DD-03 | AUTO/MANUAL là ControlMode; Online/Offline là kết nối | Đã chốt | `actuators.mode`; `devices.online` |
| DD-04 | `SET_MODE` và `SET_STATE` là hai loại command trong hợp đồng tích hợp hiện tại | Dùng cho thiết kế BUILD, P1/P4 phải cùng ký duyệt schema cuối | `control_commands.action`, targets |
| DD-05 | Không có lịch sử di chuyển ESP32 giữa Zone trong v1 | Theo quyết định hiện tại | `devices.zone_id`; không thêm DeviceZoneHistory |
| DD-06 | Một Zone tối đa một ZoneConfig hiện hành | Đã thể hiện trên ERD | UNIQUE(zone_id) |
| DD-07 | HouseRole OWNER/ADMIN/VIEWER, AggregationType HOURLY/DAILY | Đã có trên Class mới | Đồng bộ Java enums |
| DD-08 | Web/Push là 2 kênh thông báo trong mô hình hiện tại | Đã thể hiện | Không thêm SMS/Zalo |

## 2. Lỗi cụ thể nhìn thấy trên sơ đồ phải sửa (không phải câu hỏi nghiệp vụ mới)

| ID | Vấn đề | Cách sửa | Ai |
|---|---|---|---|
| ERD-FIX-01 | `control_commands.target_state` bị vẽ **hai lần** (BOOLEAN và BOOLEAN NULL) | Xóa dòng không nullable; chỉ giữ `target_state BOOLEAN NULL`; kiểm tra `action` và `target_mode` | P5 |
| ERD-FIX-02 | Telemetry chưa thể hiện UQ theo `(device_id,event_id)` | Bổ sung composite UNIQUE vào schema/ảnh ERD | P5 |
| UML-FIX-01 | `CommandAction` enum đã có nhưng `ControlCommand` chưa khai báo `action` | Bổ sung `action: CommandAction` vào Class | P3 |
| UML-FIX-02 | Hai ô `ZoneConfig` có danh sách trường khác nhau | Giữ một nguồn định nghĩa và một hình tham chiếu hoặc gộp thành một | P3 |
| DOC-FIX-01 | Bộ Markdown cũ ghi HouseRole/AggregationType chưa có; `ControlCommand.target_state` luôn NOT NULL | Đã chỉnh trong bộ Markdown mới | P5 |

## 3. Những quyết định thực sự còn OPEN

| ID | Nội dung | Phương án đề xuất cho BUILD | Người chốt |
|---|---|---|---|
| DB-OD-01 | Chọn MySQL hay PostgreSQL, cách sinh ID và timestamp | Chọn **một** hệ quản trị; dùng Flyway migration; Java JPA `Instant`/Enum String | P4 + P5 |
| DB-OD-02 | Command ACK timeout/command expiration trong hợp đồng MQTT cuối | Dùng chính các giá trị do P1–P4 ký duyệt (Task05 trước đó gợi ý 10s ACK, 15s expiry); không tự chốt khi chưa xác nhận | P1 + P4 |
| DB-OD-03 | Nullable `requested_by` với command tự động | v1 Web command có User; nếu cần lệnh System thì cho phép NULL kèm quy ước actor | P4 + P5 |
| DB-OD-04 | Cần trường `actor_type` trong `alert_histories`? | Để `changed_by=NULL` cho system nếu đủ; thêm actor_type chỉ khi audit yêu cầu | P3 + P4 + P5 |
| DB-OD-05 | Idempotency cho sự kiện AI | P2 cung cấp source event ID hoặc composite key; không dùng PK nội bộ để chống gửi lặp | P2 + P4 + P5 |
| DB-OD-06 | `detect_count` định nghĩa và uniqueness của CountLog kỳ tổng hợp | P2 định nghĩa; có thể unique theo `(device_id, aggregation_type, period_start, period_end)` nếu không có nhiều phiên bản xử lý | P2 + P5 |
| DB-OD-07 | Safe state của actuator / ngưỡng cấu hình chi tiết | P1 định nghĩa và xác minh với P4; DB chỉ lưu ngưỡng hợp lệ | P1 + P4 |
| DB-OD-08 | Alert cấp House/Device không thuộc Zone | ERD hiện yêu cầu `zone_id`; chỉ thêm `house_id`/`device_id` nếu FR cần | P3 + P4 + P5 |
| DB-OD-09 | Lưu lịch sử cấu hình Zone qua nhiều phiên bản | Hiện giữ `UNIQUE(zone_id)`; `version` là số revision của bản hiện tại, không phải nhiều row | P1 + P5 |
| DB-OD-10 | Thêm index nâng cao/partition/retention cho Telemetry | Kiểm thử trong BUILD rồi quyết định | P4 + P5 |
| DB-OD-11 | `Actuator.on` là GPIO report hay desired state | Đề xuất hiểu là trạng thái GPIO do ESP32 báo; không tuyên bố thiết bị vật lý đã chạy | P1 + P3 + P4 |
| DB-OD-12 | NFR Offline/Buffer lớn đã đạt hay chưa | Chưa có bằng chứng; giữ `Not Tested`/Deferred theo kế hoạch test | P1 + P4 |

## 4. Review & sign-off

| Vai trò | Phạm vi review | Trạng thái |
|---|---|---|
| P1 | IoT, Telemetry, Command, ZoneConfig, Heartbeat | Chờ xác nhận |
| P2 | AI CountLog/AnomalyEvent, idempotency | Chờ xác nhận |
| P3 | Class Domain, multiplicity, Enum, Alert | Chờ xác nhận |
| P4 | Spring Boot/JPA, transaction, DTO, Database migration | Chờ xác nhận |
| P5 | ERD, PK/FK/UQ/CHECK, dictionary, index | Chờ xác nhận |


