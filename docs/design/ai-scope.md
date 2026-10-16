# AI Scope – SwiftletCare

| Chức năng | Đầu vào | Đầu ra | Giới hạn |
|---|---|---|---|
| Phát hiện chim yến | Frame hình ảnh từ camera hoặc video | Bounding box, nhãn đối tượng, confidence score | Có thể bỏ sót chim khi thiếu sáng, bị che khuất hoặc nhiều chim chồng lấn. |
| Theo dõi chim yến | Kết quả phát hiện chim qua nhiều frame liên tiếp | Tracking ID và vị trí di chuyển | Tracking ID có thể bị mất hoặc thay đổi khi chim bay sát nhau hoặc bị che khuất. |
| Đếm chim Entry/Exit | Tracking ID, vị trí đối tượng và đường đếm | Số lượt chim vào, số lượt chim ra và thời gian ghi nhận | Có thể đếm sai khi mất tracking, chim bay thành đàn hoặc không xác định được hướng di chuyển. |
| Phát hiện động vật gây hại | Frame hình ảnh từ camera | Loại động vật, bounding box, confidence score và ảnh bằng chứng | Chỉ phát hiện các lớp được mô hình hỗ trợ; độ chính xác phụ thuộc chất lượng hình ảnh và dữ liệu huấn luyện. |
| Tạo sự kiện và gửi kết quả | Kết quả đếm chim, sự kiện phát hiện động vật gây hại và thông tin liên quan | Dữ liệu sự kiện gửi đến Spring Boot Backend qua HTTPS REST API | Cần xử lý lỗi kết nối và chống gửi trùng; việc lưu trữ dữ liệu và phát cảnh báo thuộc Backend. |
