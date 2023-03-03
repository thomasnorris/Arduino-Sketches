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
inline const int BLYNK_PORT = 8442; // default is 8442
inline const String BLYNK_AUTH_TOKEN = "";

// AppLogs db connection settings
inline const int LOGGER_APP_ID = -1; // get from Common.Apps
inline const String LOGGER_URL = "";
inline const String LOGGER_USERNAME = "";
inline const String LOGGER_PASSWORD = "";
inline const int LOGGER_PORT = 3306; // default is 3306

#endif