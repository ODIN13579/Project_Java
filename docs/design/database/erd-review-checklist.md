# SwiftletCare — ERD Review Checklist (DSN-07)

> **Status:** Chưa đánh dấu các mục đã PASS nếu chưa xác nhận trên file `.drawio`, migration hoặc test thực tế.  
> **Người tổng hợp:** P5; review chung: P3, P4, P1 và P2.

## A. Sửa sơ đồ ERD ngay trước BUILD

- [ ] **ERD-01 / HIGH**: Xóa dòng `target_state BOOLEAN` trùng trong ControlCommand; chỉ giữ `target_state BOOLEAN NULL`.
- [ ] **ERD-02 / HIGH**: ControlCommand có `action: CommandAction`, `target_mode: ControlMode NULL`, `command_id` duy nhất.
- [ ] **ERD-03 / HIGH**: Thêm `UNIQUE(telemetry.device_id, telemetry.event_id)` và `event_id NOT NULL` trong migration/ERD.
- [ ] **ERD-04 / HIGH**: Các phép đo `temperature/humidity/co2/light` có thể `NULL` khi sensor lỗi; `sensor_status` hợp lệ.
- [ ] **ERD-05 / HIGH**: FKs `telemetry.device_id` BIGINT nội bộ, MQTT `device_id` String ánh xạ qua `devices.device_id`.
- [ ] **ERD-06**: ZoneConfig chỉ một bảng, `zone_id` UNIQUE.
- [ ] **ERD-07**: AlertHistory `changed_by` có thể NULL cho hành động do System.
- [ ] **ERD-08**: Timestamp/nullable `ack_at`, `sent_at`, `manual_until`, `handled_at`, `delivered_at`, `file_path` đã được P4/P5 xác minh.

## B. Đối chiếu Class Diagram mới của P3

- [ ] **CLS-01**: P3 thêm `ControlCommand.action: CommandAction` vào UML (enum đã có).
- [ ] **CLS-02**: P3 xóa/đánh dấu ô `ZoneConfig` rút gọn là tham chiếu; không xem hai ZoneConfig thành hai Entity.
- [ ] **CLS-03**: `HouseRole` OWNER/ADMIN/VIEWER và `AggregationType` HOURLY/DAILY đã thống nhất.
- [ ] **CLS-04**: `ControlCommand.targetState` và `targetMode` là hai đích tùy chọn theo `action`.
- [ ] **CLS-05**: `User.passwordHash` ↔ `users.password_hash` thống nhất.
- [ ] **CLS-06**: 17 lớp ↔ 17 bảng theo `database-schema-overview.md`.

## C. Đồng bộ P1/P4 (IoT/MQTT/Java)

- [ ] **IOT-01**: MQTT JSON dùng **`event_id`**, Java `eventId`, DB `event_id`.
- [ ] **IOT-02**: Backend chỉ trả `telemetry_ack(STORED)` sau Database commit; duplicate cùng nội dung vẫn ACK, khác nội dung từ chối.
- [ ] **IOT-03**: Device Heartbeat 2s; Backend timeout 6s; `@Scheduled` 1s, tách khỏi handler nhận Heartbeat.
- [ ] **IOT-04**: `Actuator.mode` phân biệt AUTO/MANUAL; mất mạng không tự chuyển mode.
- [ ] **IOT-05**: SET_STATE chỉ hợp lệ khi Actuator đang MANUAL, sau kiểm tra an toàn.
- [ ] **IOT-06**: ControlCommand theo dõi ACK, expiry, commandId; không đồng nhất SENT với relay đã đổi trạng thái.
- [ ] **IOT-07**: Các NFR chưa test không tự ghi đạt; Buffer Offline theo thiết kế P1.

## D. AI, cảnh báo và báo cáo

- [ ] **AI-01**: P2 định nghĩa `detect_count`, `entry_count`, `exit_count` và kỳ tổng hợp.
- [ ] **AI-02**: P2/P4 chốt định danh nguồn AI để không ghi lặp AnomalyEvent.
- [ ] **ALERT-01**: `alerts.anomaly_event_id` có thể NULL, cảnh báo môi trường không cần nguồn AI.
- [ ] **ALERT-02**: `Alert.status` và `AlertHistory.toStatus` dùng `NEW/IN_PROGRESS/RESOLVED`.
- [ ] **NOTI-01**: `NotificationChannel` chỉ WEB/PUSH; DeliveryStatus đúng ngữ nghĩa.
- [ ] **REP-01**: Report có phạm vi thời gian hợp lệ, HarvestLog yield ≥0.

## E. Bằng chứng bàn giao

- [ ] P5 nộp file `database-erd.drawio` có thể chỉnh sửa và ảnh `database-erd.png` cập nhật.
- [ ] P5 nộp `README.md`, overview, data dictionary, constraints, index strategy, open decisions và checklist.
- [ ] P3 review tương thích UML và ghi kết quả.
- [ ] P4 review khả năng triển khai JPA/DDL và thống nhất MySQL/PostgreSQL.
- [ ] P1 review Telemetry, command, Heartbeat; P2 review AI.
- [ ] Có commit/PR và link Jira DSN-07.

## F. Kiểm thử tối thiểu trong BUILD (không phải tiêu chí phải PASS ở DSN thiết kế)

| Test ID | Test case | Kết quả mong đợi | Trạng thái |
|---|---|---|---|
| DB-T01 | 2 Telemetry trùng `(device_id,event_id)` | Chỉ lưu 1 row, trả ACK phù hợp | Not Tested |
| DB-T02 | Duplicate cùng ID nhưng dữ liệu khác | Conflict, không ghi đè | Not Tested |
| DB-T03 | CO₂ lỗi `co2=null`, PARTIAL_ERROR | Lưu hợp lệ, không coi null là 0 | Not Tested |
| DB-T04 | SET_MODE/MANUAL có `target_state` khác NULL | Reject bằng CHECK hoặc Service | Not Tested |
| DB-T05 | SET_STATE mà thiếu `target_state` | Reject | Not Tested |
| DB-T06 | Heartbeat 2s rồi ngắt thiết bị | Backend Offline theo 6s timeout, UI mục tiêu ≤10s | Not Tested |
| DB-T07 | Cảnh báo đổi NEW → IN_PROGRESS | `Alert` và `AlertHistory` được cập nhật atomic | Not Tested |
