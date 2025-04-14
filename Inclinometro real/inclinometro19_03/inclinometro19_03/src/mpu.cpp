#include "mpu.h"

Adafruit_MPU6050 mpu;


void mpu_init()
{

#ifdef DEBUG
  Serial.println("Adafruit MPU6050 test!");
#endif
  // Try to initialize!
  pinMode(ADO, OUTPUT);
  digitalWrite(ADO, LOW);
  if (!mpu.begin(MPU_ADDRESS))
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Coloca a range de giroscópio desejada
#ifdef DEBUG
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange())
  {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }
#endif
  // Setta  a largura de banda do filtro
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ);
  #ifdef DEBUG
   Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth())
  {
  case MPU6050_BAND_260_HZ:
     Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
     Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
     Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
     Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
     Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
     Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
     Serial.println("5 Hz");
    break;
  }

   Serial.println("");
   #endif
  delay(100);
}

void mpu_measurements(SENSOR_CONTROL_PTR sensorControl)
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float offset_x =  (-0.005125*RAD_TO_DEG);
  float offset_y =  (0.076175*RAD_TO_DEG);
  float offset_z = (0.03181*RAD_TO_DEG);
  sensorControl->mpu_gyro_x = ((g.gyro.x)*RAD_TO_DEG)-(offset_x);
  sensorControl->mpu_gyro_y = ((g.gyro.y)*RAD_TO_DEG)-(offset_y);
  sensorControl->mpu_gyro_z = ((g.gyro.z)*RAD_TO_DEG)-(offset_z);
  //Serial.println(sensorControl->mpu_gyro_x);
  //Serial.println(sensorControl->mpu_gyro_y);
  //Serial.println(sensorControl->mpu_gyro_z);
  
}