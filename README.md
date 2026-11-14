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

- `esp32_sensor/esp32_sensor.ino` – firmware. `USE_WIFI false` chỉ in dữ liệu ra USB; `true` gửi thêm lên ThingSpeak
  (điền `WIFI_SSID`, `WIFI_PASSWORD`, `TS_WRITE_KEY`). LED GPIO2 nháy khi nhiệt độ ≥ `ALERT_TEMP` (31 °C) hoặc ánh sáng ≤ `ALERT_LUX` (5 lux).
- `esp32_dashboard.html` – trang web (Chart.js). Nguồn dữ liệu: USB (Web Serial), ThingSpeak, hoặc chế độ demo.
- `tools/i2c_diag`, `tools/i2c_scan` – chương trình chẩn đoán I2C khi BH1750 không phản hồi.

## Chạy

1. Arduino IDE (hoặc arduino-cli): cài thư viện *DHT sensor library*, *Adafruit Unified Sensor*, *BH1750*; board `ESP32 Dev Module`; nạp `esp32_sensor.ino`.
2. Trong thư mục dự án: `python -m http.server 8000`, mở `http://localhost:8000/esp32_dashboard.html` bằng Chrome hoặc Edge.
3. Bấm **Kết nối USB (ESP32)** và chọn cổng COM (đóng Serial Monitor trước). Hoặc tick **Chế độ demo** để xem thử không cần mạch.

Windows cần driver CP210x (Silicon Labs) nếu không thấy cổng COM của ESP32.
