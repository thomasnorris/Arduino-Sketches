#include "Main.h"

Wifi* _wifi = new Wifi(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}

  _wifi->connect();
}

void loop() {
  Serial.println("loop");
  _wifi->checkConnection();

  delay(LOOP_DELAY_MS);
}