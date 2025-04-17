#ifndef MPU_H_
#define MPU_H_

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BusIO_Register.h>
#include "app.h"
#include "calib.h"
#include "pins.h"

#define MPU_ADDRESS     0x68
extern Adafruit_MPU6050 mpu;
void mpu_init(void);
void mpu_measurements(SENSOR_CONTROL_PTR sensorControl);

#endif