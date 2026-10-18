## 1. Mục đích và cách dùng

- **Design Target:** chỉ tiêu đã được ghi trong CV7 hoặc quyết định thiết kế mới (phải tách nhãn).
- **Test Procedure:** cách tạo điều kiện test, đo thời gian/số lượng, lưu log và đối chiếu.
- **Actual Result:** kết quả đo **thực tế**. Chưa chạy thì ghi `NOT TESTED`, không ghi PASS.
- **Status:** `NOT TESTED` / `PASS` / `FAIL` / `BLOCKED` / `DEFERRED` (DEFERRED cần lý do và người phê duyệt).
- Kiểm thử mô phỏng Wokwi và kiểm thử phần cứng thực tế **không được đánh đồng**. Mục IP65, an toàn điện không thể được xác nhận chỉ bằng Wokwi.

## 2. Ma trận 12 NFR-IOT từ CV7 (giữ nguyên Design Target)

| ID | Yêu cầu | Design Target (CV7) | Phương pháp kiểm tra | Actual Result | Status |
|---|---|---|---|---|---|
| NFR-IOT-001 | AUTO khi Offline | ESP32 tiếp tục AUTO **≥24 giờ** khi mất Internet/Server | Ngắt Wi-Fi/Internet, giữ nguồn; thay đổi sensor qua hai phía ngưỡng; quan sát actuator | Chưa đo | NOT TESTED |
| NFR-IOT-002 | Buffer Telemetry Offline | **≥17.280 record/luồng/24h** tại chu kỳ **5 giây** | Ngắt mạng, tạo và lưu các bản ghi, kết nối lại, đối chiếu `event_id`, timestamp và trùng/mất | Chưa đo | NOT TESTED |
| NFR-IOT-003 | Chu kỳ Telemetry | **5 giây mặc định**, cấu hình **1–60 giây** | Ghi thời gian message; thay đổi cấu hình từ hệ thống; đo độ lệch | Chưa đo | NOT TESTED |
| NFR-IOT-004 | Sensor hợp lệ | Phát hiện thiếu/sai/ngoài dải; **không dùng giá trị lỗi để điều khiển** | Tiêm giá trị ngoài dải, null, NaN/sensor lỗi; kiểm tra AUTO | Chưa đo | NOT TESTED |
| NFR-IOT-005 | Phát hiện sensor lỗi | **≤10 giây** | Ngắt/tạo lỗi sensor và đo thời gian nhận diện | Chưa đo | NOT TESTED |
| NFR-IOT-006 | Xử lý an toàn khi sensor lỗi | **≤3 giây** để kích hoạt xử lý lỗi | Ngắt sensor khi AUTO; đo thời gian kích hoạt hành động an toàn | Chưa đo | NOT TESTED |
| NFR-IOT-007 | Phản hồi actuator | GPIO/Relay đổi trạng thái **≤1 giây** kể từ khi ESP32 **nhận lệnh hợp lệ** | Log lúc ESP32 nhận lệnh và lúc GPIO đổi; đo nhiều lần | Chưa đo | NOT TESTED |
| NFR-IOT-008 | Bảo vệ actuator | **≥120 giây** giữa hai lần chuyển trạng thái | Cho sensor dao động quanh ngưỡng; log ON/OFF và kiểm tra khoảng cách | Chưa đo | NOT TESTED |
| NFR-IOT-009 | Quạt hysteresis | **ON khi ≥31°C**, **OFF khi ≤30°C** | Tạo nhiệt độ qua 30–31°C; kiểm tra ON/OFF/KEEP | Chưa đo | NOT TESTED |
| NFR-IOT-010 | Khôi phục config | Khôi phục tham số chính xác **≤2 giây sau khởi động** | Lưu ngưỡng, reset/tắt nguồn, so sánh cấu hình và thời điểm sẵn sàng | Chưa đo | NOT TESTED |
| NFR-IOT-011 | Xác thực MQTT | Thiết bị **sai credential/quyền topic bị từ chối** | Thử kết nối sai password, topic không có quyền, quan sát log Broker | Chưa đo | NOT TESTED |
| NFR-IOT-012 | Chống ẩm phần cứng | Mục tiêu **IP65** | Review thiết kế BOM/schematic/vỏ; thử nghiệm phần cứng khi có điều kiện | Chưa đánh giá | NOT TESTED |

### Lưu ý kiểm chứng của NFR-IOT

1. **IOT-002:** 17.280 = 86.400/5; đây là số bản ghi, **chưa phải dung lượng byte**. Nếu gửi 1s trong 24h sẽ có **86.400** bản/luồng. Nhóm cần chốt liệu mức Buffer 24h vẫn bắt buộc tại cấu hình 1s; hiện CV7 chỉ xác định 17.280 tại 5s.
2. **IOT-005 và IOT-006:** CV7 chưa nêu đầy đủ mốc bắt đầu của khoảng ≤3s. **Đề xuất để kiểm thử nhất quán:** IOT-005 từ thời điểm lỗi phát sinh (hoặc thời điểm mẫu hợp lệ cuối theo quy trình CV7) đến lúc phát hiện; IOT-006 từ thời điểm hệ thống **xác nhận lỗi** đến khi hành động safe-state bắt đầu. P1/P3 phải phê duyệt trước khi đánh giá PASS.
3. **IOT-008:** khóa chuyển trạng thái 120s không được ngăn hành động an toàn khẩn cấp; đây là **đề xuất ngoại lệ cần chốt**, không tự coi là thay đổi NFR.
4. **IOT-009:** ngưỡng quạt 31°C/30°C là từ CV7. Ngưỡng phun sương 75%/78% từ CV4 chỉ là ví dụ, không ghi thành Design Target chính thức.
5. **IOT-012:** không ghi PASS dựa trên vỏ mô phỏng, ảnh sản phẩm hoặc Wokwi; cần bằng chứng phù hợp cho mục tiêu IP65.

## 3. Các NFR liên quan từ CV7

| ID | NFR | Design Target | Test Procedure | Actual Result | Status |
|---|---|---|---|---|---|
| NFR-PER-001 | Telemetry hiển thị Dashboard | **≤2 giây** từ lúc **Broker nhận message** đến lúc **Dashboard cập nhật** | Ghi thời điểm Broker nhận và Dashboard hiển thị, lặp nhiều lần; `latency = UI Update Time − Broker Receive Time` | Chưa đo | NOT TESTED |
| NFR-BW-001 | Tối ưu băng thông AI | Giảm **≥90%** dữ liệu truyền trong 1 giờ khi chỉ gửi count/snapshot so với streaming video | Cùng điều kiện và thời lượng 1h: đo tổng byte hai kịch bản rồi tính tỷ lệ | Chưa đo | NOT TESTED |

**Phân chia:** P4/P5 phối hợp đo NFR-PER-001; P2 phụ trách nguồn AI và đo NFR-BW-001, P1 chỉ phối hợp nếu có giao tiếp liên quan. Không gán toàn bộ NFR-BW cho firmware ESP32.

## 4. Bổ sung tiêu chí đồng bộ P1 ↔ P4 (không thay thế CV7)

Những mục sau nhằm kiểm tra **quyết định đã thống nhất cho BUILD**, tách khỏi 12 ID NFR gốc. Nếu muốn trở thành NFR chính thức, P3 phải tạo mã/phiên bản trong bảng FR/NFR tổng hợp.

| ID nội bộ | Quyết định/tiêu chí tích hợp | Cách kiểm tra | Actual Result | Status |
|---|---|---|---|---|
| INT-01 | **Heartbeat 2s** từ ESP32 | Log mốc thời gian Heartbeat đến Broker | Chưa đo | NOT TESTED |
| INT-02 | Backend timeout **6s** từ `lastSeenAt`, scheduler **1s** | Dừng ESP32; đo lúc Backend đổi ONLINE → OFFLINE | Chưa đo | NOT TESTED |
| INT-03 | Dashboard hiển thị OFFLINE **≤10s** sau mất kết nối (theo CV3 và quyết định) | Đồng bộ thời gian, cắt kết nối, đo đến khi UI hiển thị Offline | Chưa đo | NOT TESTED |
| INT-04 | Telemetry dùng **`event_id`**, không đổi khi retry | Gửi lại cùng bản tin và so sánh IDs/timestamp | Chưa đo | NOT TESTED |
| INT-05 | Buffer chỉ xóa khi nhận Application ACK **`STORED` đúng event_id** sau DB commit | Giả lập DB lỗi hoặc mất ACK; kiểm tra Queue và Database | Chưa đo | NOT TESTED |
| INT-06 | `(device_id, event_id)` không tạo bản ghi trùng tại Backend | Gửi hai bản cùng ID cùng nội dung, xác nhận DB 1 bản ghi và ACK hợp lệ | Chưa đo | NOT TESTED |
| INT-07 | `sensor_status` có `OK/PARTIAL_ERROR/ERROR`, giá trị lỗi có thể **null** | Cho một sensor lỗi, xác minh Backend lưu null và AUTO vẫn dùng các sensor độc lập hợp lệ | Chưa đo | NOT TESTED |
| INT-08 | `SET_STATE` khi Actuator ở AUTO bị từ chối; `SET_MODE` đổi mode có kiểm tra điều kiện | Thử chuỗi lệnh từ Java API đến ESP32 và đọc `command_ack` | Chưa đo | NOT TESTED |
| INT-09 | Command ACK phân biệt nhận lệnh với xác nhận GPIO; command trùng/hết hạn bị xử lý an toàn | Gửi lệnh trùng `commandId`, quá `expiresAt`, kiểm tra trạng thái và relay | Chưa đo | NOT TESTED |
| INT-10 | Reconnect không chặn đọc sensor/AUTO; gửi bù cùng event_id | Ngắt Broker và kiểm tra điều khiển cục bộ, reconnect và dữ liệu DB | Chưa đo | NOT TESTED |
| INT-11 | Command ACK timeout **10s**, command expiration **15s** từ lúc tạo theo DSN-05 P4 | Gửi lệnh, không phản hồi; gửi lệnh hết hạn; đối chiếu trạng thái Backend | Chưa đo | NOT TESTED |

### Phân biệt ACK và QoS

- Tài liệu MQTT QoS cũ của P1 chọn **Telemetry QoS 0 / Retain No** và **Heartbeat QoS 0 / Retain No**; với QoS 0 **không có MQTT PUBACK**. Nếu nhóm giữ chính sách này, độ tin cậy và xóa Buffer phải dựa vào `telemetry_ack` ở tầng ứng dụng. Chính sách có thể đổi nếu P1/P4 cùng sửa MQTT Contract.
- `command_ack` (ESP32 → Backend) là phản hồi lệnh; `telemetry_ack` (Backend → ESP32) là xác nhận bản Telemetry đã được lưu. Không sử dụng thay thế cho nhau.
- LWT trên `availability` là hỗ trợ phát hiện mất kết nối, không thay nhiệm vụ Backend kiểm tra Heartbeat độc lập.

## 5. Test cases ưu tiên theo giai đoạn

### 5.1. BUILD MVP — Kiểm thử tích hợp Java

| Test ID | Đầu vào / thao tác | Kết quả kỳ vọng | Bằng chứng cần lưu | Status |
|---|---|---|---|---|
| BUILD-01 | Chạy ESP32/Wokwi; Publish Telemetry 5s | Spring Boot nhận và lưu dữ liệu đúng field/đơn vị | Serial log, Broker log, DB query | NOT TESTED |
| BUILD-02 | Gửi lại cùng `(device_id, event_id)` | Database không tăng số bản ghi; ACK hợp lệ | DB query, MQTT logs | NOT TESTED |
| BUILD-03 | Đặt `co2 = null`, `sensor_status = PARTIAL_ERROR` | Backend lưu đúng null, không sử dụng CO₂ lỗi cho AUTO | JSON, DB, log actuator | NOT TESTED |
| BUILD-04 | Gửi `SET_MODE(MANUAL)` rồi `SET_STATE(ON)` | GPIO/LED đổi; Backend nhận Command ACK | MQTT command/ack, Wokwi log | NOT TESTED |
| BUILD-05 | Gửi `SET_STATE(ON)` khi AUTO | Lệnh bị từ chối; mode không tự đổi | Command ACK, trạng thái | NOT TESTED |
| BUILD-06 | Ngắt Wi-Fi/MQTT | AUTO cục bộ hoạt động; trạng thái offline cập nhật theo 2/6/1 | Log local và Dashboard | NOT TESTED |
| BUILD-07 | Giả lập Database lỗi/loss of telemetry_ack | Không xóa Buffer, retry không tạo DB duplicate | Buffer log, DB log | NOT TESTED |
| BUILD-08 | Khôi phục kết nối | Gửi bù dữ liệu cũ với timestamp gốc; dữ liệu mới không bị chặn | Event IDs, timestamps, DB | NOT TESTED |
| BUILD-09 | Đưa nhiệt độ 30 → 31 → 30°C | Quạt ON ở ≥31, OFF ở ≤30; giữ trạng thái giữa các ngưỡng | Actuator log, mô phỏng | NOT TESTED |

### 5.2. Kiểm thử NFR bổ sung / kéo dài

| Test ID | Phạm vi | Trạng thái hiện tại | Ghi chú |
|---|---|---|---|
| EXT-01 | AUTO Offline ≥24h | NOT TESTED | Không ghi PASS bằng test ngắt mạng 1–2 phút |
| EXT-02 | Buffer ≥17.280 bản ở 5s | NOT TESTED | Kiểm tra lưu bền vững và dung lượng byte |
| EXT-03 | Chu kỳ telemetry từ xa 1–60s | NOT TESTED | Kiểm thử nhiều cấu hình |
| EXT-04 | Relay ≤1s; an toàn ≤3s; lỗi ≤10s; config ≤2s | NOT TESTED | Mốc đo phải được thống nhất |
| EXT-05 | Bảo vệ actuator 120s | NOT TESTED | Phải làm rõ ngoại lệ an toàn |
| EXT-06 | Broker ACL/credential/TLS | NOT TESTED | Xác thực bằng cấu hình Broker cụ thể |
| EXT-07 | IP65 | NOT TESTED | Không thuộc bằng chứng Wokwi |
| EXT-08 | Độ trễ Dashboard ≤2s | NOT TESTED | P4/P5 đo |
| EXT-09 | Giảm băng thông AI ≥90% | NOT TESTED | P2 đo |

**Việc hoãn test trong BUILD** chỉ được đánh dấu `DEFERRED` sau khi nhóm/giảng viên xác nhận phạm vi. Bảng này không làm mất hiệu lực Design Target ban đầu.

## 6. Form ghi kết quả thực nghiệm

```text
Test ID:
Ngày giờ (UTC/local, timezone):
Người chạy test:
Commit/Branch:
Wokwi project / ESP32 board:
MQTT Broker + cấu hình QoS:
Backend Spring Boot + Database:
Bước test / input:
Design Target:
Các lần đo / log:
Actual Result:
Status: NOT TESTED | PASS | FAIL | BLOCKED | DEFERRED
Evidence link:
Bug/Issue link (nếu có):
Ghi chú / điều kiện thử:
```

Điều kiện PASS phải dựa trên bằng chứng có thể tái hiện. Khi kiểm thử thất bại cần ghi issue và thời gian kiểm thử lại. Ví dụ với NFR-IOT-007, ghi mỗi độ trễ và giá trị lớn nhất; không chỉ ghi trung bình để che thời điểm vượt ngưỡng.

## 7. Traceability và trách nhiệm

| Yêu cầu/Quy tắc | Thiết kế liên quan | Người thực hiện/kiểm chứng |
|---|---|---|
| IOT-001, 002, 010 / INT-05, 10 | `iot-offline-recovery.md`, Buffer/Retry Flow, Storage | P1; P4 hỗ trợ ACK/DB |
| IOT-003, 004, 005 / INT-04, 07 | Sensor/Telemetry Flow, MQTT Payload Schema | P1; P4 kiểm chứng lưu dữ liệu |
| IOT-006, 007, 008, 009 / INT-08, 09 | AUTO/MANUAL, HAL/Relay, `ZoneConfig` | P1; P3 đối chiếu FR |
| IOT-011 / INT-01, 02, 03 | MQTT Contract, Broker ACL, Heartbeat/Timeout Flow | P1 + P4; P5 kiểm thử UI |
| IOT-012 | Hardware Spec, BOM/vỏ lắp | P1, test phần cứng nếu có |
| NFR-PER-001 | Java MQTT Handler + WebSocket + Dashboard | P4 + P5 |
| NFR-BW-001 | AI Ingestion / Snapshot/Count | P2 + P4 |

## 8. Các điểm phải chốt/ghi vào Jira trước khi công bố NFR đạt

| ID | Vấn đề cần quyết định | Trạng thái |
|---|---|---|
| NFR-D01 | IOT-006 ≤3s đo từ lúc phát hiện lỗi hay từ lúc lỗi phát sinh | **OPEN** |
| NFR-D02 | Chính sách 120s có ngoại lệ khi khẩn cấp và cách test | **OPEN** |
| NFR-D03 | 24h Buffer có yêu cầu tại 1s hay chỉ tại 5s | **OPEN** |
| NFR-D04 | Vị trí lưu bền vững, Buffer đầy, nguồn timestamp khi Offline | **OPEN** |
| NFR-D05 | Safe state cụ thể từng actuator khi sensor lỗi, reset, offline | **OPEN** |
| NFR-D06 | Telemetry QoS 0 + App ACK hay chuyển QoS 1 | **OPEN — P1/P4** |
| NFR-D07 | Thời hạn chờ `telemetry_ack` và retry schedule | **OPEN — P1/P4** |
| NFR-D08 | Khả năng hiện thực/kiểm thử IP65 trong phạm vi bài tập Java | **OPEN — nhóm/giảng viên** |

## 9. Điều kiện nghiệm thu file thiết kế và nghiệm thu NFR thực tế

**Để nghiệm thu tài liệu DSN-03:** 12 NFR-IOT và 2 NFR liên quan phải có ID, mục tiêu, phương pháp đo, người phụ trách và trạng thái thực tế. Các lựa chọn chưa chốt phải ghi OPEN/TBD. Không cần giả định mọi NFR đã PASS.

**Để khẳng định hệ thống đạt NFR trong BUILD/test:** chỉ được PASS những tiêu chí có log/ảnh/video/DB bằng chứng. Test kéo dài, IP65, Buffer bền vững hay hiệu năng cần kiểm thử riêng hoặc có phê duyệt ghi DEFERRED.
