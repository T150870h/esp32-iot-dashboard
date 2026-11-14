/*
  ESP32 + DHT11 (nhiet do, do am) + BH1750 (anh sang)

  Day noi (theo so do):
    DHT11  : S -> D4,  + -> 3V3,  - -> GND
    BH1750 : SDA -> D21, SCL -> D22, VCC -> 3V3, GND -> GND, ADDR de trong (dia chi 0x23)

  Hai che do:
    USE_WIFI = false : chi in du lieu ra cong USB moi 2 giay (dung de thu ngay, khong can WiFi)
    USE_WIFI = true  : ngoai in USB, con gui len ThingSpeak moi 20 giay

  Thu vien can cai (Arduino IDE -> Tools -> Manage Libraries):
    - "DHT sensor library" (Adafruit)  + "Adafruit Unified Sensor"
    - "BH1750" (Christopher Laws)
  Board: "ESP32 Dev Module", Serial Monitor: 115200 baud
*/

#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>

// ====== CAU HINH ======
#define USE_WIFI false              // doi thanh true khi muon gui len ThingSpeak
const char* WIFI_SSID     = "TEN_WIFI";
const char* WIFI_PASSWORD = "MAT_KHAU_WIFI";
const char* TS_WRITE_KEY  = "WRITE_API_KEY_CUA_CHANNEL";
// ======================

#if USE_WIFI
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>
#endif

#ifndef ALERT_TEMP
#define ALERT_TEMP 31.0   // nhiet do canh bao (do C): vuot nguong thi LED xanh tren board (GPIO2) nhap nhay
#endif
#define LED_PIN    2

#define DHT_PIN   4
#define DHT_TYPE  DHT11
#define SDA_PIN   21
#define SCL_PIN   22

const unsigned long READ_INTERVAL_MS = 2000;    // DHT11 doc toi da ~1 lan/giay
const unsigned long SEND_INTERVAL_MS = 20000;   // ThingSpeak mien phi: >= 15 giay

DHT dht(DHT_PIN, DHT_TYPE);
BH1750 lightMeter;  // ADDR de trong -> 0x23

unsigned long lastRead = 0, lastSend = 0, lastBlink = 0;
bool alertActive = false;

#if USE_WIFI
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.printf("Dang ket noi WiFi %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nDa ket noi. IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nKhong ket noi duoc WiFi, se thu lai.");
  }
}

bool sendToThingSpeak(float t, float h, float lux) {
  WiFiClientSecure client;
  client.setInsecure();  // bai thuc hanh: bo qua kiem tra chung chi
  HTTPClient http;
  String url = "https://api.thingspeak.com/update?api_key=" + String(TS_WRITE_KEY) +
               "&field1=" + String(t, 1) + "&field2=" + String(h, 1);
  if (lux >= 0) url += "&field3=" + String(lux, 1);
  if (!http.begin(client, url)) return false;
  int code = http.GET();
  String body = http.getString();  // so thu tu ban ghi, "0" = that bai
  http.end();
  Serial.printf("ThingSpeak: HTTP %d, entry %s\n", code, body.c_str());
  return code == 200 && body != "0";
}
#endif

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ESP32 khoi dong...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  dht.begin();
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("LOI: khong tim thay BH1750, kiem tra day SDA/SCL.");
  }
#if USE_WIFI
  connectWiFi();
#endif
}

void loop() {
  // LED canh bao: nhap nhay moi 250 ms khi qua nguong, tat khi binh thuong
  if (alertActive) {
    if (millis() - lastBlink >= 250) { lastBlink = millis(); digitalWrite(LED_PIN, !digitalRead(LED_PIN)); }
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  if (millis() - lastRead >= READ_INTERVAL_MS) {
    lastRead = millis();
    float t   = dht.readTemperature();
    float h   = dht.readHumidity();
    float lux = lightMeter.readLightLevel();
    if (lux < 0) {                       // BH1750 chua san sang: thu khoi tao lai, lan sau se doc duoc neu day da dung
      lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
      lux = -1;
      Serial.println("CANH BAO: chua doc duoc BH1750 (kiem tra day SDA/SCL/VCC/GND). Van gui nhiet do, do am.");
    }

    if (isnan(t) || isnan(h)) {
      Serial.println("LOI: doc DHT11 that bai, kiem tra day D4.");
    } else {
      // Dinh dang nay duoc trang web phan tich - dung doi
      Serial.printf("Nhiet do: %.1f C | Do am: %.0f %% | Anh sang: %.1f lux\n", t, h, lux);
      bool hot = t >= ALERT_TEMP;
      if (hot && !alertActive) Serial.printf("CANH BAO: nhiet do %.1f C vuot nguong %.1f C\n", t, (float)ALERT_TEMP);
      if (!hot && alertActive) Serial.println("Nhiet do da tro ve binh thuong.");
      alertActive = hot;
#if USE_WIFI
      if (millis() - lastSend >= SEND_INTERVAL_MS || lastSend == 0) {
        connectWiFi();
        if (WiFi.status() == WL_CONNECTED && sendToThingSpeak(t, h, lux)) lastSend = millis();  // that bai: thu lai o lan doc sau
      }
#endif
    }
  }
}
