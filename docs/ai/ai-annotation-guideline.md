# AI Annotation Guideline — SwiftletCare

## 1. Mục tiêu

Chuẩn hóa nhãn cho object detection và ground truth theo chuỗi thời gian để đánh giá tracking, đếm chim và phát hiện thiên địch. Quy tắc này là bản đề xuất; cần nhóm xác nhận trước khi gán nhãn hàng loạt.

## 2. Danh sách class đề xuất

| Class ID | Tên class | Định nghĩa / lưu ý |
|---:|---|---|
| 0 | `swiftlet` | Chim yến được nhận diện đủ rõ trong ảnh/video. Không tự gán cho chim không xác định được loài. |
| 1 | `snake` | Rắn nhìn thấy trong vùng camera. |
| 2 | `rat` | Chuột nhìn thấy trong vùng camera. |
| 3 | `owl` | Cú nhìn thấy trong vùng camera. |

ID chỉ là đề xuất cho version dataset này. Nếu dữ liệu chỉ xác định được `bird` mà không chắc là chim yến, không tự chuyển `bird` thành `swiftlet`. Giữ dataset nguồn riêng hoặc xác định mapping được phê duyệt. Không thêm class `unknown` vào model nếu chưa thống nhất mục tiêu và cách đánh giá.

## 3. Bounding box

- Mỗi đối tượng mục tiêu nhìn thấy được gán một box riêng.
- Box bao sát phần đối tượng nhìn thấy; không bao gồm vùng nền quá mức.
- Đối tượng bị che khuất một phần: gán box nếu vẫn nhận diện được class và có thể xác định vị trí đáng tin cậy; ghi `occluded=true` trong metadata/annotation phụ.
- Đối tượng quá nhỏ hoặc mờ đến mức không thể xác định class: không đoán nhãn. Đánh dấu `ignore` trong bảng QA/metadata nếu công cụ cho phép; ghi lý do.
- Đối tượng nằm ngoài vùng quan tâm (ROI) của cửa đếm: vẫn có thể gán nhãn cho bài toán detection nếu thuộc frame và class mục tiêu; `in_roi=false`. Chỉ dùng đối tượng trong ROI để tính Entry/Exit.
- Ảnh không có đối tượng mục tiêu là negative sample hợp lệ: giữ ảnh và file nhãn rỗng.
- Không tạo box cho bóng, hình phản chiếu, vật thể giả dạng chim hoặc vật thể không có bằng chứng rõ ràng.
- Không gộp nhiều con chim vào một box; mỗi con là một instance riêng nếu phân biệt được.
- Không tự gán class `swiftlet` cho các chấm chuyển động ở xa nếu bằng chứng hình ảnh không đủ.

## 4. Quy tắc cho video và tracking

Với mỗi video được dùng đánh giá Entry/Exit, gán nhãn theo frame được chọn và giữ timestamp/frame index:
- `video_id`, `frame_index`, `timestamp_ms`, `camera_id`, `zone_id`.
- `object_id`/`track_id` ground truth nhất quán cho cùng cá thể trong đoạn có thể theo dõi.
- `class_name`, bounding box, `occluded`, `ignore`, `in_roi`.
- Định nghĩa đường đếm bằng hai điểm `P1(x1,y1)` và `P2(x2,y2)` trên hệ tọa độ frame đã chuẩn hóa; lưu rõ phía `INSIDE` và `OUTSIDE`.
- Gán `crossing_event` khi tâm box đi qua đường đếm và hướng chuyển động có đủ bằng chứng: `ENTRY` (OUTSIDE → INSIDE), `EXIT` (INSIDE → OUTSIDE).
- Một lượt qua đường chỉ tạo một ground-truth event. Không tạo event mới chỉ vì track bị mất rồi xuất hiện lại nếu bằng chứng cho thấy cùng lượt/cá thể.
- Nếu hướng không xác định, nhiều chim chồng lấp hoặc video bị gián đoạn khiến không thể kết luận, đánh dấu `ambiguous` và loại khỏi phép tính chính; báo cáo số lượng trường hợp loại trừ.
- Track ID ground truth chỉ duy nhất trong phạm vi clip/session, không coi là ID chim vĩnh viễn.

## 5. Gán nhãn bất đồng và kiểm tra chất lượng

1. Người gán nhãn A tạo nhãn; người B rà soát toàn bộ test và tối thiểu 10% train/val.
2. Mọi bất đồng class, box hoặc hướng Entry/Exit được giải quyết bởi người rà soát; lưu phiên bản sau sửa.
3. Dùng kiểm tra tự động: box nằm trong ảnh, width/height dương, class hợp lệ, frame index tăng đúng, track ID nhất quán.
4. Ghi `annotator`, `reviewer`, `annotation_version`, `review_status`.
5. Đóng băng test labels trước khi chạy đánh giá cuối; không sửa test theo dự đoán của mô hình.

## 6. Quy tắc chất lượng tối thiểu

- Không có box ngoài khung ảnh hoặc kích thước bằng 0.
- Mọi ảnh có file nhãn tương ứng; ảnh negative có file nhãn rỗng.
- Tên class và ID phải khớp `data.yaml`.
- Video Entry/Exit phải có đường đếm/ROI và hướng INSIDE/OUTSIDE được xác định.
- Test set cần có trường hợp khó: chim nhỏ, bay nhanh, che khuất, ánh sáng yếu, đông chim và cảnh không có chim; thiên địch hiếm phải được báo cáo riêng.
