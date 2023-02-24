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

// timers
inline const unsigned long WAIT_TIME_BEFORE_CYCLE_M = 10;

// blynk server settings
inline const String BLYNK_IP = "192.168.1.63";
inline const int BLYNK_PORT = 8442;
inline const String BLYNK_AUTH_TOKEN = "";

// blynk pins
inline const int TIMER_COUNTDOWN_DISPLAY_PIN = 0;
inline const int CYCLE_MANUAL_ENABLE_PIN = 1;
inline const int DOOR_LED_VIRT_PIN = 2;
inline const int CYCLE_IN_PROGRESS_VIRT_PIN = 3;
inline const int INFO_DISPLAY_PIN = 4;

#endif