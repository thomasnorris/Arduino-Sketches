#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include <string>
#include "src/Config/Config.h"
#include "src/Arduino-Helpers/ArduinoDataClass/ArduinoDataClass.h"
#include "src/Arduino-Helpers/OTAClass/OTAClass.h"
#include "src/Arduino-Helpers/LoggerClass/LoggerClass.h"
#include "src/Arduino-Helpers/TimeHelpersClass/TimeHelpersClass.h"
#include "src/Arduino-Helpers/WifiClass/WifiClass.h"
#include "src/Arduino-Helpers/BlynkClasses/BlynkClasses.h"

void setup();
void loop();

void handleBlynkPinValueChange(int pin, String value);

#endif