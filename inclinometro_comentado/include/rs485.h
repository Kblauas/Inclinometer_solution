#ifndef RS485_H_
#define RS485_H_


#include "pins.h"
#include "filters.h"
#include "app.h"
#include <HardwareSerial.h>
#include <Wire.h>
extern HardwareSerial mySerial; //HardwareSerial utilizada para ler e escrever na porta serial, como externa para ser utilizada em mais de um local

void rs485_init(); //inicialização da comunicação rs485
void rs485_send_data(FILTER_MOVING_AVERAGE_PTR filterAvg); //função com o objetivo de enviar os dados
bool rs485_recvCommand(RS485_CONTROL_PTR rs485Control); //função com o objetivo de receber o comando provindo da interface 


#endif