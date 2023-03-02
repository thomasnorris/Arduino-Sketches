#ifndef CONFIG_H_
#define CONFIG_H_

// general
inline const int BAUD = 9600;
inline const int LOOP_DELAY_MS = 0;
inline const String HOSTNAME = "NameOfThisProject"; // to discover device OTA

// ArduinoData db connection settings
inline const int ARDUINO_DATA_APP_ID = -1; // get from Common.Apps
inline const String ARDUINO_DATA_URL = "";
inline const String ARDUINO_DATA_USERNAME = "";
inline const String ARDUINO_DATA_PASSWORD = "";
inline const int ARDUINO_DATA_PORT = 3306; // default is 3306

// wifi
inline const String WIFI_SSID = "";
inline const String WIFI_PASS = "";
inline const byte WIFI_LED_PIN = D4; // D4 is ESP8266 led, D0 is NodeMCU led
inline const byte WIFI_LED_ON_VALUE = LOW;

// blynk server settings
inline const String BLYNK_IP = "192.168.1.63";
inline const int BLYNK_PORT = 8442;
inline const String BLYNK_AUTH_TOKEN = "";

// logger
inline const String LOGGER_URL = "";
inline const String LOGGER_AUTH_HEADER = "";
inline const String LOGGER_AUTH_TOKEN = "";
inline const int LOGGER_APP_ID = -1; // get from Common.Apps

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

#endif