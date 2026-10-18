# SwiftletCare — Realtime / WebSocket Contract (giao diện đề xuất, không phải REST)

DSN-05 đã chốt **WebSocket truyền cập nhật, REST và Database là nguồn lịch sử**. Topic/destination cụ thể ở đây là thiết kế **đề xuất** cần P4/P5 duyệt.

| Event đề xuất | Data source | Trigger | Payload tối thiểu |
|---|---|---|---|
| `telemetry.updated` | `Telemetry` | DB commit ingest | `deviceId`, `eventId`, `timestamp`, `sensorStatus` |
| `device.status.changed` | `Device` | Heartbeat/LWT/scheduler đổi trạng thái | `deviceId`, `online`, `lastSeenAt` |
| `command.updated` | `ControlCommand` | Accepted/SENT/ACK/Failed | `commandId`, `status`, `actuatorId`, `timestamp` |
| `alert.created` / `alert.updated` | `Alert` | AlertService commit | `alertId`, `zoneId`, `status`, `level` |
| `ai.count.updated` | `CountLog` | AI ingest commit | `deviceId`, `periodStart`, `periodEnd` |

- WebSocket session xác thực JWT hoặc cơ chế WebSocket auth được P4 xác nhận; quyền subscribe theo House/Zone.
- Chỉ phát sự kiện nghiệp vụ sau DB commit; tránh hiển thị một record không tồn tại.
- UI nhận WS phải phân biệt realtime event với response REST; sau reconnect, dùng REST GET để lấy phần bị mất và loại trùng theo `eventId`/`commandId`.
- Heartbeat 2s không có nghĩa WebSocket gửi một bản status mỗi 2s; chỉ phát `device.status.changed` khi trạng thái thật sự đổi.
- Không đưa URL WebSocket giả định thành “đã chốt”; endpoint handshake và STOMP/plain websocket sẽ được P4/P5 ghi bổ sung.
