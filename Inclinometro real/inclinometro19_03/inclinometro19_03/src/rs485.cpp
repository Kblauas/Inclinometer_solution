#include "rs485.h"





HardwareSerial mySerial(1);
typedef uint8_t byte;

void rs485_init(){
  pinMode(RE, OUTPUT);
  digitalWrite(RE, LOW);
  mySerial.begin(115200, SERIAL_8N1, 5, 4);
}


void rs485_send_data(FILTER_MOVING_AVERAGE_PTR filterAvg)
{
#ifdef DEBUG
  Serial.println("Enviando dados acumulados via RS485...");
#endif
  digitalWrite(RE, HIGH);
  //filter_apply(&filterComp, &filterButter, filterAvg);

 
  byte buffer[9];
  buffer [0] = 0xFF;

  union{
    int16_t value;
    byte bytes[2];
  } data;

  
  data.value = (int16_t)(filterAvg->avg_roll* 100);
  buffer[1] = data.bytes[1];
  buffer[2] = data.bytes[0];

  data.value = (int16_t)(filterAvg->avg_pitch * 100);
  buffer[3] = data.bytes[1];
  buffer[4] = data.bytes[0];

  data.value = (int16_t)(filterAvg->avg_yaw * 100);
  buffer[5] = data.bytes[1];
  buffer[6] = data.bytes[0];

  data.value = (int16_t)(filterAvg->deviation * 100);
  buffer[7] = data.bytes[1];
  buffer[8] = data.bytes[0];

  int16_t angX = ((int16_t)buffer[1] << 8) | buffer[2];
  int16_t angY = ((int16_t)buffer[3] << 8) | buffer[4];
  int16_t angZ = ((int16_t)buffer[5] << 8) | buffer[6];
  int16_t gyroX = ((int16_t)buffer[7] << 8) | buffer[8];

  //float adxl_angl_x = angX / 100.0;
  //float adxl_angl_y = angY / 100.0;
  //float adxl_angl_z = angZ / 100.0;
  //float mpu_gyro_x   = gyroX / 100.0;

  //Serial.print("adxl_angl_x: ");
  //Serial.println(adxl_angl_x);
  //Serial.print("adxl_angl_y: ");
  //Serial.println(adxl_angl_y);
  //Serial.print("adxl_angl_z: ");
  //Serial.println(adxl_angl_z);
  //Serial.print("mpu_gyro_x: ");
  //Serial.println(mpu_gyro_x);


  #ifdef DEBUG
  Serial.print("Buffer enviado: ");
  for (int i = 0; i < 9; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
  }
  Serial.println();
  #endif

  mySerial.write(buffer, 9);

  mySerial.flush();

  digitalWrite(RE, LOW);
  //delay(10);

  #ifdef DEBUG
    Serial.println("Dados enviados!");
  #endif


}

bool rs485_recvCommand(RS485_CONTROL_PTR rs485Control)
{

  //String data = mySerial.readString();
  //Serial.println(data);
  uint8_t indexMsg = 0;
  if (mySerial.available())
  {
    //Serial.print("oie");
    while (mySerial.available()) // Verifica se há dados disponíveis na UART
    { 
      //Serial.print("funcionoU");
      
      
      rs485Control->rs485_message[indexMsg] = mySerial.read();
    
      Serial.println(rs485Control->rs485_message[indexMsg]);
      indexMsg++;
      #ifdef DEBUG
      Serial.print("Recebido RS485: ");
      Serial.println(receivedByte, HEX);// Exibe os dados no monitor serial em formato hexadecima
      #endif
    }
    rs485Control->rs485_size = indexMsg;
    return true;
  }
  else
    return false;
}
