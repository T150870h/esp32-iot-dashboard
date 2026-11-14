// Chan doan I2C v2: kiem tra muc logic SDA/SCL va quet nhieu cap chan
#include <Wire.h>

bool scanPair(int sda, int scl) {
  Wire.end();
  Wire.begin(sda, scl);
  Wire.setTimeOut(30);
  bool found = false;
  for (int a = 8; a < 120; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { Serial.printf("  >>> THAY THIET BI I2C 0x%02X voi SDA=D%d SCL=D%d\n", a, sda, scl); found = true; }
  }
  Wire.end();
  return found;
}

void probe(int pin) {
  pinMode(pin, INPUT_PULLDOWN); delay(10); int pd = digitalRead(pin);
  pinMode(pin, INPUT_PULLUP);   delay(10); int pu = digitalRead(pin);
  const char* verdict = (pd == 1) ? "co dien tro keo len ngoai (module co nguon)" :
                        (pu == 0) ? "BI KEO XUONG GND (chap/ngan mach)" : "tha noi (khong co gi noi hoac module khong co nguon)";
  Serial.printf("  D%-2d: pulldown=%d pullup=%d -> %s\n", pin, pd, pu, verdict);
}

void setup() { Serial.begin(115200); delay(500); }

void loop() {
  Serial.println("== Chan doan I2C v2 ==");
  probe(21); probe(22); probe(18); probe(19);
  const int pins[] = {21, 22, 18, 19, 23, 25, 26, 27, 32, 33, 16, 17, 13, 14};
  const int n = sizeof(pins) / sizeof(pins[0]);
  bool any = false;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (i == j) continue;
      pinMode(pins[i], INPUT_PULLUP); pinMode(pins[j], INPUT_PULLUP); delay(2);
      if (digitalRead(pins[i]) && digitalRead(pins[j])) any |= scanPair(pins[i], pins[j]);   // bo qua cap chan dang bi keo thap de khoi treo
    }
  }
  Serial.println(any ? "== Ket thuc: CO thiet bi I2C ==" : "== Ket thuc: khong thay thiet bi I2C tren cap chan nao ==");
  delay(3000);
}
