**TỔNG HỢP YÊU CẦU CHỨC NĂNG\
(FUNCTIONAL REQUIREMENTS - FR)**

# Module 1. Bảng điều khiển và giám sát tổng quan (FR-DASH)

  ---------------------------------------------------------------------------------------------------
  **Mã FR**         **Tên chức  **Mô tả**    **Tác    **Input**   **Output**   **Ưu     **Tiêu chí
                    năng**                   nhân**                            tiên**   chấp nhận**
  ----------------- ----------- ------------ -------- ----------- ------------ -------- -------------
  **FR-DASH-001**   Dashboard   Hiển thị các Farm     Dữ liệu     Màn hình     Must     Hiển thị các
                    giám sát    widget tóm   Owner /  tổng hợp từ Dashboard             thẻ tóm tắt
                    tổng quan   tắt tình     Admin    IoT, AI và  tổng quan             trạng thái
                                trạng nhà             hệ thống                          thực tế; từ
                                yến tại                                                 Dashboard có
                                dashboard                                               thể truy cập
                                trung tâm:                                              nhanh đến các
                                vi khí hậu,                                             trang quản lý
                                thiết bị,                                               chi tiết.
                                biến động                                               
                                đàn và cảnh                                             
                                báo khẩn.                                               

  **FR-DASH-002**   Hiển thị    Hiển thị giá Farm     Telemetry   Giá trị      Must     Widget cập
                    widget      trị vi khí   Owner /  từ ESP32    telemetry             nhật trong
                    telemetry   hậu mới nhất Admin                hiển thị              vòng ≤ 2 giây
                    thời gian   (nhiệt độ,                        trên widget           kể từ khi
                    thực        độ ẩm, CO₂,                                             Broker nhận
                                ánh sáng)                                               dữ liệu mới
                                dưới dạng                                               qua
                                thẻ chỉ số.                                             WebSocket.

  **FR-DASH-003**   Hiển thị    Hiển thị     Farm     Dữ liệu đếm Chỉ số chim  Must     Cập nhật chỉ
                    widget số   tổng lượt    Owner /  yến từ AI   vào/ra trên           số khi nhận
                    lượng chim  chim vào/ra  Admin    Service     Dashboard             được lượt
                                và biến động                                            chim ra/vào
                                trong ngày                                              mới từ AI
                                dưới dạng số                                            Camera; phân
                                liệu tóm tắt                                            biệt lượt
                                và biểu đồ                                              vào/ra với số
                                nhỏ.                                                    chim ước tính
                                                                                        hiện diện.

  **FR-DASH-004**   Hiển thị    Cho phép xem Farm     Camera      Live View    Must     Khung hình
                    widget      nhanh luồng  Owner /  feed,       tóm tắt và            bất thường
                    camera và   hình ảnh tóm Admin    anomaly     khung hình            hiển thị rõ
                    khung hình  tắt và các            snapshot    bất thường            bounding box,
                    bất thường  ảnh chụp bất                                            nhãn đối
                                thường mới                                              tượng và thời
                                nhất có                                                 gian ghi
                                bounding                                                nhận.
                                box.                                                    
  ---------------------------------------------------------------------------------------------------

# Module 2. Cảnh báo và thông báo (FR-ALT / FR-NOTI)

  -----------------------------------------------------------------------------------------------------------
  **Mã FR**         **Tên     **Mô tả**      **Tác nhân**   **Input**   **Output**     **Ưu     **Tiêu chí
                    chức                                                               tiên**   chấp nhận**
                    năng**                                                                      
  ----------------- --------- -------------- -------------- ----------- -------------- -------- -------------
  **FR-ALT-001**    Hiển thị  Hiển thị       Farm Owner /   Sự kiện bất Cảnh báo       Must     Cảnh báo xuất
                    cảnh báo  popup/banner   Admin          thường từ   popup/banner            hiện trong
                    thời gian khi phát hiện                 cảm biến,   trên Web App            vòng 3--5
                    thực      sự cố vi khí                  AI hoặc                             giây kể từ
                              hậu vượt                      ESP32                               khi sự cố
                              ngưỡng, thiên                                                     được phát
                              địch hoặc lỗi                                                     hiện; phân
                              phần cứng.                                                        biệt rõ mức
                                                                                                Warning và
                                                                                                Critical.

  **FR-ALT-002**    Quản lý   Cho phép xem,  Farm Owner /   Bản ghi     Danh sách và   Should   Lưu đúng
                    trạng     lọc và chuyển  Admin          cảnh báo    trạng thái              trạng thái,
                    thái cảnh trạng thái                                cảnh báo đã             người xử lý
                    báo       cảnh báo: Mới                             cập nhật                và thời gian
                              → Đang xử lý →                                                    xử lý trong
                              Đã xử lý.                                                         lịch sử cảnh
                                                                                                báo.

  **FR-NOTI-001**   Gửi thông Gửi Web        Hệ thống /     Sự kiện     Thông báo đẩy  Must     Thông báo
                    báo đa    Notification   Notification   cảnh báo    đến Web/Mobile          Critical được
                    kênh      và Mobile Push Service                                            gửi đến trình
                              Notification                                                      duyệt/điện
                              khi xảy ra sự                                                     thoại trong ≤
                              cố khẩn cấp.                                                      5 giây sau
                                                                                                khi sự kiện
                                                                                                được phát
                                                                                                hiện, trong
                                                                                                điều kiện
                                                                                                dịch vụ thông
                                                                                                báo khả dụng.
  -----------------------------------------------------------------------------------------------------------

# Module 3. Giám sát và điều khiển vi khí hậu / IoT (FR-IOT)

  -----------------------------------------------------------------------------------------------------------
  **Mã FR**        **Tên     **Mô tả**        **Tác    **Input**     **Output**   **Ưu     **Tiêu chí chấp
                   chức                       nhân**                              tiên**   nhận**
                   năng**                                                                  
  ---------------- --------- ---------------- -------- ------------- ------------ -------- ------------------
  **FR-IOT-001**   Theo dõi  Giám sát giá trị Farm     Dữ liệu nhiệt Giá trị và   Must     Hiển thị giá trị
                   nhiệt độ  và biểu đồ nhiệt Owner /  độ từ         đồ thị nhiệt          nhiệt độ; đánh dấu
                             độ môi trường    Admin    DHT22/SHT31   độ                    cảnh báo khi T \>
                             theo thời gian                                                31°C hoặc T \<
                             thực tại từng                                                 20°C nếu các
                             Zone.                                                         ngưỡng này được
                                                                                           nhóm xác nhận.

  **FR-IOT-002**   Theo dõi  Giám sát giá trị Farm     Dữ liệu độ ẩm Giá trị và   Must     Hiển thị độ ẩm
                   độ ẩm     và biểu đồ độ ẩm Owner /  từ cảm biến   đồ thị độ ẩm          tương đối; đánh
                             tương đối trong  Admin                                        dấu cảnh báo khi
                             nhà yến theo                                                  độ ẩm \< 75% nếu
                             thời gian thực.                                               ngưỡng này được
                                                                                           nhóm xác nhận.

  **FR-IOT-003**   Theo dõi  Giám sát cường   Farm     Dữ liệu cảm   Giá trị ánh  Should   Hiển thị đúng giá
                   cường độ  độ ánh sáng      Owner /  biến ánh sáng sáng (Lux)            trị ánh sáng và
                   ánh sáng  (Lux) bên trong  Admin                                        nhận biết khi môi
                             các tầng nhà                                                  trường bị lộ sáng
                             yến.                                                          bất thường theo
                                                                                           ngưỡng cấu hình.

  **FR-IOT-004**   Theo dõi  Giám sát nồng độ Farm     Dữ liệu cảm   Giá trị CO₂  Must     Hiển thị giá trị
                   nồng độ   CO₂ (ppm) trong  Owner /  biến CO₂      (ppm)                 CO₂ và phát cảnh
                   CO₂       nhà yến theo     Admin                                        báo khi vượt
                             thời gian thực.                                               ngưỡng an toàn đã
                                                                                           cấu hình.

  **FR-IOT-005**   Cài đặt   Cho phép thiết   Farm     Giá trị       Cấu hình     Must     Lưu cấu hình hợp
                   ngưỡng    lập ngưỡng       Owner /  ngưỡng mới    ngưỡng được           lệ và áp dụng
                   môi       trên/dưới cho    Admin                  lưu vào CSDL          ngưỡng mới cho
                   trường    nhiệt độ, độ ẩm,                                              thuật toán điều
                             CO₂ và ánh sáng                                               khiển tự động và
                             theo từng Zone.                                               cảnh báo.

  **FR-IOT-006**   Điều      Cho phép bật/tắt Farm     Lệnh bật/tắt  Lệnh điều    Must     ESP32 kích hoạt
                   khiển     máy phun sương,  Owner /  thiết bị      khiển và              relay tương ứng và
                   thiết bị  quạt thông gió,  Admin;                 trạng thái            phản hồi trạng
                   chấp hành máy sưởi và bộ   Hệ thống               phản hồi              thái trong ≤ 1
                             khuếch đại âm                                                 giây khi kết nối
                             thanh.                                                        và phần cứng hoạt
                                                                                           động bình thường.

  **FR-IOT-007**   Điều      ESP32 tự động    ESP32    Dữ liệu cảm   Quyết định   Must     Thuật toán chạy
                   khiển tự  đóng/ngắt relay  Edge     biến và       bật/tắt               tại ESP32 và tiếp
                   động      dựa trên dữ liệu Node     ngưỡng lưu    relay cục bộ          tục hoạt động khi
                             cảm biến và               tại chỗ                             mất
                             ngưỡng đã cấu                                                 Internet/Server;
                             hình.                                                         định nghĩa rõ
                                                                                           ngưỡng bật/tắt và
                                                                                           hysteresis cho
                                                                                           từng thiết bị.

  **FR-IOT-008**   Ghi đè    Cho phép bật/tắt Farm     Lệnh bật/tắt  Thiết bị     Must     Thiết bị duy trì
                   thủ công  thiết bị qua     Owner /  thủ công      chuyển sang           trạng thái được
                             Web/Mobile hoặc  Admin                  chế độ                chỉ định; quy định
                             nút bấm tại chỗ,                        Manual                rõ cách quay lại
                             tạm ngưng chế độ                                              Auto và xử lý lệnh
                             Auto theo quy                                                 khi mất kết nối.
                             tắc đã định.                                                  

  **FR-IOT-009**   Theo dõi  Giám sát trạng   Farm     Heartbeat và  Trạng thái   Must     Hiển thị ESP32
                   trạng     thái relay và    Owner /  phản hồi từ   thiết bị/kết          Offline trong tối
                   thái      trạng thái       Admin    ESP32         nối trên              đa 10 giây kể từ
                   thiết bị  Online/Offline                          giao diện             khi mất kết nối
                   và kết    của bộ điều                                                   theo chu kỳ
                   nối       khiển ESP32.                                                  heartbeat đã quy
                                                                                           định.
  -----------------------------------------------------------------------------------------------------------

# Module 4. AI Camera và thị giác máy tính (FR-AI)

  --------------------------------------------------------------------------------------------------
  **Mã FR**       **Tên chức  **Mô tả**    **Tác     **Input**   **Output**   **Ưu     **Tiêu chí
                  năng**                   nhân**                             tiên**   chấp nhận**
  --------------- ----------- ------------ --------- ----------- ------------ -------- -------------
  **FR-AI-001**   Giám sát    Hiển thị     Farm      Nguồn video Luồng video  Must     Phát được
                  luồng       luồng hình   Owner /   camera hoặc trên giao             luồng video
                  camera      ảnh/video từ Admin     video giả   diện                  trên
                  (Live View) camera tại             lập                               Web/Mobile
                              lỗ chim                                                  với độ trễ
                              ra/vào                                                   mục tiêu được
                              và/hoặc bên                                              xác định
                              trong nhà                                                trong NFR;
                              yến.                                                     video chỉ
                                                                                       truyền theo
                                                                                       chính sách
                                                                                       băng thông đã
                                                                                       thống nhất.

  **FR-AI-002**   Nhận diện   Sử dụng mô   AI        Khung hình  Bounding     Must     Trả về tọa độ
                  chim yến    hình YOLOv8  Service / video       box, nhãn             bounding box,
                  bằng YOLO   để nhận diện Camera                Swiftlet và           nhãn và độ
                              vị trí chim  Node                  confidence            tin cậy; đánh
                              yến trong                          score                 giá trên bộ
                              khung hình                                               dữ liệu kiểm
                              video.                                                   thử đã thống
                                                                                       nhất.

  **FR-AI-003**   Đếm chim    Theo dõi     AI        Luồng video Số lượt chim Must     Xác định đúng
                  ra/vào bằng chuyển động  Service / và tọa độ   vào và ra             hướng di
                  ByteTrack   qua vạch đếm Camera    tracking                          chuyển và hạn
                              tại cửa để   Node                                        chế đếm lặp
                              đếm lượt bay                                             cùng một
                              ra và bay                                                track ID khi
                              vào.                                                     chim lượn
                                                                                       quanh vạch.

  **FR-AI-004**   Theo dõi    Thống kê     Farm      Số liệu     Biểu đồ biến Must     Hiển thị lượt
                  biến động   lượt chim ra Owner /   In/Out theo động đàn yến          vào/ra; chỉ
                  đàn chim    buổi sáng và Admin     thời gian                         hiển thị số
                              vào buổi tối                                             chim hiện có
                              để tính                                                  dưới dạng ước
                              chênh lệch                                               tính khi có
                              theo ngày.                                               baseline và
                                                                                       quy tắc xử lý
                                                                                       sai số phù
                                                                                       hợp.

  **FR-AI-005**   Phát hiện   Nhận diện    AI        Khung hình  Nhãn thiên   Must     Khi phát hiện
                  thiên địch  các mối đe   Service / video       địch và               đối tượng
                              dọa như rắn, Camera    camera      bounding box          thuộc lớp
                              chuột, cú    Node                                        được hỗ trợ
                              mèo theo                                                 với ngưỡng
                              phạm vi loài                                             confidence
                              được hỗ trợ.                                             cấu hình, tạo
                                                                                       sự kiện cảnh
                                                                                       báo khẩn cấp.

  **FR-AI-006**   Lưu khung   Chụp và lưu  AI        Khung hình  Ảnh snapshot Must     Lưu ảnh, nhãn
                  hình bất    khung hình   Service / có sự kiện  lưu ở                 đối tượng, độ
                  thường      có bounding  Camera    bất thường  S3/MinIO và           tin cậy, thời
                              box khi phát Node                  metadata              gian và nguồn
                              hiện thiên                         trong CSDL            camera; người
                              địch hoặc sự                                             dùng có quyền
                              cố.                                                      có thể xem
                                                                                       lại.

  **FR-AI-007**   Đồng bộ kết Gửi kết quả  AI        Kết quả     Dữ liệu được Must     Kết quả đếm
                  quả AI về   đếm chim,    Service / phân tích   ghi nhận vào          và sự kiện
                  hệ thống    thống kê và  Python    AI          CSDL Server           cảnh báo xuất
                              ảnh snapshot Node                                        hiện trên ứng
                              cảnh báo từ                                              dụng trong
                              AI Service                                               3--5 giây
                              về Backend                                               theo điều
                              qua                                                      kiện kiểm
                              MQTT/REST                                                thử; có xử lý
                              API.                                                     lỗi truyền và
                                                                                       gửi lại.
  --------------------------------------------------------------------------------------------------

# Module 5. Giám sát âm thanh và loa yến (FR-AUD)

  ----------------------------------------------------------------------------------------------------
  **Mã FR**        **Tên     **Mô tả**    **Tác nhân** **Input**   **Output**   **Ưu     **Tiêu chí
                   chức                                                         tiên**   chấp nhận**
                   năng**                                                                
  ---------------- --------- ------------ ------------ ----------- ------------ -------- -------------
  **FR-AUD-001**   Giám sát  Đọc tín hiệu Cảm biến âm  Tín hiệu âm Chỉ số âm    Should   Nếu phần cứng
                   tín hiệu  hoặc mức âm  thanh /      thanh hoặc  thanh hiển            đã hiệu chuẩn
                   âm thanh  thanh từ     ESP32        giá trị cảm thị trên              thì hiển thị
                             microphone                biến        giao diện             dB SPL; nếu
                             đặt gần hệ                                                  chỉ có tín
                             thống loa                                                   hiệu tương
                             phát tiếng                                                  đối thì hiển
                             yến.                                                        thị đơn vị
                                                                                         tương đối,
                                                                                         không khẳng
                                                                                         định là dB
                                                                                         SPL.

  **FR-AUD-002**   Cảnh báo  Phát cảnh    Hệ thống /   Mức tín     Cảnh báo sự  Should   Cảnh báo
                   sự cố âm  báo khi loa  Background   hiệu        cố âm thanh           trong ≤ 5
                   thanh     mất tiếng    Service      và/hoặc đặc                       giây khi tín
                             hoặc tín                  trưng âm                          hiệu thấp hơn
                             hiệu âm                   thanh                             ngưỡng cấu
                             thanh bất                                                   hình liên tục
                             thường trong                                                trong khoảng
                             giờ hoạt                                                    thời gian xác
                             động.                                                       định; không
                                                                                         dùng 0 dB làm
                                                                                         tiêu chí
                                                                                         chung nếu cảm
                                                                                         biến chưa
                                                                                         hiệu chuẩn.
  ----------------------------------------------------------------------------------------------------

# Module 6. Thống kê và báo cáo (FR-REP)

  -----------------------------------------------------------------------------------------------------
  **Mã FR**        **Tên     **Mô tả**    **Tác    **Input**   **Output**   **Ưu     **Tiêu chí chấp
                   chức                   nhân**                            tiên**   nhận**
                   năng**                                                            
  ---------------- --------- ------------ -------- ----------- ------------ -------- ------------------
  **FR-REP-001**   Theo dõi  Cho phép xem Farm     Telemetry   Biểu đồ xu   Must     Trực quan hóa dữ
                   xu hướng  biểu đồ xu   Owner /  và khoảng   hướng                 liệu theo
                   môi       hướng các    Admin    thời gian                         ngày/tuần/tháng;
                   trường    chỉ số vi                                               hỗ trợ chọn dải
                             khí hậu theo                                            thời gian và xử lý
                             khoảng thời                                             khoảng dữ liệu bị
                             gian tùy                                                thiếu.
                             chọn.                                                   

  **FR-REP-002**   Xem lịch  Tra cứu và   Farm     Điều kiện   Bảng dữ liệu Must     Hiển thị đúng các
                   sử vi khí xuất dữ liệu Owner /  lọc và      lịch sử               bản ghi trong
                   hậu       lịch sử      Admin    khoảng thời                       khoảng thời gian
                             nhiệt độ, độ          gian                              và Zone được truy
                             ẩm, CO₂ và                                              vấn; múi giờ được
                             ánh sáng                                                xác định thống
                             dưới dạng                                               nhất.
                             bảng.                                                   

  **FR-REP-003**   Thống kê  Xem thống kê Farm     Dữ liệu đếm Báo cáo      Should   Hiển thị lượt
                   chim      số lượt chim Owner /  chim và     cột/đường về          ra/vào theo khoảng
                   ra/vào    ra/vào theo  Admin    khoảng thời lượt ra/vào           thời gian và làm
                             giờ, ngày             gian                              rõ thời điểm cao
                             hoặc tuần.                                              điểm nếu dữ liệu
                                                                                     đủ.

  **FR-REP-004**   Xem danh  Xem danh     Farm     Danh sách   Chi tiết báo Should   Sắp xếp theo thời
                   sách báo  sách báo cáo Owner /  báo cáo đã  cáo được              gian và cho phép
                   cáo       đã khởi tạo  Admin    lưu         chọn                  mở báo cáo người
                             và lưu trữ.                                             dùng có quyền truy
                                                                                     cập.

  **FR-REP-005**   Khởi tạo  Tạo báo cáo  Farm     Loại dữ     File báo cáo Should   Xuất báo cáo thành
                   báo cáo   tùy chỉnh    Owner /  liệu và     PDF/Excel             công với dữ liệu,
                   tổng hợp  gồm dữ liệu  Admin    khoảng thời theo phạm vi          bảng và biểu đồ
                             môi trường,           gian        triển khai            tương ứng; định
                             đếm chim và                                             dạng được chọn
                             lịch sử cảnh                                            phải được hỗ trợ.
                             báo theo                                                
                             thời gian.                                              

  **FR-REP-006**   Thống kê  Thống kê số  Farm     Dữ liệu     Số liệu và   Should   Hiển thị tổng lượt
                   phát hiện lượt phát    Owner /  nhận        biểu đồ mật           nhận diện trong
                   chim      hiện hoặc    Admin    diện/đếm    độ chim               khoảng thời gian;
                             mật độ xuất           chim                              phân biệt rõ lượt
                             hiện của                                                phát hiện với lượt
                             chim tại khu                                            qua vạch đếm.
                             vực lỗ                                                  
                             ra/vào.                                                 

  **FR-REP-007**   Phân tích Phân tích    Farm     Dữ liệu vi  Báo cáo/biểu Should   Chỉ phân tích
                   tương     mối tương    Owner /  khí hậu, dữ đồ tương              tương quan với sản
                   quan năng quan giữa độ Admin    liệu đàn và quan                  lượng khi có dữ
                   suất      ổn định vi            dữ liệu thu                       liệu thu hoạch
                             khí hậu với           hoạch nếu                         được ghi nhận;
                             biến động             có                                không diễn giải
                             đàn và sản                                              tương quan là quan
                             lượng tổ.                                               hệ nhân quả.
  -----------------------------------------------------------------------------------------------------

# Module 7. Quản lý người dùng (FR-USER)

  --------------------------------------------------------------------------------------------------
  **Mã FR**         **Tên     **Mô tả**    **Tác    **Input**   **Output**    **Ưu     **Tiêu chí
                    chức                   nhân**                             tiên**   chấp nhận**
                    năng**                                                             
  ----------------- --------- ------------ -------- ----------- ------------- -------- -------------
  **FR-USER-001**   Đăng nhập Cho phép     Người    Email và    Kết quả đăng  Must     Thông tin hợp
                    hệ thống  đăng nhập    dùng     mật khẩu    nhập và                lệ thì đăng
                              bằng email                        access token           nhập thành
                              và mật khẩu.                                             công; thông
                                                                                       tin sai trả
                                                                                       thông báo
                                                                                       chung, không
                                                                                       tiết lộ email
                                                                                       có tồn tại
                                                                                       hay không.

  **FR-USER-002**   Đăng xuất Cho phép     Người    Yêu cầu     Phiên/token   Must     Token cũ
                              người dùng   dùng đã  đăng xuất   được xử lý             không thể
                              kết thúc     đăng                 theo chính             tiếp tục gọi
                              phiên làm    nhập                 sách                   API bảo vệ
                              việc hiện                                                nếu yêu cầu
                              tại.                                                     hệ thống là
                                                                                       thu hồi ngay;
                                                                                       phải triển
                                                                                       khai cơ chế
                                                                                       thu hồi/phiên
                                                                                       phù hợp.

  **FR-USER-003**   Xem hồ sơ Cho phép     Người    Yêu cầu xem Họ tên,       Must     Hiển thị đúng
                    cá nhân   người dùng   dùng đã  hồ sơ       email, số              thông tin;
                              xem thông    đăng                 điện thoại và          không bao giờ
                              tin tài      nhập                 thông tin              hiển thị mật
                              khoản của                         được phép              khẩu hoặc dữ
                              chính mình.                       hiển thị               liệu xác thực
                                                                                       bí mật.

  **FR-USER-004**   Cập nhật  Cho phép sửa Người    Thông tin   Hồ sơ sau     Must     Dữ liệu hợp
                    hồ sơ cá  họ tên, số   dùng đã  hồ sơ cập   chỉnh sửa              lệ được lưu
                    nhân      điện thoại   đăng     nhật                               và giao diện
                              và avatar.   nhập                                        hiển thị dữ
                                                                                       liệu mới.

  **FR-USER-005**   Đổi mật   Cho phép     Người    Mật khẩu    Kết quả đổi   Must     Chỉ đổi khi
                    khẩu      người dùng   dùng đã  cũ, mật     mật khẩu               mật khẩu cũ
                              thay đổi mật đăng     khẩu mới và                        đúng và xác
                              khẩu.        nhập     xác nhận                           nhận khớp;
                                                                                       lưu mật khẩu
                                                                                       theo cơ chế
                                                                                       băm an toàn
                                                                                       và áp dụng
                                                                                       chính sách
                                                                                       phiên đã
                                                                                       thống nhất.

  **FR-USER-006**   Tìm kiếm  Cho phép     Admin có Từ khóa và  Danh sách tài Must     Chỉ hiển thị
                    và xem    quản trị     thẩm     điều kiện   khoản phù hợp          tài khoản
                    danh sách viên tìm     quyền    lọc                                thuộc phạm vi
                    tài khoản kiếm tài                                                 quản lý; kết
                              khoản trong                                              quả khớp điều
                              phạm vi được                                             kiện tìm
                              phân quyền.                                              kiếm.
  --------------------------------------------------------------------------------------------------

# Module 8. Quản lý nhà yến (FR-FARM)

  -------------------------------------------------------------------------------------------------
  **Mã FR**         **Tên     **Mô tả**    **Tác    **Input**   **Output**   **Ưu     **Tiêu chí
                    chức                   nhân**                            tiên**   chấp nhận**
                    năng**                                                            
  ----------------- --------- ------------ -------- ----------- ------------ -------- -------------
  **FR-FARM-001**   Thêm mới  Đăng ký nhà  Farm     Tên, địa    Bản ghi nhà  Must     Tạo thành
                    nhà yến   yến mới vào  Owner /  chỉ, diện   yến mới               công khi dữ
                              hệ thống.    Admin    tích và                           liệu bắt buộc
                                                    thông tin                         hợp lệ; nhà
                                                    bắt buộc                          yến mới xuất
                                                                                      hiện trong
                                                                                      danh sách.

  **FR-FARM-002**   Xem danh  Hiển thị các Farm     Yêu cầu xem Danh sách    Must     Chỉ hiển thị
                    sách nhà  nhà yến mà   Owner /  danh sách   nhà yến được          nhà yến trong
                    yến       tài khoản    Admin                phân quyền            phạm vi được
                              được cấp                                                cấp; hiển thị
                              quyền truy                                              danh sách
                              cập.                                                    trống nếu
                                                                                      chưa có.

  **FR-FARM-003**   Xem chi   Xem thông    Farm     ID nhà yến  Thông tin    Must     Hiển thị đầy
                    tiết nhà  tin địa lý,  Owner /              chi tiết nhà          đủ dữ liệu
                    yến       tầng/Zone và Admin                yến                   được phép; từ
                              thiết bị                                                chối truy cập
                              thuộc nhà                                               nếu người
                              yến.                                                    dùng không có
                                                                                      quyền.

  **FR-FARM-004**   Cập nhật  Chỉnh sửa    Farm     ID nhà yến  Kết quả cập  Must     Dữ liệu hợp
                    thông tin tên, địa chỉ Owner /  và dữ liệu  nhật                  lệ được lưu
                    nhà yến   và mô tả nhà Admin    cập nhật                          và hiển thị
                              yến.                                                    nhất quán
                                                                                      trên hệ
                                                                                      thống.

  **FR-FARM-005**   Ngừng     Chuyển nhà   Farm     ID nhà yến  Trạng thái   Should   Nhà yến được
                    quản lý   yến sang     Owner /  và xác nhận nhà yến được          đánh dấu
                    nhà yến   trạng thái   Admin                cập nhật              ngừng hoạt
                              ngừng hoạt                                              động; dữ liệu
                              động nhưng                                              lịch sử vẫn
                              giữ dữ liệu                                             có thể tra
                              lịch sử.                                                cứu theo
                                                                                      quyền.
  -------------------------------------------------------------------------------------------------

# Module 9. Quản lý tầng/khu vực (FR-ZONE)

  --------------------------------------------------------------------------------------------------
  **Mã FR**         **Tên chức **Mô tả**    **Tác    **Input**   **Output**   **Ưu     **Tiêu chí
                    năng**                  nhân**                            tiên**   chấp nhận**
  ----------------- ---------- ------------ -------- ----------- ------------ -------- -------------
  **FR-ZONE-001**   Thêm       Tạo tầng     Farm     ID nhà yến  Zone mới     Must     Zone được tạo
                    tầng/khu   hoặc Zone    Owner /  và thông    được khởi             và liên kết
                    vực        chức năng    Admin    tin Zone    tạo                   đúng với nhà
                               thuộc một                                               yến đã chọn.
                               nhà yến.                                                

  **FR-ZONE-002**   Xem danh   Hiển thị các Farm     ID nhà yến  Danh sách    Must     Hiển thị đầy
                    sách       Zone thuộc   Owner /              Zone trực             đủ Zone thuộc
                    tầng/khu   nhà yến được Admin                thuộc                 đúng nhà yến
                    vực        chọn.                                                   và nằm trong
                                                                                       phạm vi
                                                                                       quyền.

  **FR-ZONE-003**   Xem chi    Xem cảm      Farm     ID Zone     Chi tiết     Must     Hiển thị đúng
                    tiết       biến, thiết  Owner /              Zone và danh          các thiết bị
                    tầng/khu   bị chấp hành Admin                sách thiết            liên kết với
                    vực        và camera                         bị                    Zone.
                               thuộc Zone.                                             

  **FR-ZONE-004**   Cập nhật   Chỉnh sửa    Farm     ID Zone và  Kết quả      Must     Lưu thông tin
                    tầng/khu   tên, diện    Owner /  dữ liệu cập chỉnh sửa             khi dữ liệu
                    vực        tích, vị trí Admin    nhật                              đầu vào hợp
                               tầng hoặc mô                                            lệ.
                               tả Zone.                                                

  **FR-ZONE-005**   Ngừng sử   Đánh dấu     Farm     ID Zone và  Trạng thái   Should   Zone chuyển
                    dụng       Zone ngừng   Owner /  xác nhận    Zone thay             sang Ngừng sử
                    tầng/khu   hoạt động mà Admin                đổi                   dụng; liên
                    vực        không xóa dữ                                            kết và dữ
                               liệu lịch sử                                            liệu lịch sử
                               thiết bị.                                               được giữ lại.
  --------------------------------------------------------------------------------------------------

# Module 10. Quản lý thiết bị (FR-DEVICE)

  ---------------------------------------------------------------------------------------------------------
  **Mã FR**           **Tên     **Mô tả**          **Tác    **Input**   **Output**   **Ưu     **Tiêu chí
                      chức                         nhân**                            tiên**   chấp nhận**
                      năng**                                                                  
  ------------------- --------- ------------------ -------- ----------- ------------ -------- -------------
  **FR-DEVICE-001**   Đăng ký   Đăng ký ESP32, cảm Farm     Mã định     Thiết bị mới Must     Chỉ đăng ký
                      thiết bị  biến, relay hoặc   Owner /  danh và     được ghi              nếu mã định
                                camera bằng        Admin    loại thiết  nhận                  danh chưa tồn
                                MAC/Serial/UUID.            bị                                tại; từ chối
                                                                                              mã trùng.

  **FR-DEVICE-002**   Gán thiết Liên kết thiết bị  Farm     ID thiết bị Liên kết vị  Must     Thiết bị hiển
                      bị vào    đã đăng ký với     Owner /  và ID Zone  trí thiết bị          thị đúng tại
                      Zone      Zone/tầng trong    Admin                                      Zone;
                                nhà yến.                                                      telemetry
                                                                                              được gắn với
                                                                                              Zone tương
                                                                                              ứng.

  **FR-DEVICE-003**   Xem danh  Xem thiết bị theo  Farm     ID nhà yến  Danh sách    Must     Hiển thị
                      sách      nhóm cảm biến,     Owner /  hoặc ID     thiết bị              thiết bị cùng
                      thiết bị  thiết bị chấp hành Admin    Zone        phân loại             loại và vị
                                và camera.                                                    trí lắp đặt
                                                                                              chính xác.

  **FR-DEVICE-004**   Xem chi   Xem thông số cấu   Farm     ID thiết bị Thông tin    Must     Hiển thị
                      tiết và   hình, trạng thái   Owner /              chi tiết và           trạng thái
                      trạng     bật/tắt và kết nối Admin                trạng thái            vận hành và
                      thái      mạng.                                                         thời điểm
                      thiết bị                                                                nhận tín hiệu
                                                                                              gần nhất.

  **FR-DEVICE-005**   Cập nhật  Chỉnh sửa tên hiển Farm     ID thiết bị Thông tin    Must     Thông tin
                      thông tin thị, vị trí chi    Owner /  và thông    thiết bị cập          quản lý được
                      thiết bị  tiết hoặc ghi chú. Admin    tin chỉnh   nhật                  cập nhật mà
                                                            sửa                               không làm
                                                                                              gián đoạn vận
                                                                                              hành thiết
                                                                                              bị.

  **FR-DEVICE-006**   Ngừng sử  Đánh dấu thiết bị  Farm     ID thiết bị Trạng thái   Must     Giữ lịch sử
                      dụng      hỏng/ngừng sử dụng Owner /  và xác nhận vận                   thiết bị;
                      thiết bị  hoặc gỡ liên kết   Admin                hành/quản             phân biệt rõ
                                theo quy trình.                         trị cập nhật          Offline (mất
                                                                                              kết nối),
                                                                                              Disabled (bị
                                                                                              vô hiệu hóa)
                                                                                              và Fault
                                                                                              (lỗi).
  ---------------------------------------------------------------------------------------------------------

# Module 11. Cấu hình thiết bị (FR-CONFIG)

  ---------------------------------------------------------------------------------------------------
  **Mã FR**           **Tên     **Mô tả**    **Tác    **Input**   **Output**   **Ưu     **Tiêu chí
                      chức                   nhân**                            tiên**   chấp nhận**
                      năng**                                                            
  ------------------- --------- ------------ -------- ----------- ------------ -------- -------------
  **FR-CONFIG-001**   Xem cấu   Xem thông số Farm     ID thiết bị Cấu hình     Must     Hiển thị đúng
                      hình hiện vận hành áp  Owner /  hoặc ID     hiện tại              các ngưỡng và
                      tại       dụng cho     Admin    Zone                              tham số đã
                                từng thiết                                              lưu.
                                bị/Zone.                                                

  **FR-CONFIG-002**   Thiết lập Cấu hình     Farm     ID Zone và  Bản ghi cấu  Must     Kiểm tra quan
                      ngưỡng    ngưỡng nhiệt Owner /  các ngưỡng  hình ngưỡng           hệ ngưỡng
                      vận hành  độ, độ ẩm,   Admin                                      dưới/trên;
                                CO₂ và ánh                                              lưu cấu hình
                                sáng cho                                                hợp lệ và
                                từng Zone.                                              đồng bộ xuống
                                                                                        ESP32 qua
                                                                                        MQTT.

  **FR-CONFIG-003**   Chọn chế  Thiết lập    Farm     ID thiết bị Chế độ vận   Must     Chế độ được
                      độ điều   Auto hoặc    Owner /  và chế độ   hành được             lưu và ESP32
                      khiển     Manual cho   Admin    chọn        cập nhật              tuân thủ đúng
                                từng thiết                                              logic; xác
                                bị chấp                                                 định rõ ưu
                                hành.                                                   tiên giữa
                                                                                        lệnh tay tại
                                                                                        chỗ và lệnh
                                                                                        từ ứng dụng.

  **FR-CONFIG-004**   Cập nhật  Cấu hình chu Farm     ID thiết bị Thông số cấu Must     Chỉ chấp nhận
                      thông số  kỳ gửi       Owner /  và tham số  hình mới              giá trị trong
                      vận hành  telemetry,   Admin    cấu hình                          khoảng hợp
                                thời gian                                               lệ; lưu và
                                timeout và                                              đồng bộ xuống
                                tham số nâng                                            ESP32 thành
                                cao.                                                    công.

  **FR-CONFIG-005**   Khôi phục Đưa cấu hình Farm     ID thiết    Cấu hình mặc Should   Áp dụng đúng
                      cấu hình  ngưỡng và    Owner /  bị/Zone và  định được áp          bộ giá trị
                      mặc định  tham số vận  Admin    xác nhận    dụng                  mặc định đã
                                hành về giá           reset                             được phê
                                trị mặc định                                            duyệt; xác
                                đã xác định.                                            nhận thao tác
                                                                                        trước khi
                                                                                        reset.
  ---------------------------------------------------------------------------------------------------

# Module 12. Phân quyền quản trị (FR-RBAC)

  ---------------------------------------------------------------------------------------------------
  **Mã FR**         **Tên     **Mô tả**    **Tác     **Input**    **Output**   **Ưu     **Tiêu chí
                    chức                   nhân**                              tiên**   chấp nhận**
                    năng**                                                              
  ----------------- --------- ------------ --------- ------------ ------------ -------- -------------
  **FR-RBAC-001**   Gán vai   Gán vai trò  Super     ID tài khoản Vai trò tài  Must     Tài khoản
                    trò tài   cho tài      Admin /   và vai trò   khoản được            nhận đúng vai
                    khoản     khoản theo   Admin có               cập nhật              trò và quyền
                              mô hình      thẩm                                         tương ứng;
                              quyền được   quyền                                        tên vai trò
                              nhóm thống                                                phải thống
                              nhất.                                                     nhất toàn tài
                                                                                        liệu.

  **FR-RBAC-002**   Thay đổi  Chỉnh sửa    Super     ID tài khoản Vai trò mới  Must     Quyền cũ bị
                    vai trò   hoặc hạ cấp  Admin /   và vai trò   được cập              loại bỏ theo
                    tài khoản vai trò của  Admin có  mới          nhật                  chính sách
                              tài khoản.   thẩm                                         hiệu lực;
                                           quyền                                        kiểm tra lại
                                                                                        quyền ở
                                                                                        Backend,
                                                                                        không chỉ ở
                                                                                        giao diện.

  **FR-RBAC-003**   Kiểm soát Kiểm tra     Backend   Yêu cầu API  Cho phép     Must     100% API được
                    quyền     quyền Read,  System    và thông tin hoặc từ chối          bảo vệ kiểm
                    chức năng Write,                 xác thực     truy cập              tra quyền
                              Control và                                                trước khi xử
                              Delete trên                                               lý; từ chối
                              các API được                                              khi không đủ
                              bảo vệ.                                                   quyền.

  **FR-RBAC-004**   Giới hạn  Giới hạn dữ  Backend   ID người     Truy cập hợp Must     Người dùng
                    phạm vi   liệu và thao System    dùng và ID   lệ hoặc bị            không thể xem
                    nhà yến   tác trong              nhà yến      chặn                  hoặc điều
                              các nhà yến                                               khiển nhà yến
                              được chỉ                                                  ngoài phạm vi
                              định cho tài                                              được cấp.
                              khoản.                                                    

  **FR-RBAC-005**   Xem ma    Xem tài      Admin có  Tài          Ma trận phân Must     Hiển thị
                    trận phân khoản, vai   thẩm      khoản/điều   quyền                 chính xác vai
                    quyền     trò và danh  quyền     kiện lọc nếu                       trò và phạm
                              sách nhà yến           có                                 vi truy cập
                              được cấp                                                  của từng tài
                              quyền quản                                                khoản theo
                              lý.                                                       quyền người
                                                                                        xem.
  ---------------------------------------------------------------------------------------------------

1\. M1: IoT Monitoring & Control (SFSF-14)

-   **Phụ trách:** Nguyen Huy Tuan

-   **Nội dung công việc:** Lập trình vi điều khiển ESP32, kết nối cảm
    biến thu thập dữ liệu vi khí hậu, cấu hình module Relay và giao thức
    truyền thông MQTT.

-   **Các mã FR thuộc phần này:**

    -   FR-IOT-001 đến FR-IOT-004: Theo dõi Nhiệt độ, Độ ẩm, Cường độ
        ánh sáng và Nồng độ CO2.

    -   FR-IOT-005: Cài đặt Ngưỡng Môi trường.

    -   FR-IOT-006 đến FR-IOT-008: Điều khiển Actuator, Điều khiển Tự
        động (Offline Rule-based) và Ghi đè Thủ công (Manual Override).

    -   FR-IOT-009: Theo dõi Trạng thái Thiết bị & Kết nối IoT.

#### 2. M2: Swiftlet Counting & AI (SFSF-15)

-   **Phụ trách:** Pham Van Tuan

-   **Nội dung công việc:** Phát triển hệ thống AI & Computer Vision,
    huấn luyện/tích hợp mô hình YOLOv8 nhận diện chim yến và thiên địch,
    thuật toán ByteTrack đếm chim ra/vào, xử lý khung hình bất thường
    (Anomaly Snapshot) và giám sát âm thanh loa yến.

-   **Các mã FR thuộc phần này:**

    -   FR-AI-001 đến FR-AI-004: Giám sát Camera, Nhận diện YOLO, Đếm
        chim ra/vào bằng ByteTrack và Theo dõi biến động đàn.

    -   FR-AI-005 đến FR-AI-007: Phát hiện Thiên địch (Rắn, chuột, cú),
        Lưu Anomaly Snapshot và Đồng bộ kết quả AI về hệ thống.

    -   FR-AUD-001 đến FR-AUD-002: Giám sát Tín hiệu Âm thanh và Cảnh
        báo Sự cố Loa (Bổ sung mới).

#### 3. M3: Statistics & Reports (SFSF-16)

-   **Phụ trách:** Nguyen Ngoc Lam Tuan

-   **Nội dung công việc:** Xây dựng logic thống kê dữ liệu, trực quan
    hóa biểu đồ xu hướng, quản lý lịch sử vận hành và tính năng trích
    xuất báo cáo tổng hợp.

-   **Các mã FR thuộc phần này:**

    -   FR-REP-001 đến FR-REP-003: Theo dõi Xu hướng Môi trường, Xem
        Lịch sử Vi khí hậu và Thống kê Lượt Vào/Ra.

    -   FR-REP-004 đến FR-REP-005: Xem Danh sách Báo cáo và Khởi tạo Báo
        cáo Tổng hợp.

    -   FR-REP-006 đến FR-REP-007: Thống kê Phát hiện Chim và Phân tích
        Tương quan Môi trường - Năng suất.

#### 4. M4: User / House / Zone / Device (SFSF-17)

-   **Phụ trách:** Nguyen Thi Huyen Tran

-   **Nội dung công việc:** Xây dựng kiến trúc hệ thống, quản lý cơ sở
    dữ liệu và phát triển các phân hệ quản trị thực thể (Người dùng, Nhà
    yến, Tầng/Khu vực, Thiết bị, Cấu hình và Phân quyền RBAC).

-   **Các mã FR thuộc phần này:**

    -   FR-USER-001 đến FR-USER-006: Quản lý tài khoản (Đăng nhập, Đăng
        xuất, Hồ sơ, Đổi mật khẩu, Tìm kiếm tài khoản).

    -   FR-FARM-001 đến FR-FARM-005: Quản lý Vòng đời Nhà yến (Thêm,
        Xem, Sửa, Ngừng quản lý).

    -   FR-ZONE-001 đến FR-ZONE-005: Quản lý Tầng / Khu vực trực thuộc
        nhà yến.

    -   FR-DEVICE-001 đến FR-DEVICE-006: Đăng ký, Gán Zone, Theo dõi và
        Quản lý trạng thái phần cứng.

    -   FR-CONFIG-001 đến FR-CONFIG-005: Xem/Cài đặt Ngưỡng, Chế độ điều
        khiển và Khôi phục cấu hình mặc định.

    -   FR-RBAC-001 đến FR-RBAC-005: Phân quyền quản trị, gán vai trò và
        kiểm soát phạm vi truy cập nhà yến.

#### 5. M5: Alert & User Interface (SFSF-18)

-   **Phụ trách:** Ngo Thi Kim Ngan

-   **Nội dung công việc:** Thiết kế và phát triển giao diện Web
    Dashboard trực quan, tích hợp hệ thống hiển thị cảnh báo thời gian
    thực và dịch vụ đẩy thông báo đa kênh (Web/Mobile).

-   **Các mã FR thuộc phần này:**

    -   FR-DASH-001 đến FR-DASH-004: Dashboard tổng quan, Widget
        Telemetry real-time, Widget Số lượng chim và Widget
        Camera/Anomaly Frame.

    -   FR-ALT-001 đến FR-ALT-002: Hiển thị Cảnh báo Thời gian thực và
        Quản lý Trạng thái Cảnh báo.

    -   FR-NOTI-001: Gửi Thông báo đa kênh (Web Notification & Mobile
        Push).
