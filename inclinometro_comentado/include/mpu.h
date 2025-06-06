#ifndef MPU_H_
#define MPU_H_

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BusIO_Register.h>
#include "app.h"
#include "calib.h"
#include "pins.h"

#define MPU_ADDRESS     0x68 //endereço definido para o MPU, pelo pino AD0 estar em baixa
extern Adafruit_MPU6050 mpu; //definindo a biblioteca como externa, para poder utilizá-la sem redeclarar
void mpu_init(void); //inicialização do MPU
void mpu_measurements(SENSOR_CONTROL_PTR sensorControl); //medições retiradas do MPU

#endif