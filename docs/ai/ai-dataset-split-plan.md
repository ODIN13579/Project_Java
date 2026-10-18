# AI Dataset Split Plan — SwiftletCare

## 1. Mục tiêu

Tạo bộ test độc lập phản ánh dữ liệu triển khai, tránh kết quả cao giả tạo do các frame gần như giống nhau hoặc cùng một clip xuất hiện ở nhiều tập.

## 2. Tỷ lệ đề xuất

Khởi đầu với **70% train / 20% validation / 10% test theo nhóm nguồn**, không chia ngẫu nhiên từng frame. Đây là tỷ lệ khởi điểm, điều chỉnh theo số video/session thực tế và phân bố class.

| Tập | Dùng để làm gì | Quy tắc |
|---|---|---|
| Train | Huấn luyện và augmentation | Có thể dùng dữ liệu công khai đã được license review và dữ liệu đã duyệt |
| Validation | Chọn model, threshold và cấu hình | Không dùng để báo cáo kết quả test cuối |
| Test | Đánh giá cuối cùng | Khóa labels; không dùng chọn threshold/model |

## 3. Đơn vị chia

Ưu tiên chia theo thứ tự:
1. Video/session ghi hình gốc.
2. Nếu camera và thời gian có thể gây tương quan mạnh: chia theo `camera_id` hoặc ngày/phiên ghi hình.
3. Với dữ liệu công khai dạng ảnh không có video nguồn, nhóm theo nguồn/tác giả/chuỗi ảnh gần trùng trước khi chia.

Tất cả frame trích từ cùng một video phải thuộc cùng một split. Không lấy frame kề nhau từ cùng clip đưa vào train và test. Dùng perceptual hash và `sha256` để phát hiện ảnh trùng/near-duplicate.

## 4. Phân bố class và điều kiện

Trước khi khóa split, lập bảng số ảnh, box và số video có từng class theo split. Cần kiểm tra:
- `swiftlet`: kích thước nhỏ/lớn, mật độ đàn, bay nhanh, che khuất, ngày/đêm.
- `snake`, `rat`, `owl`: số đối tượng và số clip độc lập; class hiếm không được đánh giá chỉ dựa trên vài frame gần giống.
- Negative samples: cảnh không có đối tượng mục tiêu, bóng, côn trùng, vật thể dễ nhầm.
- Entry/Exit: đủ lượt ENTRY/EXIT, các trường hợp nhiều chim qua cùng lúc, track mất, quay lại, dừng gần đường đếm.

Không nhân bản một ảnh vào nhiều split để bù class. Nếu class không có đủ ví dụ độc lập trong test, ghi rõ chưa đủ bằng chứng để kết luận.

## 5. Bộ test hai tầng

- **Public/mixed development set:** hỗ trợ kiểm tra pipeline và debug, nhưng không chứng minh khả năng tổng quát hóa tại nhà yến.
- **Farm-camera holdout test:** video thực tế từ camera nhà yến, từ session/camera không xuất hiện trong train/validation. Đây là bộ ưu tiên để nghiệm thu triển khai.

Nếu chưa có video thực tế được cho phép, đánh giá Entry/Exit chỉ được báo cáo là thử nghiệm sơ bộ trên video sẵn có, không tuyên bố đã đạt NFR.

## 6. Split manifest

Mỗi hàng trong `manifests/split_manifest.csv` cần `asset_id, sha256, source_id, capture_group_id, camera_id, date_group, split, dataset_version`. Lưu seed nếu có thao tác ngẫu nhiên, nhưng seed không thay thế quy tắc nhóm theo nguồn.

## 7. Trước khi khóa

- [ ] Không có hash trùng giữa train/val/test.
- [ ] Không có frame từ cùng clip ở nhiều split.
- [ ] Class mapping thống nhất.
- [ ] Phân bố điều kiện và class đã được xem xét.
- [ ] Test labels được review độc lập và đóng băng.
- [ ] Số lượng clip/ảnh/box/class thực tế được ghi trong inventory.
