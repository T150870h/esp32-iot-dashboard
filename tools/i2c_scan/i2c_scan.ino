#include <Wire.h>
void scan(int sda, int scl) {
  Wire.end();
  Wire.begin(sda, scl);
  Serial.printf("Quet I2C voi SDA=%d SCL=%d: ", sda, scl);
  int found = 0;
  for (int a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { Serial.printf("0x%02X ", a); found++; }
  }
  Serial.println(found ? "" : "(khong thay thiet bi)");
}
void setup() { Serial.begin(115200); delay(500); }
void loop() {
  scan(21, 22);
  scan(22, 21);
  Serial.printf("Muc D21=%d D22=%d khi nhan (keo len = 1)\n", digitalRead(21), digitalRead(22));
  delay(2000);
}
