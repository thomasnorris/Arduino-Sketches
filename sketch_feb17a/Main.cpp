#include "Main.h"

// helpers
auto _wifi = new WifiClient(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
auto _ota = new OTAClient(HOSTNAME);
auto _blynk = new BlynkServer(BLYNK_IP, BLYNK_PORT, BLYNK_AUTH_TOKEN);
auto _logger = new LoggerClient(LOGGER_URL, LOGGER_AUTH_HEADER, LOGGER_AUTH_TOKEN, LOGGER_APP_ID);
auto _th = new TimeHelpers();
auto _db = new ArduinoData(ARDUINO_DATA_APP_ID, ARDUINO_DATA_URL, ARDUINO_DATA_USERNAME, ARDUINO_DATA_PASSWORD, ARDUINO_DATA_PORT);

// handle blynk virtual pin value changes here
void handleBlynkPinValueChange(int pin, String val) {
  // todo
}

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}

  // init connections
  _wifi->connect();
  _blynk->configure();
  _blynk->connect();
  _blynk->run();
  _ota->begin();
  _th->begin();
  _th->update();

  // respond done
  String init_message = "System initialized";
  _logger->init(init_message);
  _blynk->notify(init_message);
}

void loop() {
  _wifi->checkConnection();
  _blynk->checkConnection();
  _blynk->run();
  _ota->handle();
  _th->update();

  delay(LOOP_DELAY_MS);
}