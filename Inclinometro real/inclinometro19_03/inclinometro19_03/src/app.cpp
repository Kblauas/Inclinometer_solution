#include "app.h"
#include "rs485.h"
#include "adxl.h"
#include "mpu.h"
#include "filters.h"



app_state incl_state;
SENSOR_CONTROL_T sensorControl;
RS485_CONTROL_T rs485Control;


void app_init()
{
    incl_state = WAIT_TILL_START;
    filter_init(&filterComp, &filterButter, &filterAvg);
    mpu_init();
    adxl_init();
}

void app_poll()
{   
    //mpu_measurements(&sensorControl);
    //filter_complementary_update(&filterComp);
    static int measure_count = 0;  
    static unsigned long last_check = 0;  
    static int sent_count = 0;
    switch (incl_state)
    {
        case WAIT_TILL_START:
        if (millis() - last_check >= 10) {  
            last_check = millis();
            

            if (rs485_recvCommand(&rs485Control)) {
                
                if (rs485Control.rs485_message[0] == 'S') {
                    Serial.println("Comando 'S' recebido!");
                    incl_state = GET_MEASURE;
                } else {
                    Serial.println("Comando errado recebido!");
                }
            }
        }
        break;


    case GET_MEASURE:
        Serial.println("Adquirindo as medidas");
        filter_apply(&filterComp, &filterButter, &filterAvg);
        
        
        measure_count++;

        if (measure_count >= 50) {
            measure_count = 0;  // Agora reseta corretamente quando atinge 50!
            incl_state = SEND_MEASURE;
        }
        break;

    case SEND_MEASURE:
        Serial.println("Mandando as medidas");
        rs485_send_data(&filterAvg);
        filter_apply(&filterComp, &filterButter, &filterAvg);
         
        sent_count++;
        //float depth;
        //depth+=L;

        if (sent_count == 11) {
            filter_clear(&filterComp, &filterButter, &filterAvg); 
            
            sent_count = 0;
           
            incl_state = WAIT_TILL_START;
           
            
        }
      
        break;

    default:
        break;
    }
}
