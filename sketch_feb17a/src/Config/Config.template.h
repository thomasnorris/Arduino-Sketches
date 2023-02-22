#ifndef CONFIG_H_
#define CONFIG_H_

// general
inline const int BAUD = 9600;
inline const int LOOP_DELAY_MS = 250;

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

#endif