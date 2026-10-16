# ĐỊNH DẠNG BỘ DỮ LIỆU AI — SWIFTLETCARE

**Nhiệm vụ:** TASK-11  
**Mục đích:** Thống nhất cách lưu trữ dữ liệu phục vụ huấn luyện và đánh giá AI.

## 1. Cấu trúc thư mục đề xuất

```text
ai-dataset/
├── images/
│   ├── train/
│   ├── val/
│   └── test/
├── labels/
│   ├── train/
│   ├── val/
│   └── test/
├── videos/
│   └── original/          # Video gốc để đánh giá theo thời gian
├── annotations/
│   ├── tracking/          # Mã theo dõi đối tượng theo từng khung hình
│   └── events/             # Sự kiện Entry/Exit đã xác minh
├── metadata/
│   ├── sources.csv
│   └── split_manifest.csv
└── data.yaml
```

Giữ nguyên video gốc, không chỉnh sửa. Với ảnh tách từ video, lưu mã video nguồn và số thứ tự khung hình trong phần thông tin dữ liệu. Không chia các khung hình liền kề của cùng video sang các tập khác nhau.

## 2. Định dạng nhãn phát hiện đối tượng YOLO

Mỗi ảnh có một tệp nhãn văn bản cùng tên, mỗi dòng tương ứng một đối tượng:

```text
class_id x_center y_center width height
```

Các tọa độ được chuẩn hóa trong khoảng từ 0 đến 1 theo chiều rộng và chiều cao ảnh. Ví dụ minh họa cú pháp:

```text
0 0.52 0.44 0.12 0.18
```

Đây chỉ là ví dụ định dạng, không phải nhãn của ảnh thực tế. Ảnh không có đối tượng mục tiêu dùng tệp nhãn rỗng nếu công cụ YOLO được chọn yêu cầu cách lưu này.

## 3. Mẫu cấu hình `data.yaml`

```yaml
path: ./ai-dataset
train: images/train
val: images/val
test: images/test

names:
  0: swiftlet
  1: snake
  2: rat
  3: owl
```

Đây là mẫu cấu hình ban đầu. Cần sửa `path` theo vị trí bộ dữ liệu thực tế và bảo đảm mã lớp trùng khớp với hướng dẫn gán nhãn đã được duyệt. Danh sách lớp cần được xác nhận theo yêu cầu dự án và dữ liệu thực tế.

## 4. Dữ liệu đánh giá theo thời gian

Nhãn ảnh YOLO không đủ để đánh giá theo dõi đối tượng và đếm Entry/Exit. Với tập video đánh giá, cần giữ:

- Mã video nguồn, mã camera, mã khu vực, số thứ tự khung hình và dấu thời gian.
- Khung bao đối tượng chuẩn và mã theo dõi nhất quán qua các khung hình.
- Định nghĩa đường đếm (hai điểm đầu mút) và quy ước phía `INSIDE`/`OUTSIDE`.
- Sự kiện Entry/Exit được xác minh thủ công và các đoạn không rõ ràng/bị loại trừ.
- Sự kiện động vật săn mồi chuẩn và tham chiếu khung hình bằng chứng nếu có.

Định dạng lưu nhãn theo dõi/sự kiện cần được ghi rõ và quản lý phiên bản.

## 5. Quy tắc đặt tên và truy xuất

Dùng tên tệp duy nhất, ổn định. Không xem tên tệp là định danh vĩnh viễn của một con chim. Duy trì `sources.csv` và `split_manifest.csv` để truy xuất nguồn gốc và tập dữ liệu của từng mẫu.
