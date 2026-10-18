# SwiftletCare — IoT Hardware Specification

| Thuộc tính | Giá trị |
|---|---|
| Tài liệu | `iot-hardware-spec.md` |
| Sprint / Task | DSN-03 — IoT Architecture |
| Phạm vi | Prototype Wokwi/ESP32 cho bài tập lớn Lập trình Java; phần cứng thực tế chỉ là thiết kế tham khảo |
| Trạng thái | **Draft for review** — chưa xác nhận linh kiện thực tế, GPIO cuối cùng và thiết kế nguồn |
| Liên quan | `iot-firmware-architecture.drawio`, `iot-control-state-machine.drawio`, `iot-sensor-telemetry-flow.drawio`, `iot-offline-recovery.md`, `mqtt-integration-contract.md` |

## 1. Mục tiêu và ranh giới

Thiết kế bộ phận IoT để **cung cấp dữ liệu và phản hồi điều khiển cho Java Spring Boot**. P1 phụ trách nguồn Telemetry, xử lý AUTO/MANUAL trên ESP32, tín hiệu GPIO/relay, MQTT và xử lý mất kết nối. P4 phụ trách MQTT Handler, lưu Database, REST/WebSocket, xác thực người dùng và kiểm tra trạng thái Online/Offline tại Backend.

Luồng tích hợp:

```text
Cảm biến thật/mô phỏng → ESP32 (Sensor Manager, Control Manager)
                    → MQTT Broker → Java Spring Boot → Database → Dashboard
Dashboard → Java Spring Boot → MQTT Broker → ESP32 → GPIO/Relay (mô phỏng)
                                                  ↳ command_ack → Backend
```

**Không thuộc phần cứng trực tiếp của ESP32:** Camera IP/RTSP và AI Service (YOLOv8 + ByteTrack trên Edge Server) là các thành phần hệ thống liên quan nhưng **không nối GPIO vào ESP32**. Mô hình AI gửi kết quả sang Java Backend theo thiết kế Backend, không bắt buộc đi qua firmware IoT.

## 2. Mức độ xác nhận thông tin

- **Đã chốt ở phạm vi thiết kế:** ESP32 làm bộ điều khiển; dữ liệu gồm nhiệt độ, độ ẩm, CO₂, ánh sáng; relay/actuator; MQTT; chế độ AUTO/MANUAL và hoạt động AUTO cục bộ khi mất Internet; định danh Telemetry **`event_id`**; Heartbeat **2 giây**, timeout Backend **6 giây**, scheduler **1 giây** theo quyết định nhóm.
- **Mục tiêu NFR đã ghi trong CV7 nhưng chưa kiểm chứng:** Telemetry mặc định 5 giây (cấu hình 1–60 giây); các ngưỡng và mốc thời gian kiểm thử ở `iot-nfr-checklist.md`.
- **TBD — P1 phải xác nhận:** model linh kiện chính xác, số lượng relay, ánh xạ GPIO cuối cùng, điện áp nguồn, tương thích mức kích, trạng thái an toàn theo từng actuator, cấu hình mô phỏng Wokwi và bộ nhớ lưu Buffer bền vững.

> Bảng model/GPIO bên dưới là **đề xuất để chạy prototype**, không phải thông số phần cứng đã được nhóm phê duyệt hoặc đã kiểm thử.

## 3. Danh mục thiết bị và trách nhiệm

| Thành phần | Chức năng | Giao tiếp | I/O chính | Trạng thái |
|---|---|---|---|---|
| ESP32 DevKit (đề xuất ESP32-WROOM-32) | Sensor, AUTO/MANUAL, GPIO, MQTT, cấu hình cục bộ, Offline/Recovery | Wi-Fi, GPIO, I²C/UART/Digital tùy sensor | In: sensor, command, config; Out: Telemetry, Heartbeat, Command ACK, GPIO | Nền tảng đã chọn; board cụ thể TBD |
| Nhiệt độ + độ ẩm | Đọc nhiệt độ (°C), độ ẩm (%RH) | DHT22: đường dữ liệu digital; SHT31: I²C | In: môi trường; Out: số đo / trạng thái lỗi | Model TBD; DHT22 đề xuất cho demo |
| Cảm biến CO₂ | Đọc CO₂ (ppm) | UART/I²C/Analog tùy model | Out: CO₂ + trạng thái | Model TBD; có thể tạo giá trị mô phỏng trong Wokwi |
| Cảm biến ánh sáng | Đọc ánh sáng (lux) | BH1750: I²C (đề xuất) | Out: lux + trạng thái | Model TBD / có thể mô phỏng |
| Module relay/driver | Nhận tín hiệu logic từ ESP32, đóng/ngắt mạch tải | GPIO số qua mạch driver tương thích | In: GPIO; Out: trạng thái đầu ra | Đã chọn nguyên lý relay; model/kênh TBD |
| Máy phun sương | Tăng độ ẩm theo ngưỡng | Qua relay/driver | ON/OFF | Actuator dự kiến; demo bằng đèn/relay mô phỏng |
| Quạt thông gió | Hỗ trợ điều tiết nhiệt độ/môi trường | Qua relay/driver | ON/OFF | Actuator dự kiến; demo bằng đèn/relay mô phỏng |
| Máy sưởi | Hỗ trợ điều tiết nhiệt độ | Qua relay/driver | ON/OFF | Actuator dự kiến; tính khả thi/safety TBD |
| Ampli/loa | Phần thiết bị âm thanh, theo FR liên quan nếu triển khai | Qua relay/driver nếu chỉ bật/tắt nguồn | ON/OFF | Phạm vi demo âm thanh TBD |
| Camera IP | Cung cấp RTSP, snapshot cho AI Service | LAN/RTSP | Video/frame | Thành phần độc lập với ESP32 |
| Edge AI Service | YOLOv8 + ByteTrack; đếm chim/phát hiện bất thường | RTSP từ Camera, REST đến Backend | AI Event, Count, Snapshot | Phụ trách P2, không phải linh kiện ESP32 |
| MQTT Broker | Trung gian Telemetry, Heartbeat, Command, ACK | MQTT/TCP hoặc MQTTS | Publish/Subscribe | P1/P4 tích hợp |

## 4. Phạm vi mô phỏng Wokwi và cấu hình mẫu

### 4.1. Danh sách cần đủ cho BUILD

**MVP cần trình diễn:** 01 ESP32, 01 nguồn nhiệt độ/độ ẩm (DHT22 hoặc mô phỏng), dữ liệu CO₂/ánh sáng mô phỏng khi linh kiện chưa có, ít nhất 01 LED/relay đại diện quạt hoặc máy phun sương, MQTT Broker, và Java Backend nhận Telemetry.

**Mở rộng sau MVP:** nhiều relay, xử lý sensor lỗi từng loại, MANUAL có ACK, cấu hình ngưỡng, hàng đợi lưu bền vững, kiểm thử 24 giờ. Không tuyên bố đã có phần cứng/đã chạy test nếu chưa nộp minh chứng.

### 4.2. GPIO Mapping **minh họa** (chưa được phê duyệt)

| Linh kiện demo | Tín hiệu | GPIO ESP32 đề xuất | Ghi chú |
|---|---|---|---|
| DHT22 | DATA | GPIO18 | Dữ liệu Digital; cần nguồn và điện trở pull-up theo cấu hình module |
| BH1750 | SDA | GPIO21 | Nếu dùng BH1750 thật trên bus I²C |
| BH1750 | SCL | GPIO22 | Cùng bus I²C |
| CO₂ (nếu chọn sensor UART) | ESP32 RX | GPIO16 | Chỉ áp dụng sau khi chốt sensor/UART và mức logic |
| CO₂ (nếu chọn sensor UART) | ESP32 TX | GPIO17 | Có thể không cần nếu sensor chỉ gửi dữ liệu |
| Relay máy phun sương | IN | GPIO25 | Module phải tương thích logic 3,3V |
| Relay quạt | IN | GPIO26 | Không nối tải công suất trực tiếp vào GPIO |
| Relay máy sưởi | IN | GPIO27 | Dùng đèn/relay mô phỏng trong BUILD |
| Relay ampli | IN | GPIO32 | Chỉ cần nếu FR về âm thanh triển khai |

**Checklist chốt mapping:** xác nhận board ESP32 chính xác; chân sử dụng có xung đột lúc boot không; nguồn và mass chung khi cần; module relay kích HIGH hay LOW; sensor dùng I²C/UART ở tốc độ/địa chỉ nào; khả năng sử dụng linh kiện trong phiên bản Wokwi của nhóm. Không biến bảng minh họa này thành sơ đồ đấu nối 220V.

### 4.3. Phân biệt cảm biến thật và dữ liệu giả lập

- Số đo **đến từ cảm biến mô phỏng** phải ghi rõ là simulated readings; ví dụ CO₂/ánh sáng tạo bởi code thay vì cảm biến phần cứng.
- Nếu một linh kiện không được hỗ trợ trong môi trường mô phỏng, P1 có thể tạo giá trị giả lập có kiểm soát và ghi cách tạo, nhưng **không mô tả là đã đọc phần cứng thật**.
- Bằng chứng thực nghiệm lưu ở `iot/esp32-wokwi/evidence/`, không đưa các kết quả giả định vào tài liệu đặc tả.

## 5. I/O và luồng firmware

| Module/luồng | Input | Output / xử lý |
|---|---|---|
| HAL / GPIO Driver | Tín hiệu từ sensor, lệnh Control Manager | Giá trị đọc thô; mức logic xuất relay; feedback GPIO nếu có |
| Sensor Manager | Giá trị đọc thô | Kiểm tra từng giá trị; chuẩn hóa; `OK`, `PARTIAL_ERROR`, `ERROR` |
| Control Manager | Sensor hợp lệ, mode, cấu hình, safety policy | ON/OFF/giữ trạng thái; hysteresis; lệnh GPIO |
| Application / State Manager | Event kết nối, lệnh đổi mode, lỗi, reset | Control Mode = AUTO/MANUAL; Connection State = ONLINE/OFFLINE/RECOVERING |
| MQTT Manager | Queue Telemetry, configuration, command, ACK | Publish/Subscribe; retry/reconnect; gửi Heartbeat |
| Config & Buffer Manager | Cấu hình, bản tin chưa ACK, event xác nhận | Lưu ngưỡng cục bộ, giữ `event_id`/`timestamp`, gửi bù, quản lý Buffer |

**Ranh giới AUTO/MANUAL:** theo `Actuator.mode` của Class Diagram; nếu P1 chọn mode toàn thiết bị/Zone, P1+P4+P3 phải cập nhật tài liệu trước BUILD. Command đề xuất chia `SET_MODE` và `SET_STATE`; `SET_STATE` chỉ có hiệu lực khi Actuator đang MANUAL, không tự chuyển mode.

## 6. Chu kỳ và dữ liệu giao tiếp

| Thao tác | Chu kỳ / quy ước | Nguồn quyết định |
|---|---|---|
| Đọc sensor | **5 giây** trong prototype | CV3 |
| Publish Telemetry | **5 giây** mặc định; cấu hình **1–60 giây** | CV3, NFR-IOT-003 |
| Heartbeat | **2 giây** | Quyết định P1/P4 đã chốt trong trao đổi |
| Backend Offline Timeout | **6 giây** từ Heartbeat hợp lệ cuối | Quyết định P1/P4 |
| Backend Timeout Scheduler | **1 giây** | Quyết định P1/P4 |
| Hiển thị Offline ở Dashboard | Mục tiêu **≤10 giây** sau khi mất kết nối | CV3 + quyết định thiết kế |
| Telemetry ID | **`event_id`** duy nhất theo thiết bị và bản ghi | Quyết định của nhóm |
| Thời gian | ISO 8601 UTC trong payload | MQTT Contract |

Ví dụ Telemetry khi CO₂ lỗi (schema đầy đủ xem `mqtt-payload-schema.md`):

```json
{
  "schemaVersion": 1,
  "event_id": "ESP32-01-000002",
  "device_id": "ESP32-01",
  "timestamp": "2026-10-18T02:30:05Z",
  "temperature": 29.5,
  "humidity": 78.0,
  "co2": null,
  "light": 12.5,
  "sensor_status": "PARTIAL_ERROR"
}
```

Đơn vị: °C, %RH, ppm, lux. Bản ghi lỗi dùng `null` cho giá trị không đo được, không sử dụng giá trị 0 thay thế. Quy tắc lưu/loại trùng: `(device_id, event_id)` phải duy nhất tại Backend, retry giữ nguyên ID/timestamp.

## 7. Điều khiển AUTO/MANUAL và hysteresis

Theo CV4, hành vi nghiệp vụ như sau:

| Điều kiện | Mode | Kết quả |
|---|---|---|
| Độ ẩm dưới ngưỡng ON | AUTO | Bật phun sương |
| Độ ẩm ≥ ngưỡng OFF | AUTO | Tắt phun sương |
| Độ ẩm nằm giữa hai ngưỡng | AUTO | Giữ trạng thái đang có |
| Nhiệt độ ≥31°C | AUTO | Quạt ON theo mục tiêu NFR-IOT-009 |
| Nhiệt độ ≤30°C | AUTO | Quạt OFF theo mục tiêu NFR-IOT-009 |
| Nhận `SET_MODE(MANUAL)` hợp lệ | AUTO → MANUAL | Đổi chế độ theo actuator |
| Nhận `SET_STATE(ON/OFF)` khi MANUAL và an toàn | MANUAL | Điều khiển GPIO, gửi Command ACK |
| Nhận `SET_STATE` khi AUTO | AUTO | Từ chối; không tự chuyển MANUAL |
| Nhận `SET_MODE(AUTO)` hợp lệ | MANUAL → AUTO | Tính lại điều khiển theo sensor hợp lệ |
| Mất Wi-Fi/Broker | AUTO | Tiếp tục AUTO cục bộ với cấu hình hợp lệ |
| Sensor lỗi / cấu hình lỗi | AUTO/MANUAL | Không dùng dữ liệu lỗi; áp dụng safe state cho chức năng phụ thuộc |

**Cần phân biệt:** độ ẩm **75%/78% chỉ là ví dụ** trong CV4, chưa phải ngưỡng phun sương đã chốt. Nhiệt độ quạt **31°C/30°C** là mục tiêu được CV7 ghi cụ thể; vẫn cần test. Quy tắc tối thiểu 120 giây giữa hai lần chuyển trạng thái phải có ngoại lệ/ưu tiên xử lý khẩn cấp được xác nhận, tránh cản trở hành động an toàn.

## 8. Nguồn điện, logic GPIO, trạng thái an toàn

### 8.1. Điện áp / công suất (chưa chốt)

- ESP32 sử dụng GPIO logic **3,3V**; không nối trực tiếp relay coil, quạt, sưởi hoặc tải điện lưới vào chân GPIO.
- Nguồn của relay/sensor **phụ thuộc model**. Phải kiểm tra dòng tiêu thụ, mức kích, cách ly, mạch driver và nối mass thích hợp trước khi lắp phần cứng.
- Dùng LED/relay mô phỏng khi trình diễn. Không đưa sơ đồ đóng/cắt 220V thực tế vào phạm vi demo Java nếu chưa có thiết kế an toàn điện được kiểm duyệt.

### 8.2. Safe state cần chốt cho từng actuator

| Tình huống | Chính sách tối thiểu | Thông số còn TBD |
|---|---|---|
| Reset / boot / watchdog | Khởi tạo GPIO theo thứ tự an toàn, không kích relay bất ngờ; đọc cấu hình cuối hợp lệ | Mức HIGH/LOW khi khởi động cho từng kênh |
| Cảm biến độ ẩm lỗi | Không thực hiện AUTO từ số đo độ ẩm lỗi; xử lý phun sương theo trạng thái an toàn | Phun sương OFF hay giữ có giới hạn |
| Cảm biến nhiệt độ lỗi | Không thực hiện AUTO quạt/sưởi từ số đo nhiệt độ lỗi | Quạt/sưởi OFF hay phương án ưu tiên khác |
| CO₂/ánh sáng lỗi | Không dùng giá trị lỗi; chức năng độc lập vẫn hoạt động nếu an toàn | Phản ứng từng tải theo FR |
| Lệnh MANUAL xung đột safety | Từ chối hoặc giới hạn; tạo ACK có lý do | Bảng luật cụ thể |
| Mất mạng khi MANUAL | Không tự động chuyển AUTO, trừ khi chính sách timeout/safety đã chốt yêu cầu | `manualUntil`, mode sau hết hạn |
| Relay thiếu phản hồi vật lý | Chỉ xác nhận GPIO/output đã đổi; không khẳng định thiết bị thật đã vận hành | Có/không cảm biến phản hồi |

## 9. Offline, Buffer và local storage

- **Khởi tạo/lưu:** mỗi Telemetry phải được đặt vào Buffer bền vững **trước Publish** nếu cam kết khôi phục bản tin chưa ACK sau reset; phát lại giữ nguyên `event_id`, `timestamp`.
- **Giải phóng:** chỉ xóa bản tin sau `telemetry_ack` có định danh sự kiện phù hợp, trạng thái lưu thành công và từ Backend hợp lệ.
- **Mất kết nối:** tiếp tục Sensor Manager/AUTO cục bộ; MQTT retry chạy không chặn.
- **Gửi bù:** sau reconnect gửi bản tin tồn đọng với giới hạn tốc độ được chọn; Backend lưu chống trùng. Phải tiếp tục phục vụ các mẫu mới.
- **Cấu hình:** NVS phù hợp với các tham số nhỏ; **chưa có cơ sở khẳng định NVS lưu được 17.280 Telemetry**. Đề xuất đánh giá LittleFS hoặc SD/môi trường lưu trữ thích hợp trước khi chọn.
- **Dung lượng:** 24h ở 5s = **17.280 bản/luồng**; tại 1s = **86.400 bản/luồng**. Dung lượng byte tùy schema/metadata và overhead; phải đo bằng payload thực tế.
- **Buffer đầy:** P1/P4 cần xác định quy tắc (ưu tiên loại dữ liệu, báo lỗi, giãn chu kỳ, thay đổi thời gian offline được bảo đảm). Không ghi mặc định 'không mất dữ liệu' nếu chưa chọn chính sách và chứng minh dung lượng.

Chi tiết trình tự ở `iot-offline-recovery.md` và `mqtt-integration-contract.md`.

## 10. Ma trận phần cứng và kế hoạch demo BUILD

| Test/Demo | Cần chứng minh | Trạng thái |
|---|---|---|
| HW-01 | Wokwi có ESP32 và dữ liệu DHT22/mô phỏng nhiệt độ, độ ẩm | Chưa kiểm thử |
| HW-02 | Giá trị CO₂/ánh sáng có nguồn và đơn vị được ghi rõ | Chưa kiểm thử |
| HW-03 | Telemetry có `event_id`, Backend Java nhận và lưu | Chưa kiểm thử |
| HW-04 | AUTO bật/tắt LED/relay với hysteresis | Chưa kiểm thử |
| HW-05 | `SET_MODE` + `SET_STATE` được kiểm tra, phản hồi `command_ack` | Chưa kiểm thử |
| HW-06 | Mất MQTT, AUTO vẫn hoạt động và Telemetry được giữ theo cấu hình Buffer | Chưa kiểm thử |
| HW-07 | Reconnect, gửi bù cùng `event_id`, không tạo bản ghi trùng | Chưa kiểm thử |
| HW-08 | Heartbeat 2s; Java Scheduler 1s, timeout 6s; Dashboard cập nhật | Chưa kiểm thử |

Minh chứng: mã nguồn Wokwi, `diagram.json`, Serial Monitor, MQTT Subscriber/Backend logs và ảnh/video kết quả test; nộp tại DSN-S1 hoặc Sprint BUILD.

## 11. Các quyết định còn mở trước khi nghiệm thu phần Hardware

| ID | Cần xác nhận | Người chốt |
|---|---|---|
| HW-D01 | Loại ESP32, cảm biến DHT22/SHT31, CO₂ và ánh sáng sử dụng trong demo | P1 |
| HW-D02 | GPIO Mapping thật, bus UART/I²C, thiết kế nguồn và relay active LOW/HIGH | P1 |
| HW-D03 | Số relay/actuator được mô phỏng và trạng thái an toàn từng kênh | P1 + P4 (nghiệp vụ) |
| HW-D04 | MANUAL timeout/ưu tiên safety, ngoại lệ khóa chuyển trạng thái 120s | P1 + P4 + P3 |
| HW-D05 | Lưu Buffer bền vững ở đâu và chính sách đầy | P1 |
| HW-D06 | Trường hợp cấu hình 1s có giữ yêu cầu 24h Offline không | P1 + P3 |
| HW-D07 | Có mô phỏng camera/âm thanh trong demo IoT hay dành cho P2/BUILD sau | Nhóm |

## 12. Nguồn nội bộ và ghi chú

- **CV3** — chu kỳ đọc/gửi, `event_id`, lỗi sensor, Heartbeat và Offline.
- **CV4** — quy tắc AUTO/MANUAL, hysteresis và an toàn điều khiển.
- **CV5** — mất Internet, Buffer/reconnect, lỗi sensor, reset và relay feedback.
- **CV7** — NFR-IOT-001…012, NFR-PER-001, NFR-BW-001.
- **Các quyết định được người dùng chốt trong cuộc trao đổi** — `event_id`; Heartbeat 2s / timeout 6s / scheduler 1s; luồng Backend ACK trước khi xóa Buffer; tách `SET_MODE` và `SET_STATE` làm phương án triển khai tích hợp.

Các model GPIO, lựa chọn local storage và safe state cụ thể là **đề xuất/khoảng trống thiết kế**, không phải số liệu đã được thử nghiệm.
