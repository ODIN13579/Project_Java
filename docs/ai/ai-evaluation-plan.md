# KẾ HOẠCH ĐÁNH GIÁ AI — SWIFTLETCARE

**Nhiệm vụ:** TASK-11 | **Phụ trách:** P2 — AI | **Xác nhận nghiệm thu:** P3

## 1. Nguyên tắc đánh giá

Đánh giá riêng ba chức năng: phát hiện chim yến, phát hiện động vật săn mồi và đếm chim Entry/Exit. Chỉ số phát hiện đối tượng của YOLO không chứng minh rằng hệ thống đếm chim chính xác. Cần có dữ liệu chuẩn được rà soát thủ công và tập kiểm tra độc lập. Các mục tiêu dưới đây là mục tiêu đề xuất, chưa phải kết quả đã đạt.

## 2. Chỉ số và mục tiêu đề xuất

| Chức năng | Chỉ số | Mục tiêu/cách báo cáo |
|---|---|---|
| Phát hiện chim yến | Precision | ≥ 85% |
| Phát hiện chim yến | Recall | ≥ 80% |
| Phát hiện động vật săn mồi (`snake`, `rat`, `owl`) | Precision | ≥ 80% |
| Phát hiện động vật săn mồi | Recall | ≥ 75% |
| Chất lượng phát hiện | mAP | Báo cáo mAP@0.5 và nếu hỗ trợ, mAP@0.5:0.95 |
| Đếm chim | Count MAE — sai số tuyệt đối trung bình | Báo cáo sai số trung bình so với số đếm chuẩn; thống nhất ngưỡng nghiệm thu với P3 |
| Sự kiện Entry/Exit | Precision, Recall, F1 của sự kiện; sai số lượt đếm theo clip/phiên | So sánh sự kiện dự đoán với sự kiện chuẩn đã xác minh, dùng khoảng sai lệch thời gian đã thống nhất |
| Gửi cảnh báo động vật săn mồi | Cảnh báo sai, sự kiện bị bỏ sót và độ trễ đầu-cuối | Đo từ lúc đối tượng xuất hiện đến khi Backend nhận sự kiện |
| Hiệu năng chạy | Độ trễ xử lý / FPS | Ghi thiết bị, độ phân giải, tải và phạm vi đo |

## 3. Giải thích chỉ số

- **Precision:** TP / (TP + FP), tỷ lệ dự đoán đúng trong tổng số dự đoán dương tính.
- **Recall:** TP / (TP + FN), tỷ lệ đối tượng thực tế được phát hiện.
- **mAP:** Độ chính xác trung bình của mô hình phát hiện đối tượng; phải ghi rõ cách tính IoU, ví dụ mAP@0.5 hoặc mAP@0.5:0.95.
- **Count MAE:** Trung bình độ chênh lệch tuyệt đối giữa số chim dự đoán và số chim chuẩn trên các khung hình/clip được đánh giá. Phải ghi rõ đơn vị đếm và khoảng lấy mẫu.
- **Đối sánh sự kiện Entry/Exit:** Ghép sự kiện dự đoán với nhãn chuẩn theo hướng và khoảng thời gian cho phép đã định trước. Mỗi sự kiện chuẩn chỉ được ghép với tối đa một dự đoán; báo cáo dự đoán thừa và sự kiện bị bỏ sót.
- **Độ trễ:** Ghi rõ điểm bắt đầu và kết thúc, ví dụ từ lúc bắt đầu xử lý khung hình đến khi Backend nhận sự kiện. Nếu có thể, báo cáo trung vị và các phân vị cao.

## 4. Quy trình đánh giá

1. Chốt phiên bản dữ liệu, danh sách lớp, cách chia tập và phiên bản nhãn.
2. Kiểm tra nhãn chuẩn, đặc biệt với chim nhỏ, đối tượng bị che khuất và lúc vượt đường đếm.
3. Chỉ dùng Validation để chọn ngưỡng và cấu hình.
4. Chạy cấu hình cuối cùng trên Test.
5. Báo cáo Precision, Recall theo lớp, mAP, ví dụ lỗi, Count MAE, chỉ số Entry/Exit, cảnh báo sai và độ trễ.
6. Ghi thiết bị, phiên bản phần mềm/mô hình, độ phân giải, điều kiện ánh sáng và hạn chế.
7. Nếu tập kiểm tra quá nhỏ hoặc thiếu lớp/điều kiện, ghi kết quả là sơ bộ hoặc chưa thể đánh giá; không tuyên bố đã đáp ứng yêu cầu phi chức năng khi chưa kiểm chứng.

## 5. Dữ liệu chuẩn cho Entry/Exit

Dùng video có dấu thời gian và thứ tự khung hình được giữ nguyên. Người rà soát xác định đường đếm và phía trong/ngoài cho từng camera. Chỉ ghi sự kiện sau khi xác nhận chim đã vượt qua đường theo một hướng. Chim quay đầu trước khi vượt không tạo sự kiện. Không suy đoán khi mất dấu hoặc bị che khuất không rõ ràng; áp dụng cùng quy tắc loại trừ cho dự đoán và dữ liệu chuẩn.

## 6. Trạng thái mục tiêu nghiệm thu

Các ngưỡng số trong tài liệu là mục tiêu thiết kế đề xuất từ TASK-11. P3 cần xác nhận định nghĩa chỉ số, điều kiện kiểm tra và ngưỡng nghiệm thu. Chỉ ghi kết quả đo thực tế sau khi đã chạy đánh giá.
