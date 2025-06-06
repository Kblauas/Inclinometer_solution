#include "rs485.h"





HardwareSerial mySerial(1); //definição de HardwareSerial como mySerial(1)
typedef uint8_t byte; //definição do tipo de dado Byte

void rs485_init(){ //função de inicialização
  pinMode(RE, OUTPUT);
  digitalWrite(RE, LOW); //definição do pino RE como output, começando em low, iniciando o modo de recepção do transmissor
  mySerial.begin(115200, SERIAL_8N1, 5, 4); //define a baud_rate, 8 data bits, no parity 1 stop bit, e os pinos RX e TX
}


void rs485_send_data(FILTER_MOVING_AVERAGE_PTR filterAvg){//função de envio de dados
#ifdef DEBUG
  Serial.println("Enviando dados acumulados via RS485...");
#endif
  digitalWrite(RE, HIGH); //definição do pino RE como alto, iniciando o modo de transmissão do transmissor
  //filter_apply(&filterComp, &filterButter, filterAvg);

 
  byte buffer[9]; //definição do buffer de 9 bytes
  buffer [0] = 0xFF; //definição do byte de cabeçalho

  union{
    int16_t value;
    byte bytes[2];
  } data; //definição da união de valores com bytes, para facilitar os valores inscritos no buffer

  
  data.value = (int16_t)(filterAvg->avg_roll* 100); //inscrição do valor de roll a ser enviado
  buffer[1] = data.bytes[1];
  buffer[2] = data.bytes[0];

  data.value = (int16_t)(filterAvg->avg_pitch * 100); //definição do valor de pitch a ser enviado
  buffer[3] = data.bytes[1];
  buffer[4] = data.bytes[0];

  data.value = (int16_t)(filterAvg->avg_yaw * 100); //definição do valor de yaw a ser enviado
  buffer[5] = data.bytes[1];
  buffer[6] = data.bytes[0];

  data.value = (int16_t)(filterAvg->deviation * 100); //definição do valor de deviation a ser enviado
  buffer[7] = data.bytes[1];
  buffer[8] = data.bytes[0];

  //int16_t angX = ((int16_t)buffer[1] << 8) | buffer[2];
  //int16_t angY = ((int16_t)buffer[3] << 8) | buffer[4];
  //int16_t angZ = ((int16_t)buffer[5] << 8) | buffer[6];
  //int16_t dev = ((int16_t)buffer[7] << 8) | buffer[8];

  //float adxl_angl_x = angX / 100.0;
  //float adxl_angl_y = angY / 100.0;
  //float adxl_angl_z = angZ / 100.0;
  //float dev_roll   = dev / 100.0;

  //Serial.print("Sent Buffer: Roll: ");
  //Serial.print(adxl_angl_x);
  //Serial.print(" Pitch: ");
  //Serial.println(adxl_angl_y);
  //Serial.print(" Yaw: ");
  //Serial.println(adxl_angl_z);
  //Serial.print(" Deviation: ");
  //Serial.println(dev_roll);
  
  //check para ver se está funcionando

  #ifdef DEBUG
  Serial.print("Buffer enviado: ");
  for (int i = 0; i < 9; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
  }
  Serial.println();
  #endif

  mySerial.write(buffer, 9); //escreve os 9 bits do buffer na serial

  mySerial.flush(); //precisa desse flush, se não não envia os dados

  digitalWrite(RE, LOW); //retorna o transmissor para o modo de recepção 
  //delay(10);

  #ifdef DEBUG
    Serial.println("Dados enviados!");
  #endif


}

bool rs485_recvCommand(RS485_CONTROL_PTR rs485Control) //função de checagem de serial, para o recebimento do comando
{


  uint8_t indexMsg = 0;
  if (mySerial.available()) //ve se a serial está disponível
  {

    while (mySerial.available()) // Verifica se há dados disponíveis na UART
    { 
      
      
      
      rs485Control->rs485_message[indexMsg] = mySerial.read();
    
      Serial.println(rs485Control->rs485_message[indexMsg]); //printa a mensagem recebida
      indexMsg++;
      #ifdef DEBUG
      Serial.print("Recebido RS485: ");
      Serial.println(receivedByte, HEX);// Exibe os dados no monitor serial em formato hexadecima
      #endif
    }
    rs485Control->rs485_size = indexMsg;  //indexa o index da mensagem ao tamanho do rs485 
    return true; // se sim, retorna verdadeiro. Se não, retorna falso.
  }
  else
    return false;
}
