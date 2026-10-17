# CÔNG VIỆC 3 — LOGIC ĐẾM CHIM ENTRY/EXIT

## 1. Định nghĩa đường đếm

Mỗi camera được cấu hình một đường đếm tại cửa ra/vào nhà yến, xác định bởi hai điểm `P1(x1, y1)` và `P2(x2, y2)`.

Hai phía của đường đếm được quy định như sau:

- **OUTSIDE:** Phía ngoài nhà yến.
- **INSIDE:** Phía trong nhà yến.

Hệ thống sử dụng tâm của `bounding box` để xác định vị trí của từng con chim theo `tracking_id`.

## 2. Quy tắc đếm Entry/Exit

| Hướng di chuyển | Sự kiện |
|---|---|
| OUTSIDE → INSIDE | `ENTRY` — Chim bay vào |
| INSIDE → OUTSIDE | `EXIT` — Chim bay ra |
| Đến gần đường đếm rồi quay đầu | Không tạo sự kiện |
| Không đủ bằng chứng xác nhận vượt qua đường đếm | Không tạo sự kiện |

**Nguyên tắc:** Mỗi lần vượt qua đường đếm hợp lệ chỉ được ghi nhận một lần cho cùng một `tracking_id`. Nếu chim tiếp tục thực hiện một lần vượt qua mới theo hướng hợp lệ, hệ thống có thể ghi nhận sự kiện mới.

## 3. Trạng thái Tracking

| Trạng thái | Ý nghĩa |
|---|---|
| `UNKNOWN` | Chưa xác định được phía của đường đếm |
| `OUTSIDE` | Đã xác nhận ở phía ngoài |
| `INSIDE` | Đã xác nhận ở phía trong |
| `CROSSING` | Đang đi qua đường đếm |
| `LOST` | Mất `tracking_id` hoặc mất dấu đối tượng |
| `CLOSED` | Kết thúc theo dõi đối tượng |

## 4. Xử lý các trường hợp đặc biệt

**4.1. Mất tracking ID khi đang qua đường**

- Chuyển trạng thái sang `LOST`.
- Nếu chưa xác nhận hoàn tất việc vượt qua, không tạo sự kiện.
- Chỉ tiếp tục xử lý khi khôi phục được quỹ đạo đáng tin cậy; không tự suy đoán hướng di chuyển.

**4.2. Nhiều chim bay gần nhau hoặc chồng lấn**

- Theo dõi riêng từng đối tượng bằng `tracking_id`.
- Nếu không xác định rõ đối tượng hoặc hướng di chuyển, không tự tạo sự kiện.

**4.3. Chim quay đầu trước khi qua đường**

- Giữ trạng thái ở phía ban đầu.
- Không tạo sự kiện `ENTRY` hoặc `EXIT`.

**4.4. Chim xuất hiện qua nhiều frame**

- Không đếm theo số frame xuất hiện.
- Chỉ tạo sự kiện khi xác nhận hoàn tất một lần vượt qua đường đếm hợp lệ.

## 5. Cấu trúc dữ liệu sự kiện

Mỗi sự kiện cần lưu các thông tin: mã sự kiện, camera, khu vực, mã tracking, loại sự kiện, hướng di chuyển và thời gian ghi nhận.

**Ví dụ JSON:**

```json
{
  "event_id": "bird-event-001",
  "camera_id": "CAM-HOUSE01-01",
  "zone_id": "ZONE01",
  "tracking_id": "track-125",
  "event_type": "ENTRY",
  "direction": "OUTSIDE_TO_INSIDE",
  "event_timestamp": "2026-10-14T03:30:15Z"
}
```

Trong đó:

- `event_id`: Mã sự kiện duy nhất.
- `camera_id`: Mã camera ghi nhận sự kiện.
- `zone_id`: Mã khu vực nhà yến.
- `tracking_id`: Mã theo dõi đối tượng.
- `event_type`: Loại sự kiện `ENTRY` hoặc `EXIT`.
- `direction`: Hướng di chuyển qua đường đếm.
- `event_timestamp`: Thời gian xảy ra sự kiện theo định dạng ISO 8601; hậu tố `Z` biểu thị UTC.

Hệ thống cần sử dụng `event_id` để nhận diện sự kiện đã xử lý, tránh ghi nhận trùng khi gửi lại dữ liệu.

## 6. Phân biệt lượt chim và số chim trong nhà

Các chỉ số được xác định như sau:

- `entry_count`: Tổng số lượt chim bay vào.
- `exit_count`: Tổng số lượt chim bay ra.
- `initial_birds`: Số chim ước tính ban đầu trong nhà.

Công thức ước tính số chim hiện có:

```text
estimated_birds = initial_birds + entry_count - exit_count
```

**Lưu ý:** `estimated_birds` là số lượng ước tính, không phải số chim thực tế đã được xác minh. Một con chim có thể ra vào nhiều lần, do đó tổng lượt `ENTRY` và `EXIT` không đồng nghĩa với số lượng chim riêng biệt trong nhà yến.
