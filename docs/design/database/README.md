# SwiftletCare — DSN-07 Database ERD (bản đồng bộ)

**Phạm vi:** ERD 17 bảng; đối chiếu **Class Diagram P3 mới nhất**, **MQTT Integration Contract** và **Task 05 Backend Architecture**.  
**Người phụ trách tài liệu ERD:** P5 theo phân công nhóm; P3 review domain, P4 review Spring Boot/JPA, P1 review IoT, P2 review AI.  
**Trạng thái:** `READY FOR REVIEW`, **chưa** thay thế sơ đồ ERD gốc hay các quyết định chưa được nhóm xác nhận.

## Các tài liệu

| File | Mục đích |
|---|---|
| [`database-schema-overview.md`](database-schema-overview.md) | Danh sách bảng, quan hệ, mapping Class ↔ Table và các quyết định đã chốt |
| [`database-data-dictionary.md`](database-data-dictionary.md) | Từ điển dữ liệu đầy đủ 17 bảng, tên cột, kiểu, khóa, nullable |
| [`database-constraints.md`](database-constraints.md) | PK/FK/UQ/CHECK, transaction, chống trùng và chính sách xóa |
| [`database-index-strategy.md`](database-index-strategy.md) | Index tối thiểu và index thêm khi cần theo truy vấn BUILD |
| [`database-open-decisions.md`](database-open-decisions.md) | Những việc còn cần P1/P2/P3/P4/P5 chốt trước migration |
| [`erd-review-checklist.md`](erd-review-checklist.md) | Checklist nghiệm thu DSN-07 và kiểm thử tích hợp với Spring Boot |

## Cách sử dụng trên GitHub

```text
docs/design/database/
├── README.md
├── database-erd.drawio                
├── database-erd.png                   
├── database-schema-overview.md
├── database-data-dictionary.md
├── database-constraints.md
├── database-index-strategy.md
├── database-open-decisions.md
└── erd-review-checklist.md
```

> **Quan trọng:** File `.drawio` và `.png` **không** được thay đổi bởi bộ Markdown này. P5 phải tự chỉnh sơ đồ ở các điểm lỗi đã phát hiện, sau đó xuất lại PNG và cập nhật `database-open-decisions.md` khi nhóm chốt.

## Quy tắc đọc tài liệu

- **[BASELINE]**: thông tin thực sự nhìn thấy trên Class Diagram/ERD mới nhất.
- **[CHỐT P1–P4]**: quyết định đã được người dùng chốt trong quá trình thiết kế IoT/MQTT: Telemetry dùng `event_id`; Heartbeat mỗi 2 giây, Backend Offline timeout 6 giây, scheduler mỗi 1 giây; `SET_MODE` / `SET_STATE` được dùng làm quy ước đề xuất chung ở bản tích hợp.
- **[CẦN SỬA SƠ ĐỒ]**: sai khác hoặc thiếu trong sơ đồ hiện tại phải cập nhật để có thể tạo migration nhất quán.
- **[ĐỀ XUẤT BUILD]**: ràng buộc và kiểu SQL gợi ý để hiện thực Spring Boot, chưa phải migration chính thức.
- **[OPEN]**: cần quyết định bổ sung, không được coi là đã phê duyệt.


