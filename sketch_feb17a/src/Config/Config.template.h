#ifndef CONFIG_H_
#define CONFIG_H_

// general
inline const int BAUD = 9600;
inline const int LOOP_DELAY_MS = 0;
inline const String HOSTNAME = "NameOfThisProject"; // to discover device OTA

// cron schedules (see https://crontab.cronhub.io/), 6 fields format
inline const String DB_DATA_REFRESH_CRON = "0 0 0 * * *";   // every day at midnight
inline const String BLYNK_DATA_UPDATE_CRON = "0 */15 * * * *"; // every 15 minutes

// ArduinoData db connection settings
inline const int ARDUINO_DATA_APP_ID = -1; // get from Common.Apps
inline const String ARDUINO_DATA_URL = "";
inline const String ARDUINO_DATA_USERNAME = "";
inline const String ARDUINO_DATA_PASSWORD = "";
inline const int ARDUINO_DATA_PORT = 3306; // default is 3306

// ArduinoData.DataPointTypes (DPT) for corresponding ARDUINO_DATA_APP_ID (above)
inline const int DOOR_OPENED_DPT = 4;
inline const int DOOR_CLOSED_DPT = 5;
inline const int LAST_OPENED_TIME_DPT = 6;
inline const int LAST_CLOSED_TIME_DPT = 7;

// wifi
inline const String WIFI_SSID = "";
inline const String WIFI_PASS = "";
inline const byte WIFI_LED_PIN = D4; // D4 is ESP8266 led, D0 is NodeMCU led
inline const byte WIFI_LED_ON_VALUE = LOW;

// blynk server settings
inline const String BLYNK_IP = "192.168.1.63";
inline const int BLYNK_PORT = 8442;
inline const String BLYNK_AUTH_TOKEN = "";

// blynk terminal commands
inline const String TERM_HELP = "?";
inline const String TERM_CRON = "cron";
inline const String TERM_CLEAR = "clear";
inline const String TERM_RESET = "reset";
inline const String TERM_REFRESH = "refresh";

// AppLogs db connection settings
inline const int LOGGER_APP_ID = -1; // get from Common.Apps
inline const String LOGGER_URL = "";
inline const String LOGGER_USERNAME = "";
inline const String LOGGER_PASSWORD = "";
inline const int LOGGER_PORT = 3306; // default is 3306

// door sensor
inline const byte DOOR_PIN = D1; // gpio 5
inline const byte DOOR_PIN_PINMODE = INPUT_PULLUP; // default floating to be HIGH

// door led
inline const byte DOOR_LED_PIN = D0;
inline const byte DOOR_LED_ON_VALUE = LOW;

// blynk pins
inline const int DOOR_LED_VPIN = 0;
inline const int SYSTEM_UPTIME_DISPLAY_VPIN = 1;
inline const int LAST_OPENED_TIME_VPIN = 2;
inline const int LAST_CLOSED_TIME_VPIN = 3;
inline const int COUNT_TIMES_OPENED_VPIN = 4; // no count closed vpin
inline const int TERMINAL_VPIN = 5;
inline const int CONTROL_ENABLE_VPIN = 6;
inline const int MANUAL_TRIGGER_VPIN = 7;

#endif