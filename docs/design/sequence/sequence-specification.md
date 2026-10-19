# Sequence Specification – chi tiết và đối chiếu từng sơ đồ

## Quy ước

- Mermaid `.mmd` là biểu diễn tuần tự có `alt`, `opt`, `loop`; Draw.io cùng tên là bản đồ họa dễ điều chỉnh, **không thay cho flow branch trong Mermaid**.
- Mọi thời điểm `HTTP 202`, MQTT `CONNACK/SUBACK`, `telemetry_ack`, `command_ack`, AI REST 2xx mang ý nghĩa khác nhau. Không đồng nhất ACK MQTT với DB commit.
- Khi không có input hoặc fail schema: không tạo dữ liệu/sự kiện giả.

## SD-01 – Telemetry

**Trigger:** ESP32 đọc sensor sau chu kỳ mặc định 5 giây. **Tiền điều kiện:** thiết bị có cấu hình hợp lệ; Backend biết `Device` tương ứng. **Hậu điều kiện:** Telemetry lưu hoặc giữ Buffer; Dashboard chỉ thấy dữ liệu commit.

Nhánh cần có: OK/PARTIAL_ERROR/ERROR; disconnected; duplicate cùng nội dung; duplicate khác nội dung; DB unavailable; ACK lost. **`event_id` là tên MQTT đã chốt**, còn Java DTO `eventId`, SQL `event_id`. Trên QoS0 **không dùng PUBACK**.

## SD-02 – Web Device Control

**Trigger:** người dùng đủ quyền gửi yêu cầu điều khiển. **POST:** `/api/v1/actuators/{actuatorId}/commands`. `SET_MODE` cập nhật mode; `SET_STATE` chỉ khi MANUAL. `commandId` ổn định cho duplicate; `expiresAt` và safety phải được ESP32 kiểm tra. **Hậu điều kiện:** CommandStatus sau ACK; HTTP 202 không chứng minh thực thi.

## SD-03 – AI Counting

**Trigger:** Camera/video có frame hợp lệ. Track crossing có hướng, ID scoped session. Backend baseline nhận **CountLog theo kỳ** chứ không nhận từng raw crossing. `detectCount` semantics, idempotency key của CountLog vẫn [TBD]. Đối chiếu class `CountLog(periodStart,periodEnd,entryCount,exitCount,detectCount,aggregationType,processedAt)`.

## SD-04 – Alert & Notification

**Hai nguồn độc lập:** (A) Camera → AI Service → HTTP Anomaly; (B) ESP32 → MQTT Telemetry → môi trường rule; sau đó dùng chung Backend AlertService. Không buộc Backend gọi AI cho mỗi camera frame. **`AnomalyEvent` lưu trước** khi suy ra Alert theo rule; mẫu Mermaid biểu diễn transaction gộp khi phù hợp; quyết định cùng transaction hay hai transaction [TBD]. NotificationDelivery độc lập với trạng thái đã đọc. Lỗi Notification không xóa Alert.

**Quan trọng về transaction:** nếu lưu AnomalyEvent rồi phát hiện không thỏa ngưỡng tạo Alert, vẫn có thể lưu AnomalyEvent mà không tạo Alert (theo FR/retention đã chốt); sơ đồ nhánh no-alert chỉ có nghĩa không sinh **Alert**, không tự quyết định có bỏ AnomalyEvent.

## SD-05 – Offline / Recovery

Mất mạng không đồng nghĩa mất nguồn; ESP32 giữ AUTO nếu cảm biến/actuator an toàn. Buffered event có `event_id` và timestamp không đổi. Reconnect và replay chạy nền. Backend phải phát hiện Offline độc lập nhờ Scheduler 1s + heartbeat 2s, timeout 6s. Tránh xóa Buffer khi chỉ thấy CONNACK/SUBACK; **Application ACK STORED mới là điều kiện xóa**.

## Các điểm cần xác nhận để đóng các sơ đồ

| Điểm | Hiện trạng |
|---|---|
| AI idempotency CountLog/Anomaly | OPEN, thiếu trong DSN-09 baseline |
| AI Camera→Device mapping | OPEN |
| Cơ chế timeout/Retry của Telemetry ACK | OPEN |
| MQTT Telemetry QoS | P1 baseline QoS0; sửa SD-05 không dùng QoS1 PUBACK nếu chưa change contract |
| Ngưỡng/cooldown predator và alert | OPEN |
| Thời hạn MANUAL/relay safety state | OPEN |
| Phương án storage bằng chứng | OPEN |

## Kiểm thử cần có khi BUILD

| TC | Bước kiểm tra | Kỳ vọng |
|---|---|---|
| SD01-A | Publish Telemetry có `co2:null`, `PARTIAL_ERROR` | DB lưu null, không ghi thành 0 |
| SD01-B | Gửi lại cùng `event_id` | 1 record và ACK lại nếu payload tương đương |
| SD01-C | DB down sau MQTT delivery | Không ACK STORED, ESP32 giữ Buffer |
| SD02-A | `SET_STATE` trong AUTO | Từ chối an toàn (REST hoặc ESP32) |
| SD02-B | Command hết hạn/duplicate | Không thực thi sai/đôi |
| SD03-A | Một track qua đường đúng hướng | Count tăng một lượt |
| SD03-B | Đăng lại CountLog kỳ giống nhau | Kết quả theo cơ chế idempotency **chưa chốt**, không được tuyên bố pass |
| SD04-A | Predator hợp lệ | Lưu AnomalyEvent + Alert nếu rule thỏa, WebSocket sau commit |
| SD04-B | Notification service lỗi | Alert vẫn còn trong DB |
| SD05-A | Mất Wi-Fi | AUTO local còn chạy; Buffer tăng |
| SD05-B | ACK mất sau commit | Replay cùng `event_id`, DB không trùng |
| SD05-C | Heartbeat dừng | Scheduler phát hiện Offline, Dashboard cập nhật theo mục tiêu ≤10s |
