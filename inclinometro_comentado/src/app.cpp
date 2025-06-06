#include "app.h"
#include "rs485.h"
#include "adxl.h"
#include "mpu.h"
#include "filters.h"



app_state incl_state; //declaração dos estados
SENSOR_CONTROL_T sensorControl; //declaração do vetor sensorControl
RS485_CONTROL_T rs485Control; //declaração do vetor rs485Control


void app_init() //função de início da aplicação
{
    incl_state = WAIT_TILL_START;   //inicialização no estado de standby
    filter_init(&filterComp, &filterButter, &filterAvg); //inicialização dos filtros
    //mpu_init();
    //adxl_init();
}

void app_poll() //definição da máquina de estados
{   
    //mpu_measurements(&sensorControl);
    //filter_complementary_update(&filterComp);
    static int measure_count = 0;  //inicialização das medições em zero
    static unsigned long last_check = 0;  //inicialização da checagem de valores
    static int sent_count = 0; //inicialização das medidas mandadas
    switch (incl_state) //switch case com os estados da máquinas de estados
    {
        case WAIT_TILL_START: //case de standby, o primeiro
        if (millis() - last_check >= 10) {  //checagem de checagem, utilizando a função de millis para milisegundos
            last_check = millis(); //check se iguala a millis, para fazer a checagem
            

            if (rs485_recvCommand(&rs485Control)) { //recebimento do comando, a partir da função do rs485
                
                if (rs485Control.rs485_message[0] == 'S') { //checagem se o rs485 recebeu o comando S
                    Serial.println("Comando 'S' recebido!"); 
                    incl_state = GET_MEASURE; //Se o comando 'S' for recebido, progride a máquina de estados 
                } else {
                    Serial.println("Comando errado recebido!"); //se for recebido outro comando, mensagem de erro
                }
            }
        }
        break;


    case GET_MEASURE: //estado de aquisição de medidas
        Serial.println("Adquirindo as medidas");
        filter_apply(&filterComp, &filterButter, &filterAvg); //aplica os filtros
        
        
        measure_count++; //soma uma medida a cada vez e aloca o valor nesta variável

        if (measure_count >= 50) { //condição de progressão de estados
            measure_count = 0;  // Agora reseta corretamente quando atinge 50!
            incl_state = SEND_MEASURE; //ao atingir 50, progride para o estado que manda as variáveis
        }
        break;

    case SEND_MEASURE: //estado de envio das medidas
        Serial.println("Mandando as medidas");
        rs485_send_data(&filterAvg); //função de envio das medidas pelo RS485 
        filter_apply(&filterComp, &filterButter, &filterAvg); //novamente corre a função de aplicação dos filtros, 11 vezes. 
         
        sent_count++; 
        
        if (sent_count == 11) {
            filter_clear(&filterComp, &filterButter, &filterAvg); //ao atingir 11 medidas enviadas, limpa os filtros
            
            sent_count = 0; 
           
            incl_state = WAIT_TILL_START; //volta novamente ao estado em standby
           
            
        }
      
        break;

    default:
        break;
    }
}
