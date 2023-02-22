#include "Main.h"

void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}
}

void loop() {
  Serial.println("loop");
  delay(LOOP_DELAY_MS);
}