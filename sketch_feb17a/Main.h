#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include <string>
#include "src/Config/Config.h"
#include "src/Arduino-Helpers/OTAClass/ArduinoOTA.h"
#include "src/Arduino-Helpers/LoggerClass/LoggerClass.h"
#include "src/Arduino-Helpers/TimeHelpers/TimeHelpers.h"
#include "src/Arduino-Helpers/GpioClass/Gpio.h"
#include "src/Arduino-Helpers/LedClass/Led.h"
#include "src/Arduino-Helpers/WifiClass/Wifi.h"
#include "src/Arduino-Helpers/GoogleAssistantClass/GoogleAssistant.h"
#include "src/Arduino-Helpers/BlynkClasses/BlynkClasses.h"

extern std::chrono::time_point<std::chrono::system_clock> getTimeNow();
extern double getElapsedTimeS(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end);
extern String formatSeconds(int seconds);

void setup();
void loop();

void handleBlynkPinValueChange(int pin, String value);

void timerStart();
void timerStop();
void cycleIfReady();
void updateUptime();
void cycleIfEnabled(bool manual = false); 

#endif