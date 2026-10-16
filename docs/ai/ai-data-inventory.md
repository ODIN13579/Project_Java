# KIỂM KÊ DỮ LIỆU AI — SWIFTLETCARE

**Nhiệm vụ:** TASK-11 — Đặc tả bộ dữ liệu AI  
**Phụ trách:** P2 — AI  
**Phối hợp:** P3 — Yêu cầu chức năng/phi chức năng; P4 — Tích hợp Backend  
**Trạng thái:** Bản dự thảo. Chỉ xác nhận dữ liệu đã có sau khi kiểm tra tệp thực tế.

## 1. Mục đích

Ghi nhận các nguồn dữ liệu phục vụ ba chức năng: phát hiện chim yến, phát hiện động vật săn mồi và đếm chim bay vào/ra. Không đánh dấu một nguồn là đã xác minh nếu nhóm chưa tải hoặc truy cập, kiểm tra nội dung và xác nhận quyền sử dụng.

## 2. Danh mục nguồn dữ liệu dự kiến

| Mã nguồn | Nguồn/vị trí | Loại dữ liệu | Số lượng | Đối tượng dự kiến | Trạng thái |
|---|---|---|---|---|---|
| SRC-01 | Video thực tế từ camera nhà yến, nếu có | Video | Chưa kiểm kê | Chim yến và các đối tượng trong vùng quan sát | Chưa xác minh |
| SRC-02 | Video thử nghiệm của TASK-S2 | Video | Chưa kiểm kê | Theo nội dung thực tế của video | Chưa xác minh |
| SRC-03 | Ảnh/video công khai phù hợp | Ảnh/video | Ghi sau khi tải và kiểm tra | Chim yến, rắn, chuột, cú nếu có | Chưa chọn/xác minh |
| SRC-04 | Mô hình đã huấn luyện hoặc trọng số có sẵn để làm mốc so sánh | Mô hình | Ghi phiên bản và nguồn | Chỉ các lớp mà mô hình thực sự hỗ trợ | Chưa xác minh |
| SRC-05 | Dữ liệu nhóm tự thu thập hoặc mô phỏng | Ảnh/video | Chưa kiểm kê | Theo danh sách lớp đã thống nhất | Chưa xác minh |

**Lưu ý:** Đây là các nhóm nguồn cần kiểm tra, không có nghĩa nhóm đã sở hữu bộ dữ liệu được xác minh.

## 3. Thông tin cần ghi cho từng nguồn

- Tên nguồn, đường dẫn, ngày truy cập/tải xuống và người phụ trách.
- Số lượng ảnh, số video, tổng thời lượng, tốc độ khung hình và độ phân giải nếu có.
- Các lớp đối tượng xuất hiện và số lượng mẫu ước tính cho từng lớp.
- Điều kiện ghi hình: ban ngày/ban đêm, sáng/tối, góc camera, khoảng cách, che khuất, nhòe chuyển động và điều kiện môi trường.
- Chất lượng dữ liệu, hạn chế và lỗi đã biết.
- Giấy phép, quyền sử dụng, yêu cầu ghi nguồn và giới hạn phân phối lại.
- Tình trạng gán nhãn và định dạng nhãn.
- Mã nhóm nguồn dùng để chia tập dữ liệu, tránh đưa các khung hình gần giống nhau vào cả tập huấn luyện và tập kiểm tra.

## 4. Quy trình kiểm kê

1. Tìm nguồn dữ liệu và xác nhận quyền truy cập.
2. Mở, kiểm tra và xác nhận tệp sử dụng được.
3. Ghi số lượng, điều kiện ghi hình và các lớp đối tượng.
4. Kiểm tra giấy phép trước khi dùng dữ liệu công khai.
5. Chỉ đánh dấu **Đã xác minh** sau khi đã kiểm tra nội dung và quyền sử dụng.
6. Ghi riêng các lớp hoặc điều kiện ghi hình còn thiếu.

## 5. Dữ liệu còn thiếu cần xác nhận

Nội dung nhiệm vụ hiện chưa xác nhận số lượng ảnh/video thực tế, tên bộ dữ liệu công khai đã chọn, giấy phép hay nội dung cụ thể của TASK-S2. P2 cần kiểm tra nguồn thật rồi bổ sung; không tự điền số lượng hoặc ghi dữ liệu chưa kiểm tra là đã có.
