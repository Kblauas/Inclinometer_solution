#include "adxl.h"



PL::ADXL355 adxl355;



void adxl_init(void){
auto range = PL::ADXL355_Range::range2g; //set range of operations
    adxl355.beginI2C(ADXL_ADDRESS);
    auto deviceInfo = adxl355.getDeviceInfo();
    if (deviceInfo.deviceId != 0xED) {
        Serial.println("Could not find ADXL355!");
    while (1);
  }
    //Serial.println("ADXL355 initialized successfully!");  

  // Set ADXL355 range
  adxl355.setRange(range);  
  // Enable ADXL355 measurement
  adxl355.enableMeasurement();
  
}

void adxl_measurements(SENSOR_CONTROL_PTR sensorControl){
  auto accelerations = adxl355.getAccelerations();
  float h[3] = {accelerations.x, accelerations.y, accelerations.z};
  float Ainv [3] [3] = {{0.983329,0.003401,-0.000053},{0.003401,0.976592,0.002209},{-0.000053,0.002209,0.958527}};
  float b [3] = {-0.009316, 0.007176 , -0.04096};
  

  calib_accel_init(&sensorCalib, h, Ainv, b);
  sensorControl->adxl_angl_x = atan2f(sensorCalib.acc_x_calib,(sqrt((sensorCalib.acc_y_calib*sensorCalib.acc_y_calib)+(sensorCalib.acc_z_calib*sensorCalib.acc_z_calib))))*(RAD_TO_DEG);
  sensorControl->adxl_angl_y = atan2f(sensorCalib.acc_y_calib,(sqrt((sensorCalib.acc_x_calib*sensorCalib.acc_x_calib)+(sensorCalib.acc_z_calib*sensorCalib.acc_z_calib))))*(RAD_TO_DEG);
  sensorControl->adxl_angl_z = atan2f(sensorCalib.acc_z_calib, (sqrt((sensorCalib.acc_x_calib*sensorCalib.acc_x_calib)+(sensorCalib.acc_y_calib*sensorCalib.acc_y_calib))))*(RAD_TO_DEG);;

  //Serial.println(sensorCalib.acc_x_calib);
  //Serial.println(sensorCalib.acc_y_calib);
  //Serial.println(sensorCalib.acc_z_calib);
  //Serial.println(sensorControl->adxl_angl_x);
  //Serial.println(sensorControl->adxl_angl_y);
  //Serial.println(sensorControl->adxl_angl_z);
}