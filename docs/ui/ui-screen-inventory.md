# UI Screen Inventory & Component Behaviors — SwiftletCare



## 1. Danh sách màn hình

| ID | Tên màn hình | Tên frame trong Figma | Mục đích |
|---|---|---|---|
| UI-01 | Dashboard tổng quan | `01 - Dashboard` | Theo dõi nhanh tình trạng nhà yến |
| UI-02 | Giám sát môi trường và thiết bị | `02 - Environment & Devices` | Theo dõi dữ liệu môi trường, trạng thái và điều khiển thiết bị |
| UI-03 | Cảnh báo và sự kiện AI | `03 - Alerts & AI Events` | Xem cảnh báo, bằng chứng AI và cập nhật trạng thái xử lý |
| UI-04 | Thống kê và báo cáo | `04 - Statistics & Reports` | Theo dõi biến động chim và xu hướng môi trường |

## 2. Cấu trúc điều hướng

### 2.1 Thành phần điều hướng dùng chung

| Thành phần | Mục đích | Hành vi |
|---|---|---|
| Header | Hiển thị tên hệ thống và thông tin người dùng | Giữ cách trình bày thống nhất giữa các màn hình |
| Navigation menu | Điều hướng đến các màn hình chính | Khi chọn một mục, mở màn hình tương ứng |
| Active menu | Cho biết màn hình hiện tại | Đánh dấu mục menu đang được chọn |
| Bộ chọn nhà yến/khu vực | Chọn phạm vi dữ liệu cần xem | Tải lại dữ liệu theo phạm vi mới |
| Chỉ báo kết nối | Hiển thị trạng thái kết nối nếu có dữ liệu | Phân biệt trạng thái đang kết nối, mất kết nối hoặc chưa xác định |
| Thời điểm cập nhật | Cho biết dữ liệu được cập nhật lần cuối | Cập nhật theo dữ liệu nhận được |
| Tài khoản người dùng | Hiển thị thông tin tài khoản | Chỉ cung cấp thao tác phù hợp với phạm vi thiết kế và quyền được cấp |

### 2.2 Luồng điều hướng chính

- Dashboard → chọn widget môi trường hoặc thiết bị → Environment & Devices.
- Dashboard → chọn cảnh báo → Alerts & AI Events.
- Dashboard → chọn ảnh sự kiện AI → Alerts & AI Events.
- Dashboard → chọn widget thống kê → Statistics & Reports.
- Từ màn hình chi tiết → quay lại màn hình trước nếu ứng dụng hỗ trợ điều hướng này.

### 2.3 Nguyên tắc điều hướng

- Tên màn hình và menu phải nhất quán với các frame Figma.
- Khi người dùng đổi nhà yến hoặc khu vực, các dữ liệu liên quan phải được tải lại theo lựa chọn mới.
- Không trộn dữ liệu của khu vực trước với khu vực hiện tại.
- Nếu người dùng không có quyền truy cập một chức năng, giao diện không nên cho phép thực hiện thao tác đó.
- Backend vẫn phải kiểm tra quyền; việc ẩn hoặc vô hiệu hóa nút trên giao diện không thay thế được kiểm tra quyền phía máy chủ.

## 3. UI-01 — Dashboard tổng quan

### 3.1 Mục đích

Dashboard cung cấp cái nhìn tổng quan về tình trạng nhà yến, bao gồm dữ liệu môi trường, trạng thái thiết bị, hoạt động của đàn chim, cảnh báo và sự kiện AI gần nhất.

### 3.2 Thành phần và hành vi

| Thành phần | Nội dung hiển thị | Hành vi khi người dùng thao tác hoặc khi dữ liệu thay đổi |
|---|---|---|
| Bộ chọn nhà yến/khu vực | Nhà yến và khu vực hiện đang xem | Khi thay đổi lựa chọn, Dashboard tải lại dữ liệu theo phạm vi mới |
| Thẻ nhiệt độ | Giá trị nhiệt độ, đơn vị đo, thời điểm cập nhật | Cập nhật khi có dữ liệu cảm biến mới |
| Thẻ độ ẩm | Giá trị độ ẩm, đơn vị đo, thời điểm cập nhật | Cập nhật khi có dữ liệu cảm biến mới |
| Thẻ CO₂ | Giá trị CO₂, đơn vị đo, thời điểm cập nhật | Cập nhật khi có dữ liệu cảm biến mới |
| Thẻ ánh sáng | Giá trị ánh sáng, đơn vị đo, thời điểm cập nhật | Cập nhật khi có dữ liệu cảm biến mới |
| Trạng thái ESP32 | Trạng thái kết nối nếu hệ thống cung cấp | Hiển thị trạng thái mới nhất đã nhận được |
| Trạng thái cảm biến và thiết bị | Trạng thái Online/Offline/Unknown nếu có dữ liệu | Khi chọn thành phần, chuyển đến UI-02 nếu có liên kết |
| Thống kê chim IN/OUT | Lượt chim vào và lượt chim ra | Cập nhật theo dữ liệu đếm chim được hệ thống cung cấp |
| Số chim ước tính | Số chim ước tính hiện diện nếu hệ thống có hỗ trợ | Hiển thị riêng, không gộp với lượt IN/OUT |
| Danh sách cảnh báo gần đây | Thời gian, mức độ, loại cảnh báo, khu vực | Khi chọn một cảnh báo, mở UI-03 tại cảnh báo tương ứng nếu hỗ trợ |
| Ảnh sự kiện AI gần nhất | Ảnh bằng chứng, nhãn, thời gian và bounding box nếu có | Khi chọn ảnh, mở thông tin chi tiết sự kiện AI |
| Thời điểm cập nhật dữ liệu | Thời điểm dữ liệu gần nhất được nhận | Cập nhật cùng dữ liệu và thể hiện khi dữ liệu đã cũ |

### 3.3 Hành vi khi mở màn hình

1. Khi người dùng mở Dashboard, giao diện hiển thị trạng thái Loading trong lúc tải dữ liệu.
2. Khi dữ liệu được tải thành công, các widget hiển thị giá trị và thời điểm cập nhật.
3. Nếu một widget không có dữ liệu, widget đó hiển thị thông báo chưa có dữ liệu.
4. Nếu tải dữ liệu thất bại, giao diện hiển thị trạng thái lỗi và cho phép thử lại nếu phù hợp.
5. Nếu dữ liệu không được cập nhật trong khoảng thời gian dự kiến, giao diện phải thể hiện thời điểm cập nhật cuối cùng hoặc trạng thái dữ liệu cũ.
6. Khi người dùng đổi nhà yến hoặc khu vực, dữ liệu của các widget phải được tải lại theo phạm vi mới.

### 3.4 Hành vi khi nhận dữ liệu mới

Khi hệ thống nhận telemetry mới từ Broker:

1. Dữ liệu được chuyển đến thành phần giao diện tương ứng.
2. Widget cập nhật giá trị mới và timestamp.
3. Nếu không nhận được dữ liệu mới, giao diện không được tự giả định rằng giá trị cũ vẫn là dữ liệu thời gian thực.
4. Yêu cầu cập nhật telemetry trong vòng không quá 2 giây kể từ khi Broker nhận dữ liệu cần được kiểm thử trong quá trình tích hợp. Wireframe chỉ mô tả hành vi mong muốn, không tự chứng minh được hiệu năng.


## 4. UI-02 — Giám sát môi trường và thiết bị

### 4.1 Mục đích

Màn hình cho phép người dùng theo dõi lịch sử môi trường, xem trạng thái kết nối thiết bị và gửi lệnh điều khiển nếu người dùng có quyền.

### 4.2 Thành phần và hành vi

| Thành phần | Nội dung hiển thị | Hành vi |
|---|---|---|
| Bộ lọc nhà yến | Nhà yến cần theo dõi | Khi thay đổi, tải lại dữ liệu của nhà yến được chọn |
| Bộ lọc khu vực | Khu vực cần theo dõi | Khi thay đổi, tải lại dữ liệu môi trường và thiết bị tương ứng |
| Bộ lọc thời gian | Khoảng thời gian cần xem | Khi áp dụng, biểu đồ và bảng lịch sử cập nhật theo khoảng thời gian |
| Biểu đồ nhiệt độ | Dữ liệu nhiệt độ theo thời gian | Cập nhật theo bộ lọc hiện tại |
| Biểu đồ độ ẩm | Dữ liệu độ ẩm theo thời gian | Cập nhật theo bộ lọc hiện tại |
| Biểu đồ CO₂ | Dữ liệu CO₂ theo thời gian | Cập nhật theo bộ lọc hiện tại |
| Biểu đồ ánh sáng | Dữ liệu ánh sáng theo thời gian | Cập nhật theo bộ lọc hiện tại |
| Ngưỡng môi trường | Ngưỡng được cấu hình nếu hệ thống cung cấp | Hiển thị giá trị và đơn vị; chỉ cho chỉnh sửa nếu tính năng thuộc phạm vi và người dùng có quyền |
| Danh sách thiết bị | Tên, loại, khu vực và trạng thái thiết bị | Cho phép chọn thiết bị để xem thông tin chi tiết nếu hỗ trợ |
| Trạng thái kết nối | Online/Offline/Unknown theo dữ liệu nhận được | Cập nhật khi hệ thống nhận trạng thái mới |
| Điều khiển ON/OFF | Thao tác gửi lệnh bật hoặc tắt thiết bị | Gửi lệnh và hiển thị trạng thái chờ phản hồi |
| Điều khiển AUTO/MANUAL | Chọn chế độ điều khiển | Gửi yêu cầu đổi chế độ và chờ xác nhận |
| Trạng thái lệnh | Pending/Sent, Acknowledged/Received, Executed hoặc Failed/Timeout nếu backend cung cấp | Cập nhật theo phản hồi thực tế |
| Nút làm mới | Yêu cầu tải lại dữ liệu | Tải dữ liệu mới và giữ nguyên bộ lọc hiện tại nếu phù hợp |

### 4.3 Hành vi khi thay đổi bộ lọc

1. Người dùng chọn nhà yến, khu vực hoặc khoảng thời gian.
2. Người dùng áp dụng bộ lọc.
3. Giao diện hiển thị trạng thái đang tải.
4. Hệ thống tải dữ liệu theo phạm vi đã chọn.
5. Biểu đồ và bảng lịch sử cập nhật đồng bộ theo cùng bộ lọc.
6. Nếu không có dữ liệu, giao diện hiển thị trạng thái Empty.
7. Nếu tải thất bại, giao diện hiển thị lỗi và cung cấp cách thử lại nếu phù hợp.

### 4.4 Hành vi khi bật/tắt thiết bị

Đây là hành vi quan trọng cần mô tả rõ trong thiết kế.

1. Người dùng chọn nút ON hoặc OFF của thiết bị.
2. Giao diện gửi yêu cầu điều khiển đến Backend.
3. Trong khi chờ phản hồi, giao diện hiển thị Pending hoặc Sent.
4. Nếu Backend xác nhận đã nhận lệnh, giao diện hiển thị Received/Acknowledged nếu trạng thái này được cung cấp.
5. Việc Backend nhận lệnh không có nghĩa là thiết bị đã thực thi lệnh.
6. Khi nhận được phản hồi xác nhận thực thi hoặc trạng thái thực tế mới, giao diện mới cập nhật trạng thái thiết bị theo dữ liệu đó.
7. Nếu lệnh thất bại hoặc hết thời gian chờ, giao diện hiển thị Failed/Timeout và giữ trạng thái thực tế cuối cùng đã biết.
8. Người dùng chỉ có thể thử lại khi chức năng được hỗ trợ và thao tác đó phù hợp với trạng thái hiện tại.

### 4.5 Hành vi khi chuyển AUTO/MANUAL

1. Người dùng chọn chế độ AUTO hoặc MANUAL.
2. Giao diện gửi yêu cầu đổi chế độ.
3. Giao diện hiển thị trạng thái đang chờ xác nhận.
4. Khi nhận được xác nhận phù hợp, giao diện cập nhật chế độ hiện tại.
5. Nếu yêu cầu thất bại, giao diện thông báo lỗi và giữ chế độ hiện tại đã được xác nhận.

Theo FR-IOT-007, chế độ AUTO cục bộ trên ESP32 tiếp tục hoạt động khi mất Internet hoặc kết nối đến server. Do đó, giao diện cần phân biệt mất kết nối với server và việc thiết bị thực sự bị hỏng.

## 5. UI-03 — Cảnh báo và sự kiện AI

### 5.1 Mục đích

Màn hình cho phép người dùng xem danh sách cảnh báo, kiểm tra chi tiết và bằng chứng liên quan, đồng thời cập nhật trạng thái xử lý nếu chức năng được hỗ trợ.

### 5.2 Thành phần và hành vi

| Thành phần | Nội dung hiển thị | Hành vi |
|---|---|---|
| Danh sách cảnh báo | Thời gian, mức độ, loại, khu vực và trạng thái | Hiển thị các cảnh báo phù hợp với phạm vi truy vấn |
| Bộ lọc mức độ | Warning/Critical theo định nghĩa của hệ thống | Lọc danh sách theo mức độ được chọn |
| Bộ lọc loại | Loại cảnh báo được hệ thống định nghĩa | Lọc danh sách theo loại |
| Bộ lọc trạng thái | New/Processing/Resolved nếu dùng quy trình này | Lọc danh sách theo trạng thái |
| Chi tiết cảnh báo | Mô tả, thời gian, khu vực và giá trị liên quan nếu có | Hiển thị thông tin của cảnh báo đã chọn |
| Bằng chứng AI | Snapshot, nhãn, thời gian, khu vực và bounding box nếu có | Cho phép xem bằng chứng liên quan đến sự kiện |
| Nút cập nhật trạng thái | Thao tác cập nhật trạng thái cảnh báo | Gửi yêu cầu và chờ xác nhận lưu thành công |
| Lịch sử xử lý | Người xử lý, thời gian và thay đổi trạng thái nếu có | Hiển thị lịch sử đã được hệ thống lưu |
| Web Notification | Nội dung thông báo và cảnh báo liên quan | Khi chọn thông báo, mở cảnh báo tương ứng nếu có liên kết |

### 5.3 Hành vi khi xem danh sách cảnh báo

1. Khi mở màn hình, giao diện tải danh sách cảnh báo.
2. Trong lúc tải, hiển thị Loading.
3. Khi người dùng thay đổi bộ lọc, danh sách được tải hoặc lọc lại theo lựa chọn.
4. Nếu không có kết quả, hiển thị thông báo không có cảnh báo phù hợp.
5. Khi người dùng chọn một cảnh báo, giao diện mở phần chi tiết của cảnh báo đó.
6. Người dùng có thể quay lại danh sách nếu thiết kế có hỗ trợ điều hướng này.

### 5.4 Hành vi khi có cảnh báo mới

1. Khi hệ thống phát hiện cảnh báo mới, giao diện hiển thị thông báo theo yêu cầu.
2. Người dùng có thể chọn thông báo để mở chi tiết cảnh báo nếu chức năng liên kết được hỗ trợ.
3. FR-ALT-001 đặt mục tiêu hiển thị banner/popup trong khoảng 3–5 giây sau khi phát hiện. Thời gian này cần được kiểm thử tích hợp thực tế.
4. Nếu dữ liệu cảnh báo chưa tải được, giao diện phải thể hiện trạng thái phù hợp thay vì hiển thị nội dung giả.

### 5.5 Hành vi khi xử lý cảnh báo

1. Người dùng mở chi tiết cảnh báo.
2. Người dùng chọn thao tác cập nhật trạng thái theo quy trình đã thống nhất.
3. Giao diện gửi yêu cầu cập nhật đến Backend.
4. Trong khi chờ phản hồi, giao diện thể hiện trạng thái đang xử lý yêu cầu.
5. Chỉ sau khi Backend xác nhận lưu thành công, giao diện mới cập nhật trạng thái hiển thị.
6. Nếu cập nhật thất bại, giao diện giữ trạng thái đã xác nhận gần nhất, hiển thị lỗi và cho phép thử lại nếu phù hợp.
7. Lịch sử xử lý phải phản ánh dữ liệu đã lưu, không tự ghi nhận thao tác thành công khi chưa có xác nhận.

### 5.6 Hành vi khi xem sự kiện AI

Khi người dùng chọn một sự kiện AI, giao diện hiển thị các thông tin có sẵn:
- Nhãn hoặc loại đối tượng được phát hiện.
- Thời điểm phát hiện.
- Nhà yến/khu vực liên quan.
- Ảnh hoặc snapshot bằng chứng.
- Bounding box nếu dịch vụ AI cung cấp.
- Trạng thái xử lý nếu sự kiện được liên kết với quy trình cảnh báo.

Nếu không có ảnh bằng chứng, giao diện hiển thị thông báo chưa có ảnh thay vì sử dụng ảnh minh họa không liên quan.

### 5.7 Lưu ý về thông báo

Tài liệu FR đã cung cấp nêu Web Notification và Mobile Push Notification trong FR-NOTI-001. Không ghi SMS hoặc Zalo là tính năng đã được yêu cầu nếu nhóm chưa cập nhật và thống nhất lại FR.



## 6. UI-04 — Thống kê và báo cáo

### 6.1 Mục đích

Màn hình cung cấp các biểu đồ và số liệu lịch sử để người dùng theo dõi hoạt động của đàn chim và xu hướng môi trường trong phạm vi dữ liệu hệ thống hỗ trợ.

### 6.2 Thành phần và hành vi

| Thành phần | Nội dung hiển thị | Hành vi |
|---|---|---|
| Bộ lọc thời gian | Ngày, tuần, tháng hoặc khoảng thời gian | Khi thay đổi, cập nhật các biểu đồ theo phạm vi mới |
| Bộ lọc nhà yến | Nhà yến cần xem | Tải lại dữ liệu theo nhà yến được chọn |
| Bộ lọc khu vực | Khu vực cần xem | Tải lại dữ liệu theo khu vực được chọn |
| Biểu đồ chim IN | Lượt chim vào theo thời gian | Cập nhật theo bộ lọc |
| Biểu đồ chim OUT | Lượt chim ra theo thời gian | Cập nhật theo bộ lọc |
| Số chim ước tính | Số chim ước tính hiện diện nếu có dữ liệu | Hiển thị riêng với số lượt IN/OUT |
| Biểu đồ môi trường | Xu hướng nhiệt độ, độ ẩm, CO₂ và ánh sáng | Cập nhật theo khoảng thời gian và khu vực |
| Khu vực báo cáo | Báo cáo môi trường hoặc hoạt động đàn chim trong phạm vi hệ thống | Mở/xem báo cáo nếu chức năng có trong scope |
| Chức năng xuất báo cáo | Định dạng xuất nếu được yêu cầu | Chỉ hiển thị nếu định dạng xuất thuộc FR/scope đã duyệt |

### 6.3 Hành vi khi thay đổi bộ lọc

1. Người dùng chọn khoảng thời gian.
2. Người dùng chọn nhà yến hoặc khu vực cần xem.
3. Giao diện tải dữ liệu tương ứng.
4. Các biểu đồ được cập nhật theo cùng phạm vi lọc.
5. Nếu dữ liệu không tồn tại, hiển thị trạng thái Empty.
6. Nếu tải dữ liệu thất bại, hiển thị trạng thái Error và cách thử lại nếu phù hợp.

### 6.4 Nguyên tắc thể hiện số liệu

- Lượt chim IN và lượt chim OUT phải được phân biệt rõ.
- Số chim ước tính hiện diện không được trình bày như số lượt chim ra/vào.
- Chỉ thể hiện phân tích tương quan giữa môi trường và năng suất nếu hệ thống có định nghĩa và thu thập dữ liệu năng suất.
- Chỉ hiển thị chức năng xuất PDF/Excel nếu tài liệu FR hoặc phạm vi triển khai đã xác nhận.


## 7. Trạng thái giao diện dùng chung

| Trạng thái | Điều kiện | Hành vi giao diện |
|---|---|---|
| Loading | Đang tải dữ liệu | Hiển thị chỉ báo tải; chưa kết luận thành công |
| Empty | Không có dữ liệu phù hợp | Hiển thị thông báo không có dữ liệu |
| Error | Yêu cầu thất bại | Hiển thị lỗi dễ hiểu và nút thử lại nếu phù hợp |
| Offline | Mất kết nối được hệ thống báo cáo | Hiển thị thành phần mất kết nối; không suy diễn thiết bị vật lý đã hỏng |
| Stale data | Dữ liệu không được cập nhật như dự kiến | Hiển thị thời điểm dữ liệu cuối cùng và nhãn dữ liệu cũ |
| Pending/Sent | Yêu cầu đang được gửi hoặc chờ phản hồi | Không thông báo thao tác đã hoàn tất |
| Acknowledged/Received | Có xác nhận đã nhận yêu cầu/lệnh | Chỉ xác nhận đã nhận, chưa khẳng định đã thực thi |
| Executed | Có xác nhận thực thi phù hợp | Cập nhật trạng thái theo dữ liệu xác nhận |
| Failed/Timeout | Lệnh thất bại hoặc hết thời gian chờ | Hiển thị lỗi, giữ trạng thái thực tế cuối đã biết |
| Success | Hệ thống xác nhận thao tác thành công | Hiển thị kết quả đã được xác nhận |

## 8. Bảng hành vi nhanh — Công việc 3

| Thành phần | Hành vi cần mô tả |
|---|---|
| Chọn khu vực | Dashboard tải dữ liệu của khu vực được chọn |
| Chọn khoảng thời gian | Biểu đồ và bảng lịch sử cập nhật theo bộ lọc |
| Bật/tắt thiết bị | Gửi lệnh điều khiển và hiển thị trạng thái chờ xác nhận |
| Chọn AUTO/MANUAL | Gửi yêu cầu đổi chế độ và chỉ cập nhật sau xác nhận |
| Chọn cảnh báo | Mở chi tiết cảnh báo và bằng chứng liên quan |
| Xử lý cảnh báo | Cập nhật trạng thái sau khi hệ thống xác nhận lưu thành công |
| Chọn sự kiện AI | Hiển thị nhãn, thời gian, khu vực và ảnh bằng chứng nếu có |
| Chọn bộ lọc cảnh báo | Lọc danh sách theo mức độ, loại hoặc trạng thái |
| Chọn widget Dashboard | Điều hướng tới màn hình chi tiết tương ứng |
| Chọn khoảng thống kê | Cập nhật các biểu đồ theo cùng phạm vi thời gian |
| Làm mới dữ liệu | Tải lại dữ liệu và giữ bộ lọc hiện tại nếu phù hợp |

**Quy tắc bắt buộc:** Khi người dùng nhấn bật/tắt thiết bị, giao diện không được lập tức khẳng định thiết bị đã bật/tắt chỉ vì Backend đã nhận yêu cầu. Cần phân biệt trạng thái gửi lệnh, xác nhận nhận lệnh và trạng thái thực tế của thiết bị.

