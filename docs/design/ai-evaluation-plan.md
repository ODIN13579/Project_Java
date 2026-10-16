# AI Evaluation Plan — SwiftletCare

## 1. Mục tiêu đánh giá

| Chức năng | Precision | Recall |
|---|---:|---:|
| Phát hiện/đếm chim | ≥ 85% | ≥ 80% |
| Phát hiện thú săn mồi | ≥ 80% | ≥ 75% |

Các giá trị trên là mục tiêu thiết kế, chưa phải kết quả thực nghiệm.

## 2. Dữ liệu kiểm thử

- Sử dụng video có nhãn thủ công làm dữ liệu chuẩn (ground truth).
- Tách video kiểm thử khỏi dữ liệu huấn luyện và điều chỉnh mô hình.
- Chia dữ liệu theo video hoặc phiên ghi hình.

## 3. Phương pháp đánh giá

- Tính Precision và Recall cho phát hiện đối tượng.
- So sánh số lượt chim ra/vào của AI với nhãn thủ công, tính sai số MAE.
- Ghi nhận số lần phát hiện đúng, bỏ sót và cảnh báo nhầm thú săn mồi.
- Đo độ trễ xử lý và ghi lại cấu hình phần cứng.

## 4. Kết quả

Ghi nhận kết quả thực tế sau khi chạy kiểm thử và so sánh với các mục tiêu đã đề ra.
