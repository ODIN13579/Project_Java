# CÁC QUYẾT ĐỊNH CÒN MỞ — TASK-11 SWIFTLETCARE

**Mục đích:** Theo dõi các nội dung cần thống nhất trước khi xem đặc tả dữ liệu và kế hoạch đánh giá là hoàn tất.

| Mã | Nội dung cần quyết định | Người phối hợp | Kết quả cần có |
|---|---|---|---|
| D-01 | Video nhà yến thực tế và video TASK-S2 nào có sẵn, được phép sử dụng? | P2 và nhóm | Danh mục đã kiểm tra, số lượng, đường dẫn và quyền sử dụng |
| D-02 | Chọn bộ dữ liệu công khai/trọng số mô hình nào, giấy phép cho phép những gì? | P2 | Đường dẫn, phiên bản, giấy phép và hạn chế |
| D-03 | Danh sách lớp cuối cùng có phải `swiftlet`, `snake`, `rat`, `owl` không? Có cần lớp chim khác hoặc quy tắc bỏ qua không? | P2 + P3 | Danh sách lớp được duyệt |
| D-04 | Quy tắc về kích thước, che khuất, vùng quan tâm và đối tượng không rõ là gì? | P2 + P3 | Hướng dẫn gán nhãn được duyệt |
| D-05 | Đường đếm và quy ước phía trong/ngoài của từng camera là gì? | P2 + P3 | Cấu hình đường đếm và quy tắc gán nhãn sự kiện |
| D-06 | Dữ liệu có bao nhiêu mẫu và các lớp/điều kiện có đủ trong Validation và Test không? | P2 | Thống kê theo lớp, nguồn và tập dữ liệu |
| D-07 | Tỷ lệ chia 70/20/10 có phù hợp với dữ liệu thực tế không? | P2 | Kế hoạch chia và danh sách phân chia được duyệt |
| D-08 | Khoảng thời gian ghép sự kiện và ngưỡng Count MAE nghiệm thu là bao nhiêu? | P2 + P3 | Quy tắc nghiệm thu định lượng |
| D-09 | Thiết bị, độ phân giải, FPS/độ trễ và điều kiện kiểm tra nào được dùng để nghiệm thu? | P2 + P3 | Quy trình đánh giá có thể lặp lại |
| D-10 | Tên trường sự kiện, giao thức truyền, nơi lưu bằng chứng, gửi lại và chống trùng ra sao? | P2 + P4 | Quy ước dữ liệu sự kiện được Backend xác nhận |
| D-11 | Khi mất mạng, nút biên lưu tạm và đồng bộ sự kiện thế nào để không bị trùng? | P2 + P4 | Quy tắc hoạt động ngoại tuyến và đồng bộ |
| D-12 | Quy trình rà soát nhãn và tỷ lệ bất đồng cho phép là bao nhiêu? | P2 + P3 | Quy trình kiểm soát chất lượng được thống nhất |

## Điều kiện sẵn sàng chuyển sang giai đoạn BUILD

- Nguồn dữ liệu dự kiến đã được kiểm tra và xác nhận quyền sử dụng.
- Danh sách lớp và quy tắc gán nhãn được thống nhất.
- Định dạng dữ liệu và cách chia tập đã được ghi rõ.
- Chỉ số đánh giá, cách tạo dữ liệu chuẩn và ngưỡng nghiệm thu được thống nhất.
- Trường dữ liệu AI gửi Backend cùng quy tắc gửi lại/chống trùng đã được rà soát.
- Dữ liệu còn thiếu và quyết định chưa chốt được ghi rõ; không báo cáo mục tiêu chưa đo là kết quả đã đạt.
