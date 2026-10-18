# SwiftletCare — System Architecture Consistency Checklist

> **Mục tiêu:** Kiểm tra tính đầy đủ, nhất quán và khả năng triển khai của kiến trúc hệ thống.  
> Đây là checklist **kỹ thuật**, không phải bảng phân công hay đánh giá tiến độ. Trạng thái mặc định chưa đánh dấu khi chưa có bằng chứng cấu hình hoặc phép đo.

## 1. Sơ đồ và phạm vi hệ thống

- [ ] **ARC-01** — Ba vùng Device/Edge, Communication/Backend/Data và Presentation/Push được phân tách rõ.
- [ ] **ARC-02** — Các mũi tên chỉ đúng nguồn, đích, hướng và giao thức; luồng tùy chọn được đánh dấu.
- [ ] **ARC-03** — Ảnh sơ đồ đủ rõ để đọc tên thành phần và giao thức.
- [ ] **ARC-04** — File chỉnh sửa gốc và ảnh hiển thị cùng một phiên bản (nếu có file gốc).
- [ ] **ARC-05** — Những lựa chọn có `(*)` không được mô tả là thành phần chắc chắn đã triển khai.

## 2. Device / Edge

- [ ] **ARC-06** — Các loại cảm biến nhiệt độ, độ ẩm, CO₂, ánh sáng được định nghĩa rõ theo giao thức cảm biến.
- [ ] **ARC-07** — Firmware tách Sensor, Control, MQTT, Buffer/Config và HAL/GPIO về mặt trách nhiệm.
- [ ] **ARC-08** — Chế độ AUTO/MANUAL độc lập với trạng thái ONLINE/OFFLINE/RECOVERING.
- [ ] **ARC-09** — AUTO có thể điều khiển cục bộ khi mất mạng nếu các điều kiện an toàn còn đúng.
- [ ] **ARC-10** — Các trạng thái relay/actuator và nguyên tắc safety không mâu thuẫn với Control Command.
- [ ] **ARC-11** — AI Service chạy riêng trên nguồn xử lý video, không được gán cho ESP32.

## 3. MQTT, Telemetry, Command và Heartbeat

- [ ] **ARC-12** — MQTT Broker đảm nhiệm chuyển bản tin, không đảm nhiệm logic nghiệp vụ hay commit Database.
- [ ] **ARC-13** — Telemetry `event_id` được ánh xạ đúng sang Java `eventId` và SQL `event_id`.
- [ ] **ARC-14** — Backend lưu chống trùng theo `(device_id, event_id)`.
- [ ] **ARC-15** — Application ACK `STORED` chỉ gửi sau khi dữ liệu hợp lệ đã lưu hoặc xác nhận bản trùng có sẵn.
- [ ] **ARC-16** — Buffer được giữ tới ACK; retry giữ nguyên ID và thời điểm đo, không chặn AUTO.
- [ ] **ARC-17** — `SET_MODE` và `SET_STATE` có ngữ nghĩa, điều kiện và ACK riêng; `SET_STATE` khi AUTO bị từ chối.
- [ ] **ARC-18** — REST `202 Accepted` không bị hiểu là trạng thái đã thực thi trên GPIO.
- [ ] **ARC-19** — Heartbeat interval 2s; Backend timeout 6s; Scheduler 1s, luồng nhận và timeout độc lập.
- [ ] **ARC-20** — Availability/LWT được hiểu là tín hiệu bổ trợ; không thay thế Timeout Checker.

## 4. Backend, Database và AI

- [ ] **ARC-21** — Spring Boot đảm nhiệm REST, xác thực/RBAC, xử lý MQTT, Service, Repository, WebSocket.
- [ ] **ARC-22** — Transaction và thông báo realtime bảo đảm không công bố dữ liệu lưu chưa thành công.
- [ ] **ARC-23** — Class Diagram và ERD cùng mô tả 17 Entity/bảng baseline; các FK và enum phù hợp.
- [ ] **ARC-24** — Device ID chuỗi của MQTT không bị nhầm với khóa ngoại BIGINT trong Database.
- [ ] **ARC-25** — Các trường đo hỏng cho phép giá trị `null` và `SensorStatus` phân biệt OK/PARTIAL_ERROR/ERROR.
- [ ] **ARC-26** — Camera/AI truyền CountLog, AnomalyEvent và snapshot metadata qua giao diện nội bộ có xác thực.
- [ ] **ARC-27** — Các thao tác trên nhà yến, Zone, Device đều bị giới hạn bởi quyền truy cập tương ứng.

## 5. Presentation, Realtime và Notification

- [ ] **ARC-28** — React truy vấn REST và nhận cập nhật WebSocket, không truy cập DB trực tiếp.
- [ ] **ARC-29** — UI có chiến lược lấy lại snapshot khi kết nối realtime phục hồi.
- [ ] **ARC-30** — WEB/PUSH có mô hình NotificationDelivery; SENT/DELIVERED khác trạng thái đã đọc.
- [ ] **ARC-31** — Mobile Push và Media Gateway chỉ được thể hiện trong phạm vi đã xác định.
- [ ] **ARC-32** — Live View sử dụng giao thức Browser hỗ trợ và kiểm tra quyền trước khi cấp phát stream.

## 6. Quyết định và giới hạn kỹ thuật

- [ ] **ARC-33** — Broker, Database, lưu ảnh, provider Push có trạng thái CONFIRMED/PROPOSED/OPEN.
- [ ] **ARC-34** — MQTT Topic, Payload, ACK, QoS và REST DTO không mâu thuẫn nhau.
- [ ] **ARC-35** — Có chính sách xử lý dữ liệu lỗi, Broker Offline, Database lỗi và retry.
- [ ] **ARC-36** — Không nhầm `ControlMode` với `Device.online`.
- [ ] **ARC-37** — Không tuyên bố đạt NFR khi chưa có kết quả đo.

## 7. Tiêu chí đo NFR (chưa phải kết quả thực tế)

| Chỉ tiêu | Phương pháp xác minh kỹ thuật | Trạng thái |
|---|---|---|
| Broker → Dashboard ≤2s | So timestamp qua Broker/Backend/React | NOT TESTED |
| AI event → Mobile ≤5s | So thời điểm AI event với thời điểm nhận push | NOT TESTED |
| Giảm băng thông ≥90%/1h | Đo lưu lượng đối chứng và dữ liệu truyền thực tế cùng điều kiện | NOT TESTED |
| UI hiển thị Offline ≤10s | Ngắt kết nối thiết bị, đo đến khi UI đổi trạng thái | NOT TESTED |
| AUTO offline ≥24h | Theo dõi điều khiển địa phương liên tục với nguồn cấp ổn định | NOT TESTED |
| Buffer ≥17.280 bản ghi/24h (5s) | Kiểm tra bytes lưu, reset/replay/ACK và DB dedup | NOT TESTED |

## 8. Lưu ý tính nhất quán của sơ đồ hiện tại

- Heartbeat 2s/6s/1s đã có trong hợp đồng giao tiếp nhưng chưa được ghi chi tiết trực tiếp trên ảnh tổng thể.
- Chú thích Application ACK `STORED` cần phân biệt với MQTT QoS/PUBACK.
- WebSocket realtime khác Mobile Push qua nhà cung cấp thông báo.
- Những mũi tên nét đứt tới Media Gateway thể hiện phương án Live View tùy chọn.
- Các mục tiêu NFR trên hình không phải bằng chứng đã đạt hiệu năng.
