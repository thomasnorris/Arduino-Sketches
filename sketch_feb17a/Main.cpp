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
auto _remoteRelay = new Gpio(REMOTE_RELAY, REMOTE_RELAY_PINMODE);
auto _handler = new ExceptionHandler();

// blynk leds
auto _doorVirtLed = new VirtualLed(DOOR_LED_VPIN);

// blynk buttons
auto _controlEnableVirtBtn = new VirtualPin(CONTROL_ENABLE_VPIN);
auto _manualTriggerVirtBtn = new VirtualPin(MANUAL_TRIGGER_VPIN);

// blynk displays
auto _uptimeDisplay = new VirtualPin(SYSTEM_UPTIME_DISPLAY_VPIN);
auto _lastTimeOpenedDisplay = new VirtualPin(LAST_OPENED_TIME_VPIN);
auto _lastTimeClosedDisplay = new VirtualPin(LAST_CLOSED_TIME_VPIN);
auto _countTimesOpenedDisplay = new VirtualPin(COUNT_TIMES_OPENED_VPIN);

// blynk terminals
auto _terminal = new VirtualTerminal(TERMINAL_VPIN, _th);

// variables
std::chrono::time_point<std::chrono::system_clock> _uptime_start;
bool _last_time_opened_recorded = false;
bool _last_time_closed_recorded = false;

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
    case CONTROL_ENABLE_VPIN:
      _controlEnableVirtBtn->set(val);
      break;
    case MANUAL_TRIGGER_VPIN:
      _manualTriggerVirtBtn->set(val);
      break;
    default:
      break;
  }
}

// handle caught exceptions here
void handleException(String origin, String message, String details) {
  _logger->error(message, origin + ": " + details);

  message = "Error in " + origin + ": " + message;
  _blynk->notify(message, true);
  _terminal->error(message);
}

void setup() {
  try {
    Serial.begin(BAUD);
    while (!Serial) {}

    // init connections
    _wifi->connect();
    _blynk->configure();
    _blynk->connect();
    _blynk->run();
    _blynk->notifySetEnabled(BLYNK_ENABLE_NOTIFICATIONS);
    _ota->begin();
    _th->begin();
    _th->update();

    // init blynk i/o
    _terminal->clear();
    _terminal->init("Initializing...");
    _doorVirtLed->off();
    _controlEnableVirtBtn->on();
    _remoteRelay->off();

    // init physical i/0
    _remoteRelay->off();

    // cron schedules
    Cron.create(const_cast<char*>(DB_DATA_REFRESH_CRON.c_str()), refreshDatabaseData, false);
    Cron.create(const_cast<char*>(BLYNK_DATA_UPDATE_CRON.c_str()), updateBlynkData, false);

    // get data
    refreshDatabaseData();

    // start uptime now
    _uptime_start = _th->getClockTimeNow();

    // respond done
    String init_message = "System initialized";
    String my_ip = _wifi->getIPAddress();
    _logger->init(init_message, _wifi->getIPAddress());
    _terminal->init(init_message);
    _terminal->info("IP: " + my_ip);
    _terminal->info("Type \"" + TERM_HELP + "\" to list custom commands");

    _blynk->notify(init_message + " - Other notifications " + String(_blynk->notifyGetEnabled() ? "enabled" : "disabled"), true);
  }
  catch (...) {
    _handler->handle("Main.cpp/setup()");
  }
}

void loop() {
  try {
    Cron.delay();
    updateUptime();

    _wifi->checkConnection();
    _blynk->checkConnection();
    _blynk->run();
    _ota->handle();
    _th->update();

    auto localDateTime = _th->getCurrentLocalDateTime12hr();

    // manual trigger?
    if (_manualTriggerVirtBtn->isOn()) {
      _manualTriggerVirtBtn->off();

      triggerIfEnabled();
      return;
    }

    // low means the sensor has made a connection
    if (_doorSensor->isLow()) {
      _doorLed->on();
      _doorVirtLed->on();

      _last_time_opened_recorded = false;

      if (!_last_time_closed_recorded) {
        _last_time_closed_recorded = true;

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

      _last_time_closed_recorded = false;

      if (!_last_time_opened_recorded) {
        _last_time_opened_recorded = true;

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

void triggerIfEnabled() {
  String opposite_state = "closing";

  // if we are closed, we will want to be opening it
  if (_doorSensor->isLow()) {
    opposite_state = "opening";
  }

  String message = "";
  if (_controlEnableVirtBtn->isOn()) {
    message = "Door is " + opposite_state + ", please wait...";

    // TODO: perform button click now!
    clickRemote();

    // log
    _terminal->info(message);
    _logger->info(message);
  }
  else {
    message = "Not " + opposite_state + "; disabled";
    _terminal->warning(message);
    _logger->warning(message);
  }
}

void updateUptime() {
  auto current_time = _th->getClockTimeNow();
  int uptime_s = _th->getElapsedTimeS(_uptime_start, current_time);
  _uptimeDisplay->write(_th->prettyFormatS(uptime_s));
}

// handling for any custom commands send through a VirtualTerminal
void handleCustomTerminalCommands(VirtualTerminal* term, String val) {
  if (val == TERM_HELP) {
    term->help("\"" + TERM_CRON + "\" - lists cron info for enabled jobs");
    term->help("\"" + TERM_CLEAR + "\" - clears this terminal display");
    term->help("\"" + TERM_RESET + "\" - performs a hard reset");
    term->help("\"" + TERM_REFRESH + "\" - refreshes data from the database");
    term->emptyln();
    return;
  }

  if (val == TERM_CRON) {
    term->println("Database data refresh: " + DB_DATA_REFRESH_CRON, "-->", false);
    term->println("Blynk data update: " + BLYNK_DATA_UPDATE_CRON, "-->", false);
    term->emptyln();
    return;
  }

  if (val == TERM_CLEAR) {
    term->clear();
    return;
  }

  if (val == TERM_RESET) {
    performHardReset();
    return;
  }

  if (val == TERM_REFRESH) {
    refreshDatabaseData();
    return;
  }

  if (val == TERM_TOGGLE_NOTIFS) {
    _blynk->notifyToggleEnabled();
    String notify_message = "Blynk notifications " + String(_blynk->notifyGetEnabled() ? "enabled" : "disabled");

    term->println(notify_message, "-->", false);
    _blynk->notify(notify_message);
    term->emptyln();
    return;
  }

  term->help("Command \"" + val + "\" is invalid");
  term->help("Type \"" + TERM_HELP + "\" to list custom commands");
  term->emptyln();
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

// sends current data to blynk
void updateBlynkData() {
  _countTimesOpenedDisplay->write(_countTimesOpenedDisplay->read());
  _lastTimeOpenedDisplay->write(_lastTimeOpenedDisplay->read());
  _lastTimeClosedDisplay->write(_lastTimeClosedDisplay->read());

  // don't log as it'll just pollute the db
}

void clickRemote() {
  _remoteRelay->on();
  delay(1000);
  _remoteRelay->off();
}