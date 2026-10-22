# BUILD1-04 — kết quả thực tế ngày 23/10/2026 (Asia/Saigon)

## Cập nhật đồng bộ tài liệu và validation NFR-IOT-004

Lượt `mvn -f backend/pom.xml clean verify` mới kết thúc **02:00:59 +07:00 ngày 23/10/2026**: **BUILD SUCCESS; 13 tests, 0 failures, 0 errors, 0 skipped**. BackendIntegrationTest: 2; IngestionIntegrationTest: 11. PostgreSQL/MQTT riêng của project swiftletcare-build104-e2e; không chạy trên DB local nghiệp vụ.

- Giữ 10 test cũ và thêm 3 method: biên inclusive của cả bốn phép đo + null/ERROR; 8 trường hợp ngay ngoài hai biên của từng dải qua MQTT (không row/STORED); NaN/+Infinity/-Infinity cho từng trường qua service (12 trường hợp, từ chối trước lưu).
- Test MQTT ngoài dải gửi một bản hợp lệ tiếp theo làm mốc xác nhận handler đã xử lý; ACK đầu phải thuộc bản hợp lệ, bản ngoài dải không có row. Các test cũ tiếp tục kiểm tra transaction/ACK, duplicate, heartbeat, REST, WS, CountLog và JWT.
- NFR dùng nguyên dải đã ghi: temperature -10..60°C, humidity 0..100%, co2 0..5000 ppm, light 0..100000 lux. Không có ngoại lệ BUILD1-04 trong nguồn đã đọc; contract MQTT trước đây mô tả validation yếu hơn là thiếu implementation, không phải căn cứ nới NFR.
- OpenAPI structural validator PASS; đối chiếu metadata/catalog với code: **6 REST operation đã triển khai / 41 operation giữ thiết kế giai đoạn sau**. Kiểm tra đường dẫn nội bộ các tài liệu bàn giao đã sửa: không có link hỏng.
- Log mới: `backend/logs/build104-nfr-verify.log`, `backend/logs/build104-doc-validation.log`. Giữ toàn bộ log cũ và nội dung bằng chứng cũ bên dưới. Báo cáo JUnit XML hiện phản ánh 13 test của lượt mới.
- Chỉ thay logic `Ingestion.validate()` và bổ sung test; V1/V2, cấu hình Docker/MQTT và các API nghiệp vụ khác giữ nguyên. Validation mới áp dụng service ingest, không tự sửa dữ liệu lịch sử hoặc tăng DB CHECK trong V2 đã áp dụng.
- **Không chạy lại Docker build/container smoke hoặc recovery JAR độc lập trong đợt này**; kết quả các phép đó ở bảng cũ bên dưới thuộc lượt trước sửa NFR. Chưa nghiệm thu firmware/React/AI thật, AUTO an toàn hoặc toàn bộ NFR-IOT-004 trên phần cứng.

## Hồ sơ kiểm thử trước đợt đồng bộ NFR

Nhánh `htran`; working tree ban đầu sạch. Không tìm thấy AGENTS.md trong workspace hoặc các thư mục cha đã kiểm tra. Đã đọc code ban đầu và tài liệu API/conventions/security/realtime, ERD/data dictionary/constraints, class diagram, integration flows, IoT offline/recovery và AI event contract. Không commit/push/merge; V1 không thay đổi; không xóa dữ liệu/volume hiện có.

## Môi trường kiểm thử

- Windows PowerShell, Temurin Java 21.0.12.1, Maven 3.10.0, Docker Desktop Linux containers.
- Compose project riêng `swiftletcare-build104-e2e`, file `compose.build104-test.yml`.
- PostgreSQL 17, user `build104`, DB `build104_test`, port 55432; volume riêng `swiftletcare-build104-e2e_build104_test_data`.
- Mosquitto 2, port 51883. Không dùng DB/container local ban đầu ở port 5432.
- JAR smoke dùng DB riêng `build104_smoke` trên PostgreSQL test, port HTTP 58080. Container smoke dùng port HTTP 58081, cùng DB smoke và network test.

## Kết quả nền tảng trước đợt đồng bộ NFR

| Kiểm tra đã chạy | Kết quả thực tế |
|---|---|
| `mvn -f backend/pom.xml clean verify` với DB_TEST_ENABLED=true | **BUILD SUCCESS**, 10 tests, 0 failures, 0 errors, 0 skipped; kết thúc 01:05:08 +07:00 |
| Flyway/JPA | DB smoke mới áp dụng V1 rồi V2; DB test chạy lại không có migration mới; JPA validate và EntityManagerFactory thành công |
| MQTT → DB → ACK | Subscribe/publish thật qua Mosquitto; nhận STORED rồi query DB bằng connection độc lập thấy bản ghi đã commit |
| Duplicate/conflict | Retry giống nhận ACK, DB giữ 1 row; cùng ID khác dữ liệu không có STORED; 8 request ingest đồng thời chỉ 1 insert |
| Payload/device | JSON sai, humidity sai, sensor_status không khớp null, unknown field/device, disabled device, retained replay bị từ chối; không có ACK thành công |
| Timestamp precision | Timestamp nanosecond chuẩn hóa microsecond; retry nhận lại STORED, không xung đột giả |
| Lỗi ngay tại commit | Constraint trigger DEFERRABLE INITIALLY DEFERRED gây lỗi commit; không ACK/realtime telemetry, không row; gỡ trigger rồi retry thành công |
| DB dừng thật | Không ACK thành công; REST Device, health, readiness trả 503; liveness 200; khởi động DB lại rồi retry có ACK và đúng 1 row |
| Heartbeat/WS | Online → timeout Offline → Online, WebSocket nhận status sau commit; lastSeenAt theo receipt time dù timestamp heartbeat nguồn rất cũ |
| Timeout mặc định JAR | HEARTBEAT_TIMEOUT_MS=6000, scan=1000; lượt smoke cuối đo **6,70s** tới Offline bằng REST polling (lượt trước 6,20s) |
| MQTT reconnect | Restart broker khi JAR đang chạy → tự reconnect/resubscribe → DB/ACK thành công |
| REST | Device single/list theo Zone; telemetry latest theo timestamp đo; history from inclusive/to exclusive; phân trang tách bản ghi, metadata tổng; size/range sai trả 400 |
| WebSocket thật | Java HTTP WebSocket client nhận telemetry.updated và device.status.changed; dữ liệu đã commit; cookie JWT handshake hoạt động; origin lạ bị chặn |
| CountLog | 201 hợp lệ, 200 retry giống cùng ID; 409 nội dung xung đột; 400 counts âm/ESP32; 401 thiếu service key; 4 request đồng thời chỉ 1 response 201 |
| JWT/membership/CORS | Login đúng/sai; thiếu/sai/hết hạn JWT; User hoặc Membership inactive không đọc được; preflight origin React có header cho phép |
| Dockerfile build | `docker build -t swiftletcare-backend:build104-test backend` thành công với mã cuối |
| Container chạy thật | UID/GID 999 swiftlet; health/JPA/JWT/REST thành công; khởi động lúc broker chưa chạy, bật broker sau đó tự connect/subscribe và trả ACK cho row đã lưu |
| OpenAPI structural validator | Parse YAML, refs, operationIds, parameters, responses, security **PASS**: 47 operations/36 paths/70 schemas. Không phải full official meta-schema validation |

JUnit gồm 2 test nền tảng trong `BackendIntegrationTest` và 8 test tích hợp trong `IngestionIntegrationTest`. Các test gộp nhiều assertion nghiệp vụ; số 10 là số method JUnit, không phải số dòng trong bảng trên. Timeout của suite được rút còn 1200ms/scan100ms; phép đo 6,70s dùng JAR với cấu hình mặc định.

Các lỗi phát hiện và sửa trước lượt cuối: CORS source chưa được Spring Security chọn; xung đột bean với MVC introspector; REST khi không mở được transaction cần ánh xạ CannotCreateTransactionException sang 503; timestamp cần chuẩn hóa trước khi PostgreSQL làm tròn. Smoke ban đầu có HTTP warmup timeout; runner cuối chờ readiness và dùng IPv4 loopback. Không tính các lượt lỗi là pass.

Log cục bộ không commit nằm trong `backend/logs/`: `build104-verify.log`, `build104-smoke.log`, `build104-docker-build.log`, `build104-container-smoke.log`, `build104-openapi.log` và các log chẩn đoán/build khác. Báo cáo XML vẫn nằm trong `backend/target/surefire-reports/`. Việc chuyển thư mục giữ nguyên nội dung bằng chứng kiểm thử.

## File / chức năng bàn giao

| File hoặc thư mục | Chức năng |
|---|---|
| `backend/src/main/resources/db/migration/V2__mvp_ingestion.sql` | 7 bảng MVP, FK, unique/check/index; V1 giữ nguyên |
| `backend/src/main/java/com/swiftletcare/management/` | User/House/Zone/Device và repository |
| `backend/src/main/java/com/swiftletcare/iot/` | Telemetry/repository, transactional upsert, MQTT ACK, heartbeat và scheduler |
| `backend/src/main/java/com/swiftletcare/ai/` | CountLog/repository, service-key endpoint và dedup aggregate |
| `backend/src/main/java/com/swiftletcare/api/` | DTO, Device/Telemetry API, phân trang và error envelope |
| `backend/src/main/java/com/swiftletcare/security/` | BCrypt login, JWT, CORS, kiểm tra membership |
| `backend/src/main/java/com/swiftletcare/realtime/` | Plain WebSocket theo device, cookie/Bearer, origin và quyền |
| `backend/src/main/java/com/swiftletcare/config/LocalData.java` | Seed opt-in profile local; không overwrite dữ liệu |
| `backend/pom.xml`, `application.yml`, application entrypoint | Dependencies, biến cấu hình và scheduling |
| `docker-compose.yml`, `infra/mosquitto/mosquitto.conf`, `backend/Dockerfile`, `.dockerignore`, `.env.example` | Broker local, app profile, image multi-stage chạy non-root và biến mẫu |
| `compose.build104-test.yml`, hai integration test, `scripts/test-build104-recovery.ps1`, `scripts/build104-smoke.py` | Stack test riêng và các kiểm tra tự động/recovery |
| `backend/README.md`, `docs/mqtt/topics.md`, `docs/api/build104-mvp.md`, `openapi.yaml`, `realtime-contract.md` | Lệnh Windows, ví dụ MQTT/HTTP/React WebSocket và quyết định contract MVP |

## Checklist BUILD1-04

1. **Xong:** Entity/Repository User, House, Zone, Device, Telemetry; thêm CountLog và bảng membership để scope quyền.
2. **Xong:** MQTT subscribe Telemetry/Heartbeat, reconnect và broker Compose local.
3. **Xong:** Validate payload/device/topic; unique (device_id,event_id), concurrent dedup và conflict.
4. **Xong:** Transactional telemetry; chỉ STORED sau commit, ACK lại duplicate, không ACK khi sai/lỗi DB.
5. **Xong:** Online/Offline theo receipt heartbeat, timeout/scan cấu hình được.
6. **Xong:** Device và telemetry latest/history REST, DTO, time filter, paging, thống nhất lỗi.
7. **Xong trong MVP:** WS telemetry/status sau commit, origin React, JWT và House scope.
8. **Xong trong MVP:** CountLog validate/persist; unique aggregate window bất biến, cross-review P2 còn cần.
9. **Xong tối thiểu:** JWT login/access token 15 phút; **chưa xong** refresh/logout/revocation/rate limit.
10. **Xong:** Dockerfile build và chạy thật, README/ví dụ, dữ liệu seed chỉ profile local/test.

## Giới hạn chưa hoàn thành / chưa kiểm thử

- MQTT topic trước đó rỗng và CountLog key còn mở: lựa chọn MVP đã ghi rõ, chưa có bằng chứng P1/P2/P4/P5 phê duyệt hoặc tích hợp firmware/AI thực.
- Chưa có outbox/durable delivery cho ACK và realtime; producer phải retry, frontend phải REST catch-up. MVP một backend instance.
- Broker anonymous chỉ local; chưa triển khai hoặc kiểm thử TLS/ACL/auth broker sản xuất. Key AI dùng chung, chưa scope theo từng camera/producer.
- Chưa có refresh/logout/revocation JWT, CRUD account/membership hoặc toàn bộ endpoint thiết kế OpenAPI; chỉ subset trong build104-mvp.md có implementation.
- Chưa chạy UI React thực để chứng minh mục tiêu Offline <=10s toàn tuyến; 6,70s là phép đo backend qua REST. Chưa kiểm thử tải lớn, HA, buffer firmware 24h hay mất nguồn ESP32.
- Chưa chạy full official OpenAPI meta-schema validator. Structural validator hiện có đã chạy và pass.

Sau kiểm thử, dừng các container/JAR do phiên này tạo; giữ database/volume test và image. Stack PostgreSQL local ban đầu vẫn chạy, không đổi dữ liệu. Lệnh chạy lại suite và recovery nằm trong [backend README](../../backend/README.md); contract frontend nằm trong [build104-mvp.md](build104-mvp.md).
