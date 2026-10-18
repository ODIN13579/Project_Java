# SwiftletCare — API Endpoint Catalog

> Sinh từ cùng danh sách với `openapi.yaml`; 47 thao tác / 36 URL. Mọi URL, enum và schema đang ở trạng thái **review** trước sign-off P4/P5/P1/P2.

**Quy ước:** `deviceId` trong REST path là `devices.id: BIGINT`; JSON REST `eventId` map MQTT/SQL `event_id`. `commandId` là chuỗi định danh lệnh. `MVP` là ưu tiên BUILD đề xuất.

## Authentication

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `POST` | `/api/v1/auth/login` | `LoginRequest` | `TokenPairResponse` | MVP |
| `POST` | `/api/v1/auth/refresh` | `RefreshRequest` | `TokenPairResponse` | Extended |
| `POST` | `/api/v1/auth/logout` | `LogoutRequest` | `—` | Extended |

## Users

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/users/me` | `—` | `UserResponse` | MVP |
| `PATCH` | `/api/v1/users/me` | `UserUpdateRequest` | `UserResponse` | Extended |

## Houses

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/houses` | `—` | `HouseListResponse` | MVP |
| `POST` | `/api/v1/houses` | `HouseCreateRequest` | `HouseResponse` | MVP |
| `GET` | `/api/v1/houses/{houseId}` | `—` | `HouseResponse` | MVP |
| `PATCH` | `/api/v1/houses/{houseId}` | `HouseUpdateRequest` | `HouseResponse` | Extended |

## Memberships

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/houses/{houseId}/members` | `—` | `MembershipListResponse` | Extended |
| `POST` | `/api/v1/houses/{houseId}/members` | `MembershipCreateRequest` | `HouseMembershipResponse` | Extended |
| `PATCH` | `/api/v1/houses/{houseId}/members/{membershipId}` | `MembershipUpdateRequest` | `HouseMembershipResponse` | Extended |

## Zones

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/houses/{houseId}/zones` | `—` | `ZoneListResponse` | MVP |
| `POST` | `/api/v1/houses/{houseId}/zones` | `ZoneCreateRequest` | `ZoneResponse` | MVP |
| `GET` | `/api/v1/zones/{zoneId}` | `—` | `ZoneResponse` | MVP |
| `PATCH` | `/api/v1/zones/{zoneId}` | `ZoneUpdateRequest` | `ZoneResponse` | Extended |

## Devices

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/zones/{zoneId}/devices` | `—` | `DeviceListResponse` | MVP |
| `POST` | `/api/v1/zones/{zoneId}/devices` | `DeviceCreateRequest` | `DeviceResponse` | MVP |
| `GET` | `/api/v1/devices/{deviceId}` | `—` | `DeviceResponse` | MVP |
| `PATCH` | `/api/v1/devices/{deviceId}` | `DeviceUpdateRequest` | `DeviceResponse` | Extended |

## Actuators

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/devices/{deviceId}/actuators` | `—` | `ActuatorListResponse` | MVP |

## Telemetry

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/devices/{deviceId}/telemetry/latest` | `—` | `TelemetryResponse` | MVP |
| `GET` | `/api/v1/devices/{deviceId}/telemetry` | `—` | `TelemetryListResponse` | MVP |

## Zone Configuration

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/zones/{zoneId}/config` | `—` | `ZoneConfigResponse` | MVP |
| `PUT` | `/api/v1/zones/{zoneId}/config` | `ZoneConfigUpsertRequest` | `ZoneConfigResponse` | MVP |

## Control Commands

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `POST` | `/api/v1/actuators/{actuatorId}/commands` | `ControlCommandCreateRequest` | `ControlCommandResponse` | MVP |
| `GET` | `/api/v1/commands/{commandId}` | `—` | `ControlCommandResponse` | MVP |
| `GET` | `/api/v1/actuators/{actuatorId}/commands` | `—` | `ControlCommandListResponse` | Extended |

## AI Events

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `POST` | `/api/v1/internal/ai/count-logs` | `CountLogIngestRequest` | `CountLogResponse` | MVP |
| `POST` | `/api/v1/internal/ai/anomalies` | `AnomalyEventIngestRequest` | `AnomalyEventResponse` | MVP |
| `GET` | `/api/v1/devices/{deviceId}/count-logs` | `—` | `CountLogListResponse` | MVP |
| `GET` | `/api/v1/zones/{zoneId}/anomalies` | `—` | `AnomalyListResponse` | MVP |
| `GET` | `/api/v1/anomalies/{anomalyId}` | `—` | `AnomalyEventResponse` | MVP |

## Alerts

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/alerts` | `—` | `AlertListResponse` | MVP |
| `GET` | `/api/v1/alerts/{alertId}` | `—` | `AlertResponse` | MVP |
| `PATCH` | `/api/v1/alerts/{alertId}/status` | `AlertStatusUpdateRequest` | `AlertResponse` | MVP |
| `GET` | `/api/v1/alerts/{alertId}/history` | `—` | `AlertHistoryListResponse` | MVP |

## Notifications

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/notifications` | `—` | `NotificationListResponse` | Extended |

## Harvest

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/zones/{zoneId}/harvest-logs` | `—` | `HarvestLogListResponse` | MVP |
| `POST` | `/api/v1/zones/{zoneId}/harvest-logs` | `HarvestLogCreateRequest` | `HarvestLogResponse` | MVP |

## Statistics & Reports

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/reports/environment` | `—` | `EnvironmentStatsResponse` | MVP |
| `GET` | `/api/v1/reports/bird-count` | `—` | `BirdCountStatsResponse` | MVP |
| `GET` | `/api/v1/reports/productivity-correlation` | `—` | `CorrelationResponse` | Extended |
| `POST` | `/api/v1/reports` | `ReportCreateRequest` | `ReportResponse` | Extended |
| `GET` | `/api/v1/reports/{reportId}` | `—` | `ReportResponse` | Extended |
| `GET` | `/api/v1/reports/{reportId}/download` | `—` | `—` | Extended |

## Dashboard

| Method | Endpoint | Request DTO | Response DTO | Ưu tiên |
|---|---|---|---|---|
| `GET` | `/api/v1/dashboard/summary` | `—` | `DashboardSummaryResponse` | MVP |

## Quy tắc quan trọng theo endpoint

1. `POST /auth/login`: trả JWT; refresh/logout phụ thuộc chính sách phiên của P4.
2. `POST /houses`: body có `initialZone` là **giải pháp đề xuất**, để tương thích `House 1..* Zone` từ Class; chưa chốt với P3/P4.
3. `GET /devices/{deviceId}/telemetry/latest`: sắp theo `timestamp DESC`, `receivedAt DESC`, tránh gửi bù muộn trở thành “mới nhất”.
4. `GET .../telemetry`: lọc UTC `from` inclusive, `to` exclusive, phân trang.
5. `PUT /zones/{zoneId}/config`: Backend lưu/cập nhật phiên bản, không tự tuyên bố ESP32 đã áp dụng.
6. `POST /actuators/{actuatorId}/commands`: `202` là đã tiếp nhận/lưu lệnh chứ không phải đã bật relay. `SET_MODE` vs `SET_STATE` kiểm tra XOR; `SET_STATE` yêu cầu MANUAL.
7. `GET /commands/{commandId}`: path là commandId chuỗi, không phải `ControlCommand.id` BIGINT.
8. `POST /internal/ai/...`: chỉ AI Service gọi, không cấp quyền cho người dùng React. Chống trùng sự kiện AI vẫn TBD.
9. `PATCH /alerts/{alertId}/status`: ghi `AlertHistory` cùng transaction với update `Alert`.
10. `GET /reports/productivity-correlation`: correlation mang tính thống kê khám phá, không chứng minh quan hệ nhân quả; cách ghép thời gian/dữ liệu TBD.
11. REST lấy lịch sử, WebSocket đẩy cập nhật; sau reconnect React tải REST để đồng bộ.

