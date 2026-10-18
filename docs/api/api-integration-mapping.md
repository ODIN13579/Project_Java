# SwiftletCare — Integration Mapping: REST / Domain / MQTT / AI / React

## 1. Nguồn sự thật theo chiều giao tiếp

| Tình huống | Web/REST | Service Java | Bảng/Entity | MQTT/AI/Realtime | Ai xác nhận |
|---|---|---|---|---|---|
| Xem dữ liệu cảm biến | GET latest/history | TelemetryService | `Telemetry` + `Device` | ESP32 `telemetry` → Java MQTT Handler; `telemetry_ack` sau commit | P1/P4 |
| Gửi lệnh actuator | POST command, GET status | ControlService | `ControlCommand`, `Actuator` | Backend `command` → ESP32 `command_ack` | P1/P4 |
| Điều chỉnh AUTO/MANUAL | POST command SET_MODE | ControlService | `ControlCommand.action/targetMode`; `Actuator.mode` | ESP32 áp dụng/ACK | P1/P4 |
| Đổi ngưỡng Zone | PUT config | ZoneConfigService | `ZoneConfig` | MQTT `configuration` (**schema chưa chốt**) | P1/P4 |
| Hiển thị Online/Offline | GET device / Dashboard | DeviceStatusScheduler | `Device.lastSeenAt`, `Device.online` | Heartbeat 2s, timeout 6s, scheduler 1s, WebSocket change | P1/P4/P5 |
| Đếm chim IN/OUT | GET count logs / statistics | AiEvent/BirdStatsService | `CountLog` | P2 POST `/internal/ai/count-logs` | P2/P4 |
| Phát hiện bất thường | GET anomalies/alerts | AiEvent/AlertService | `AnomalyEvent`, `Alert` | P2 POST `/internal/ai/anomalies`; WebSocket alert | P2/P4 |
| Tiếp nhận cảnh báo | PATCH alert status | AlertService | `Alert`, `AlertHistory` (one transaction) | WebSocket after commit | P3/P4/P5 |
| Thông báo | GET notifications | NotificationService | `NotificationDelivery` | WEB/PUSH statuses; WS/Push not necessarily read receipts | P4/P5 |
| Tổng quan | GET dashboard summary | Aggregation services | House/Zone/Device/Alert | WebSocket updates + REST recovery | P5/P4 |
| Tương quan | GET productivity-correlation | ReportService | Telemetry/HarvestLog | Statistic pipeline algorithm TBD | P3/P4/P5 |

## 2. Định danh quan trọng

- `devices.id BIGINT` → REST `{deviceId}`; `devices.device_id VARCHAR` → MQTT `{deviceId}` và payload `device_id`. Spring Boot resolves mapping.
- `telemetry.event_id VARCHAR` ↔ Java `Telemetry.eventId` ↔ MQTT `event_id`; REST `eventId` theo camelCase DTO. UNIQUE `(device_id,event_id)` DB, retry không tạo ID mới.
- `control_commands.command_id VARCHAR` ↔ REST `{commandId}` ↔ MQTT `commandId`; không đồng nhất PK `ControlCommand.id`.
- `control_commands.target_state BOOLEAN NULL`, `target_mode ControlMode NULL`, `action CommandAction` theo ERD đã cập nhật; P3 cần bổ sung `action` vào lớp Class Diagram.
- `zone_configs.zone_id` unique hiện hành; `version` là revision, không lưu nhiều version row trong v1.

## 3. Chuyển dữ liệu MQTT về DTO

```text
MQTT JSON (ESP32): { event_id, device_id, timestamp, temperature,... }
    └─ MqttTelemetryDTO.eventId via @JsonProperty("event_id")
       └─ TelemetryService.ingest(dto)
          └─ resolve Device by devices.device_id
          └─ transaction insert if not duplicate
          └─ after commit publish telemetry_ack(STORED, event_id)
             └─ REST DTO: { eventId, deviceId: devices.id, timestamp, ... }
```

Mất ACK sau commit: không xóa bản ghi; ESP32 gửi lại cùng event_id → Java phát ACK lần nữa, không insert trùng. Backend không đưa Endpoint POST Telemetry công khai cho React.

## 4. Lệnh và HTTP 202

```text
React POST /actuators/25/commands {action: SET_STATE, targetState: true}
→ JWT + HouseMembership + actuator MANUAL validation
→ Save ControlCommand(PENDING) → HTTP 202 (commandId)
→ Publisher MQTT command (CommandStatus SENT)
→ ESP32 executes GPIO/safety or REJECTS → command_ack
→ ControlService updates status → WebSocket update → UI GET if reconnect
```

Nếu lỗi MQTT xảy ra trước lúc phát lệnh, P4 phải chốt chính sách outbox/retry hoặc trả lỗi nhất quán; không báo actuator đã bật. `ACKNOWLEDGED` nghĩa P1 báo đã xử lý GPIO/mode, không chứng minh tải vật lý chạy.

## 5. Ranh giới team

- **P1:** MQTT payload/topic, ESP32/Wokwi, telemetry/command/ACK/Heartbeat, Relay simulation.
- **P2:** AI data definition, CountLog and AnomalyEvent internal API, source dedup TBD.
- **P3:** API contract, FR/Use Case mapping, Class Diagram, review.
- **P4:** Spring Boot Controllers/DTOs/Services/Security/MQTT Adapter/JPA, SQL migrations.
- **P5:** React API consumption, Dashboard, ERD constraints/data dictionary.