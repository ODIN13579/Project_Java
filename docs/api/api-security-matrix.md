# SwiftletCare — API Security Matrix (đề xuất chờ P3/P4 phê duyệt)

**Nguyên tắc:** `User.role: RoleName` (OWNER/ADMIN) là quyền toàn hệ thống; `HouseMembership.role: HouseRole` (OWNER/ADMIN/VIEWER) quyết định quyền ở *nhà yến cụ thể*. Không tự khẳng định `User.role=ADMIN` được phép điều khiển mọi House nếu chính sách chưa chốt. Ai Service dùng credential riêng.

| Phạm vi endpoint | House OWNER | House ADMIN | House VIEWER | Mức |
|---|---|---|---|---|
| Đọc House/Zone/Device/Telemetry/Alert/CountLog/Report | Có | Có | Có | MVP |
| Tạo/sửa Zone, Device | Có | Đề xuất Có | Không | MVP |
| Cập nhật ZoneConfig | Có | Đề xuất Có | Không | MVP |
| `POST` Actuator Command | Có | Đề xuất Có | Không | MVP |
| Chuyển trạng thái Alert | Có | Đề xuất Có | Không | MVP |
| Ghi HarvestLog | Có | Đề xuất Có | Không | MVP |
| Thêm/sửa HouseMember | Có | Cần quyết định quyền Admin | Không | Extended |
| Đổi thông tin House | Có | Cần quyết định quyền Admin | Không | Extended |
| Xem các thông báo của bản thân | Có | Có | Có | Extended |
| Internal AI ingestion | Không | Không | Không | Service credential |

## Cách kiểm tra ở Spring Boot

1. JWT hợp lệ → tra `User.active` → xác định `houseId` gián tiếp qua resource.
2. `HouseMembership(userId,houseId,active=true)` → role đủ để xem/sửa? Không được tin `houseId` client gửi khi nó khác thực thể đích.
3. Request AI `/internal/ai/*` không được mở bằng quyền JWT của browser; sử dụng service authentication và kiểm tra CAMERA thực sự thuộc Zone và nguồn hợp lệ.
4. WebSocket subscribe cũng phải áp dụng cùng quyền House/Zone, kiểm tra lại khi quyền bị thu hồi.
5. Audit action sửa cấu hình, gửi lệnh, đổi Alert, thay HouseMembership; không ghi secret.

## Quyết định chưa chốt

- Các vai trò global `RoleName` có thực sự được dùng để cấp quyền vượt House không?
- House ADMIN có được phân quyền thành viên, chỉnh sửa House và quản lý người dùng không?
- Có thể thêm một `House` trước khi tạo Zone hay bắt buộc tạo initial Zone? Nếu giữ UML `House 1..* Zone`, cần transaction tạo đồng thời hoặc đổi multiplicity.
- Lựa chọn `X-Service-Key` cho AI chỉ là giải pháp đơn giản cho prototype; môi trường ngoài demo cần cơ chế an toàn tương ứng NFR.
