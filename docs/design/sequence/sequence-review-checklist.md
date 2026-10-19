# DSN-12 – Review Checklist

- [ ] Có đủ SD-01 .. SD-05, không thiếu Web Device Control.
- [ ] Mỗi sơ đồ chỉ ra participant đúng (AI riêng, ESP32 khác Backend).
- [ ] SD-01 dùng `event_id`, Buffer trước Publish, QoS0 không PUBACK, ACK sau commit.
- [ ] SD-01 có PARTIAL_ERROR/null, duplicate và DB failure.
- [ ] SD-02 phân biệt SET_MODE/SET_STATE và HTTP 202 vs Command ACK.
- [ ] SD-02 có nhánh 401/403/409, expired, reject/safety.
- [ ] SD-03 trackId theo session, CountLog theo kỳ, không giả là số chim hiện tại.
- [ ] SD-03 ghi idempotency AI là OPEN nếu chưa sửa ERD/API.
- [ ] SD-04 Camera→AI→REST là luồng chính, môi trường→MQTT là luồng riêng.
- [ ] SD-04 persist Alert/NotificationDelivery, không rollback Alert khi Push lỗi.
- [ ] SD-05 AUTO offline độc lập connection state, Replay không chặn Sensor.
- [ ] SD-05 Heartbeat 2s/timeout 6s/Scheduler 1s, App ACK STORED.
- [ ] Có `.mmd` nguồn và `.drawio` mới dựng; khi dùng PNG phải xuất từ nguồn mới.
- [ ] Tên URL/DTO trong sequence đối chiếu với DSN-09 OpenAPI.
- [ ] Những [TBD] được đăng ký thay đổi, không tự coi đã nghiệm thu.
