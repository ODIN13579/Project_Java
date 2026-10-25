# Báo cáo thử nghiệm baseline

## Cấu hình
- Model: `yolov8n.pt` pretrained trên COCO.
- Class filter: COCO class ID 14 (`bird`).
- Tracking: ByteTrack qua `model.track(..., persist=True)`.
- Đếm: tâm bounding box vượt đường P1-P2, theo tracking ID, hysteresis và cooldown.
- Backend: POST CountLog aggregate theo API contract Spring Boot.

## Kiểm thử code
- Unit tests logic Entry/Exit: 6 tests passed trong môi trường kiểm thử của bản chỉnh sửa.
- Đây không phải đánh giá độ chính xác nhận diện trên dataset.

## Metrics nhận diện
Precision, Recall, mAP50/mAP50-95 chưa có: thiếu tập test gán nhãn và ground truth SwiftletCare.

## Giới hạn
- COCO `bird` không phân biệt riêng chim yến.
- Chưa fine-tune mô hình.
- Cần xác minh nguồn/license của video mẫu.
- Cần xác nhận đường đếm và hướng inside cho từng camera.
- Chưa thực hiện end-to-end POST thực tế tới Spring Boot trong môi trường người dùng; cần chạy smoke test với backend đang hoạt động và `deviceId`/`AI_SERVICE_KEY` hợp lệ.
