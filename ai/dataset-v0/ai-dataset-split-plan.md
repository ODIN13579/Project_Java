# Kế hoạch chia Dataset v0

Tỷ lệ mục tiêu: train 70%, validation 20%, test 10%.

- Chia theo video/session/camera-day trước, không chia ngẫu nhiên từng frame, để tránh rò rỉ dữ liệu gần trùng.
- Tập train dùng huấn luyện; validation dùng chọn cấu hình/threshold; test chỉ dùng đánh giá cuối.
- Đảm bảo mỗi lớp có mẫu ở các tập nếu dữ liệu thực tế cho phép; ghi rõ lớp thiếu mẫu.
- Cố định random seed và lưu danh sách file của mỗi split.
- Nếu dữ liệu ít, báo cáo số lượng theo lớp và cảnh báo độ tin cậy thấp.

## Trạng thái hiện tại
Chưa thể chia tập thực tế vì chưa có ảnh đã thu thập và gán nhãn. Tỷ lệ trên là kế hoạch, không phải kết quả split đã chạy.
