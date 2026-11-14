# Giám sát môi trường với ESP32 (DHT11 + BH1750)

Bài thực hành IoT: ESP32 đo nhiệt độ, độ ẩm (DHT11) và ánh sáng (BH1750), hiển thị trên web bằng
đồ thị đường, cột, tròn, phân tán; cảnh báo khi nhiệt độ ≥ 31 °C hoặc ánh sáng ≤ 5 lux (quá tối).

## Sơ đồ nối dây

| Linh kiện | Chân module | Chân ESP32 |
|---|---|---|
| DHT11 | S, +, − | D4, 3V3, GND |
| BH1750 | SDA (DAT), SCL | D21, D19 |
| BH1750 | VCC, GND | 3V3, GND (ADDR để trống → 0x23) |

## Thành phần

- `esp32_sensor/esp32_sensor.ino` – firmware. Không có `wifi_config.h` thì chỉ in dữ liệu ra USB; có file này thì ESP32 tự nối WiFi và gửi lên Firebase (xem mục WiFi bên dưới). LED GPIO2 nháy khi nhiệt độ ≥ `ALERT_TEMP` (31 °C) hoặc ánh sáng ≤ `ALERT_LUX` (5 lux).
- `esp32_dashboard.html` – trang web (Chart.js). Nguồn dữ liệu: USB (Web Serial), ThingSpeak, hoặc chế độ demo.
- `tools/i2c_diag`, `tools/i2c_scan` – chương trình chẩn đoán I2C khi BH1750 không phản hồi.

## Chạy

1. Arduino IDE (hoặc arduino-cli): cài thư viện *DHT sensor library*, *Adafruit Unified Sensor*, *BH1750*; board `ESP32 Dev Module`; nạp `esp32_sensor.ino`.
2. Trong thư mục dự án: `python -m http.server 8000`, mở `http://localhost:8000/esp32_dashboard.html` bằng Chrome hoặc Edge.
3. Bấm **Kết nối USB (ESP32)** và chọn cổng COM (đóng Serial Monitor trước). Hoặc tick **Chế độ demo** để xem thử không cần mạch.

Windows cần driver CP210x (Silicon Labs) nếu không thấy cổng COM của ESP32.

## Lưu dữ liệu lên Firebase (tùy chọn)

Trang web ghi mỗi lần đo lên **Firebase Realtime Database** bằng REST (không cần cài SDK), và có thể đọc lại từ bất kỳ máy nào.
URL và token chỉ nhập trong ô Cấu hình (lưu trong trình duyệt), **không nằm trong mã nguồn**.

1. Vào <https://console.firebase.google.com> → **Add project** → **Build → Realtime Database → Create database** (chọn vùng bất kỳ).
2. Tab **Rules**, dán đoạn dưới rồi **Publish**: ai cũng đọc được, chỉ được *thêm* bản ghi đúng định dạng, không sửa hoặc xóa.

   ```json
   {
     "rules": {
       "readings": {
         ".read": true,
         "$id": {
           ".write": "!data.exists()",
           ".validate": "newData.hasChildren(['ts','temp','hum']) && newData.child('ts').isNumber() && newData.child('temp').isNumber() && newData.child('hum').isNumber()"
         }
       }
     }
   }
   ```
3. Chép **Database URL** (dạng `https://<tên-dự-án>-default-rtdb.firebaseio.com`) vào ô *Firebase Database URL* trong Cấu hình.
4. **Lưu:** tick *Lưu mỗi lần đo lên Firebase*, bấm **Kết nối USB**; cứ 5 giây trang ghi 1 bản ghi `{ts, temp, hum, lux, alert}` vào nút `readings`.
5. **Xem lại:** tick *Xem dữ liệu từ Firebase*, bấm **Áp dụng & kết nối**; trang đọc N bản ghi mới nhất và làm mới theo chu kỳ. Máy khác chỉ cần dán cùng URL.

Lưu ý: URL cơ sở dữ liệu mà bị lộ thì người khác có thể thêm bản ghi giả. Với dự án học tập thì chấp nhận được; muốn chặt hơn, đặt `".write": "auth != null"` và dùng ô *Firebase token*.

## ESP32 tự gửi lên Firebase qua WiFi (không cần máy tính)

1. Trong `esp32_sensor/`, sao chép `wifi_config.h.example` thành `wifi_config.h` và điền `WIFI_SSID`, `WIFI_PASSWORD` (WiFi 2.4 GHz) và `FIREBASE_URL`.
   File `wifi_config.h` đã nằm trong `.gitignore` nên **không lên GitHub**.
2. Nạp lại firmware. ESP32 in `Da ket noi. IP: …` rồi cứ 10 giây gửi một bản ghi, serial hiện `Firebase: HTTP 200`. Thời gian do máy chủ Firebase đóng dấu nên ESP32 không cần NTP.
3. Mở link xem từ xa (`…/esp32_dashboard.html#fb=<Database URL>`) trên bất kỳ thiết bị nào để xem số liệu. Có thể rút cáp USB, chỉ cần cấp nguồn.
4. Nếu bỏ file `wifi_config.h`, firmware quay về chế độ chỉ USB. Khai báo thêm `TS_WRITE_KEY` trong file này nếu muốn gửi cả ThingSpeak.
