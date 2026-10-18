# SwiftletCare — DSN-09 REST API Specification

> **Jira:** DSN-09 • **Chủ trì:** P3 • **Review/triển khai Java:** P4 • **Phối hợp:** P1 (IoT), P2 (AI), P5 (React/ERD)  
> **Phiên bản:** `0.9.0-review` • **Ngày:** 2026-10-18  
> **Trạng thái:** **Bản đặc tả sẵn sàng review, CHƯA phải hợp đồng đã được toàn nhóm ký duyệt.**

## Mục tiêu

Thiết kế REST API cho React ↔ Spring Boot và AI Service → Spring Boot, khớp với Class Diagram DSN-06 (17 entity), ERD DSN-07, Backend Architecture DSN-05, MQTT Integration Contract DSN-08. Chuẩn bị cho giai đoạn BUILD môn Lập trình Java.

**Không dùng REST để thay thế MQTT:** ESP32 publish Telemetry/Heartbeat, nhận Command và trả ACK **qua MQTT**. Web tạo lệnh bằng REST; Backend publish Command tới MQTT; React đọc REST và cập nhật qua WebSocket.

## Nội dung bàn giao

| File | Mục đích |
|---|---|
| `openapi.yaml` | **Nguồn contract chính** cho 47 operation, 36 path; request/response DTO, auth, lỗi và ví dụ chính |
| `api-endpoint-catalog.md` | Danh sách toàn bộ endpoint, Request/Response, nhóm và độ ưu tiên |
| `fr-api-mapping.md` | Truy vết theo nhóm FR/module (mã FR chi tiết cần bản FR final để điền) |
| `api-conventions.md` | Quy ước URL, DTO, HTTP status, page, timestamp, validation |
| `api-security-matrix.md` | Quyền theo HouseRole và phân biệt service credential |
| `api-integration-mapping.md` | Mapping REST–Entity–MQTT–AI–WebSocket, trách nhiệm P1–P5 |
| `realtime-contract.md` | Giao diện realtime đề xuất, không thay thế REST |
| `api-test-cases.md` | Kịch bản kiểm thử trước/trong BUILD |
| `api-open-decisions.md` | Quyết định đã chốt vs còn chờ duyệt |
| `api-review-checklist.md` | Checklist nghiệm thu DSN-09/Jira |
| `validate_openapi.py` | Kiểm tra cú pháp, local `$ref`, operationId, request/response, security (không thay cho full OpenAPI validator) |

## Kiến trúc tích hợp

```text
React Web ─── HTTPS REST + JWT ───> Spring Boot ─── JPA ───> DB
     ^                                  ^    |
     |                                  |    +── MQTT Publish command/config → Broker → ESP32
     +──────── Auth WebSocket ──────────+    <── MQTT telemetry/heartbeat/ACK ──┘
Python AI Service ── internal REST auth ────> Spring Boot (CountLog / AnomalyEvent)
```

## Cách review & dùng

1. P3 chạy `python validate_openapi.py` rồi kiểm tra cảnh báo.
2. P4 review Controller/Service/DTO, HTTP status, security và database; P1 xác nhận lệnh/ZoneConfig/MQTT mapping; P2 xác nhận AI payload; P5 xác nhận React screen cần dữ liệu gì và ERD.
3. Chốt `api-open-decisions.md` và cập nhật `openapi.yaml` trước khi đánh dấu Done. Nếu chưa có FR v1.0 hoàn chỉnh, chỉ xác nhận mapping theo **nhóm FR**, không giả lập ID cụ thể.
4. Khi BUILD, code Spring Boot theo OpenAPI, triển khai DTO riêng biệt với JPA Entity, bật springdoc/Swagger UI để đối chiếu.

## Lưu ý về mức ưu tiên

`MVP`/`Extended` là **ưu tiên thực hiện được đề xuất** cho bài tập Java, không phải quyết định bỏ yêu cầu FR. Các endpoint `Extended` vẫn nằm trong thiết kế; nếu FR final yêu cầu thì nhóm cần triển khai hoặc chốt thay đổi phạm vi với giảng viên.

## Nguồn đối chiếu sử dụng

- Class Diagram P3 — sơ đồ mới có `CommandAction`, `HouseRole`, `AggregationType`, `ControlCommand.targetMode` nhưng lớp còn thiếu `action`.
- ERD P5 — 17 bảng, `control_commands.action`, `target_mode`, `target_state` nullable; cấu trúc hiện còn dòng `target_state` trùng trên ảnh, cần sửa.
- DSN-05 Backend Architecture — Spring Boot monolith module; JWT, service auth, Controller→Service→Repository; command ACK timeout/expiry.
- DSN-08 MQTT Contract — Telemetry `event_id`, heartbeat 2s, offline 6s, scheduler 1s, `SET_MODE`/`SET_STATE` (một số mục MQTT vẫn TBD).
- NFR và FR module SwiftletCare — **mã FR từng yêu cầu chưa được xác minh từ bảng FR v1.0 cuối**, cần bổ sung vào mapping trước nghiệm thu traceability tuyệt đối.
