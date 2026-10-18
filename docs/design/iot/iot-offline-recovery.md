## 1. Phạm vi và các quyết định đã chốt

ESP32 vẫn đọc sensor và điều khiển AUTO cục bộ khi mất Wi-Fi, MQTT Broker hoặc Java Backend, nếu phần cứng và cấu hình còn an toàn. **AUTO/MANUAL** là chế độ điều khiển theo *Actuator*; **ONLINE/OFFLINE/RECOVERING** là trạng thái kết nối. Không coi OFFLINE là một chế độ thay thế AUTO/MANUAL.

- Telemetry dùng trường định danh **`event_id`** (snake_case) trong JSON, Java DTO và Entity; không dùng `messageId` hoặc `eventId` trong hợp đồng Telemetry.
- Một bản ghi được giữ nguyên `event_id`, `device_id`, `timestamp` và nội dung qua mọi lần gửi lại. Backend chống trùng bằng `UNIQUE(device_id, event_id)`.
- ESP32 **lưu hàng đợi trước khi Publish**. Chỉ giải phóng bản ghi khi nhận **Telemetry Application ACK** có đúng `event_id` và `status = STORED`, do Backend gửi **sau database commit**.
- Heartbeat **2 giây**, Backend offline timeout **6 giây**, Backend chạy tác vụ kiểm tra mỗi **1 giây** (phương án A đã chọn). Mục tiêu UI Offline ≤10 giây phải được đo thực tế.
- Telemetry mặc định **5 giây**, dải cấu hình theo NFR-IOT-003 là **1–60 giây**.
- MQTT Telemetry hiện **QoS 0 / retain=false** theo tài liệu QoS P1; cơ chế Application ACK + retry ở tầng ứng dụng đảm nhiệm xác nhận lưu ở Backend. Command/ACK, LWT có chính sách riêng trong DSN-08.
- ESP32 được gán **cố định theo Zone trong MQTT contract v1** theo tài liệu P1 hiện hành.

> **Chưa được chứng minh bởi tài liệu thiết kế:** Buffer bền vững đủ 17.280 bản ghi, hoạt động offline ≥24 giờ, độ trễ thực tế và khả năng khôi phục sau mất nguồn. BUILD demo có thể mô phỏng một phần nhưng phải phân biệt kết quả demo và đạt NFR.

## 2. Thành phần và trách nhiệm

| Thành phần | Trách nhiệm |
|---|---|
| Sensor Manager | Lấy mẫu, kiểm tra từng giá trị và tạo dữ liệu/sự kiện lỗi |
| Application / State Manager | Quản lý mode actuator, tình trạng kết nối, safety policy |
| Control Manager | Threshold, hysteresis, thực thi AUTO/MANUAL theo policy |
| MQTT Manager | Publish/subscribe, heartbeat, reconnect, nhận ACK, chuyển lệnh |
| Config & Buffer Manager | Cấu hình hợp lệ cuối cùng, lưu bản tin chưa ACK, phát lại, xóa theo ACK |
| HAL / GPIO | Đọc sensor, điều khiển relay và chỉ báo trạng thái GPIO nếu có |
| Spring Boot — P4 | Validate, deduplicate, lưu Telemetry, commit rồi mới gửi ACK; lưu trạng thái Device và cảnh báo |

## 3. Vòng đời Telemetry

1. Theo chu kỳ đọc, Sensor Manager đo nhiệt độ, độ ẩm, CO₂ và ánh sáng; kiểm tra từng phép đo.
2. Tạo bản tin Telemetry duy nhất với `event_id`, `device_id`, `timestamp` tại **thời điểm đo**; dùng `null` cho phép đo lỗi, `sensor_status = OK | PARTIAL_ERROR | ERROR`.
3. Config & Buffer Manager **ghi bản tin vào hàng đợi**. Nếu yêu cầu bảo toàn sau reset/mất điện, hàng đợi phải được ghi vào lưu trữ **bền vững** trước khi publish.
4. Nếu MQTT kết nối: MQTT Manager publish vào topic `.../telemetry`. Nếu mất kết nối: giữ bản tin, lên lịch reconnect.
5. P4 nhận và kiểm tra payload/nguồn/thiết bị; kiểm tra `(device_id, event_id)`; lưu trong transaction và commit.
6. Sau commit, P4 publish lên `.../telemetry_ack` với `event_id` tương ứng và `status: STORED`; nếu bản tin trùng **cùng nội dung** và đã lưu, cũng gửi `STORED`.
7. P1 nhận ACK qua topic đã subscribe, kiểm tra thiết bị, `event_id`, trạng thái và nguồn MQTT hợp lệ; đánh dấu hoàn thành, sau đó giải phóng bản tin khỏi hàng đợi. ACK sai ID hoặc báo không thành công **không cho phép xóa**.
8. Nếu timeout/mất ACK/Backend lỗi: giữ bản tin, lên lịch retry có backoff; không làm ngừng chu kỳ Sensor và AUTO.

### 3.1. Phân biệt xác nhận

- **MQTT QoS 0:** không có PUBACK; lời gọi publish thành công không chứng minh Broker đã chuyển bản tin tới Backend.
- **MQTT PUBACK (nếu QoS 1):** chỉ xác nhận đầu nhận MQTT của hop tương ứng, **không** xác nhận database commit.
- **Application ACK `STORED`:** P4 phát sau commit; đây là điều kiện để P1 xóa bản ghi khỏi Buffer.
- **Command ACK:** phản hồi việc xử lý **lệnh** từ ESP32, không thay cho Telemetry ACK.

### 3.2. Pseudo-flow không chặn

```text
Sensor timer (5s):
  data = read_and_validate_sensors()
  event = create_telemetry(event_id=stable_id(), timestamp=measurement_time)
  buffer.persist(event)          # nếu bật bảo toàn sau mất nguồn
  enqueue_for_publish(event)

MQTT worker (background):
  if disconnected: reconnect_with_backoff(); return
  for event in pending_messages_by_policy():
      if event.due_for_retry: publish(event); mark_waiting_ack(event)

ACK handler:
  if authenticated_topic && ack.status == "STORED" && ack.event_id exists in pending:
      buffer.mark_confirmed_and_remove(ack.event_id)
  else:
      log_ack_and_keep_buffer()
```

Tuyệt đối không `delay()`/chờ ACK theo kiểu chặn luồng điều khiển. Công tác đồng bộ hàng đợi sau reconnect không chặn gửi mẫu mới và không ảnh hưởng xử lý safety.

## 4. Các tình huống offline và phục hồi

### A. Mất Wi-Fi

- Phát hiện trạng thái kết nối; đánh dấu `connectionState=OFFLINE`; không mặc định đổi `controlMode`.
- Tiếp tục AUTO cục bộ theo ngưỡng đã lưu; MANUAL tuân thủ thời hạn override và safe policy.
- Lưu Telemetry/sự kiện cần gửi vào Buffer; không tiếp tục coi publish là thành công.
- Wi-Fi reconnect theo backoff; sau đó khôi phục MQTT connection và subscribe lại.

### B. Wi-Fi còn nhưng MQTT Broker mất kết nối

- Không đồng nhất tình huống này với mất cảm biến; ESP32 tiếp tục đo và điều khiển.
- Buffer bản tin chờ ACK; reconnect MQTT; subscribe lại `command`, `configuration`, `telemetry_ack` và topic liên quan trước khi xử lý tiếp.
- Khi khôi phục publish Availability ONLINE theo contract; tiếp tục gửi dữ liệu tồn đọng ở tốc độ được giới hạn.

### C. Broker còn nhưng Java Backend hoặc Database không hoạt động

- ESP32 có thể publish Telemetry vào Broker, nhưng **không nhận Application ACK**.
- Bản tin **không bị xóa**; sau ACK timeout lên lịch resend cùng `event_id`.
- Khi P4 phục hồi, Backend xử lý duplicate idempotent và ACK cho dữ liệu đã lưu.
- Không sử dụng tín hiệu `connectionState=ONLINE` để kết luận Backend/database khỏe; đây là hai điều kiện riêng.

### D. ESP32 reset/mất điện

- Khởi tạo GPIO/relay an toàn, đọc cấu hình **hợp lệ cuối cùng** và kiểm tra trước khi vào AUTO.
- Nếu dùng queue bền vững: khôi phục các bản tin chưa ACK, giữ nguyên `event_id`/timestamp, tránh lỗi ghi nửa chừng; cần kiểm thử thực tế.
- Không mặc định phát lại Command đã hết hạn hoặc thực thi lại Command trùng.
- Nếu chỉ dùng RAM queue để demo: sau reset có thể mất bản tin. Phải ghi rõ **không đạt** tiêu chí buffer bền vững/24h.

### E. Reconnect và RECOVERING

1. Kết nối Wi-Fi và MQTT; kiểm tra việc subscribe thành công.
2. Publish Availability ONLINE; bắt đầu lại heartbeat 2 giây và telemetry mới theo lịch.
3. Kiểm tra cấu hình có phiên bản mới; không tự áp dụng cấu hình chưa hợp lệ.
4. Retry các Telemetry chưa ACK, bảo toàn ID/timestamp, và áp dụng giới hạn số bản tin gửi bù mỗi vòng.
5. Chỉ rời RECOVERING khi đạt tiêu chí đã chọn (ví dụ MQTT ổn định và đồng bộ cấu hình); **không đợi Buffer trống hoàn toàn mới cho phép AUTO hoặc dữ liệu mới**.

## 5. Backoff, timeout, Buffer-full

| Tham số | Quy định |
|---|---|
| ESP32 reconnect | **Đề xuất chờ P1/P4 xác nhận:** 1, 2, 4, 8, 16, tối đa 30 giây (có thể áp dụng chính sách giống Backend nhưng không bắt buộc) |
| Telemetry ACK timeout | **TBD P1 + P4**, tách khỏi Command ACK timeout 10 giây |
| Command ACK timeout | **10 giây** theo DSN-05 P4, không áp dụng cho Telemetry ACK |
| Command expiration | **15 giây kể từ lúc tạo** theo DSN-05 P4 |
| Telemetry buffer NFR | **≥17.280 record/24h ở chu kỳ 5s** (mục tiêu, chưa test) |
| Buffer đầy | **TBD P1 + P4:** cảnh báo, ghi rõ số record bị mất; chính sách giữ/bỏ nào cũng phải được phê duyệt |
| 1–60s | Dải cấu hình chu kỳ Telemetry; ở chu kỳ 1s cần đến 86.400 record/24h nếu giữ toàn bộ mẫu |

**Không được tuyên bố đạt bảo đảm không mất dữ liệu** nếu chưa có lưu bền vững đủ dung lượng, xử lý buffer-full và kiểm thử mất nguồn. Việc đánh dấu `STORED` sau commit kết hợp unique constraint tạo *at-least-once delivery + idempotent persistence* trong các điều kiện vận hành đã giả định, không biến mạng thành always-on.

## 6. Sensor error và safe state

- Kiểm tra theo **từng** cảm biến. `PARTIAL_ERROR` vẫn giữ giá trị sensor hợp lệ; sensor lỗi mang `null` và không được dùng để điều khiển AUTO phụ thuộc.
- `ERROR`: không còn đủ dữ liệu để thực hiện chính sách cho một số actuator, hoặc điều kiện nghiêm trọng theo đặc tả. Chuyển actuator liên quan sang safe handling đã chốt; không tự suy luận `OFF` là an toàn cho mọi thiết bị.
- Chuyển từ MANUAL sang AUTO chỉ khi có yêu cầu đổi mode hợp lệ, timeout override nếu được cho phép, hoặc safety override theo policy; **mất mạng không tự chuyển mode**.
- P1 phải chốt safe state riêng cho MIST/FAN/HEATER/AMPLIFIER (hoặc đánh dấu TBD và không kích tải nguy hiểm trong demo).
- Lỗi sensor được phản ánh trong Telemetry và/hoặc status/error event đã được định nghĩa trong DSN-08, không tạo một topic tùy ý ngoài contract.

## 7. Heartbeat, Availability và Dashboard

**Phương án A đã chọn:** P1 publish heartbeat mỗi **2 giây**; P4 ghi `lastSeenAt` khi nhận heartbeat hợp lệ, chạy `@Scheduled` **mỗi 1 giây**, đánh dấu OFFLINE khi `now - lastSeenAt > 6 giây`. P4 phát WebSocket khi Device đổi trạng thái; P5 cập nhật Dashboard. Mục tiêu end-to-end OFFLINE ≤10 giây **cần đo thực tế**, bao gồm thời gian scheduler, broker, mạng và UI.

- Heartbeat và Availability là hai cơ chế bổ trợ. LWT publish `OFFLINE` khi Broker phát hiện ngắt kết nối theo MQTT Keep Alive; không cam kết luôn phát ngay lập tức.
- Khi ESP32 kết nối lại MQTT, publish Availability `ONLINE` (QoS 1, retained), nhưng Java nên xác nhận heartbeat hợp lệ và bảo vệ khỏi bản tin status cũ đến muộn.
- **Quan trọng:** Sơ đồ cần **2 luồng độc lập**: (1) ESP32 → Broker → Backend cập nhật `lastSeenAt`; (2) Java Scheduler → kiểm tra timeout → ONLINE/OFFLINE → WebSocket/Dashboard. Không kiểm tra timeout chỉ tại thời điểm nhận heartbeat.

## 8. Kịch bản kiểm thử tích hợp BUILD

| ID | Kịch bản | Kết quả mong đợi |
|---|---|---|
| IT-01 | ESP32 publish Telemetry hợp lệ | Java lưu DB, trả ACK `STORED`, P1 xóa đúng `event_id` |
| IT-02 | Java nhận lại `event_id` cùng payload | Không chèn trùng, trả ACK `STORED` |
| IT-03 | Java nhận cùng `event_id` nhưng nội dung khác | Từ chối và ghi lỗi, không ACK `STORED` |
| IT-04 | Tắt Backend, giữ Broker | P1 không xóa Buffer, tiếp tục AUTO; khi P4 khởi động lại dữ liệu được gửi bù |
| IT-05 | Mất Wi-Fi/Broker ngắn | ESP32 tiếp tục AUTO, reconnect, gửi bù và không làm nghẽn mẫu mới |
| IT-06 | Ngắt heartbeat | Backend scheduler phát hiện OFFLINE, UI cập nhật; đo toàn tuyến ≤10s |
| IT-07 | Một cảm biến trả lỗi | Payload dùng null/PARTIAL_ERROR; phần AUTO độc lập vẫn hoạt động |
| IT-08 | ESP32 nhận lệnh trùng/hết hạn | Không thực thi trùng; Command ACK phản ánh kết quả |
| IT-09 | Reset khi còn pending | Kiểm tra khôi phục dữ liệu nếu queue bền vững được triển khai; nếu chỉ RAM ghi FAIL/DEFERRED |
| IT-10 | Buffer đầy và mạng phục hồi | Ứng xử theo chính sách buffer-full đã chốt; đo thất thoát thực tế |



