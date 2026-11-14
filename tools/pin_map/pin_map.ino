// Ban do chan GPIO + quet I2C tren cac cap chan co dien tro keo len (tim ra SDA/SCL thuc te cua BH1750)
#include <Wire.h>
const int pins[] = {5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};   // bo D4 (DHT11)
int pulled[20]; int np = 0;

bool scanPair(int sda, int scl) {
  Wire.end(); Wire.begin(sda, scl); Wire.setTimeOut(30);
  bool found = false;
  for (int a = 8; a < 120; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { Serial.printf("  >>> THAY BH1750/I2C 0x%02X: SDA=D%d, SCL=D%d\n", a, sda, scl); found = true; }
  }
  Wire.end();
  return found;
}

void setup() { Serial.begin(115200); delay(500); }

void loop() {
  Serial.println("== Ban do chan ==");
  np = 0;
  for (int p : pins) {
    pinMode(p, INPUT_PULLDOWN); delay(5); int pd = digitalRead(p);
    pinMode(p, INPUT_PULLUP);   delay(5); int pu = digitalRead(p);
    if (pd == 1)      { Serial.printf("  D%-2d: KEO LEN ngoai\n", p); pulled[np++] = p; }
    else if (pu == 0) { Serial.printf("  D%-2d: KEO XUONG GND\n", p); }
  }
  bool any = false;
  for (int i = 0; i < np; i++) for (int j = 0; j < np; j++) if (i != j) any |= scanPair(pulled[i], pulled[j]);
  Serial.println(any ? "== Ket qua: TIM THAY cam bien ==" : "== Ket qua: khong thay cam bien I2C ==");
  delay(3000);
}
