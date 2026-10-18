# SwiftletCare — Database Constraints & Data Integrity

> **Trạng thái:** Quy tắc áp dụng cho ERD 17 bảng phiên bản mới. Những ràng buộc không thể hiện trên ảnh được đánh dấu **đề xuất cho migration**, không tự xem là đã tồn tại trong Database.

## 1. Khóa chính, khóa ngoại

- Cả 17 bảng có `id BIGINT` là PK; cách sinh ID thuộc quyết định chọn DB/ORM.
- Các FK trong `database-data-dictionary.md` phải tồn tại thật ở Database, không chỉ là đường nối hình vẽ.
- Không cascade xóa Telemetry, CountLog, AnomalyEvent, Alert, History, Delivery, Command, HarvestLog và Report khi vô hiệu hóa House/Zone/Device/User; ưu tiên `RESTRICT` và soft-delete/`active=false`.
- FK nullable chỉ khi được nghiệp vụ cho phép: `alerts.anomaly_event_id`, `alert_histories.changed_by` (SYSTEM), thời gian chưa phát sinh.

## 2. Unique và key chống trùng

| Constraint | Trạng thái | Lý do |
|---|---|---|
| `UNIQUE(users.email)` | Đề xuất triển khai | Email là định danh đăng nhập |
| `UNIQUE(devices.device_id)` | Có ký hiệu UQ trên ERD | Mã thiết bị MQTT duy nhất |
| `UNIQUE(house_memberships.user_id, house_memberships.house_id)` | Đề xuất theo mô hình một membership/cặp | Tránh duplicate quyền |
| `UNIQUE(zone_configs.zone_id)` | **Có ký hiệu UQ trên ERD** | Mỗi Zone tối đa một cấu hình hiện hành |
| `UNIQUE(telemetry.device_id, telemetry.event_id)` | **CẦN SỬA ERD + MIGRATION** | Không ghi trùng khi MQTT retry |
| `UNIQUE(control_commands.command_id)` | **CẦN GHI UQ RÕ TRÊN ERD** | Không thực thi cùng lệnh hai lần |
| `UNIQUE(anomaly_events.source_event_id)` | **Chưa có field — OPEN** | Chống trùng AI cần quyết định với P2; không tự thêm |

**Đặc biệt:** `telemetry.event_id NOT NULL`; cặp `(device_id, event_id)` phải được Database ràng buộc để tránh race condition khi hai bản tin giống nhau đến đồng thời.

## 3. CHECK / validation gợi ý cho BUILD

| Đối tượng | Quy tắc | Ghi chú |
|---|---|---|
| `control_commands` | `(action='SET_STATE' AND target_state IS NOT NULL AND target_mode IS NULL) OR (action='SET_MODE' AND target_mode IS NOT NULL AND target_state IS NULL)` | Bắt buộc sau khi sửa ERD `target_state` trùng |
| `control_commands` | `expires_at > requested_at` | Các thời điểm gửi/ACK có thể nullable |
| `zone_configs` | `humidity_on < humidity_off`, `temp_min < temp_max`, `co2_min <= co2_max`, `light_min <= light_max` | Chỉ áp dụng khi hai đầu ngưỡng đều có giá trị |
| `zone_configs` | `telemetry_interval_sec BETWEEN 1 AND 60`, `heartbeat_interval_sec > 0`, `version >= 1` | Telemetry theo NFR; Heartbeat 2s đã chốt |
| `count_logs` | `period_start < period_end`, counts ≥0 | `detect_count` vẫn cần P2 xác nhận nghĩa |
| `anomaly_events` | `confidence BETWEEN 0 AND 1` | Khi confidence đã chuẩn hóa |
| `harvest_logs` | `yield_kg >= 0` | Sản lượng hợp lệ |
| `reports` | `range_from < range_to` | Kỳ báo cáo không rỗng |
| `notification_deliveries` | Enum hợp lệ và thời điểm gửi/trạng thái nhất quán | `DELIVERED` không có nghĩa đã đọc |

**Ví dụ kiểm tra logic `control_commands` (SQL minh họa, không phải migration đa DB):**

```sql
CHECK (
    (action = 'SET_STATE' AND target_state IS NOT NULL AND target_mode IS NULL)
 OR (action = 'SET_MODE' AND target_mode IS NOT NULL AND target_state IS NULL)
)
```

Cần kiểm tra phiên bản MySQL hoặc PostgreSQL cụ thể trước khi dùng CHECK trong DDL.

## 4. Quy tắc toàn vẹn nghiệp vụ do Spring Boot bảo đảm

1. **Telemetry:** Parse/validate payload từ ESP32; tìm `devices.id` bằng `devices.device_id` từ MQTT. Kiểm tra nội dung trùng đúng cặp `(device_id,event_id)`; cùng ID khác nội dung là conflict.
2. **Telemetry ACK:** Chỉ publish `telemetry_ack(STORED, event_id)` sau khi transaction đã commit thành công; nếu Database lỗi, không ACK lưu thành công. ACK từ MQTT Broker không thay ACK này.
3. **Thiết bị Offline:** nhận Heartbeat cập nhật `last_seen_at`; job riêng chạy mỗi 1s kiểm tra quá 6s, chỉ WebSocket khi trạng thái đổi; retry/recovery của ESP32 không tạo bản đo trùng.
4. **Command:** Backend kiểm tra quyền trên House/Zone, mode, thời hạn trước khi gửi. ESP32 chống command trùng theo `commandId`. `SENT` không chứng minh actuator vật lý đã hoạt động.
5. **Alert:** cập nhật `alerts.status` và thêm `alert_histories` cùng một transaction. SYSTEM có thể dùng `changed_by=NULL` nếu P4/P5 đồng ý; thêm `actor_type` chỉ sau thay đổi mô hình được duyệt.
6. **AI:** không đếm trùng `CountLog`/`AnomalyEvent` khi AI Service gửi lại; phải xác định source event key với P2 trước khi tích hợp.
7. **RBAC:** `users.role` (toàn cục) và `house_memberships.role` (trong nhà yến) được kiểm tra độc lập, không dùng vai trò nhà thay cho toàn hệ thống.

## 5. Các chỉnh sửa cần P5 áp dụng trên Draw.io

- Xóa **dòng `target_state BOOLEAN` không có NULL** nằm phía trên `requested_at` trong bảng ControlCommand; chỉ giữ một `target_state BOOLEAN NULL` cạnh `target_mode`.
- Ghi rõ `action: CommandAction` **NOT NULL** và `target_mode: ControlMode NULL`.
- Gắn `UQ(device_id,event_id)` cho Telemetry, `UQ(command_id)` cho ControlCommand.
- Nếu chọn `changed_by NULL` cho SYSTEM thì ghi rõ `NULL` trên ERD; không tự vẽ thêm `actor_type` khi UML chưa có.
- Chưa cần bảng `TelemetryAck`/`Heartbeat`/`DeviceZoneHistory` cho v1.

## 6. Những kiểm thử tối thiểu trước merge

- 2 Telemetry cùng device/event_id, nội dung giống nhau → 1 row DB, vẫn trả ACK lưu.
- 2 Telemetry cùng ID nhưng giá trị khác → reject conflict.
- `sensor_status=PARTIAL_ERROR` với `co2=NULL` → lưu thành công; AUTO không dùng CO₂ lỗi.
- Insert ControlCommand SET_MODE có target_state khác NULL → reject.
- Insert ControlCommand SET_STATE không có target_state → reject.
- Heartbeat 2s và mất heartbeat liên tục → Backend cập nhật OFFLINE sau timeout 6s, UI mục tiêu ≤10s (cần đo).
- User không có HouseMembership → bị từ chối điều khiển thiết bị thuộc House đó.


