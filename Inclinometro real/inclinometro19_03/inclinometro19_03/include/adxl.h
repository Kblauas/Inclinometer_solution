#ifndef ADXL_H_
#define ADXL_H_


#include "calib.h"
#include "app.h"
#include <PL_ADXL355.h>
#define ADXL_ADDRESS     0x53

extern PL::ADXL355 adxl355;




void adxl_init(void);
void adxl_measurements(SENSOR_CONTROL_PTR sensorControl);

#endif