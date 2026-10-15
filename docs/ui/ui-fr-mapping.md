# UI–FR Mapping — SwiftletCare (DSN-10)

**Trạng thái:** Mapping sơ bộ; kiểm tra mã FR và acceptance criteria với bản chuẩn trên GitHub trước khi merge.

| UI / thành phần | FR liên quan | Cách thể hiện | Ghi chú kiểm thử / ngoài UI |
|---|---|---|---|
| Dashboard — widget môi trường | FR-DASH-001, FR-DASH-002 | Nhiệt độ, độ ẩm, CO₂, ánh sáng, timestamp | ≤2 giây sau khi Broker nhận dữ liệu phụ thuộc Broker/WebSocket/backend |
| Dashboard — trạng thái thiết bị | FR-DASH-001; FR-IOT phù hợp | Trạng thái sensor/ESP32/actuator | Cần telemetry/heartbeat; UI không tự xác nhận trạng thái vật lý |
| Dashboard — chim vào/ra | FR-DASH-003; FR-AI-003/004 theo FR chuẩn | Số lượt IN/OUT, tách khỏi số chim ước tính | Đếm do AI Service cung cấp; xác minh mã FR chính xác |
| Dashboard — camera/ảnh AI | FR-DASH-004; FR-AI-005–007 theo FR chuẩn | Ảnh bất thường, nhãn, thời gian, bounding box nếu có | Phụ thuộc camera/AI service |
| Environment & Devices — telemetry/lịch sử | FR-IOT-001–009; FR-DASH-002 khi phù hợp | Bộ lọc, biểu đồ, ngưỡng, trạng thái kết nối | Đối chiếu từng FR-IOT với chức năng thật sự hiển thị |
| Environment & Devices — điều khiển | FR-IOT-001–009; FR-CONFIG-001–005 theo chức năng | ON/OFF, AUTO/MANUAL, trạng thái Pending/Confirmed/Executed/Failed | Gửi lệnh và thực thi do backend/firmware/device đảm nhiệm |
| Alerts — danh sách/xử lý | FR-ALT-001, FR-ALT-002 | Mức độ, loại, thời gian, zone, trạng thái, lịch sử | Kiểm thử thời gian cảnh báo theo FR-ALT-001 |
| Notifications | FR-NOTI-001 | Web Notification; Mobile Push nếu nằm trong phạm vi | Không tự cam kết SMS/Zalo |
| AI Events — bằng chứng | FR-AI-005–007 theo FR chuẩn | Ảnh, nhãn, bounding box, thời gian, khu vực | Metadata/phát hiện do AI Service cung cấp |
| Statistics & Reports | FR-REP-001–007 theo FR chuẩn; FR-AI-003/004 cho IN/OUT | Bộ lọc, biểu đồ và báo cáo | Kiểm tra từng FR-REP; chỉ tương quan năng suất nếu có dữ liệu |
| Quản trị người dùng/role (ngoài 4 frame) | FR-USER-001–006; FR-RBAC-001–005 | Ghi nhận ngoài phạm vi wireframe chính | Không tuyên bố đã thiết kế màn quản trị |
| Quản trị farm/zone/device (ngoài 4 frame) | FR-FARM-001–005; FR-ZONE-001–005; FR-DEVICE-001–006 | Ghi nhận ngoài phạm vi 4 màn hình | Chi tiết quản trị chưa phải trọng tâm DSN-10 |

## Checklist truy vết
- [ ] Mỗi thành phần chính trên 4 frame có FR hoặc được ghi rõ là điều hướng/trạng thái dùng chung.
- [ ] FR trong scope được gắn với UI hoặc ghi rõ xử lý tại Backend/Firmware/AI Service.
- [ ] Xác minh mã FR và tiêu chí chấp nhận trên bản chuẩn GitHub.
- [ ] Không thêm SMS/Zalo, PDF/Excel hoặc năng suất nếu chưa được FR/scope phê duyệt.
