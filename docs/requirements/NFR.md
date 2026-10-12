# TỔNG HỢP YÊU CẦU PHI CHỨC NĂNG[cite: 9]

## 1. Nhóm Khả năng Hoạt động Ngoại tuyến & IoT (Phụ trách: P1)[cite: 9]

### NFR-IOT-001: Khả năng hoạt động ngoại tuyến[cite: 9]
* **Nội dung:** ESP32 tiếp tục thực thi quy tắc điều khiển vi khí hậu cục bộ khi mất kết nối Internet hoặc máy chủ[cite: 9]. 
* **Chỉ số kỹ thuật:** Mục tiêu duy trì điều khiển tự động ít nhất 24 giờ khi mất Internet[cite: 9]. 
* **Phương pháp kiểm chứng:** Ngắt Internet/Wi-Fi nhưng giữ nguồn ESP32; tạo giá trị cảm biến ở hai phía ngưỡng; xác minh quy tắc điều khiển vẫn hoạt động[cite: 9].

### NFR-IOT-002: Lưu đệm dữ liệu ngoại tuyến[cite: 9]
* **Nội dung:** ESP32 lưu tạm dữ liệu telemetry khi mất kết nối và đồng bộ lên máy chủ khi có mạng trở lại[cite: 9]. 
* **Chỉ số kỹ thuật:** Với chu kỳ 5 giây trong 24 giờ, tối thiểu 17.280 bản ghi cho mỗi luồng[cite: 9]. 
* **Phương pháp kiểm chứng:** Ngắt mạng trong 1 giờ, tạo dữ liệu, khôi phục mạng và đối chiếu số lượng, timestamp, bản ghi trùng/mất[cite: 9].

### NFR-IOT-003: Hiệu năng telemetry[cite: 9]
* **Nội dung:** ESP32 gửi dữ liệu cảm biến lên MQTT Broker theo chu kỳ cấu hình được từ xa[cite: 9]. 
* **Chỉ số kỹ thuật:** Chu kỳ mặc định 5 giây/bản ghi (cho phép cấu hình từ 1-60 giây)[cite: 9]. 
* **Phương pháp kiểm chứng:** Ghi timestamp tại các bản tin MQTT liên tiếp; thay đổi chu kỳ từ xa và xác minh chu kỳ thực tế[cite: 9].

### NFR-IOT-004: Tính hợp lệ dữ liệu cảm biến[cite: 9]
* **Nội dung:** Hệ thống phát hiện dữ liệu thiếu, không hợp lệ hoặc ngoài dải đo; dữ liệu lỗi không được dùng để điều khiển[cite: 9]. 
* **Chỉ số kỹ thuật:** Dải kiểm tra: Nhiệt độ -10 đến 60°C; Độ ẩm 0-100%; CO₂ 0-5.000 ppm; Ánh sáng 0-100.000 lux[cite: 9]. 
* **Phương pháp kiểm chứng:** Tiêm dữ liệu giả lập ngoài dải, NaN hoặc dữ liệu thiếu; kiểm tra hệ thống đánh dấu/từ chối[cite: 9].

### NFR-IOT-005: Phát hiện lỗi cảm biến[cite: 9]
* **Nội dung:** Hệ thống phát hiện cảm biến bị ngắt kết nối hoặc không cập nhật dữ liệu và phát cảnh báo lỗi[cite: 9]. 
* **Chỉ số kỹ thuật:** Phát hiện trong ≤10 giây kể từ khi dữ liệu ngưng nhận[cite: 9]. 
* **Phương pháp kiểm chứng:** Ngắt dây tín hiệu; đo thời gian từ lần đọc hợp lệ cuối đến lúc hệ thống ghi nhận lỗi[cite: 9].

### NFR-IOT-006: An toàn khi cảm biến lỗi[cite: 9]
* **Nội dung:** Khi cảm biến lỗi, hệ thống cảnh báo và chuyển thiết bị liên quan sang trạng thái an toàn được định nghĩa riêng[cite: 9]. 
* **Chỉ số kỹ thuật:** Kích hoạt xử lý lỗi trong ≤3 giây[cite: 9]. 
* **Phương pháp kiểm chứng:** Ngắt từng cảm biến ở chế độ Auto; đo thời gian phát hiện và kiểm tra hành vi actuator theo ma trận[cite: 9].

### NFR-IOT-007: Độ trễ điều khiển actuator[cite: 9]
* **Nội dung:** Sau khi nhận lệnh hợp lệ, GPIO điều khiển Relay thay đổi trạng thái trong thời gian quy định[cite: 9]. 
* **Chỉ số kỹ thuật:** ≤1 giây từ lúc nhận lệnh đến lúc GPIO đổi trạng thái[cite: 9]. 
* **Phương pháp kiểm chứng:** Ghi timestamp lúc nhận lệnh và lúc GPIO thay đổi; lặp lại đo độ trễ trung bình và lớn nhất[cite: 9].

### NFR-IOT-008: Bảo vệ actuator khỏi đóng/ngắt liên tục[cite: 9]
* **Nội dung:** Hệ thống giới hạn tần suất chuyển trạng thái để bảo vệ thiết bị điện[cite: 9]. 
* **Chỉ số kỹ thuật:** Thời gian tối thiểu giữa hai lần chuyển trạng thái là 120 giây[cite: 9]. 
* **Phương pháp kiểm chứng:** Cho cảm biến dao động quanh ngưỡng; ghi nhận thời điểm đóng/ngắt và xác minh thời gian giãn cách[cite: 9].

### NFR-IOT-009: Điều khiển có hysteresis[cite: 9]
* **Nội dung:** Chế độ Auto sử dụng ngưỡng bật/tắt khác nhau để tránh chattering[cite: 9]. 
* **Chỉ số kỹ thuật:** Quạt bật khi nhiệt độ ≥31°C và tắt khi ≤30°C[cite: 9]. 
* **Phương pháp kiểm chứng:** Tạo dữ liệu dao động quanh ngưỡng; xác minh actuator chỉ đổi trạng thái khi vượt qua ngưỡng bật/tắt tương ứng[cite: 9].

### NFR-IOT-010: Khôi phục cấu hình[cite: 9]
* **Nội dung:** Sau khi khởi động lại, ESP32 khôi phục các cấu hình ngưỡng và chế độ Auto/Manual đã lưu[cite: 9]. 
* **Chỉ số kỹ thuật:** Khôi phục 100% tham số chính xác trong ≤2 giây[cite: 9]. 
* **Phương pháp kiểm chứng:** Lưu cấu hình; ngắt và cấp nguồn lại; đối chiếu từng tham số trước/sau khởi động[cite: 9].

### NFR-IOT-011: Xác thực thiết bị MQTT[cite: 9]
* **Nội dung:** Broker chỉ cho phép thiết bị có thông tin xác thực và quyền topic phù hợp kết nối[cite: 9]. 
* **Chỉ số kỹ thuật:** 100% trường hợp sai thông tin xác thực bị từ chối[cite: 9]. 
* **Phương pháp kiểm chứng:** Thử kết nối không có thông tin xác thực, sai mật khẩu hoặc sai quyền topic; kiểm tra log[cite: 9].

### NFR-IOT-012: An toàn phần cứng và chống ẩm[cite: 9]
* **Nội dung:** Thiết kế hộp và mạch prototype xem xét độ ẩm, chống nước và cách ly điện[cite: 9]. 
* **Chỉ số kỹ thuật:** Mục tiêu thiết kế vỏ IP65 (cần kiểm tra BOM, datasheet và lắp ráp thực tế)[cite: 9]. 
* **Phương pháp kiểm chứng:** Kiểm tra sơ đồ, BOM, cách bố trí linh kiện và vỏ thực tế[cite: 9].

---

## 2. Nhóm Hiệu năng Hệ thống & Độ tin cậy (Phụ trách: P3)[cite: 9]

### NFR-PER-001: Độ trễ Dashboard[cite: 9]
* **Nội dung:** Telemetry sau khi Broker nhận được phải hiển thị trên Web Dashboard[cite: 9]. 
* **Chỉ số kỹ thuật:** ≤2 giây từ lúc Broker nhận bản tin đến lúc UI hiển thị[cite: 9]. 
* **Phương pháp kiểm chứng:** Gắn timestamp bản tin; đo thời điểm Broker nhận và thời điểm UI cập nhật qua nhiều mẫu[cite: 9].

### NFR-PER-002: Độ trễ cảnh báo AI[cite: 9]
* **Nội dung:** Kết quả đếm chim hoặc phát hiện bất thường được chuyển đến thiết bị di động[cite: 9]. 
* **Chỉ số kỹ thuật:** ≤5 giây từ khi AI phát ra sự kiện đến khi mobile nhận thông báo[cite: 9]. 
* **Phương pháp kiểm chứng:** Ghi timestamp lúc AI phát hiện và lúc thiết bị nhận push notification[cite: 9].

### NFR-REL-001: Độ tin cậy điều khiển cục bộ[cite: 9]
* **Nội dung:** Edge Node tiếp tục thực hiện quy tắc điều khiển cốt lõi khi mất Internet[cite: 9]. 
* **Chỉ số kỹ thuật:** 100% lần thực thi đúng trong kịch bản ngoại tuyến[cite: 9]. 
* **Phương pháp kiểm chứng:** Ngắt Internet, chạy kịch bản mô phỏng và kiểm tra log quyết định điều khiển[cite: 9].

---

## 3. Nhóm Tối ưu hóa Băng thông & Chi phí (Phụ trách: P1 & P2)[cite: 9]

### NFR-BW-001: Tối ưu băng thông[cite: 9]
* **Nội dung:** Hệ thống ưu tiên truyền dữ liệu đếm và ảnh snapshot, không truyền video liên tục lên Cloud[cite: 9]. 
* **Chỉ số kỹ thuật:** Giảm ≥90% dung lượng truyền trong 1 giờ so với truyền video liên tục[cite: 9]. 
* **Phương pháp kiểm chứng:** Đo tổng bytes gửi đi của hai phương án (snapshot/count vs streaming) trong cùng điều kiện[cite: 9].

### NFR-COST-001: Hiệu quả chi phí[cite: 9]
* **Nội dung:** Tổng chi phí phần cứng bộ Kit prototype nằm trong ngân sách[cite: 9]. 
* **Chỉ số kỹ thuật:** ≤3.000.000 VNĐ/bộ (bao gồm ESP32, cảm biến, Relay, nguồn, dây nối, camera/vỏ hộp)[cite: 9]. 
* **Phương pháp kiểm chứng:** Lập bảng Bill of Materials (BOM) chi tiết số lượng và đơn giá thực tế[cite: 9].

---

## 4. Nhóm Độ chính xác AI & Computer Vision (Phụ trách: P2)[cite: 9]

### NFR-AI-001: Độ chính xác AI đếm chim[cite: 9]
* **Nội dung:** Mô hình đếm chim ra/vào đánh giá trên tập video kiểm thử độc lập Ground Truth[cite: 9]. 
* **Chỉ số kỹ thuật:** Precision ≥85%, Recall ≥80% (kèm đánh giá MAE sai số đếm)[cite: 9]. 
* **Phương pháp kiểm chứng:** Chạy mô hình trên tập test độc lập, so sánh kết quả dự đoán với nhãn Ground Truth thủ công[cite: 9].

### NFR-AI-002: Độ chính xác phát hiện thiên địch[cite: 9]
* **Nội dung:** Mô hình phát hiện thiên địch đánh giá trên tập ảnh/video kiểm thử[cite: 9]. 
* **Chỉ số kỹ thuật:** Precision ≥80%, Recall ≥75% cho các lớp được hỗ trợ (chuột, rắn, cú mèo)[cite: 9]. 
* **Phương pháp kiểm chứng:** Sử dụng tập test độc lập, tính toán Precision/Recall theo từng lớp đối tượng[cite: 9].

---

## 5. Nhóm Bảo mật Hệ thống (Phụ trách: P4)[cite: 9]

### NFR-SEC-001: Xác thực người dùng Web[cite: 9]
* **Nội dung:** Các API và chức năng quản trị yêu cầu token xác thực hợp lệ[cite: 9]. 
* **Chỉ số kỹ thuật:** 100% request thiếu hoặc sai token bị từ chối (trả mã lỗi 401 Unauthorized)[cite: 9]. 
* **Phương pháp kiểm chứng:** Gửi request trực tiếp không có token, token giả mạo hoặc token hết hạn tới các endpoint[cite: 9].

### NFR-SEC-002: Phân quyền theo vai trò (RBAC)[cite: 9]
* **Nội dung:** Hệ thống chỉ cho phép thực hiện chức năng phù hợp với vai trò được cấp[cite: 9]. 
* **Chỉ số kỹ thuật:** 100% trường hợp vượt quyền bị chặn (trả mã lỗi 403 Forbidden)[cite: 9]. 
* **Phương pháp kiểm chứng:** Thử nghiệm truy cập endpoint quản trị bằng tài khoản có quyền thấp hơn (như Viewer)[cite: 9].

### NFR-SEC-003: Bảo vệ mật khẩu[cite: 9]
* **Nội dung:** Mật khẩu người dùng không được lưu dạng văn bản thuần (Plaintext)[cite: 9]. 
* **Chỉ số kỹ thuật:** 100% mật khẩu được băm bằng thuật toán bcrypt trước khi lưu CSDL[cite: 9]. 
* **Phương pháp kiểm chứng:** Kiểm tra trực tiếp bảng dữ liệu CSDL và file log hệ thống để xác thực chuỗi băm[cite: 9].

### NFR-SEC-004: Bảo mật kênh truyền[cite: 9]
* **Nội dung:** Kênh giao tiếp giữa client và backend được mã hóa khi triển khai thực tế[cite: 9]. 
* **Chỉ số kỹ thuật:** Sử dụng giao thức HTTPS/TLS tại môi trường triển khai sản phẩm[cite: 9]. 
* **Phương pháp kiểm chứng:** Kiểm tra cấu hình endpoint và chứng thư SSL/TLS (nếu chạy môi trường local, ghi rõ giới hạn)[cite: 9].

---

## 6. Nhóm UI/UX & Khả năng Sử dụng (Phụ trách: P5)[cite: 9]

### NFR-UX-001: Tính dễ sử dụng[cite: 9]
* **Nội dung:** Người dùng thử nghiệm hoàn thành các tác vụ Dashboard cơ bản mà không cần hướng dẫn trực tiếp[cite: 9]. 
* **Chỉ số kỹ thuật:** ≥80% người tham gia hoàn thành toàn bộ kịch bản tác vụ được giao[cite: 9]. 
* **Phương pháp kiểm chứng:** Tổ chức kiểm thử thực tế với người dùng thực hiện xem nhiệt độ, số lượng chim và bật/tắt thiết bị[cite: 9].

### NFR-UX-002: Thiết kế giao diện thích ứng (Responsive)[cite: 9]
* **Nội dung:** Web Dashboard hiển thị tốt trên các kích thước màn hình máy tính và thiết bị di động[cite: 9]. 
* **Chỉ số kỹ thuật:** Hỗ trợ tối thiểu độ phân giải 1920×1080 (Desktop) và 375×812 (Mobile)[cite: 9]. 
* **Phương pháp kiểm chứng:** Kiểm tra giao diện qua công cụ giả lập hoặc thiết bị thực tế, đảm bảo không tràn màn hình và nút bấm thao tác tốt[cite: 9].

---

## 7. Nhóm Khả năng Mở rộng (Phụ trách: P4)[cite: 9]

### NFR-SCALE-001: Khả năng mở rộng hệ thống[cite: 9]
* **Nội dung:** Hệ thống tiếp nhận dữ liệu từ nhiều thiết bị/khu vực mà vẫn đảm bảo hiệu năng[cite: 9]. 
* **Chỉ số kỹ thuật:** Hỗ trợ đồng thời 1 nhà yến, 2 khu vực (Zone) và 5 thiết bị IoT mô phỏng, duy trì độ trễ Dashboard ≤2 giây[cite: 9]. 
* **Phương pháp kiểm chứng:** Phát telemetry đồng loạt từ các thiết bị giả lập chu kỳ 5 giây và đo độ trễ phản hồi của giao diện[cite: 9].