#include "Main.h"

Wifi* _wifi = new Wifi(WIFI_SSID, WIFI_PASS, WIFI_LED_PIN, WIFI_LED_ON_VALUE);
Gpio* _doorSensor = new Gpio(DOOR_PIN, DOOR_PIN_PINMODE);
Led* _doorLed = new Led(DOOR_LED_PIN, DOOR_LED_ON_VALUE);
GAClient* _gaClient = new GAClient(GA_URL, GA_AUTH_HEADER, GA_AUTH_TOKEN);

unsigned long start_time = 0;
bool timer_started = false;
bool timer_allow_reset = false;

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}

  _wifi->connect();
}

void loop() {
  _wifi->checkConnection();

  if (timer_started) {
    cycleIfReady();
  }

  // low means sensor has made a connection
  if (_doorSensor->isLow()) {
    _doorLed->on();

    // allow timer to be reset on next open
    timer_allow_reset = true;
  }
  else {
    // door has been opened
    _doorLed->off();

    // start the timer once
    // either if it has never been started, or if it can be reset
    if (!timer_started || timer_allow_reset) {
      timerStart();
    }
  }

  delay(LOOP_DELAY_MS);
}

void timerStart() {
  Serial.println("Starting the timer.");
  timer_started = true;
  timer_allow_reset = false;
  start_time = millis();
}

void timerStop() {
  timer_started = false;
  timer_allow_reset = false;
  start_time = 0;
}

void cycleIfReady() {
  unsigned long current_time = millis();
  unsigned long wait_time = WAIT_TIME_BEFORE_CYCLE_M * 60 * 1000;
  
  if (current_time >= start_time + wait_time) {
    Serial.println("Cycle time!");
    _gaClient->send(CYCLE_COMMAND);
    timerStop();
  }
  else {
    Serial.print("Remaining time: ");
    Serial.print((start_time + wait_time - current_time));
    Serial.println(" milliseconds");
  }
}