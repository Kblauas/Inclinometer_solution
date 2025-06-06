#ifndef CALIB_H_
#define CALIB_H_
#include <Adafruit_MPU6050.h>


typedef struct{
    float acc_x_calib;
    float acc_y_calib;
    float acc_z_calib;
} SENSOR_CALIB_T, *SENSOR_CALIB_PTR; // struct que define os parâmetros após serem calibrados
extern SENSOR_CALIB_T sensorCalib; //está em extern por ser utilizado em mais de uma "aba", e precisar manter o seu valor

void calib_accel_init(SENSOR_CALIB_PTR sensorCalib, float h[3], float Ainv[3][3], float b[3]); //função que define a estrutura de calibração utilizada

#endif