#include "Main.h"

// helpers
auto _wifi = new Wifi(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
auto _doorSensor = new Gpio(DOOR_PIN, DOOR_PIN_PINMODE);
auto _doorLed = new Led(DOOR_LED_PIN, DOOR_LED_ON_VALUE);
auto _gaClient = new GAClient(GA_URL, GA_AUTH_HEADER, GA_AUTH_TOKEN);
auto _blynk = new BlynkServer(BLYNK_IP, BLYNK_PORT, BLYNK_AUTH_TOKEN);
auto _logger = new LoggerClient(LOGGER_URL, LOGGER_AUTH_HEADER, LOGGER_AUTH_TOKEN, LOGGER_APP_ID);

// leds
auto _doorVirtLed = new VirtualLed(DOOR_LED_VPIN);
auto _cycleInProgressVirtLed = new VirtualLed(CYCLE_IN_PROGRESS_LED_VPIN);
auto _cycleTimingLed = new VirtualLed(CYCLE_TIMING_LED_VPIN);

// buttons
auto _cycleEnableVirtBtn = new VirtualPin(CYCLE_ENABLE_VPIN);

// displays
auto _timerCountdownDisplay = new VirtualPin(TIMER_COUNTDOWN_DISPLAY_VPIN);
auto _infoDisplay = new VirtualPin(INFO_DISPLAY_VPIN);
auto _cycleCountDisplay = new VirtualPin(CYCLE_COUNT_DISPLAY_VPIN);
auto _missedCycleCountDisplay = new VirtualPin(MISSED_CYCLE_COUNT_DISPLAY_VPIN);
auto _cycleCooldownDisplay = new VirtualPin(CYCLE_COOLDOWN_DISPLAY_VPIN);
auto _uptimeDisplay = new VirtualPin(SYSTEM_UPTIME_DISPLAY_VPIN);

// variables
std::chrono::time_point<std::chrono::system_clock> _start_time;
bool _timer_started = false;
bool _timer_allow_reset = false;
bool _door_closed_after_cycle = false;
int _cycle_count = 0;
int _missed_cycle_count = 0;
std::chrono::time_point<std::chrono::system_clock> _uptime_start;

// handle blynk virtual pin value changes here
void handleBlynkPinValueChange(int pin, String val) {
  switch (pin) {
    case CYCLE_ENABLE_VPIN:
      _cycleEnableVirtBtn->set(val);
      break;
    case TIMER_COUNTDOWN_DISPLAY_VPIN:
      _timerCountdownDisplay->set(val);
    case INFO_DISPLAY_VPIN:
      _infoDisplay->set(val);
      break;
    case CYCLE_COUNT_DISPLAY_VPIN:
      _cycleCountDisplay->set(val);
      break;
    case MISSED_CYCLE_COUNT_DISPLAY_VPIN:
      _missedCycleCountDisplay->set(val);
      break;
    case CYCLE_COOLDOWN_DISPLAY_VPIN:
      _cycleCooldownDisplay->set(val);
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}

  _uptime_start = getTimeNow();

  // init connections
  _wifi->connect();
  _blynk->configure();
  _blynk->connect();
  _blynk->run();

  // init bylnk i/o 
  _doorVirtLed->off();
  _cycleInProgressVirtLed->off();
  _cycleEnableVirtBtn->off();
  _timerCountdownDisplay->write(formatSeconds(WAIT_TIME_BEFORE_CYCLE_M * 60));
  _cycleCountDisplay->write(_cycle_count);
  _missedCycleCountDisplay->write(_missed_cycle_count);
  _cycleCooldownDisplay->write(formatSeconds(CYCLE_COOLDOWN_DELAY_S));
  _cycleTimingLed->off();

  String init_message = "System initialized";
  _infoDisplay->write(init_message);
  _logger->init(init_message);
}

void loop() {
  updateUptime();
  
  _wifi->checkConnection();
  _blynk->checkConnection();
  _blynk->run();

  if (_timer_started) {
    cycleIfReady();
  }

  // low means sensor has made a connection
  if (_doorSensor->isLow()) {
    _doorLed->on();
    _doorVirtLed->on();

    // allow timer to be reset on next open
    _timer_allow_reset = true;
    _door_closed_after_cycle = true;

    if (!_timer_started) {
      _infoDisplay->write("Ready for cycle");      
    }
  }
  else {
    // door has been opened
    _doorLed->off();
    _doorVirtLed->off();

    // make sure the door has been closed once
    if (!_door_closed_after_cycle) {
      _infoDisplay->write("Door open after previous cycle");
      return;
    }

    // start the timer once
    // either if it has never been started, or if it can be reset
    if (!_timer_started || _timer_allow_reset) {
      timerStart();
    }
  }

  delay(LOOP_DELAY_MS);
}

void sendCycleCommand() {
  _gaClient->send(CYCLE_COMMAND);
}

void performCycleCooldown() {
  auto delay_s = CYCLE_COOLDOWN_DELAY_S;

  _cycleInProgressVirtLed->on();
  _cycleCooldownDisplay->write(formatSeconds(delay_s));

  // count down and update display
  while (delay_s != 0) {
    _cycleCooldownDisplay->write(formatSeconds(--delay_s));
    
    if (delay_s == CYCLE_COOLDOWN_DELAY_S - 2) {
      _infoDisplay->write("Cycle in cooldown");
    }

    delay(1000);
  }

  // reset
  _cycleCooldownDisplay->write(formatSeconds(delay_s));
  _cycleInProgressVirtLed->off();
}

void timerStart() {
  String message = "Timer started";
  _cycleTimingLed->on();
  _infoDisplay->write(message);
  _logger->info(message);

  _timer_started = true;
  _timer_allow_reset = false;
  _start_time = getTimeNow();
}

void timerStop() {
  String message = "Timer stopped";
  _cycleTimingLed->off();
  _infoDisplay->write(message);
  _logger->info(message);

  _timer_started = false;
  _timer_allow_reset = false;
  _door_closed_after_cycle = false;
}

void cycleIfReady() {
  auto current_time = getTimeNow();
  int elapsed_time_s = getElapsedTimeS(_start_time, current_time);
  int wait_time_s = WAIT_TIME_BEFORE_CYCLE_M * 60;

  if (elapsed_time_s >= wait_time_s) {
    _timerCountdownDisplay->write(0);

    // stop timing
    timerStop();

    String message;
    if (_cycleEnableVirtBtn->isOn()) {
      sendCycleCommand();

      message = "Cycle command sent";
      _infoDisplay->write(message);
      _logger->info(message);

      _cycleCountDisplay->write(++_cycle_count);
      _logger->info("Cycles since last reboot", String(_cycle_count));

      performCycleCooldown();
    }
    else {
      message = "Not cycling; disabled";
      _infoDisplay->write(message);
      _logger->info(message);

      _missedCycleCountDisplay->write(++_missed_cycle_count);
      _logger->info("Missed cycles since last reboot", String(_missed_cycle_count));

      delay(2000);
    }
  }
  else {
    _infoDisplay->write("Timer running");
    int time_remaining_s = wait_time_s - elapsed_time_s;
    _timerCountdownDisplay->write(formatSeconds(time_remaining_s));
  }
}

void updateUptime() {
  auto current_time = getTimeNow();
  int uptime_s = getElapsedTimeS(_uptime_start, current_time);
  _uptimeDisplay->write(formatSeconds(uptime_s));
}