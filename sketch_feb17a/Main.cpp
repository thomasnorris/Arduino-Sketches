#include "Main.h"

// helpers
auto _wifi = new WifiClient(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
auto _ota = new OTAClient(HOSTNAME);
auto _blynk = new BlynkServer(BLYNK_IP, BLYNK_PORT, BLYNK_AUTH_TOKEN);
auto _logger = new LoggerClient(LOGGER_URL, LOGGER_AUTH_HEADER, LOGGER_AUTH_TOKEN, LOGGER_APP_ID);
auto _th = new TimeHelpers();
auto _db = new ArduinoData(ARDUINO_DATA_APP_ID, ARDUINO_DATA_URL, ARDUINO_DATA_USERNAME, ARDUINO_DATA_PASSWORD, ARDUINO_DATA_PORT);
auto _doorSensor = new Gpio(DOOR_PIN, DOOR_PIN_PINMODE);
auto _doorLed = new Led(DOOR_LED_PIN, DOOR_LED_ON_VALUE);

// blynk leds
auto _doorVirtLed = new VirtualLed(DOOR_LED_VPIN);

// blynk displays
auto _uptimeDisplay = new VirtualPin(SYSTEM_UPTIME_DISPLAY_VPIN);

// variables
std::chrono::time_point<std::chrono::system_clock> _uptime_start;

// handle blynk virtual pin value changes here
void handleBlynkPinValueChange(int pin, String val) {
  switch (pin) {
    case SYSTEM_UPTIME_DISPLAY_VPIN:
      _uptimeDisplay->set(val);
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}

  _uptime_start = _th->getClockTimeNow();

  // init connections
  _wifi->connect();
  _blynk->configure();
  _blynk->connect();
  _blynk->run();
  _ota->begin();
  _th->begin();
  _th->update();

  // init bylnk i/o
  _doorVirtLed->off();

  // respond done
  String init_message = "System initialized";
  _logger->init(init_message, _wifi->getIPAddress());
  _blynk->notify(init_message);
}

void loop() {
  updateUptime();

  _wifi->checkConnection();
  _blynk->checkConnection();
  _blynk->run();
  _ota->handle();
  _th->update();

  delay(LOOP_DELAY_MS);
}

void updateUptime() {
  auto current_time = _th->getClockTimeNow();
  int uptime_s = _th->getElapsedTimeS(_uptime_start, current_time);
  _uptimeDisplay->write(_th->prettyFormatS(uptime_s));
}