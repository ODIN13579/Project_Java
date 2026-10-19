# DSN-04 – AI Architecture

Tài liệu tổng hợp AI xử lý video của SwiftletCare. AI chạy trên dịch vụ riêng; không chạy YOLOv8/ByteTrack trên ESP32.

| File | Mục đích |
|---|---|
| `ai-architecture-overview.md` | Ranh giới thành phần, pipeline và lỗi/khôi phục |
| `ai-backend-integration.md` | REST, `CountLog`, `AnomalyEvent`, idempotency, evidence |
| `ai-processing-pipeline.mmd` | Sơ đồ xử lý AI dạng Mermaid |
| `ai-processing-pipeline.drawio` | Sơ đồ native Draw.io được dựng lại |
| `ai-review-checklist.md` | Điều kiện review kiến trúc AI |
| `dataset/` | DSN-11: chuẩn dữ liệu, không phải thư mục ảnh thực tế |

Đầu vào: camera RTSP/LAN hoặc file video thử nghiệm; đầu ra: thống kê Entry/Exit và sự kiện bất thường thông qua HTTP nội bộ đến Spring Boot. Nhà cung cấp object storage, live video gateway, model cuối và cơ chế chống trùng HTTP còn [TBD].
