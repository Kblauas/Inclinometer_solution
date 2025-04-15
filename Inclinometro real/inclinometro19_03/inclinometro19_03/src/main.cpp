#include <Arduino.h>
#include "main.h"


void setup() {
  pins_init();
  adxl_init();
  mpu_init();
  app_init();
  rs485_init();
  Serial.begin(115200);
  
}

void loop() {
  app_poll();
}
