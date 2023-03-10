#ifndef CONFIG_H_
#define CONFIG_H_

// general
inline const int BAUD = 9600;
inline const int LOOP_DELAY_MS = 0;
inline const String HOSTNAME = "LitterBoxCycler";

// cron schedules (see https://crontab.cronhub.io/), 6 fields format
inline const String DB_DATA_REFRESH_CRON = "0 0 0 * * *";   // every day at midnight
inline const String BLYNK_DATA_UPDATE_CRON = "0 */1 * * * *"; // every 1 minute

// ArduinoData db connection settings
inline const int ARDUINO_DATA_APP_ID = 11;
inline const String ARDUINO_DATA_URL = "";
inline const String ARDUINO_DATA_USERNAME = "";
inline const String ARDUINO_DATA_PASSWORD = "";
inline const int ARDUINO_DATA_PORT = 3360;

// ArduinoData.DataPointTypes (DPT) for corresponding ARDUINO_DATA_APP_ID (above)
inline const int CYCLE_COUNT_DPT = 1;
inline const int MISSED_CYCLE_COUNT_DPT = 2;
inline const int LAST_CYCLE_TIME_DPT = 3;

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

// google assistant (no https)
inline const String GA_SEND_ENDPOINT = "http://192.168.1.62:1000/send";
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

// blynk notifications
inline const bool BLYNK_ENABLE_NOTIFICATIONS = false;

// blynk pins
inline const int TIMER_COUNTDOWN_DISPLAY_VPIN = 0;
inline const int CYCLE_ENABLE_VPIN = 1;
inline const int DOOR_LED_VPIN = 2;
inline const int CYCLE_IN_PROGRESS_LED_VPIN = 3;
inline const int TERMINAL_VPIN = 4;
inline const int CYCLE_COOLDOWN_DISPLAY_VPIN = 7;
inline const int CYCLE_TIMING_LED_VPIN = 8;
inline const int SYSTEM_UPTIME_DISPLAY_VPIN = 9;
inline const int MANUAL_CYCLE_VPIN = 10;
inline const int CANCEL_COUNTDOWN_VPIN = 11;
inline const int LAST_CYCLE_TIME_VPIN = 12;
inline const int TOTAL_CYCLES_TODAY = 14;
inline const int TOTAL_MISSED_CYCLES_TODAY = 15;
inline const int TOTAL_CYCLES_THIS_WEEK = 5;

// blynk terminal commands
inline const String TERM_HELP = "?";
inline const String TERM_CRON = "cron";
inline const String TERM_CLEAR = "clear";
inline const String TERM_RESET = "reset";
inline const String TERM_REFRESH = "refresh";
inline const String TERM_TOGGLE_NOTIFS = "notifs";

// AppLogs db connection settings
inline const int LOGGER_APP_ID = -1;
inline const String LOGGER_URL = "";
inline const String LOGGER_USERNAME = "";
inline const String LOGGER_PASSWORD = "";
inline const int LOGGER_PORT = 3360;

#endif