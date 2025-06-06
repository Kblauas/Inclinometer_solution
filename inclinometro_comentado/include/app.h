#ifndef APP_H_
#define APP_H_

#include <stdint.h>
#include <HardwareSerial.h> //importação da biblioteca "third party" para leitura de serial
#include <Wire.h> // importação da biblioteca para uso da comunicação IIC

typedef enum{   
    WAIT_TILL_START,
    GET_MEASURE,
    SEND_MEASURE
} app_state; //struct que define os estados da máquina de estados

typedef struct{
    float adxl_angl_x;
    float adxl_angl_y;
    float adxl_angl_z;
    float mpu_gyro_x;
    float mpu_gyro_y;
    float mpu_gyro_z;

} SENSOR_CONTROL_T, *SENSOR_CONTROL_PTR;  //struct que contém os parâmetros desejados para o cálculo

extern SENSOR_CONTROL_T sensorControl; //é definido como extern pois o mesmo vetor precisa estar presente, e carregando os mesmos valores, em mais de uma "aba"

typedef struct{
    uint8_t rs485_message[100];
    char rs485_command;
    uint8_t rs485_size;
    uint8_t indexrs485;

} RS485_CONTROL_T, *RS485_CONTROL_PTR; //struct que define os parâmetros do rs485

void app_init(void); //inicializa a aplicação
void app_poll(void); //define a máquina de estados e a sua progressão

#endif