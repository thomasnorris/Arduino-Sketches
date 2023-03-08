#include "Main.h"

// helpers
auto _wifi = new WifiClient(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
auto _ota = new OTAClient(HOSTNAME);
auto _doorSensor = new Gpio(DOOR_PIN, DOOR_PIN_PINMODE);
auto _doorLed = new Led(DOOR_LED_PIN, DOOR_LED_ON_VALUE);
auto _gaClient = new GAClient(GA_SEND_ENDPOINT, GA_AUTH_HEADER, GA_AUTH_TOKEN);
auto _blynk = new BlynkServer(BLYNK_IP, BLYNK_PORT, BLYNK_AUTH_TOKEN);
auto _logger = new LoggerClient(LOGGER_APP_ID, LOGGER_URL, LOGGER_USERNAME, LOGGER_PASSWORD, LOGGER_PORT);
auto _th = new TimeHelpers();
auto _data = new ArduinoDataClient(ARDUINO_DATA_APP_ID, ARDUINO_DATA_URL, ARDUINO_DATA_USERNAME, ARDUINO_DATA_PASSWORD, ARDUINO_DATA_PORT);
auto _handler = new ExceptionHandler();

// blynk leds
auto _doorVirtLed = new VirtualLed(DOOR_LED_VPIN);
auto _cycleInProgressVirtLed = new VirtualLed(CYCLE_IN_PROGRESS_LED_VPIN);
auto _cycleTimingLed = new VirtualLed(CYCLE_TIMING_LED_VPIN);

// blynk buttons
auto _cycleEnableVirtBtn = new VirtualPin(CYCLE_ENABLE_VPIN);
auto _manualCycleVirtBtn = new VirtualPin(MANUAL_CYCLE_VPIN);
auto _cancelCountdownVirtBtn = new VirtualPin(CANCEL_COUNTDOWN_VPIN);

// blynk displays
auto _timerCountdownDisplay = new VirtualPin(TIMER_COUNTDOWN_DISPLAY_VPIN);
auto _cycleCooldownDisplay = new VirtualPin(CYCLE_COOLDOWN_DISPLAY_VPIN);
auto _uptimeDisplay = new VirtualPin(SYSTEM_UPTIME_DISPLAY_VPIN);
auto _lastCycleTimeDisplay = new VirtualPin(LAST_CYCLE_TIME_VPIN);
auto _totalCyclesTodayDisplay = new VirtualPin(TOTAL_CYCLES_TODAY);
auto _totalMissedCyclesTodayDisplay = new VirtualPin(TOTAL_MISSED_CYCLES_TODAY);
auto _totalCyclesThisWeekDisplay = new VirtualPin(TOTAL_CYCLES_THIS_WEEK);

// blynk terminals
auto _terminal = new VirtualTerminal(TERMINAL_VPIN, _th);

// variables
std::chrono::time_point<std::chrono::system_clock> _start_time;
bool _timer_started = false;
bool _timer_allow_reset = false;
bool _door_closed_after_open = false;
std::chrono::time_point<std::chrono::system_clock> _uptime_start;

// handle blynk virtual pin value changes here
void handleBlynkPinValueChange(int pin, String val) {
  switch (pin) {
    case CYCLE_ENABLE_VPIN:
      _cycleEnableVirtBtn->set(val);
      break;
    case TIMER_COUNTDOWN_DISPLAY_VPIN:
      _timerCountdownDisplay->set(val);
      break;
    case TERMINAL_VPIN:
      // custom command handling here
      handleCustomTerminalCommands(_terminal, val);
      break;
    case CYCLE_COOLDOWN_DISPLAY_VPIN:
      _cycleCooldownDisplay->set(val);
      break;
    case SYSTEM_UPTIME_DISPLAY_VPIN:
      _uptimeDisplay->set(val);
      break;
    case MANUAL_CYCLE_VPIN:
      _manualCycleVirtBtn->set(val);
      break;
    case CANCEL_COUNTDOWN_VPIN:
      _cancelCountdownVirtBtn->set(val);
      break;
    case LAST_CYCLE_TIME_VPIN:
      _lastCycleTimeDisplay->set(val);
    case TOTAL_CYCLES_TODAY:
      _totalCyclesTodayDisplay->set(val);
      break;
    case TOTAL_MISSED_CYCLES_TODAY:
      _totalMissedCyclesTodayDisplay->set(val);
      break;
    case TOTAL_CYCLES_THIS_WEEK:
      _totalCyclesThisWeekDisplay->set(val);
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

    // start uptime now
    _uptime_start = _th->getClockTimeNow();

    // init bylnk i/o
    _terminal->clear();
    _terminal->init("Initializing...");
    _doorVirtLed->off();
    _cycleInProgressVirtLed->off();
    _timerCountdownDisplay->write(_th->prettyFormatS(WAIT_TIME_BEFORE_CYCLE_M * 60));
    _cycleCooldownDisplay->write(_th->prettyFormatS(CYCLE_COOLDOWN_DELAY_S));
    _cycleTimingLed->off();
    _manualCycleVirtBtn->off();
    _cancelCountdownVirtBtn->off();

    // cron schedules
    Cron.create(const_cast<char*>(DB_DATA_REFRESH_CRON.c_str()), refreshDatabaseData, false);
    Cron.create(const_cast<char*>(BLYNK_DATA_UPDATE_CRON.c_str()), updateBlynkData, false);

    // get data
    refreshDatabaseData();

    // enable cycling
    _cycleEnableVirtBtn->on();

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

    // manual cycle?
    if (_manualCycleVirtBtn->isOn()) {
      _manualCycleVirtBtn->off();

      // only stop the timer if enabled
      if (_timer_started) {
        timerStop();
      }

      cycleIfEnabled(true);
      return;
    }

    // stop the timer?
    if (_cancelCountdownVirtBtn->isOn()) {
      _cancelCountdownVirtBtn->off();

      // only stop if it's running
      if (_timer_started) {
        _logger->info("Countdown manually cancelled");
        timerStop(true);
      }
      else {
        _terminal->warning("Timer not running");
      }
      return;
    }

    // should we start the timer?
    if (_timer_started) {
      cycleIfReady();
    }

    // low means sensor has made a connection
    if (_doorSensor->isLow()) {
      _doorLed->on();
      _doorVirtLed->on();

      // allow timer to be reset on next open
      _timer_allow_reset = true;
      _door_closed_after_open = true;

      if (!_timer_started) {
        _terminal->info("Ready for cycle");
      }
    }
    else {
      // door has been opened
      _doorLed->off();
      _doorVirtLed->off();

      // make sure the door has been closed once
      if (!_door_closed_after_open) {
        _terminal->warning("Door open after previous cycle");
        return;
      }

      // start the timer once
      // either if it has never been started, or if it can be reset
      if (!_timer_started || _timer_allow_reset) {
        _terminal->info("Door opened");
        timerStart();
      }
    }

    delay(LOOP_DELAY_MS);
  }
  catch (...) {
    _handler->handle("Main.cpp/loop()");
  }
}

// pulls data from the db and updates displays
void refreshDatabaseData() {
  _totalCyclesTodayDisplay->write(_data->getSumToday(CYCLE_COUNT_DPT));
  _totalMissedCyclesTodayDisplay->write(_data->getSumToday(MISSED_CYCLE_COUNT_DPT));
  _lastCycleTimeDisplay->write(_data->getLast(LAST_CYCLE_TIME_DPT));
  _totalCyclesThisWeekDisplay->write(_data->getSumThisWeek(CYCLE_COUNT_DPT));

  String refresh_message = "Database data refreshed";
  _logger->info(refresh_message);
  _terminal->println(refresh_message, "[CRON]");
}

// sends current data to blynk (mainly so superchart looks correct)
void updateBlynkData() {
  _totalCyclesTodayDisplay->write(_totalCyclesTodayDisplay->read());
  _totalMissedCyclesTodayDisplay->write(_totalMissedCyclesTodayDisplay->read());
  _lastCycleTimeDisplay->write(_lastCycleTimeDisplay->read());
  _totalCyclesThisWeekDisplay->write(_totalCyclesThisWeekDisplay->read());

  // don't log as it'll just pollute the db
}

void performCycleCooldown() {
  auto delay_s = CYCLE_COOLDOWN_DELAY_S;

  _cycleInProgressVirtLed->on();
  _cycleCooldownDisplay->write(_th->prettyFormatS(delay_s));

  // count down and update display
  while (delay_s != 0) {
    _cycleCooldownDisplay->write(_th->prettyFormatS(--delay_s));

    if (delay_s == CYCLE_COOLDOWN_DELAY_S - 2) {
      _terminal->info("Cycle in cooldown");
    }

    delay(1000);
  }

  // reset
  _cycleCooldownDisplay->write(_th->prettyFormatS(CYCLE_COOLDOWN_DELAY_S));
  _cycleInProgressVirtLed->off();
  _manualCycleVirtBtn->off();
}

void timerStart() {
  String message = "Timer started";
  _cycleTimingLed->on();
  _terminal->info(message);
  _logger->info(message);
  _blynk->notify(message);

  _timer_started = true;
  _timer_allow_reset = false;
  _start_time = _th->getClockTimeNow();
}

void timerStop(bool manual) {
  String message = "Timer stopped";

  if (manual) {
    message += " manually";
  }

  _cycleTimingLed->off();
  _terminal->info(message);
  _logger->info(message);

  _timer_started = false;
  _timer_allow_reset = false;
  _door_closed_after_open = false;

  _timerCountdownDisplay->write(_th->prettyFormatS(WAIT_TIME_BEFORE_CYCLE_M * 60));
}

void cycleIfReady() {
  auto current_time = _th->getClockTimeNow();
  int elapsed_time_s = _th->getElapsedTimeS(_start_time, current_time);
  int wait_time_s = WAIT_TIME_BEFORE_CYCLE_M * 60;

  if (elapsed_time_s >= wait_time_s) {
    _timerCountdownDisplay->write(_th->prettyFormatS(0));

    // stop timing
    timerStop();

    cycleIfEnabled();
  }
  else {
    int time_remaining_s = wait_time_s - elapsed_time_s;
    _timerCountdownDisplay->write(_th->prettyFormatS(time_remaining_s));
    _doorLed->toggleOnOff(500);
  }
}

void cycleIfEnabled(bool manual) {
  // update display
  auto localDateTime = _th->getCurrentLocalDateTime12hr();
  _lastCycleTimeDisplay->write(localDateTime);
  _data->updateDataPoint(LAST_CYCLE_TIME_DPT, localDateTime);

  String message;
  if (_cycleEnableVirtBtn->isOn()) {
    _terminal->info("Cycling...");
    _gaClient->send(CYCLE_COMMAND);

    message = "Cycle command sent";
    if (manual) {
      message = "Cycle command manually initiated";
    }

    _logger->info(message);

    if (!manual) {
      _blynk->notify(message);
      _terminal->info(message);
    }
    else {
      _terminal->println(message, "[MANUAL]");
    }

    _data->insertDataPoint(CYCLE_COUNT_DPT, 1);
    _totalCyclesTodayDisplay->write(_data->getSumToday(CYCLE_COUNT_DPT));
    _totalCyclesThisWeekDisplay->write(_data->getSumThisWeek(CYCLE_COUNT_DPT));

    performCycleCooldown();
  }
  else {
    message = "Not cycling; disabled";
    _terminal->warning(message);
    _logger->warning(message);

    _data->insertDataPoint(MISSED_CYCLE_COUNT_DPT, 1);
    _totalMissedCyclesTodayDisplay->write(_data->getSumToday(MISSED_CYCLE_COUNT_DPT));
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
    term->help("\"" + TERM_CRON + "\"    - lists cron info for enabled jobs");
    term->help("\"" + TERM_CLEAR + "\"   - clears this terminal display");
    term->help("\"" + TERM_RESET + "\"   - performs a hard reset");
    term->help("\"" + TERM_REFRESH + "\" - refreshes data from the database");
    term->help("\"" + TERM_TOGGLE_NOTIFS + "\"  - toggles Blynk push notifications on or off");
    term->emptyln();
    return;
  }

  if (val == TERM_CRON) {
    term->println("Database data refresh: " + DB_DATA_REFRESH_CRON, "-->", false);
    term->println("Blynk data update:     " + BLYNK_DATA_UPDATE_CRON, "-->", false);
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

    _terminal->println(notify_message, "-->", false);
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