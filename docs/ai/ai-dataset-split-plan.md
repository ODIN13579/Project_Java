# KẾ HOẠCH CHIA BỘ DỮ LIỆU — SWIFTLETCARE

**Nhiệm vụ:** TASK-11 | **Phụ trách:** P2 — AI | **Rà soát:** P3

## 1. Tỷ lệ chia đề xuất

| Tập dữ liệu | Tỷ lệ mục tiêu | Mục đích |
|---|---:|---|
| Huấn luyện (Train) | 70% | Huấn luyện mô hình |
| Xác thực (Validation) | 20% | So sánh cấu hình và điều chỉnh ngưỡng |
| Kiểm tra (Test) | 10% | Đánh giá cuối cùng sau khi đã chốt cấu hình |

Đây là tỷ lệ đề xuất, chưa phải kết quả chia dữ liệu thực tế. Nếu dữ liệu ít hoặc thiếu lớp, cần ghi rõ lý do và thống nhất phương án chia khác trước khi đánh giá.

## 2. Chia theo nguồn video, không chia ngẫu nhiên các khung hình liền kề

Với dữ liệu video, hãy đưa toàn bộ video, phiên ghi hình hoặc nhóm camera/ngày ghi hình vào cùng một tập. Không phân tán ngẫu nhiên các khung hình liền kề của cùng video vào Train, Validation và Test vì chúng gần giống nhau, có thể khiến kết quả đánh giá cao giả tạo.

Nếu nhiều đoạn clip được cắt từ cùng một bản ghi liên tục, nên giữ chúng trong cùng nhóm. Ghi từng phân chia trong `metadata/split_manifest.csv`.

## 3. Kiểm tra độ bao phủ dữ liệu

Trước khi huấn luyện và đánh giá, cần xác nhận:

- Mỗi lớp mục tiêu có mẫu phù hợp trong các tập dùng để đánh giá lớp đó.
- Các điều kiện ngày/đêm, sáng/tối, ánh sáng, góc camera, khoảng cách và che khuất được đại diện nếu dữ liệu có.
- Có mẫu âm tính không chứa đối tượng mục tiêu.
- Tập kiểm tra không có mẫu trùng hoặc gần trùng với tập huấn luyện.
- Nếu một lớp động vật săn mồi không có trong tập kiểm tra, ghi là **chưa thể đánh giá**, không coi như đã đạt.
- Nhãn tập kiểm tra đã được rà soát trước khi đánh giá cuối cùng.

## 4. Ngăn rò rỉ dữ liệu

Không dùng tập Test để lựa chọn mô hình hoặc điều chỉnh ngưỡng. Dùng tập Validation để chọn ngưỡng tin cậy và cấu hình. Nếu phải thay đổi cách chia, cần ghi lý do và cập nhật phiên bản phân chia.

## 5. Thông tin phải bàn giao

Ghi phiên bản dữ liệu, giá trị khởi tạo ngẫu nhiên nếu có, quy tắc gom nhóm, số mẫu theo lớp và từng tập, mã nguồn video, các điểm thiếu, người phê duyệt và ngày phê duyệt. Khi đã có dữ liệu thực tế, cần ghi số lượng ảnh/video cụ thể chứ không chỉ ghi tỷ lệ phần trăm.
