# AI Open Decisions — SwiftletCare

Danh sách các quyết định chưa được chốt trong TASK-11. Không coi các đề xuất dưới đây là yêu cầu đã phê duyệt.

| ID | Vấn đề cần quyết định | Đề xuất hiện tại | Người phối hợp | Điều kiện đóng |
|---|---|---|---|---|
| OD-01 | Video TASK-S2 ở đâu, có bao nhiêu clip và được phép dùng không? | Kiểm kê file, duration, FPS, camera, quyền sở hữu | P2 + P3 | Có manifest và xác nhận quyền dùng |
| OD-02 | Có video thực tế từ nhà yến không? | Ưu tiên camera thực tế cho holdout test | P2 + chủ trang trại | Có nguồn, giấy phép/đồng ý và điều kiện ghi hình |
| OD-03 | Class `swiftlet` có thể xác định chắc từ dữ liệu công khai không? | Giữ `bird` tách biệt; không tự map thành `swiftlet` | P2 + P3 | Có định nghĩa class và ví dụ đã review |
| OD-04 | Class cuối cùng là `swiftlet`, `snake`, `rat`, `owl` hay có `bird/unknown`? | Bắt đầu với bốn class mục tiêu; không thêm unknown khi chưa có quy tắc | P2 + P3 | Class map/version được duyệt |
| OD-05 | Quy tắc đối tượng nhỏ, mờ, che khuất, ngoài ROI? | Áp dụng `ai-annotation-guideline.md` bản draft | P2 | Annotators thống nhất và có ví dụ biên |
| OD-06 | Tỷ lệ chia tập và đơn vị nhóm? | 70/20/10 theo video/session/camera-ngày | P2 + P3 | Split manifest không rò rỉ |
| OD-07 | Dataset công khai nào được phê duyệt? | Review license/version/citation trước khi import | P2 | Có log tải và license review |
| OD-08 | Ground truth Entry/Exit và tolerance thời gian? | Event-level matching với `±Δt`, chốt trước test | P2 + P3 | Định nghĩa line, sides, ambiguous và Δt được duyệt |
| OD-09 | Chỉ số nghiệm thu chính cho Entry/Exit? | Event Precision/Recall/F1 + Count MAE theo hướng | P2 + P3 | Protocol được thống nhất |
| OD-10 | Latency đo từ đâu đến đâu? | Đo frame receipt đến Backend/mobile receipt; báo p50/p95/p99 | P2 + P4 | Timestamp và tải thử nghiệm được chốt |
| OD-11 | Schema event và naming chính thức? | Draft `schemaVersion=1.0` trong event contract | P2 + P4 | P4 duyệt fields, enums, transport và API mapping |
| OD-12 | Evidence lưu ở đâu và bao lâu? | URI tham chiếu có ACL, không nhúng media vào event | P4 + chủ trang trại | Có storage, retention, permission và retry policy |
| OD-13 | Event deduplication và track/session lifecycle? | `eventId` idempotent + `trackingSessionId` | P2 + P4 | Có quy tắc khi restart, reconnect và mất track |
| OD-14 | Mục tiêu Precision/Recall có được nghiệm thu ngay không? | Xem là target dự kiến, không phải kết quả | P2 + P3 | Có test độc lập và báo cáo đo lường |
| OD-15 | Có thể chia sẻ dataset đã gán nhãn ra ngoài repository không? | Chỉ chia sẻ sau khi xác nhận license và quyền của từng nguồn | P2 + nhóm | Có quyết định bằng văn bản |

## Ưu tiên xử lý

1. **Nguồn dữ liệu và quyền sử dụng** — đặc biệt video TASK-S2 và video nhà yến.
2. **Class map và annotation guideline** — trước khi gán nhãn hàng loạt.
3. **Split theo nguồn/video/session** — trước khi chọn model hoặc tune threshold.
4. **Ground truth và protocol đánh giá** — trước khi báo cáo chỉ số.
5. **Event schema/evidence lifecycle** — chốt với P4 trước tích hợp Backend.

## Ngoài phạm vi TASK-11

- Huấn luyện mô hình production hoàn chỉnh.
- Thu thập dữ liệu quy mô lớn.
- Cam kết đạt Precision/Recall khi chưa đo trên test độc lập.
- Chốt endpoint/API chi tiết thay cho TASK-05.
- Khẳng định dataset public tổng quát hóa được cho camera nhà yến nếu chưa có đánh giá thực tế.
