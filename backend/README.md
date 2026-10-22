# SwiftletCare Backend — BUILD1-04, bước đầu

Java 21, Maven 3.6.3+, Spring Boot 4.0.8, PostgreSQL 17, Spring Data JPA,
Flyway và Actuator. Cài JDK 21, Maven và Docker Desktop (Linux containers),
mở Docker Desktop trước khi chạy. Kiểm tra `java -version`, `mvn -version`,
`docker compose version` trong PowerShell.

## Kết quả kiểm tra ngày 22/10/2026

- `docker compose ... config --quiet` hợp lệ; PostgreSQL 17 chạy healthy.
- Java 21, `mvn clean verify`: BUILD SUCCESS; 2 integration test pass,
  không có test bị bỏ qua khi bật `DB_TEST_ENABLED=true`.
- Flyway áp dụng V1 trên DB mới, chạy lại không có migration mới;
  EntityManagerFactory của JPA khởi tạo thành công.
- JAR chạy được; khi DB hoạt động health/readiness/liveness trả 200 UP.
  Dừng DB: health và readiness trả 503, liveness vẫn 200 UP.
- Đã dừng backend và container kiểm thử project Compose `swiftletcare-build104`;
  giữ volume kiểm thử. Các lệnh chạy phía dưới dùng project Compose mặc định.

## Chạy trên Windows (PowerShell)

Từ thư mục gốc repository `System-for-Swiftlet-Farming`:

```powershell
Copy-Item .env.example .env
# Chỉ copy lần đầu; sửa .env nếu cần đổi mật khẩu hoặc cổng.
docker compose up -d --wait postgres

# Đọc cấu hình local cho phiên PowerShell hiện tại.
$dbConfig = ConvertFrom-StringData (Get-Content .env -Raw)
$env:DB_URL = "jdbc:postgresql://localhost:$($dbConfig.POSTGRES_PORT)/$($dbConfig.POSTGRES_DB)"
$env:DB_USERNAME = $dbConfig.POSTGRES_USER
$env:DB_PASSWORD = $dbConfig.POSTGRES_PASSWORD

mvn -f backend/pom.xml spring-boot:run
```

`.env` dành cho Docker Compose; Spring Boot nhận biến môi trường `DB_URL`,
`DB_USERNAME`, `DB_PASSWORD`, không tự đọc `.env`. File mẫu dùng mật khẩu
local minh họa. Không commit `.env`. `SERVER_PORT` mặc định 8080.

Mở PowerShell khác để kiểm tra:

```powershell
Invoke-RestMethod http://localhost:8080/actuator/health
Invoke-RestMethod http://localhost:8080/actuator/health/liveness
Invoke-RestMethod http://localhost:8080/actuator/health/readiness
```

Health tổng hợp và readiness kiểm tra DB thật; trả HTTP 200 với `status: UP`
khi sẵn sàng, HTTP 503 khi DB không khả dụng. Liveness chỉ kiểm tra ứng dụng,
không phụ thuộc DB. Không công khai chi tiết kết nối. Đây là endpoint vận hành,
khác các API nghiệp vụ `/api/v1` trong OpenAPI.

## Build và test

Trong PowerShell đã thiết lập các biến DB phía trên:

```powershell
$env:DB_TEST_ENABLED = 'true'
mvn -f backend/pom.xml clean verify
Remove-Item Env:DB_TEST_ENABLED
java -jar backend/target/swiftletcare-backend-0.0.1-SNAPSHOT.jar
```

Integration test khởi động HTTP trên cổng ngẫu nhiên, dùng PostgreSQL được
chỉ định bởi `DB_URL`; kiểm tra migration, chạy lại migration không phát sinh
thay đổi, JPA khởi tạo và ba endpoint health. Nên dùng database local/test riêng.
Nếu không đặt `DB_TEST_ENABLED=true`, Maven vẫn build nhưng bỏ qua integration
test; build thành công khi đó không chứng minh kết nối DB.

## Package và schema

Package theo nhóm nghiệp vụ của `docs/design/class/class-diagram-overview.md`:

| Package dưới `com.swiftletcare` | Phạm vi |
|---|---|
| `management` | User, House, Membership, Zone, Device |
| `iot` | ZoneConfig, Sensor, Actuator, Telemetry, ControlCommand |
| `ai` | CountLog, AnomalyEvent |
| `alert` | Alert, AlertHistory, NotificationDelivery |
| `reporting` | HarvestLog, Report |

Các package hiện có `package-info.java` xác định ranh giới; sẽ đặt entity,
repository, service, controller trong từng module khi triển khai nghiệp vụ.
Actuator cung cấp health nên không cần controller kiểm tra DB tự viết.

Flyway chạy trước JPA khi khởi động. `V1__create_application_schema.sql` tạo
schema `swiftletcare`; bảng lịch sử `public.flyway_schema_history` do Flyway
quản lý. JPA dùng `ddl-auto=validate`, không sinh/sửa schema, tắt Open Session
in View, dùng UTC. Thêm migration `V2__...sql` trở đi cho thay đổi mới; không
sửa migration đã áp dụng. Không bật baseline tự động trên DB có dữ liệu.
Entrypoint đặt múi giờ JVM thành UTC trước khi mở JDBC để không phụ thuộc
tên múi giờ Windows (ví dụ PostgreSQL có thể từ chối `Asia/Saigon`).

Bước này chưa triển khai 17 bảng/entity nghiệp vụ: tài liệu ERD vẫn ghi các
điểm cần review (ControlCommand, actor hệ thống, khóa chống trùng AI). Không
thêm bảng giả cho health. Chưa triển khai MQTT, WebSocket, JWT hoặc API nghiệp vụ.
PostgreSQL và Flyway được chọn theo yêu cầu BUILD1-04; tài liệu thiết kế cũ vẫn
ghi đây là các lựa chọn mở. Compose gốc và backend ban đầu đều là file rỗng.

## Dừng và xử lý lỗi

Nhấn `Ctrl+C` để dừng backend. Từ gốc repository:

```powershell
docker compose logs postgres --tail 50
docker compose stop postgres
# Hoặc gỡ container/network, vẫn giữ dữ liệu trong volume:
docker compose down
```

Không dùng `down -v` nếu cần giữ dữ liệu. Thay mật khẩu trong `.env` không đổi
mật khẩu của database đã khởi tạo trong volume; cần dùng credential cũ hoặc
đổi mật khẩu trong PostgreSQL. Nếu cổng 5432 bận, đổi `POSTGRES_PORT` trong
`.env`, tạo lại container và nạp lại biến môi trường. Nếu Docker báo lỗi named
pipe, kiểm tra Docker Desktop và quyền truy cập. Nếu Maven báo lỗi cache
`C:\.m2\repository`, kiểm tra Maven settings hoặc chỉ định cache có quyền ghi:
`mvn "-Dmaven.repo.local=$env:USERPROFILE\.m2\repository" -f backend/pom.xml verify`.
