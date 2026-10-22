# MQTT contract MVP — BUILD1-04

Bổ sung ngày 23/10/2026: file này trước đó rỗng. Đây là lựa chọn triển khai MVP dựa trên `docs/design/iot/iot-offline-recovery.md`; cần P1/P4 review trước khi tích hợp firmware thực.

Topic prefix: `swiftletcare/{houseId}/{zoneId}/{deviceCode}`. House/Zone là PK số; deviceCode là `devices.device_id`, không phải PK số. Mã không chứa `/`, `+`, `#`. Backend đối chiếu cả topic và device/Zone/House đang active.

| Topic suffix | Hướng | QoS publish | Retain |
|---|---|---|---|
| `telemetry` | ESP32 → Backend | 0 | false |
| `heartbeat` | ESP32 → Backend | 0 | false |
| `telemetry_ack` | Backend → ESP32 | 1 | false |

Backend subscribe `swiftletcare/+/+/+/telemetry` và `swiftletcare/+/+/+/heartbeat`; tự reconnect và subscribe lại, kể cả broker chưa sẵn sàng lúc ứng dụng khởi động. Mỗi instance phải có MQTT_CLIENT_ID khác nhau; MVP chỉ chạy một backend instance.

Telemetry JSON (UTF-8, tối đa 16 KiB):

```json
{"device_id":"ESP32-LOCAL","event_id":"sample-001","timestamp":"2026-10-23T00:00:00Z","temperature":29.5,"humidity":80,"co2":null,"light":12,"sensor_status":"PARTIAL_ERROR"}
```

Bắt buộc device_id (1..100 ký tự), event_id (1..150), timestamp có timezone, sensor_status. Bốn trường đo nullable; OK khi cả bốn có số, ERROR khi cả bốn null, PARTIAL_ERROR khi có 1..3 số. Theo [NFR-IOT-004](../requirements/NFR.md), số phải hữu hạn và nằm trong dải **temperature -10..60°C, humidity 0..100%, co2 0..5000 ppm, light 0..100000 lux**, bao gồm cả hai biên. Ngoài dải bị từ chối trước khi lưu, không có ACK STORED; cảm biến lỗi gửi null với status tương ứng, không dùng số 0 giả. Timestamp không quá 5 phút trong tương lai; bản đo cũ được phép replay nhưng vẫn phải qua validation. Trường lạ (kể cả schemaVersion) bị từ chối trong MVP. PostgreSQL/DTO chuẩn hóa timestamp về microsecond, bỏ phần nhỏ hơn microsecond trước khi lưu/so sánh.

Heartbeat:

```json
{"device_id":"ESP32-LOCAL","timestamp":"2026-10-23T00:00:00Z"}
```

Backend dùng thời điểm **nhận** trên DB, không dùng timestamp nguồn để tính Online. Heartbeat nguồn cũ vẫn có thể xác nhận kết nối đang hoạt động. Chỉ thiết bị ESP32 được gửi Telemetry/Heartbeat trong MVP; CAMERA là nguồn CountLog riêng.

Application ACK:

```json
{"device_id":"ESP32-LOCAL","event_id":"sample-001","status":"STORED"}
```

Chỉ phát sau commit. Retry cùng `(device_id,event_id)` và cùng nội dung nhận lại STORED, không phát thêm telemetry.updated. Khác nội dung, sai payload/device/topic hoặc DB/commit lỗi: không phát STORED. ACK bị mất → producer retry cùng bản tin. Không có outbox; ACK/realtime không được bảo đảm giao nếu process/broker chết sau commit. Frontend hồi phục qua REST, ESP32 hồi phục qua retry. Retained replay bị từ chối; producer phải giữ retain=false (MQTT 3 không luôn chuyển cờ retain của publish trực tiếp tới subscriber đang kết nối).

Heartbeat mặc định mỗi 2 giây, timeout 6000 ms, scheduler mỗi 1000 ms; cấu hình HEARTBEAT_TIMEOUT_MS và STATUS_SCAN_MS. Chỉ phát status event khi Online thay đổi. Chưa subscribe availability/LWT trong MVP.

Broker Compose chỉ dành cho local: anonymous, bind host 127.0.0.1. Trước triển khai ngoài local cần MQTT authentication/ACL theo device/topic và TLS; topic khớp payload không thay thế xác thực publisher.
