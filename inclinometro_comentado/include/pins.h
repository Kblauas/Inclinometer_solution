#ifndef PINS_H_
#define PINS_H_

#include <Wire.h>

#define SCL     37 //definição do pino SCL para comunicação IIC
#define SDA     21 //definição do pino SDA para a comunicação IIC
#define RX      5 // definição do pino RX, para monitoramento da comunicação
#define TX      4 //definição do pino TX, para monitoramento da comunicação
#define RE      3 //definição do pino de recebimento do RS485
#define ADO     38 //definição do pino AD0 do MPU
void pins_init(void);  //inicialização de todos os pinos

#endif