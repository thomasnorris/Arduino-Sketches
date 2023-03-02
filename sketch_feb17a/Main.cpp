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
auto _lastTimeOpenedDisplay = new VirtualPin(LAST_OPENED_TIME_VPIN);
auto _lastTimeClosedDisplay = new VirtualPin(LAST_CLOSED_TIME_VPIN);
auto _countTimesOpenedDisplay = new VirtualPin(COUNT_TIMES_OPENED_VPIN);

// variables
std::chrono::time_point<std::chrono::system_clock> _uptime_start;
bool last_time_opened_recorded = false;
bool last_time_closed_recorded = false;

// handle blynk virtual pin value changes here
void handleBlynkPinValueChange(int pin, String val) {
  switch (pin) {
    case SYSTEM_UPTIME_DISPLAY_VPIN:
      _uptimeDisplay->set(val);
      break;
    case LAST_OPENED_TIME_VPIN:
      _lastTimeOpenedDisplay->set(val);
      break;
    case LAST_CLOSED_TIME_VPIN:
      _lastTimeClosedDisplay->set(val);
      break;
    case COUNT_TIMES_OPENED_VPIN:
      _countTimesOpenedDisplay->set(val);
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

  // get and set persistent data
  _countTimesOpenedDisplay->write(_db->getSumToday(DOOR_OPENED_DPT));
  _lastTimeOpenedDisplay->write(_db->getLast(LAST_OPENED_TIME_DPT));
  _lastTimeClosedDisplay->write(_db->getLast(LAST_CLOSED_TIME_DPT));

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

  auto localDateTime = _th->getCurrentLocalDateTime();

  // low means the sensor has made a connection
  if (_doorSensor->isLow()) {
    _doorLed->on();
    _doorVirtLed->on();

    last_time_opened_recorded = false;

    if (!last_time_closed_recorded) {
      last_time_closed_recorded = true;

      // record
      _lastTimeClosedDisplay->write(localDateTime);
      _db->updateDataPoint(LAST_CLOSED_TIME_DPT, localDateTime);
      _db->insertDataPoint(DOOR_CLOSED_DPT, 1);
      _logger->info("Door closed");
      _blynk->notify("Garage door closed");
    }
  }
  else {
    _doorLed->off();
    _doorVirtLed->off();

    last_time_closed_recorded = false;

    if (!last_time_opened_recorded) {
      last_time_opened_recorded = true;

      // record
      _lastTimeOpenedDisplay->write(localDateTime);
      _db->updateDataPoint(LAST_OPENED_TIME_DPT, localDateTime);
      _db->insertDataPoint(DOOR_OPENED_DPT, 1);
      _logger->info("Door opened");
      _blynk->notify("Garage door opened");

      _countTimesOpenedDisplay->write(_db->getSumToday(DOOR_OPENED_DPT));
    }
  }

  delay(LOOP_DELAY_MS);
}

void updateUptime() {
  auto current_time = _th->getClockTimeNow();
  int uptime_s = _th->getElapsedTimeS(_uptime_start, current_time);
  _uptimeDisplay->write(_th->prettyFormatS(uptime_s));
}