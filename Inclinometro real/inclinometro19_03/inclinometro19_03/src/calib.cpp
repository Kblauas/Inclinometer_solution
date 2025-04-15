#include "calib.h"
#include "mpu.h"

SENSOR_CALIB_T sensorCalib;

//atualizar h3 com as medidas  a cada momento

void calib_accel_init(SENSOR_CALIB_PTR sensorCalib, float h[3], float Ainv[3][3], float b[3]){
    float f[3];
    float acalibrado[3];
    for (int i = 0; i < 3; i++) {
        f[i] = h[i] - b[i];
    }
    for(int i=0; i<3; i++){
        acalibrado[i]=0;
        for(int j=0;j<3;j++){
            acalibrado[i]+=Ainv[i][j]*f[j];
        }
    }
    sensorCalib->acc_x_calib = acalibrado[0];
    sensorCalib->acc_y_calib = acalibrado[1];
    sensorCalib->acc_z_calib = acalibrado[2];
   
    
}

