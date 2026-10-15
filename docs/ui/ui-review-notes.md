# UI Review Notes — SwiftletCare DSN-10

**Phiên bản:** v1.0. Đây là checklist để tự review trực tiếp trên Figma, không phải xác nhận rằng các frame đã được kiểm tra.

## 1. Kiểm tra chung
- [ ] Tên frame: `01 - Dashboard`, `02 - Environment & Devices`, `03 - Alerts & AI Events`, `04 - Statistics & Reports`.
- [ ] Header/navigation nhất quán.
- [ ] Có tiêu đề, đơn vị đo và timestamp phù hợp.
- [ ] Có Loading, Empty, Error, Offline/Stale data khi phù hợp.
- [ ] Bộ lọc thể hiện giá trị đang chọn.
- [ ] Critical nổi bật hơn Warning; không dùng màu sắc làm tín hiệu duy nhất.
- [ ] Hành động có nhãn rõ và phản hồi thành công/thất bại.

## 2. Dashboard
- [ ] Có nhiệt độ, độ ẩm, CO₂, ánh sáng và thời điểm cập nhật.
- [ ] Thiết bị/sensor thể hiện Online/Offline/Unknown.
- [ ] IN/OUT tách khỏi số chim ước tính hiện diện.
- [ ] Cảnh báo/ảnh AI có thời gian, nhãn và đường dẫn xem chi tiết.
- [ ] Dữ liệu cũ/mất kết nối được đánh dấu.

## 3. Environment & Devices
- [ ] Có bộ lọc farm/zone/khoảng thời gian.
- [ ] Biểu đồ có đơn vị, trục thời gian và ngưỡng nếu cần.
- [ ] ON/OFF và AUTO/MANUAL chỉ cho người có quyền.
- [ ] Phân biệt Pending/Sent, Acknowledged, Executed, Failed.
- [ ] Mất kết nối server không bị mô tả như chắc chắn thiết bị hỏng; ghi chú AUTO cục bộ ESP32.

## 4. Alerts & AI Events
- [ ] Có thời gian, mức độ, loại, farm/zone và trạng thái.
- [ ] Bằng chứng AI có nhãn/bounding box khi dữ liệu có sẵn.
- [ ] New → Processing → Resolved có phản hồi kết quả.
- [ ] Lịch sử có người xử lý và thời điểm.
- [ ] Không cam kết SMS/Zalo nếu FR chưa cập nhật.

## 5. Statistics & Reports
- [ ] Có bộ lọc ngày/farm/zone.
- [ ] IN/OUT và số chim ước tính được đặt tên rõ.
- [ ] Biểu đồ có đơn vị và trạng thái không có dữ liệu.
- [ ] Chỉ tương quan năng suất khi dữ liệu năng suất đã định nghĩa/thu thập.
- [ ] Nút export chỉ xuất hiện nếu thuộc FR/scope đã duyệt.

## 6. Thử luồng
1. Dashboard → widget môi trường → Environment & Devices.
2. Dashboard → cảnh báo/ảnh AI → Alerts & AI Events.
3. Alerts → chuyển New sang Processing rồi Resolved.
4. Environment & Devices → gửi lệnh → quan sát Pending và phản hồi thực tế.
5. Statistics → đổi ngày/farm/zone → biểu đồ cập nhật.

## 7. Kết quả review (điền sau khi kiểm tra)
- Blocker:
- Major:
- Minor:
- Người review / ngày:
- Quyết định: Approved / Needs changes
