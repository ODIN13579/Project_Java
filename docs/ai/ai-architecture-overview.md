# AI Architecture – SwiftletCare

## 1. Phạm vi và ranh giới

**[THEO NGUỒN]** Camera / file video → Frame Acquisition & Preprocessing → YOLOv8 Detection → hai nhánh: (A) ByteTrack + Entry/Exit Counter, (B) Predator Detection → tạo CountLog/AnomalyEvent → HTTPS REST → Spring Boot → DB → WebSocket / Alert.

- **Camera/video source:** cung cấp video/ảnh; không trực tiếp ghi Database Java.
- **AI Edge Service:** đọc frame, inference, tracking, counting và lưu tham chiếu bằng chứng (nếu được bật). Không điều khiển GPIO hoặc tạo `ControlCommand`.
- **Java Backend:** xác thực service; kiểm tra schema/quyền camera-device/zone, lưu `CountLog` hoặc `AnomalyEvent`; đánh giá quy tắc Alert, lưu lịch sử và phát realtime sau commit.
- **ESP32:** độc lập với pipeline video, xử lý cảm biến môi trường và AUTO. Không dùng MQTT để đẩy video thô.

## 2. Pipeline từng bước

1. **Acquire frame:** nguồn camera RTSP/LAN hoặc video thử nghiệm có `cameraSourceId`, timestamp và `sessionId` nội bộ. [TBD] Chọn giao thức, tốc độ lấy frame và chính sách reconnect.
2. **Validate / preprocess:** frame không giải mã được → log, bỏ frame, không sinh detection giả; resize/normalize đồng nhất cấu hình model. [ĐỀ XUẤT] Ghi phiên bản pipeline và độ phân giải đầu vào.
3. **Inference – YOLOv8:** đầu ra `class`, `bbox`, `confidence`, timestamp. Các class `swiftlet`, `snake`, `rat`, `owl` chỉ là **danh sách dự kiến** trong tài liệu dataset.
4. **Bird tracking:** chỉ đưa detection chim đủ điều kiện vào ByteTrack; ID là `trackId` **trong một session/camera**, không phải chim vĩnh viễn. Camera reboot/session restart không nối track tùy tiện.
5. **Entry/Exit:** một track cắt vạch theo hướng hợp lệ thì tăng đúng bộ đếm; chống đếm nhiều lần trên cùng một crossing. Chốt ROI, vector hướng, cooldown/hysteresis/crossing rules theo video mẫu.
6. **Aggregate CountLog:** [ĐỀ XUẤT phù hợp OpenAPI hiện tại] tích lũy dữ liệu theo khoảng `periodStart`, `periodEnd`; ghi `entryCount`, `exitCount`, `detectCount`, `aggregationType`, `processedAt`. Trong v1 **không cần API public cho từng raw crossing**.
7. **Predator:** nếu class/độ tin cậy/ROI hợp lệ → tạo detection/sự kiện bất thường theo chính sách [TBD] (threshold + thời gian quan sát tối thiểu + cooldown). Không gửi mỗi frame thành một cảnh báo.
8. **Evidence:** chọn snapshot và lưu ở phương án được duyệt (local filesystem/Object Storage). Backend giữ `snapshotPath`/reference; không mặc định URL công khai. [TBD] retention và quyền truy cập.
9. **HTTP AI → Spring Boot:** gửi JSON tới hai endpoint nội bộ hiện có; API trả thành công mới coi là được tiếp nhận/lưu theo contract. Retry bị giới hạn, chống gửi trùng cần quyết định ở phần tích hợp.
10. **Backend:** validate, DB transaction, công bố CountLog/Alert event sau commit; Dashboard lấy dữ liệu ban đầu bằng REST và cập nhật mới qua WebSocket.

## 3. Các trường hợp lỗi và hành động tối thiểu

| Trường hợp | Hành vi đề xuất | Không được làm |
|---|---|---|
| Camera mất kết nối, mất frame | Ghi trạng thái nguồn OFFLINE, reconnect có giới hạn, tiếp tục khi nguồn phục hồi | Tạo `CountLog` giả từ frame không tồn tại |
| Frame lỗi/không giải mã được | Bỏ frame, tăng counter lỗi/ghi log | Coi như đã phát hiện 0 con nếu không có quan sát |
| Model/YOLO thất bại | Ghi log model/version, ngắt nhánh output lỗi, báo trạng thái | Tạo `AnomalyEvent` với confidence suy đoán |
| No detection | Không phát sinh crossing/anomaly; tùy thiết kế ghi số đếm 0 nếu có **kỳ quan sát hợp lệ** | Đồng nhất `no detection` với `camera offline` |
| Track ID thay đổi | Không gộp ID qua session không có bằng chứng | Xem trackId là ID chim vĩnh viễn |
| Bất thường xuất hiện nhiều frame | Debounce / cooldown theo class/camera [TBD] | Tạo hàng trăm Alert trùng |
| Backend REST timeout | Retry có giới hạn; giữ cùng idempotency key nếu đã được chốt; log failure | Gửi lại như event hoàn toàn mới nếu đã chốt chống trùng |
| DB/Notification lỗi | Backend giữ transaction Alert/Anomaly độc lập Notification; retry gửi sau | Rollback Alert đã tạo chỉ vì push lỗi |

## 4. Phân biệt đầu ra

| Output | Đơn vị/ngữ nghĩa | Entity hiện có |
|---|---|---|
| `entryCount` / `exitCount` | **Số lượt** qua đường đếm trong một kỳ | `CountLog` |
| `detectCount` | Số lần/đối tượng phát hiện theo quy ước đo [TBD] | `CountLog` |
| `species` + `confidence` + `detectedAt` | Một sự kiện bất thường đã lọc | `AnomalyEvent` |
| `snapshotPath` | Tham chiếu ảnh bằng chứng | `AnomalyEvent` |
| Alert level/status | Do rule engine Backend quyết định | `Alert`, `AlertHistory` |

**Không đồng nhất lượt vào/ra với số lượng chim hiện có**. Để ước tính đàn chim cần baseline/công thức riêng, không suy ra trực tiếp từ `entryCount-exitCount` khi camera bỏ frame.

## 5. NFR, thông số và trạng thái

- **[THEO NGUỒN]** Mục tiêu tham khảo: Bird Precision ≥85%, Recall ≥80%; Predator Precision ≥80%, Recall ≥75% (chưa phải kết quả đã đạt).
- **[THEO KIẾN TRÚC]** Mục tiêu thời gian phát sự kiện AI → thông báo mobile ≤5s xuất hiện trên sơ đồ tổng thể, cần **định nghĩa đầu-cuối và đo**, không khẳng định khả thi khi chưa có phần Push và mô hình thật.
- **[TBD]** FPS mục tiêu, GPU/CPU, độ phân giải, độ trễ p95, ngưỡng confidence, IoU/NMS, cách lưu ảnh, retention, phân quyền service.