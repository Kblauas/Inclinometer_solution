#include "mpu.h"

Adafruit_MPU6050 mpu;


void mpu_init()
{

#ifdef DEBUG
  Serial.println("Adafruit MPU6050 test!");
#endif
  // Tentativa de inicialização do sensor
  pinMode(ADO, OUTPUT);
  digitalWrite(ADO, LOW); //seta o pino AD0 como low, para assegurar o endereço desejado
  if (!mpu.begin(MPU_ADDRESS))//checagem de endereço: se não for achado, trava o código
  {
    Serial.println("Failed to find MPU6050 chip"); 
    while (1) 
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
#ifdef DEBUG //seleção da range desejada para o osciloscópio
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
  
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ); 
  #ifdef DEBUG //seleção da largura de banda do filtro do MPU
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

void mpu_measurements(SENSOR_CONTROL_PTR sensorControl) //aquisição das medidas do MPU
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float offset_x =  (-0.005125*RAD_TO_DEG); //offset calculado a partir de um algorítmo externo, presente na pasta "calibration" do github
  float offset_y =  (0.076175*RAD_TO_DEG);
  float offset_z = (0.03181*RAD_TO_DEG);
  sensorControl->mpu_gyro_x = ((g.gyro.x)*RAD_TO_DEG)-(offset_x);
  sensorControl->mpu_gyro_y = ((g.gyro.y)*RAD_TO_DEG)-(offset_y);
  sensorControl->mpu_gyro_z = ((g.gyro.z)*RAD_TO_DEG)-(offset_z);
  //alocação dos valores calibrados ao vetor de controle
  #ifdef DEBUG_MEASURE
  //Serial.println(sensorControl->mpu_gyro_x);
  //Serial.println(sensorControl->mpu_gyro_y);
  //Serial.println(sensorControl->mpu_gyro_z);
  #endif 
}
