#include "Main.h"

Wifi* _wifi = new Wifi(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
Gpio* _doorSensor = new Gpio(DOOR_PIN, DOOR_PIN_PINMODE);
Led* _doorLed = new Led(DOOR_LED_PIN, DOOR_LED_ON_VALUE);
GAClient* _gaClient = new GAClient(GA_URL, GA_AUTH_HEADER, GA_AUTH_TOKEN);

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}

  _wifi->connect();
}

void loop() {
  _wifi->checkConnection();
  _doorSensor->readAndPrint();

  if (_doorSensor->isLow()) {
    _doorLed->on();
  }
  else {
    _doorLed->off();
  }

  delay(LOOP_DELAY_MS);
}