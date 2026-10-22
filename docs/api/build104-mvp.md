# BUILD1-04 — hợp đồng triển khai MVP, 23/10/2026

Đọc cùng `openapi.yaml`, `api-conventions.md`, `realtime-contract.md` và `../mqtt/topics.md`. OpenAPI vẫn chứa nhiều endpoint thiết kế chưa triển khai; chỉ các endpoint trong bảng này đã có code BUILD1-04. Các bổ sung bên dưới là quyết định MVP để giải quyết phần contract còn mở, cần cross-review P1/P2/P4/P5.

| Method | URL | Auth / response |
|---|---|---|
| POST | `/api/v1/auth/login` | `{email,password}` → `{accessToken,tokenType:"Bearer",expiresIn:900}`, cookie SC_WS |
| GET | `/api/v1/zones/{zoneId}/devices?page=0&size=20` | Bearer JWT; list Device |
| GET | `/api/v1/devices/{deviceId}` | Bearer JWT; DeviceResponse |
| GET | `/api/v1/devices/{deviceId}/telemetry/latest` | Bearer JWT; TelemetryResponse; 404 nếu chưa có mẫu |
| GET | `/api/v1/devices/{deviceId}/telemetry?from=...&to=...&page=0&size=20` | Bearer JWT; list Telemetry |
| POST | `/api/v1/internal/ai/count-logs` | X-Service-Key; CountLogResponse, 201 mới / 200 retry giống / 409 xung đột |
| GET upgrade | `/ws/devices/{deviceId}` | Plain WebSocket JSON, cookie SC_WS hoặc Authorization Bearer cho client hỗ trợ header |

REST IDs là BIGINT PK. Telemetry latest: timestamp DESC, receivedAt DESC, id DESC. History: from inclusive/to exclusive, from < to, thứ tự như latest. Page 0-based; size 1..100; envelope `{items:[],page:{page,size,totalElements,totalPages}}`. DTO theo OpenAPI, không serialize entity/passwordHash. Error `{code,message,timestamp,path}`; validation 400, JWT/service key 401, membership 403, missing 404, conflict 409, lỗi DB tại request 503.

Mapping triển khai: [Entity/database MVP](../design/erd/build104-mvp-mapping.md). Telemetry ingest tuân thủ NFR-IOT-004: temperature -10..60°C, humidity 0..100%, co2 0..5000 ppm, light 0..100000 lux, bao gồm hai biên; từ chối số không hữu hạn/ngoài dải trước transaction và không trả STORED. Null + sensorStatus lỗi vẫn hợp lệ theo [MQTT contract](../mqtt/topics.md). Đây là sửa thiếu validation của implementation trước, không phải nới hoặc thay NFR.

## Quyền và JWT tối thiểu

JWT HS256, issuer swiftletcare, subject users.id, hạn 15 phút. JWT_SECRET ít nhất 32 byte UTF-8, AI_SERVICE_KEY ít nhất 24 ký tự, lấy từ môi trường; không có secret mặc định trong ứng dụng. BCrypt login. Read API/WS kiểm tra User active, HouseMembership active, House/Zone active theo thiết bị. Global OWNER/ADMIN không vượt membership. WS kiểm tra lại quyền và hạn token trước mỗi lần gửi event; khi mất quyền thì đóng socket. AI service key độc lập, không nhận JWT browser thay service key.

MVP **chưa có refresh/logout/revoke token**, CRUD tài khoản/membership, rate limit hoặc phân quyền riêng từng camera cho nhiều AI producer. Login trả access token, **không trả TokenPairResponse/refreshToken** của bản thiết kế rộng hơn. Đăng nhập lại khi hết 15 phút. SECURE_COOKIE=true khi dùng HTTPS; local cùng site localhost dùng cookie HttpOnly, SameSite=Strict, path=/ws. CORS/origin liệt kê cụ thể qua FRONTEND_ORIGINS (dấu phẩy); không dùng wildcard. Với frontend khác site cần thiết kế lại cookie hoặc auth handshake trước khi triển khai.

## CountLog: chốt khóa chống trùng MVP

Giữ payload CountLogIngestRequest hiện có, không thêm eventId chưa được định nghĩa cho aggregate. UNIQUE `(device_id, aggregation_type, period_start, period_end)`. Mỗi camera/type/window chỉ có một aggregate bất biến; retry phải giữ nguyên entryCount/exitCount/detectCount/processedAt. Bản sửa aggregate cùng khóa trả 409, không âm thầm ghi đè hoặc cộng dồn. Các event BIRD_ENTRY/BIRD_EXIT trong AI event draft là hợp đồng khác, chưa được ingest qua endpoint aggregate này. P2 cần gửi window ổn định, không tự tạo timestamp mới khi retry.

Validate camera enabled, đúng loại CAMERA, House/Zone active; counts là integer >=0, HOURLY/DAILY, periodStart < periodEnd <= processedAt, processedAt không quá 5 phút trong tương lai. Các timestamp chuẩn hóa microsecond. Service key dùng chung cho AI MVP, chưa scope theo camera. Không tải evidence URI hoặc xử lý YOLO trong backend.

```json
{"deviceId":2,"periodStart":"2026-10-22T00:00:00Z","periodEnd":"2026-10-22T01:00:00Z","entryCount":12,"exitCount":3,"detectCount":15,"aggregationType":"HOURLY","processedAt":"2026-10-22T01:00:01Z"}
```

## Frontend / WebSocket

```javascript
const base = 'http://localhost:8080';
const login = await fetch(`${base}/api/v1/auth/login`, {
  method: 'POST', credentials: 'include',
  headers: {'Content-Type':'application/json'},
  body: JSON.stringify({email:'local@example.test', password:'your-local-password'})
});
const {accessToken} = await login.json();
const device = await fetch(`${base}/api/v1/devices/1`, {
  headers: {Authorization:`Bearer ${accessToken}`}
}).then(r => r.json());
const ws = new WebSocket('ws://localhost:8080/ws/devices/1');
ws.onmessage = ({data}) => console.log(JSON.parse(data));
```

ID 1/2 chỉ minh họa; lấy ID thật qua zone device list/local database. Không đặt JWT trong URL. Cookie được trình duyệt gửi cho socket cùng site sau login credentials=include.

Envelope `{type,data}`:

- `telemetry.updated`: data là TelemetryResponse đầy đủ, chỉ sau commit bản mới.
- `device.status.changed`: data là DeviceResponse đầy đủ; online và lastSeenAt phản ánh heartbeat/scheduler đã commit.
- `ai.count.updated`: data là CountLogResponse đã commit, chỉ khi insert mới.

Mỗi socket subscribe đúng một device qua path. Không có STOMP/SockJS. UI reconnect phải đăng nhập lại nếu token hết hạn rồi GET history/latest để bù khoảng thiếu, loại trùng theo deviceId/eventId. WebSocket không lưu hàng đợi/outbox và chưa kiểm thử scale nhiều instance. NFR UI offline <=10s chưa được chứng minh trên React thực.

REST catch-up trên chỉ áp dụng Telemetry/Device trong MVP. Chưa có GET CountLog nên không bảo đảm phục hồi sự kiện ai.count.updated bị lỡ qua REST. Bộ chọn House/Zone phải nhận ID đã biết từ cấu hình/DB local; các API lấy House/Zone trong thiết kế tổng thể chưa được triển khai. Không dùng command/alert/report trên UI BUILD1-04 như chức năng đã có.
