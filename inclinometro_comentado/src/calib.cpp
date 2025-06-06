#include "calib.h"
#include "mpu.h"

SENSOR_CALIB_T sensorCalib;

//atualizar h3 com as medidas  a cada momento

void calib_accel_init(SENSOR_CALIB_PTR sensorCalib, float h[3], float Ainv[3][3], float b[3]){ //definição da função de calibrar o acelerômetro
    float f[3]; //definição da matriz f
    float acalibrado[3]; //definição da matriz de valores calibrados
    for (int i = 0; i < 3; i++) { 
        f[i] = h[i] - b[i]; //subtração de matrizes 
    }
    for(int i=0; i<3; i++){
        acalibrado[i]=0; 
        for(int j=0;j<3;j++){
            acalibrado[i]+=Ainv[i][j]*f[j]; //multiplicação de matrizes, onde o valor de a calibrado é adquirido por meio da multiplicação da matriz
                                            //Ainversa com f, que é o produto da matriz do valor das acelerações (h) subtraída aos valores de bias (b)
        }
    }
    sensorCalib->acc_x_calib = acalibrado[0];
    sensorCalib->acc_y_calib = acalibrado[1];
    sensorCalib->acc_z_calib = acalibrado[2];
   //alocação dos valores da matriz calibrada aos do vetor sensorCalib
    
}

