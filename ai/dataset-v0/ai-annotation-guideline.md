# Quy tắc gán nhãn AI Dataset v0

## Class ID
- `0`: swiftlet — chim yến.
- `1`: snake — rắn.
- `2`: rat — chuột.
- `3`: owl — cú.

## Định dạng YOLO detection
Mỗi ảnh có một file `.txt` cùng tên. Mỗi dòng:
`class_id x_center y_center width height`

Tọa độ bounding box được chuẩn hóa về [0, 1] theo chiều rộng/cao ảnh. Mỗi đối tượng nhìn thấy rõ có một box. Không gán box cho đối tượng không thể xác định đáng tin cậy; ghi trường hợp mơ hồ vào nhật ký annotation để review. Không tạo file nhãn giả chỉ để đủ số lượng.

## Kiểm tra chất lượng
1. So khớp số ảnh và nhãn.
2. Kiểm tra class ID chỉ thuộc 0–3.
3. Kiểm tra tọa độ nằm trong [0,1], width/height > 0.
4. Review chéo một phần dữ liệu bởi người gán nhãn thứ hai.
5. Giữ nguyên nhóm video/session/camera khi chia train/val/test để tránh frame gần trùng lọt qua các tập.

## Trạng thái
Hướng dẫn mới là quy tắc dự kiến. Dataset trong ZIP chưa có ảnh và nhãn đã gán.
