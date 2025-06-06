#include <Arduino.h>
#include "main.h"


void setup() {
  pins_init();
  adxl_init();
  mpu_init();
  app_init();
  rs485_init();
  Serial.begin(115200);
  //setup do código com o valor da baud_rate do monitor serial desejada, que é de seleção do usuário
  
}

void loop() {
  app_poll(); //loop roda a poll
}
