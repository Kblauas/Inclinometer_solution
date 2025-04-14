#ifndef CALIB_H_
#define CALIB_H_
#include <Adafruit_MPU6050.h>


typedef struct{
    float acc_x_calib;
    float acc_y_calib;
    float acc_z_calib;
    float gyro_x_offset;
    float gyro_y_offset;
    float gyro_z_offset;
} SENSOR_CALIB_T, *SENSOR_CALIB_PTR;
extern SENSOR_CALIB_T sensorCalib;

void calib_accel_init(SENSOR_CALIB_PTR sensorCalib, float h[3], float Ainv[3][3], float b[3]);

#endif