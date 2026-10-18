# SwiftletCare — DSN-09 REST API Review Checklist

> **Chủ trì:** P3 • **Reviewer:** P4, P1, P2, P5. Mặc định mọi mục chưa ký duyệt là `[ ]`; tài liệu đã được soạn **không đồng nghĩa đã pass review**.

## A. Phạm vi và đặc tả

- [ ] Có `openapi.yaml` hợp lệ và đủ tất cả API MVP dự kiến.
- [ ] Từng HTTP operation có `operationId`, path/method duy nhất, response, schema và security.
- [ ] Thống nhất REST `eventId` ↔ MQTT `event_id` ↔ SQL `event_id`.
- [ ] REST không tạo endpoint public để giả MQTT Telemetry/Command ACK.
- [ ] GET history có thời gian, phân trang, quyền House, thứ tự thời gian rõ.
- [ ] HTTP Error DTO và status nhất quán; 202 không bị hiểu là Device đã chạy.
- [ ] Các API trong nhóm `Extended` không bị bỏ FR nếu chưa chốt đổi phạm vi.

## B. Domain/Database/MQTT

- [ ] Class `ControlCommand` được thêm `action: CommandAction`, targetState/targetMode nullable XOR; P3 xác nhận.
- [ ] ERD xóa `target_state` trùng, giữ một cột nullable; P5 xác nhận.
- [ ] Telemetry UNIQUE(device_id,event_id); REST nullable sensor measurements.
- [ ] ZoneConfig có tối đa một config hiện hành; version conflict xử lý 409.
- [ ] GET/POST Device phân biệt PK số với MQTT mã chuỗi.
- [ ] P1/P4 chốt `SET_MODE`/`SET_STATE`, QoS command, ACK timeout, config publish semantics.
- [ ] Heartbeat 2s, timeout 6s, scheduler 1s; REST chỉ đọc Device.online.
- [ ] P2/P4 chốt internal AI auth, idempotency, payload CountLog/AnomalyEvent.
- [ ] Alert status update lưu lịch sử đúng transaction.

## C. Security / React / Reporting

- [ ] P4 ký `api-security-matrix.md`, quyền theo HouseRole và phạm vi tài nguyên.
- [ ] P4 chốt JWT login/refresh/logout và cơ chế logout thật.
- [ ] P5 xác nhận response đủ dữ liệu cho Wireframe Dashboard, điều khiển, Alert, AI, Report.
- [ ] WebSocket event names, auth và reconnect đồng bộ P4/P5.
- [ ] Báo cáo môi trường/INOUT/thu hoạch/correlation đối chiếu FR thực và phạm vi thời gian.
- [ ] P3 điền ID FR chính thức vào `fr-api-mapping.md` sau khi có FR v1.0.
- [ ] Các mục TBD trong `api-open-decisions.md` có owner, quyết định, ngày hoàn tất.

## D. Nghiệm thu Jira/GitHub

- [ ] `python validate_openapi.py` chạy thành công, kết quả được lưu trong PR.
- [ ] P1, P2, P4, P5 đã review phần của mình; P3 tổng hợp sửa đổi.
- [ ] `openapi.yaml` là nguồn duy nhất cho endpoint/schema; không có sai lệch với Endpoint Catalog.
- [ ] PR merge vào repo; link PR/commit/Jira DSN-09 đã lưu.
- [ ] Task chỉ được đổi sang Done khi P4 đồng ý có thể bắt đầu code Controller/DTO và các blocker MVP đã giải quyết.
