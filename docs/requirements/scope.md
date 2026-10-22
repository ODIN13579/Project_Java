# Phạm vi triển khai BUILD1-04

Tài liệu này ghi phạm vi backend theo task BUILD1-04 và implementation hiện hành, không thay thế FR/NFR toàn hệ thống hoặc tự xác nhận sign-off liên nhóm.

- Có: Java 21/Spring Boot, PostgreSQL/Flyway/JPA; entity User/House/Zone/Device/Telemetry và CountLog; membership để scope quyền đọc.
- Có: MQTT Telemetry/Heartbeat, validate/dedup/transaction, ACK sau commit, reconnect, Online/Offline scheduler.
- Có: 6 REST operation cho login, Device/Telemetry read và CountLog ingest; WebSocket theo device, CORS, JWT access token tối thiểu. Xem danh sách chính xác trong [contract MVP](../api/build104-mvp.md).
- NFR-IOT-004 áp dụng cho validation Telemetry trong task này: temperature -10..60°C, humidity 0..100%, co2 0..5000 ppm, light 0..100000 lux (inclusive), chỉ số hữu hạn; null biểu diễn lỗi cảm biến theo sensorStatus. Không có miễn trừ dải đo trong yêu cầu task. Phần xử lý AUTO/actuator của NFR vẫn thuộc firmware/giai đoạn sau, chưa được chứng minh bởi backend test.
- Ngoài task này: alert/control/report, API House/Zone, GET CountLog, anomaly/raw event/evidence, refresh/logout/revocation, CRUD user/membership, MQTT TLS/ACL sản xuất và outbox.

FR-DASH/FR-IOT/FR-AI và NFR hiệu năng/offline/tải vẫn là yêu cầu hệ thống; có code ingest hoặc đo backend không đồng nghĩa nghiệm thu React <=2s, UI Offline <=10s, buffer ESP32 24h hay AI model. Không đánh dấu phần ngoài MVP là lỗi thiếu implementation của BUILD1-04. Kết quả chỉ theo phép kiểm thử đã chạy trong [báo cáo](../api/build104-verification.md).
