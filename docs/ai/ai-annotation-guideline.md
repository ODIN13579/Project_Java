# HƯỚNG DẪN GÁN NHÃN DỮ LIỆU AI — SWIFTLETCARE

**Nhiệm vụ:** TASK-11 | **Phụ trách:** P2 — AI | **Rà soát:** P3

## 1. Danh sách lớp dự kiến

| Mã lớp dự kiến | Tên lớp | Quy tắc gán nhãn |
|---:|---|---|
| 0 | `swiftlet` — chim yến | Gán nhãn từng con chim yến khi hình ảnh đủ rõ để nhận diện đáng tin cậy. |
| 1 | `snake` — rắn | Gán nhãn rắn nhìn thấy trong vùng camera được thống nhất. |
| 2 | `rat` — chuột | Gán nhãn chuột nhìn thấy trong vùng camera được thống nhất. |
| 3 | `owl` — cú | Gán nhãn cú nhìn thấy trong vùng camera được thống nhất. |

Mã lớp trên là đề xuất ban đầu, cần được nhóm xác nhận và phải thống nhất với tệp `data.yaml`. Không tự thêm lớp `unknown` nếu chưa có định nghĩa và quy tắc gán nhãn riêng.

## 2. Quy tắc vẽ khung bao (bounding box)

- Vẽ một khung bao sát đối tượng cho mỗi đối tượng mục tiêu có thể nhận diện.
- Nếu ảnh có nhiều chim, gán một khung riêng cho từng con khi có thể phân biệt.
- Với đối tượng bị che khuất một phần, chỉ gán nhãn khi vẫn có đủ căn cứ nhận diện và ước lượng vị trí; áp dụng cùng một quy tắc cho toàn bộ dữ liệu.
- Với đối tượng quá nhỏ, nhòe hoặc không chắc chắn, không đoán nhãn. Đánh dấu để rà soát hoặc loại khỏi tập gán nhãn theo quy tắc đã thống nhất.
- Đối tượng ngoài vùng quan tâm của camera phải được xử lý nhất quán: loại khỏi gán nhãn hoặc áp dụng quy tắc riêng đã ghi rõ.
- Giữ lại ảnh không có đối tượng mục tiêu làm mẫu âm tính; dùng đúng cách biểu diễn nhãn rỗng theo công cụ đã chọn.
- Không gán loài chim khác thành `swiftlet` nếu không đủ căn cứ xác định đó là chim yến.

## 3. Gán nhãn video và theo dõi đối tượng

Để đánh giá Entry/Exit, chỉ có khung bao phát hiện đối tượng là chưa đủ. Cần giữ video gốc, thứ tự khung hình, dấu thời gian và mã camera. Với tập dùng đánh giá, cần kiểm tra mã theo dõi của từng đối tượng qua các khung hình và gán nhãn thủ công cho sự kiện vượt đường đếm.

Mỗi sự kiện cần ghi:
- `event_id`, `camera_id`, `zone_id`, `track_id` (chỉ có ý nghĩa trong phiên theo dõi),
- hướng: `ENTRY` (`OUTSIDE_TO_INSIDE`) hoặc `EXIT` (`INSIDE_TO_OUTSIDE`),
- thời gian/số thứ tự khung hình và trạng thái rà soát nhãn.

Mã `track_id` không phải mã định danh vĩnh viễn của một con chim. Nếu chim quay đầu trước khi vượt qua đường đếm thì không tạo sự kiện. Nếu mất dấu hoặc không xác định chắc chắn đối tượng/hướng di chuyển, đánh dấu đoạn không rõ ràng và loại khỏi phép tính độ chính xác sự kiện theo quy tắc đã thống nhất.

## 4. Kiểm tra chất lượng nhãn

1. Người gán nhãn sử dụng danh sách lớp và định dạng đã thống nhất.
2. Người thứ hai kiểm tra một phần dữ liệu, ưu tiên chim nhỏ, đối tượng bị che khuất, động vật săn mồi và các khung hình gần thời điểm vượt đường đếm.
3. Giải quyết bất đồng và lưu quyết định cuối cùng.
4. Kiểm tra lại nhãn khi thay đổi danh sách lớp hoặc quy tắc.
5. Ghi phiên bản nhãn, người gán nhãn, người rà soát và ngày rà soát.

## 5. Các quy tắc cần chốt trước khi gán nhãn hàng loạt

- Mức độ rõ/kích thước tối thiểu để gán nhãn chim yến.
- Cách xử lý che khuất một phần và đối tượng ở sát mép khung hình.
- Ranh giới vùng quan tâm cho từng camera.
- Cách xử lý đối tượng không chắc chắn: bỏ qua, đánh dấu rà soát hay dùng nhãn loại trừ riêng.
- Tỷ lệ kiểm tra chéo và tiêu chí chấp nhận chất lượng nhãn.
