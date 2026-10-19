# AI Evaluation Plan — SwiftletCare

## 1. Phạm vi

Đánh giá riêng: (A) phát hiện chim yến, (B) phát hiện thiên địch, (C) tracking/Entry-Exit và (D) latency từ xử lý đến Backend. Kết quả phải đo trên test độc lập; không suy ra hiệu năng từ tên kiến trúc hoặc số liệu model/dataset công khai.

## 2. Ground truth

- **Detection:** bounding box và class được gán nhãn theo `ai-annotation-guideline.md`; test labels đã review.
- **Entry/Exit:** người gán nhãn xác định từng lượt qua đường đếm, timestamp/frame, camera/zone, direction và trường hợp ambiguous. Hai người rà soát các đoạn test quan trọng.
- **Latency:** đồng bộ timestamp trên thiết bị xử lý và Backend; ghi lại thời điểm frame được nhận, inference xong, event được tạo/gửi và Backend nhận.
- Báo cáo số lượng ảnh, video, box, event, camera/session và trường hợp bị loại khỏi đánh giá.

## 3. Chỉ số detection

Với IoU threshold đã thống nhất, dự đoán được ghép với ground truth cùng class. Một dự đoán đúng (TP) phải vượt IoU threshold và được ghép một-một; dự đoán dư là FP; ground truth bị bỏ sót là FN.

- `Precision = TP / (TP + FP)`
- `Recall = TP / (TP + FN)`
- `mAP@0.5`: mean Average Precision với IoU = 0.50.
- `mAP@0.5:0.95`: mAP trung bình trên IoU 0.50 đến 0.95, bước 0.05.

Báo cáo theo từng class và overall; ghi confidence threshold, IoU matching, image size, model version, hardware và điều kiện chạy. Nếu mẫu số bằng 0, ghi N/A, không thay bằng 0 tùy ý.

## 4. Mục tiêu dự kiến — chưa phải kết quả

| Hạng mục | Mục tiêu dự kiến |
|---|---:|
| Bird / swiftlet detection Precision | ≥ 85% |
| Bird / swiftlet detection Recall | ≥ 80% |
| Predator detection Precision | ≥ 80% |
| Predator detection Recall | ≥ 75% |

Các giá trị trên là mục tiêu thiết kế từ đề bài TASK-11, chưa phải kết quả đo. Cần chốt test set, protocol và ngưỡng trước nghiệm thu. mAP là chỉ số bổ sung; không tự đặt mức đạt nếu nhóm chưa thống nhất.

## 5. Đánh giá đếm chim và Entry/Exit

### Count MAE

Tính trên các cửa sổ thời gian hoặc clip được định nghĩa trước:

`Count MAE = mean(|predicted_count - ground_truth_count|)`

Báo cáo riêng số chim theo từng hướng (ENTRY, EXIT), theo clip/camera và tổng hợp. Ghi độ dài cửa sổ; không so sánh các cửa sổ có độ dài khác nhau mà không chuẩn hóa.

### Entry/Exit event metrics

So khớp event dự đoán với ground truth cùng `camera_id`, `zone_id`, `direction` trong cửa sổ thời gian `±Δt` được chốt trước khi test:
- Event Precision = số event dự đoán ghép đúng / tổng event dự đoán.
- Event Recall = số event ground truth được ghép / tổng event ground truth.
- Event F1 = harmonic mean của precision và recall.
- Count MAE theo ENTRY/EXIT và tổng số lượt.
- Báo cáo số event trùng, bỏ sót, sai hướng, event ngoài cửa sổ thời gian và clip ambiguous.

“Entry/Exit accuracy” không nên chỉ là phần trăm tổng số đếm đúng. Báo cáo tối thiểu event precision/recall/F1 và count MAE, đồng thời định nghĩa rõ phép ghép event và `Δt`.

### Tracking

Có thể bổ sung IDF1/HOTA hoặc ID switches nếu công cụ và ground truth cho phép. Track ID nội bộ không phải ID chim vĩnh viễn. Không đánh giá logic đếm chỉ bằng mAP detection.

## 6. Đánh giá cảnh báo thiên địch

Ngoài Precision/Recall theo class, ghi:
- False alerts trên mỗi giờ video không có thiên địch.
- Thời gian từ lúc đối tượng xuất hiện trong ground truth đến khi Backend nhận event.
- Số trường hợp bỏ sót, cảnh báo trùng và cảnh báo sai class.
- Kết quả riêng cho ngày/đêm, che khuất, vật thể nhỏ và từng camera.

## 7. Latency và NFR

NFR trong đề xuất: dữ liệu sensor hiển thị trong 2 giây; cảnh báo/đếm từ camera đến thiết bị trong khoảng 3–5 giây. Đây là mục tiêu hệ thống cần kiểm chứng, không phải mặc định đạt được.

Định nghĩa:
- `T0`: timestamp frame được nhận tại node xử lý.
- `T1`: inference/tracking hoàn tất.
- `T2`: event được publish.
- `T3`: Backend nhận và xác thực event.
- `T4`: Dashboard/mobile nhận được cập nhật (nếu có telemetry).

Báo cáo `T3-T0` và `T4-T0` bằng p50, p95, p99 và tỷ lệ event trong ngưỡng. Đo trên hardware mục tiêu, resolution/FPS đã chốt, tải mạng thực tế; nêu rõ thời gian warm-up, số lần chạy, mất mạng và retry. Kiểm tra edge vẫn điều khiển tại chỗ khi mất Internet theo NFR, nhưng đây là kiểm thử tích hợp riêng với phần IoT/control.

## 8. Báo cáo tái lập

Mỗi lần chạy ghi: `dataset_version`, split manifest hash, model/checkpoint, code commit, confidence/IoU thresholds, hardware, runtime, image size, batch size, video FPS, test date, per-class metrics, confusion cases và giới hạn mẫu.

Không chỉnh threshold trên test. Nếu thay đổi model/threshold sau khi xem test, cần test holdout mới hoặc ghi rõ test đã bị dùng để phát triển.
