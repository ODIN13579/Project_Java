# DSN-04 – AI Architecture

Tài liệu tổng hợp AI xử lý video của SwiftletCare. AI chạy trên dịch vụ riêng; không chạy YOLOv8/ByteTrack trên ESP32.

| File | Mục đích |
|---|---|
| [ai-architecture-overview.md](ai-architecture-overview.md) | Thiết kế toàn hệ thống, pipeline và lỗi/khôi phục |
| [Contract Backend MVP](../api/build104-mvp.md) | Endpoint CountLog, request/response, service key và khóa aggregate hiện hành |
| [ai-processing-pipeline.png](ai-processing-pipeline.png) | Ảnh pipeline hiện có; không có nguồn Mermaid/Draw.io tương ứng trong thư mục này |
| [dataset/](dataset/) | DSN-11: chuẩn dữ liệu, không phải thư mục ảnh thực tế |

Đầu vào thiết kế: camera RTSP/LAN hoặc file video thử nghiệm; đầu ra tổng thể: thống kê Entry/Exit và sự kiện bất thường. **BUILD1-04 chỉ nhận aggregate CountLog** qua `POST /api/v1/internal/ai/count-logs`, `X-Service-Key`, `deviceId` là PK số của CAMERA đã đăng ký. Giữ nguyên kỳ, counts và processedAt khi retry; 201 mới, 200 retry giống, 409 xung đột. Chưa có GET CountLog, endpoint anomaly/raw crossing hoặc evidence storage. Object storage, live video gateway và model cuối vẫn [TBD].
