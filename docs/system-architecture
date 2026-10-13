Hệ thống SwiftletCare gồm các thành phần và giao thức chính sau:

| Thành phần | Vai trò và giao thức |
|---|---|
| **Cảm biến, ESP32 và relay** | ESP32 đọc dữ liệu cảm biến, điều khiển relay qua **GPIO** và duy trì AUTO bằng cấu hình đã lưu khi mất Internet. **Wokwi** dùng để mô phỏng. |
| **MQTT Broker** | Chuyển tiếp telemetry, heartbeat, lệnh điều khiển và ACK giữa ESP32 với Backend qua **MQTT**, kết hợp **TLS** để bảo vệ kết nối. |
| **Spring Boot Backend** | Xử lý nghiệp vụ, xác thực, phân quyền, lưu dữ liệu và tạo cảnh báo. Giao tiếp với frontend qua **REST API**, với Broker qua **MQTT**. |
| **Database** | Lưu người dùng, nhà yến, khu vực, thiết bị, cấu hình, dữ liệu môi trường và cảnh báo. Backend truy cập qua **JDBC/SQL**. |
| **React frontend và WebSocket** | React hiển thị dashboard và gửi yêu cầu quản lý, điều khiển qua **HTTPS REST API**. Backend cập nhật dữ liệu, trạng thái và cảnh báo qua **WSS**. |
| **Camera và AI Service** | Camera cung cấp video/frame qua **RTSP hoặc nguồn video mô phỏng**. AI nhận diện, đếm chim và phát hiện bất thường; đề xuất gửi kết quả về Backend qua **HTTPS REST API**. |
| **Kho ảnh bằng chứng** | Lưu snapshot bất thường; Database lưu metadata và mã tham chiếu ảnh. Backend truy cập kho ảnh qua **HTTPS API**. |
| **Dịch vụ thông báo** | Backend gọi **HTTPS API** để gửi cảnh báo qua **Web Push/Mobile Push**. Nhà cung cấp dịch vụ cần nhóm thống nhất. |
