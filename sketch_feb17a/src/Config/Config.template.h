#ifndef CONFIG_H_
#define CONFIG_H_

// general
inline const int BAUD = 9600;
inline const int LOOP_DELAY_MS = 0;
inline const String HOSTNAME = "LitterBoxCycler";

// wifi
inline const String WIFI_SSID = "";
inline const String WIFI_PASS = "";
inline const byte WIFI_LED_PIN = D4; // D4 is ESP8266 led, D0 is NodeMCU led
inline const byte WIFI_LED_ON_VALUE = LOW;

// door sensor
inline const byte DOOR_PIN = D1; // gpio 5
inline const byte DOOR_PIN_PINMODE = INPUT_PULLUP; // default floating to be HIGH

// door led
inline const byte DOOR_LED_PIN = D0;
inline const byte DOOR_LED_ON_VALUE = LOW;

// google assistant
inline const String GA_URL = "";
inline const String GA_AUTH_HEADER = "";
inline const String GA_AUTH_TOKEN = "";
inline const String CYCLE_COMMAND = "Activate scene Ellie pooped";

// timers
inline const unsigned long WAIT_TIME_BEFORE_CYCLE_M = 10;
inline const unsigned long CYCLE_COOLDOWN_DELAY_S = 180;

// blynk server settings
inline const String BLYNK_IP = "192.168.1.63";
inline const int BLYNK_PORT = 8442;
inline const String BLYNK_AUTH_TOKEN = "";

// blynk pins
inline const int TIMER_COUNTDOWN_DISPLAY_VPIN = 0;
inline const int CYCLE_ENABLE_VPIN = 1;
inline const int DOOR_LED_VPIN = 2;
inline const int CYCLE_IN_PROGRESS_LED_VPIN = 3;
inline const int INFO_DISPLAY_VPIN = 4;
inline const int CYCLE_COUNT_DISPLAY_VPIN = 5;
inline const int MISSED_CYCLE_COUNT_DISPLAY_VPIN = 6;
inline const int CYCLE_COOLDOWN_DISPLAY_VPIN = 7;
inline const int CYCLE_TIMING_LED_VPIN = 8;
inline const int SYSTEM_UPTIME_DISPLAY_VPIN = 9;
inline const int MANUAL_CYCLE_VPIN = 10;

// logger
inline const String LOGGER_URL = "";
inline const String LOGGER_AUTH_HEADER = "";
inline const String LOGGER_AUTH_TOKEN = "";
inline const int LOGGER_APP_ID = 11;

#endif