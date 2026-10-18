# SwiftletCare — REST API Conventions v0.9 (review)

## 1. Giao thức & tên

- JSON UTF-8 trên `HTTP(S)`; prefix `/api/v1/`. Trong BUILD dùng `http://localhost:8080`; triển khai thực tế dùng HTTPS.
- Danh từ số nhiều, resource ID dùng path; GET đọc, POST tạo, PATCH cập nhật một phần, PUT replace/upsert cấu hình theo version.
- REST JSON dùng **camelCase** theo DTO Java. MQTT (P1) **dùng `event_id`** cho Telemetry; JPA `Telemetry.eventId` map cột `event_id`.
- REST `{deviceId}` là khóa số `devices.id` (`BIGINT`), còn `deviceId` trong MQTT topic/payload là mã thiết bị dạng chuỗi (`devices.device_id`). Tránh lẫn trường này bằng tài liệu DTO.
- `User.passwordHash` không xuất trong DTO trả về. Không trả credential MQTT/JWT qua JSON ngoài API Auth.

## 2. Thời gian, phân trang và lọc

- API `date-time` ISO-8601 UTC (`2026-10-18T08:30:00Z`); Java `Instant`, SQL timestamp/UTC theo DB được chọn.
- `from` inclusive, `to` exclusive; `from < to` (400 nếu ngược). Ví dụ `?from=2026-10-18T00:00:00Z&to=2026-10-19T00:00:00Z`.
- `page` 0-based default 0, `size` default 20, max 100 (đề xuất cần P4/P5 duyệt); list response `{items: [...], page: {page,size,totalElements,totalPages}}`.
- Tất cả các list phải lọc theo quyền House/Zone; kể cả khi client tự cung cấp ID không được vượt RBAC.
- Telemetry `latest` dựa theo **timestamp đo**, không chỉ `receivedAt`, vì Offline replay dữ liệu cũ có thể đến muộn.
- Response dùng các Enum đúng Class: `SensorStatus`, `CommandStatus`, `AlertStatus`, `HouseRole`, v.v.; DB lưu enum name bằng `EnumType.STRING` nếu JPA.

## 3. Quy ước HTTP

| Status | Ý nghĩa |
|---|---|
| `200` | Truy vấn/cập nhật đồng bộ thành công |
| `201` | Tạo một Entity thành công |
| `202` | Đã **tiếp nhận** tác vụ bất đồng bộ (ControlCommand); chưa xác nhận ESP32 thực thi |
| `204` | Thành công, không trả body |
| `400` | JSON/parameter sai, thời gian sai, trùng field không hợp lệ |
| `401` | Thiếu/không hợp lệ JWT hoặc service key |
| `403` | Có định danh nhưng không đủ quyền resource |
| `404` | Không tìm thấy hoặc resource không được công khai |
| `409` | Trùng định danh, version conflict, actuator sai mode, command conflict |
| `422` | Dữ liệu đúng cú pháp nhưng không hợp lệ theo nghiệp vụ (tùy nhóm chọn 400/422) |
| `503` | Broker/DB/service thiết yếu không khả dụng; không giả trả success |

**Error DTO thống nhất:**

```json
{
  "code": "ACTUATOR_NOT_IN_MANUAL_MODE",
  "message": "Actuator must be MANUAL for SET_STATE",
  "timestamp": "2026-10-18T08:35:00Z",
  "path": "/api/v1/actuators/25/commands"
}
```

Các mã lỗi nghiệp vụ đề xuất: `VALIDATION_ERROR`, `RESOURCE_NOT_FOUND`, `ACCESS_DENIED`, `VERSION_CONFLICT`, `DEVICE_OFFLINE`, `ACTUATOR_NOT_IN_MANUAL_MODE`, `COMMAND_EXPIRED`, `BROKER_UNAVAILABLE`, `AI_SOURCE_INVALID`, `DUPLICATE_EVENT`. P4 chốt một bộ tên trước code.

## 4. Các quy tắc dữ liệu khó

1. `Telemetry`: `eventId` camelCase REST; các trường đo `temperature`, `humidity`, `co2`, `light` **nullable**, `sensorStatus: OK/PARTIAL_ERROR/ERROR`. Không đặt `0` để thay sensor bị lỗi. Device gửi bằng MQTT, Backend persist và ACK ở MQTT sau commit.
2. `ControlCommand`: `action=SET_MODE` **chỉ** có `targetMode`, `action=SET_STATE` **chỉ** có `targetState`. JSON schema cho phép cả hai field tùy chọn nhưng phải có một trong hai đúng action tại validation Service.
3. `ControlCommand.status`: `PENDING/SENT/ACKNOWLEDGED/FAILED/REJECTED/EXPIRED`; ACK timeout nếu không thêm Enum mới thì lưu `FAILED + failureReason=ACK_TIMEOUT`. HTTP `202` là tạo lệnh, không phải `ACKNOWLEDGED`.
4. `ZoneConfig`: `version` làm optimistic concurrency; `PUT` lưu DB thành công không đồng nghĩa ESP32 đã nhận cấu hình. `syncStatus` trong Response là **đề xuất DTO dẫn xuất**, không phải cột DB hiện có.
5. `Alert`: `NEW → IN_PROGRESS → RESOLVED`, việc ghi `AlertHistory` phải cùng transaction; `handledAt` nullable trước khi hoàn tất.
6. `NotificationDelivery`: `DELIVERED` khác “đã đọc”; không thêm `readAt` trừ khi FR mới yêu cầu.
7. `Report`: `Report` là metadata file đã tạo, thống kê GET có thể tính trực tiếp mà không sinh một bản Report.
8. `AnomalyEvent`: `snapshotPath` là đường dẫn/khóa bằng chứng do P2 và P4 chốt; không công khai URL vượt quyền House.

## 5. Security & kiểm thử

- User API qua Bearer JWT; AI internal API qua service authentication (dự thảo `X-Service-Key`, cần P2/P4 xác nhận). Không dùng JWT Web cho ESP32.
- Authorization ở Service theo HouseMembership và quan hệ `Zone→House`, `Device→Zone`, `Actuator→Device`, `Alert→Zone`.
- Validate dữ liệu/DTO ở Spring Boot (`@Valid`, bean validation + cross-field validators) và DB constraint. Dữ liệu không hợp lệ không đổi trạng thái điều khiển.
- Secret nằm trong environment variables; log không chứa JWT/password/service key.
