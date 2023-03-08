#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include <string>
#include <CronAlarms.h>
#include "src/Config/Config.h"
#include "src/Arduino-Helpers/ArduinoDataClass/ArduinoDataClass.h"
#include "src/Arduino-Helpers/OTAClass/OTAClass.h"
#include "src/Arduino-Helpers/LoggerClass/LoggerClass.h"
#include "src/Arduino-Helpers/TimeHelpersClass/TimeHelpersClass.h"
#include "src/Arduino-Helpers/GpioClass/GpioClass.h"
#include "src/Arduino-Helpers/LedClass/LedClass.h"
#include "src/Arduino-Helpers/WifiClass/WifiClass.h"
#include "src/Arduino-Helpers/GoogleAssistantClass/GoogleAssistantClass.h"
#include "src/Arduino-Helpers/BlynkClasses/BlynkClasses.h"
#include "src/Arduino-Helpers/ExceptionHandlerClass/ExceptionHandlerClass.h"

void setup();
void loop();

// defined in BlynkClasses.cpp
void handleBlynkPinValueChange(int pin, String value);
// defined in ExceptionHandlerClass.cpp
void handleException(String message, String details);

void timerStart();
void timerStop(bool manual = false);
void cycleIfReady();
void updateUptime();
void cycleIfEnabled(bool manual = false);
void performHardReset();

void handleCustomTerminalCommands(VirtualTerminal* term, String val);

void refreshDatabaseData();
void updateBlynkData();

#endif