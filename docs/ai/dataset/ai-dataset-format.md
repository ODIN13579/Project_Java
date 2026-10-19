# AI Dataset Format — SwiftletCare

## 1. Cấu trúc thư mục

```text
docs/
└── ai/
    ├── ai-data-inventory.md
    ├── ai-annotation-guideline.md
    ├── ai-dataset-format.md
    ├── ai-dataset-split-plan.md
    ├── ai-evaluation-plan.md
    ├── ai-event-data-contract.md
    └── ai-open-decisions.md

ai-dataset/
├── README.md
├── data.yaml
├── images/
│   ├── train/
│   ├── val/
│   └── test/
├── labels/
│   ├── train/
│   ├── val/
│   └── test/
├── manifests/
│   ├── assets.csv
│   ├── groups.csv
│   └── split_manifest.csv
├── videos/
│   ├── raw/                 # không commit video lớn/nhạy cảm nếu chính sách không cho phép
│   └── evaluation_clips/
├── annotations/
│   ├── tracking_ground_truth.jsonl
│   └── crossing_events.csv
├── metadata/
│   └── image_metadata.csv
└── reports/
    └── quality_checks/
```

Raw assets nên lưu ở kho dữ liệu có phân quyền hoặc Git LFS/object storage theo chính sách nhóm; repository có thể chỉ lưu manifest, tài liệu và script kiểm tra. Không commit video riêng tư hoặc dữ liệu chưa rõ quyền sử dụng.

## 2. Định dạng nhãn YOLO

Mỗi ảnh có file `.txt` cùng tên. Mỗi dòng:

```text
class_id x_center y_center width height
```

Tọa độ được chuẩn hóa theo chiều rộng/cao ảnh, trong khoảng 0–1. Ví dụ cú pháp (không phải nhãn ảnh thật):

```text
0 0.52 0.44 0.12 0.18
```

`data.yaml` mẫu:

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

Giữ mapping class cố định trong cùng một dataset version. Nếu nguồn ngoài dùng `bird`, không đổi thành `swiftlet` nếu chưa được xác minh.

## 3. Metadata ảnh — `image_metadata.csv`

Các cột đề xuất:

| Cột | Ý nghĩa |
|---|---|
| `asset_id` | ID nội bộ duy nhất |
| `relative_path` | Đường dẫn tương đối, không chứa credential |
| `sha256` | Hash để kiểm tra trùng |
| `source_id` | Tham chiếu `ai-data-inventory.md` |
| `source_url` | URL nguồn nếu được phép lưu |
| `license` | Giấy phép đã kiểm tra |
| `capture_group_id` | Nhóm clip/session/camera-ngày dùng chia tập |
| `camera_id`, `zone_id` | ID camera/khu vực nếu có |
| `captured_at` | Timestamp có timezone nếu biết |
| `frame_index`, `timestamp_ms` | Bắt buộc khi ảnh trích từ video |
| `width`, `height`, `fps` | Thông số media; fps áp dụng video |
| `lighting` | `day`, `night`, `low_light`, `unknown` |
| `quality_flags` | Ví dụ `blur`, `occluded`, `small_object` |
| `split` | `train`, `val`, `test`, `unassigned` |
| `annotation_version`, `review_status` | Quản lý chất lượng nhãn |

Không suy diễn metadata không biết; dùng `unknown`/để trống và ghi lý do.

## 4. Ground truth tracking — JSONL

Mỗi dòng là một đối tượng ở một frame:

```json
{
  "video_id": "clip-001",
  "frame_index": 120,
  "timestamp_ms": 4000,
  "camera_id": "camera-01",
  "zone_id": "zone-01",
  "object_id": "gt-track-07",
  "class_name": "swiftlet",
  "bbox_xyxy": [120, 80, 156, 118],
  "occluded": false,
  "in_roi": true,
  "annotation_status": "reviewed"
}
```

Đây là schema minh họa. `object_id` là ID ground truth trong clip/session, không phải định danh sinh học vĩnh viễn.

## 5. Ground truth crossing event — CSV

Cột đề xuất: `gt_event_id, video_id, camera_id, zone_id, timestamp_ms, frame_index, object_id, direction, line_id, status, reviewer`.

`direction` chỉ nhận `ENTRY` hoặc `EXIT`; trường hợp mơ hồ được ghi `AMBIGUOUS` trong dữ liệu annotation nhưng loại khỏi phép tính chính và báo cáo riêng.

## 6. Quản lý phiên bản và chất lượng

- Mỗi bản dataset có `dataset_version`, ngày tạo, nguồn, license, class map và commit/hash của manifest.
- Lưu raw data bất biến; annotation và split manifest có version.
- Chạy kiểm tra: file thiếu, hash trùng, nhãn lỗi, class ID không hợp lệ, box ngoài biên, video/ảnh trùng giữa split.
- Augmentation chỉ áp dụng trên train; không augment val/test.
- Test labels và split manifest phải được khóa trước đánh giá cuối.
