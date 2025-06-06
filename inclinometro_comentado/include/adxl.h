#ifndef ADXL_H_
#define ADXL_H_


#include "calib.h" //importação da biblioteca de calibração
#include "app.h"    // importação da biblioteca que possui o vetor sensorControl
#include <PL_ADXL355.h> // biblioteca "third party" importada
#define ADXL_ADDRESS     0x53 //endereço utilizado, pelo pino VSSUPPLY estar em alta

extern PL::ADXL355 adxl355; //definição da biblioteca




void adxl_init(void); //funcão de inicialização
void adxl_measurements(SENSOR_CONTROL_PTR sensorControl);  //função de medições

#endif