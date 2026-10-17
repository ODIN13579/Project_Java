# AI Data Inventory — SwiftletCare

> **Task:** TASK-11 — AI Dataset Specification  
> **Owner:** P2 — AI | **Phối hợp:** P3 — FR/NFR, P4 — Backend  
> **Trạng thái:** Draft — nguồn công khai đã tìm thấy; chưa xác nhận dữ liệu thực tế của nhóm.

## 1. Mục đích

Kiểm kê dữ liệu cho ba nhánh: (A) phát hiện chim yến, (B) phát hiện thiên địch, (C) tracking và đếm Entry/Exit. Không coi nguồn là dữ liệu đã được nhóm tải, kiểm tra hoặc chấp thuận sử dụng cho đến khi hoàn thành xác minh.

## 2. Nguồn dữ liệu

| ID | Nguồn | Loại / số lượng được công bố | Class liên quan | Điều kiện / hạn chế | Giấy phép và trạng thái |
|---|---|---|---|---|---|
| SRC-01 | [Bird Detection Dataset — GitHub](https://github.com/zhizhen777/bird-detection-yolo-dataset) | 2.754 ảnh đã gán nhãn YOLO; 2.203 train, 551 val; 2.745 box; 12 ảnh nền âm tính theo README | `bird` | Chim nói chung; không xác nhận là chim yến, không thay thế video camera tại cửa nhà yến; cần kiểm tra nguồn gốc và phân bố ảnh | README công bố CC BY 4.0; cần giữ attribution/citation. **Chưa tải/kiểm tra độc lập** |
| SRC-02 | [Snake and Rat — Roboflow Universe](https://universe.roboflow.com/ncut-hjziq/snake-and-rat-3s7ef) | Trang công bố 800 ảnh; có 3 phiên bản dataset | `snake`, `rat` | Trang hiển thị số lượng ảnh và phiên bản; cần chọn version cụ thể, tải xuống, kiểm tra nhãn và trùng lặp | Trang công bố CC BY 4.0; xác minh điều khoản của version được tải. **Chưa tải/kiểm tra độc lập** |
| SRC-03 | [Owl Dataset — Roboflow Universe](https://universe.roboflow.com/group-7-cao/owl-48d3k) | Trang công bố 547 ảnh, 2 versions | `owl` | Cần xác minh ảnh thật, góc nhìn, kích thước và mức phù hợp camera nhà yến | Trang công bố CC BY 4.0; xác minh điều khoản của version được tải. **Chưa tải/kiểm tra độc lập** |
| SRC-04 | Video thử nghiệm TASK-S2 | Chưa có số lượng/tên file trong tài liệu TASK-11 | Tùy nội dung | Cần P2/P3 xác nhận đường dẫn, thời lượng, FPS, camera, điều kiện quay và ground truth | Quyền sử dụng nội bộ cần được xác nhận; **chưa kiểm kê** |
| SRC-05 | Video thực tế từ camera nhà yến | Chưa xác nhận có | `swiftlet`, có thể `snake`, `rat`, `owl` | Nguồn quan trọng nhất để đánh giá domain thực tế: cửa ra/vào, ngày/đêm, ánh sáng yếu, motion blur, che khuất | Cần chủ sở hữu cho phép thu thập/lưu trữ và quy định bảo mật; **chưa xác nhận** |
| SRC-06 | Ảnh/video tự thu thập hoặc mô phỏng | Chưa có số liệu | Các class mục tiêu | Có thể bổ sung cảnh hiếm và negative samples; không dùng dữ liệu mô phỏng để thay thế hoàn toàn test thực tế | Lưu nguồn, ngày thu thập, quyền sử dụng và cách tạo dữ liệu |

**Lưu ý:** số lượng là số liệu hiển thị tại trang nguồn khi khảo sát, không phải số lượng đã tải vào repository. Chúng có thể thay đổi theo version. Các dataset rời rạc không tự cung cấp ground truth Entry/Exit theo chuỗi thời gian.

## 3. Quy trình xác minh trước khi nhập dataset

Với từng nguồn, ghi lại:
- URL, tác giả/chủ sở hữu, ngày truy cập, version/release và giấy phép.
- Ngày tải, số ảnh/video thực tế, dung lượng, độ phân giải, FPS/thời lượng nếu là video.
- Danh sách class, số lượng box theo class, số ảnh không có đối tượng, số nhãn lỗi.
- Điều kiện quay: ban ngày/ban đêm, ánh sáng, góc nhìn, khoảng cách, che khuất, camera cố định/di động.
- Hash file để phát hiện trùng; nhóm video/session gốc để ngăn rò rỉ giữa các tập.
- Quyền sử dụng, yêu cầu ghi công, điều kiện chia sẻ lại và mọi hạn chế.
- Trạng thái: `DISCOVERED` → `DOWNLOADED` → `LICENSE_REVIEWED` → `QUALITY_CHECKED` → `APPROVED`.

Không đưa dữ liệu vào train/val/test chính thức khi chưa kiểm tra giấy phép và chất lượng. Không coi model metrics được hiển thị trên trang cộng đồng là kết quả của SwiftletCare.

## 4. Kết luận hiện tại

Chưa có bằng chứng trong tài liệu nhiệm vụ rằng nhóm đã có dataset thực tế từ nhà yến hoặc video TASK-S2 đã được kiểm kê. Có thể dùng SRC-01 làm dữ liệu khởi đầu để thử quy trình phát hiện chim nói chung và SRC-02/SRC-03 để khảo sát thiên địch. Cần bổ sung dữ liệu thực tế từ camera nhà yến trước khi tuyên bố hiệu năng phù hợp môi trường triển khai.

## 5. Việc cần làm tiếp

1. P2: tải đúng version, lưu README/LICENSE/citation và ghi số lượng thực tế.
2. P3/P2: xác nhận video TASK-S2 và yêu cầu FR/NFR liên quan.
3. P4: xác nhận chính sách lưu evidence, retention và quyền truy cập.
4. Nhóm: bổ sung video nhà yến có sự cho phép, đặc biệt là các lượt Entry/Exit đã được gán nhãn thủ công.
