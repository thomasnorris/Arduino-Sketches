#include "Main.h"

// helpers
auto _wifi = new WifiClient(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
auto _ota = new OTAClient(HOSTNAME);
auto _blynk = new BlynkServer(BLYNK_IP, BLYNK_PORT, BLYNK_AUTH_TOKEN);
auto _logger = new LoggerClient(LOGGER_APP_ID, LOGGER_URL, LOGGER_USERNAME, LOGGER_PASSWORD, LOGGER_PORT);
auto _th = new TimeHelpers();
auto _data = new ArduinoDataClient(ARDUINO_DATA_APP_ID, ARDUINO_DATA_URL, ARDUINO_DATA_USERNAME, ARDUINO_DATA_PASSWORD, ARDUINO_DATA_PORT);
auto _doorSensor = new Gpio(DOOR_PIN, DOOR_PIN_PINMODE);
auto _doorLed = new Led(DOOR_LED_PIN, DOOR_LED_ON_VALUE);
auto _handler = new ExceptionHandler();

// blynk leds
auto _doorVirtLed = new VirtualLed(DOOR_LED_VPIN);

// blynk displays
auto _uptimeDisplay = new VirtualPin(SYSTEM_UPTIME_DISPLAY_VPIN);
auto _lastTimeOpenedDisplay = new VirtualPin(LAST_OPENED_TIME_VPIN);
auto _lastTimeClosedDisplay = new VirtualPin(LAST_CLOSED_TIME_VPIN);
auto _countTimesOpenedDisplay = new VirtualPin(COUNT_TIMES_OPENED_VPIN);

// blynk terminals
auto _terminal = new VirtualTerminal(TERMINAL_VPIN, _th);

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
    case TERMINAL_VPIN:
      // custom command handling here
      handleCustomTerminalCommands(_terminal, val);
      break;
    default:
      break;
  }
}

// handle caught exceptions here
void handleException(String origin, String message, String details) {
  _logger->error(message, origin + ": " + details);

  message = "Error in " + origin + ": " + message;
  _blynk->notify(message);
  _terminal->error(message);
}

void setup() {
  try {
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
    _terminal->clear();
    _terminal->init("Initializing...");
    _doorVirtLed->off();

    // get data
    refreshDatabaseData();

    // respond done
    String init_message = "System initialized";
    String my_ip = _wifi->getIPAddress();
    _logger->init(init_message, _wifi->getIPAddress());
    _blynk->notify(init_message);
    _terminal->init(init_message);
    _terminal->info("IP: " + my_ip);
    _terminal->info("Type \"" + TERM_HELP + "\" to list custom commands");
  }
  catch (...) {
    _handler->handle("Main.cpp/setup()");
  }
}

void loop() {
  try {
    updateUptime();

    _wifi->checkConnection();
    _blynk->checkConnection();
    _blynk->run();
    _ota->handle();
    _th->update();

    auto localDateTime = _th->getCurrentLocalDateTime12hr();

    // low means the sensor has made a connection
    if (_doorSensor->isLow()) {
      _doorLed->on();
      _doorVirtLed->on();

      last_time_opened_recorded = false;

      if (!last_time_closed_recorded) {
        last_time_closed_recorded = true;

        // say
        String message = "Door closed";
        _logger->info(message);
        _blynk->notify(message);
        _terminal->info(message);

        // record
        _lastTimeClosedDisplay->write(localDateTime);
        _data->updateDataPoint(LAST_CLOSED_TIME_DPT, localDateTime);
        _data->insertDataPoint(DOOR_CLOSED_DPT, 1);
      }
    }
    else {
      _doorLed->off();
      _doorVirtLed->off();

      last_time_closed_recorded = false;

      if (!last_time_opened_recorded) {
        last_time_opened_recorded = true;

        // say
        String message = "Door opened";
        _logger->info(message);
        _blynk->notify(message);
        _terminal->info(message);

        // record
        _lastTimeOpenedDisplay->write(localDateTime);
        _data->updateDataPoint(LAST_OPENED_TIME_DPT, localDateTime);
        _data->insertDataPoint(DOOR_OPENED_DPT, 1);
        _countTimesOpenedDisplay->write(_data->getSumToday(DOOR_OPENED_DPT));
      }
    }

    delay(LOOP_DELAY_MS);
  }
  catch (...) {
    _handler->handle("Main.cpp/loop()");
  }
}

void updateUptime() {
  auto current_time = _th->getClockTimeNow();
  int uptime_s = _th->getElapsedTimeS(_uptime_start, current_time);
  _uptimeDisplay->write(_th->prettyFormatS(uptime_s));
}

// handling for any custom commands send through a VirtualTerminal
void handleCustomTerminalCommands(VirtualTerminal* term, String val) {
  bool valid_command = false;
  bool skip_done_print = false;

  if (val == TERM_HELP) {
    valid_command = true;
    term->println("\"" + TERM_CRON + "\" - lists cron info for enabled jobs");
    term->println("\"" + TERM_CLEAR + "\" - clears this terminal display");
    term->println("\"" + TERM_RESET + "\" - performs a hard reset");
    term->println("\"" + TERM_REFRESH + "\" - refreshes data from the database");
  }

  // if (val == TERM_CRON) {
  //   valid_command = true;
  //   term->println("Database data refresh: " + DB_DATA_REFRESH_CRON);
  //   term->println("Blynk data update: " + BLYNK_DATA_UPDATE_CRON);
  // }

  if (val == TERM_CLEAR) {
    valid_command = true;
    skip_done_print = true;
    term->clear();
  }

  if (val == TERM_RESET) {
    valid_command = true;
    skip_done_print = true;
    performHardReset();
  }

  if (val == TERM_REFRESH) {
    valid_command = true;
    skip_done_print = true;
    refreshDatabaseData();
  }

  if (!valid_command) {
    term->println("Command \"" + val + "\" is invalid");
    term->println("Type \"" + TERM_HELP + "\" to list custom commands");
  }
  else {
    if (!skip_done_print) {
      term->println("Done printing info for command \"" + val + "\"");
    }
  }
}

void performHardReset() {
  _terminal->warning("Performing hard reset...");
  _logger->warning("Hard reset via Blynk");

  ESP.restart();
}

// pulls data from the db and updates displays
void refreshDatabaseData() {
  _countTimesOpenedDisplay->write(_data->getSumToday(DOOR_OPENED_DPT));
  _lastTimeOpenedDisplay->write(_data->getLast(LAST_OPENED_TIME_DPT));
  _lastTimeClosedDisplay->write(_data->getLast(LAST_CLOSED_TIME_DPT));

  String refresh_message = "Database data refreshed";
  _logger->info(refresh_message);
  _terminal->println(refresh_message, "[CRON]");
}