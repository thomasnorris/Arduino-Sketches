#include "Main.h"

// helpers
auto _wifi = new Wifi(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
auto _doorSensor = new Gpio(DOOR_PIN, DOOR_PIN_PINMODE);
auto _doorLed = new Led(DOOR_LED_PIN, DOOR_LED_ON_VALUE);
auto _gaClient = new GAClient(GA_URL, GA_AUTH_HEADER, GA_AUTH_TOKEN);
auto _blynk = new BlynkServer(BLYNK_IP, BLYNK_PORT, BLYNK_AUTH_TOKEN);

// leds
auto _doorVirtLed = new VirtualLed(DOOR_LED_VIRT_PIN);
auto _cycleInProgressVirtLed = new VirtualLed(CYCLE_IN_PROGRESS_VIRT_PIN);

// buttons
auto _cycleEnableVirtBtn = new VirtualPin(CYCLE_MANUAL_ENABLE_PIN);

// displays
auto _timerCountdownDisplay = new VirtualPin(TIMER_COUNTDOWN_DISPLAY_PIN);
auto _infoDisplay = new VirtualPin(INFO_DISPLAY_PIN);

// variables
std::chrono::time_point<std::chrono::system_clock> _start_time;
bool _timer_started = false;
bool _timer_allow_reset = false;

// handle blynk virtual pin value changes here
void handleBlynkPinValueChange(int pin, String val) {
  switch (pin) {
    case CYCLE_MANUAL_ENABLE_PIN:
      _cycleEnableVirtBtn->set(val);
      break;
    case TIMER_COUNTDOWN_DISPLAY_PIN:
      _timerCountdownDisplay->set(val);
    default:
      break;
  }
}

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}

  // init connections
  _wifi->connect();
  _blynk->configure();
  _blynk->connect();
  _blynk->run();

  // init bylnk i/o 
  _doorVirtLed->off();
  _cycleInProgressVirtLed->off();
  _cycleEnableVirtBtn->off();
  _timerCountdownDisplay->write(WAIT_TIME_BEFORE_CYCLE_M * 60);
  _infoDisplay->write("System initialized");
}

void loop() {
  _wifi->checkConnection();
  _blynk->checkConnection();

  if (_timer_started) {
    cycleIfReady();
  }

  // low means sensor has made a connection
  if (_doorSensor->isLow()) {
    _doorLed->on();
    _doorVirtLed->on();

    // allow timer to be reset on next open
    _timer_allow_reset = true;
  }
  else {
    // door has been opened
    _doorLed->off();
    _doorVirtLed->off();

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

void delayAfterCycle() {
  delay(AFTER_CYCLE_DELAY_S * 1000);
}

void timerStart() {
  _infoDisplay->write("Timer started");
  _timer_started = true;
  _timer_allow_reset = false;
  _start_time = getTimeNow();
}

void timerStop() {
  _infoDisplay->write("Timer stopped");
  _timer_started = false;
  _timer_allow_reset = false;
}

void cycleIfReady() {
  auto current_time = getTimeNow();
  int elapsed_time_s = getElapsedTimeS(_start_time, current_time);
  int wait_time_s = WAIT_TIME_BEFORE_CYCLE_M * 60;

  if (elapsed_time_s >= wait_time_s) {
    _timerCountdownDisplay->write(0);

    if (_cycleEnableVirtBtn->isOn()) {
      _cycleInProgressVirtLed->on();
      sendCycleCommand();
      _infoDisplay->write("Cycle command sent");
      delayAfterCycle();
      _cycleInProgressVirtLed->off();
    }
    else {
      _infoDisplay->write("Cycling not enabled");
    }

    timerStop();
  } else {
    int time_remaining_s = wait_time_s - elapsed_time_s;
    _timerCountdownDisplay->write(time_remaining_s);
  }
}