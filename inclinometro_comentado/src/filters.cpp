#include "filters.h"

// ALPHA: coeficiente do filtro complementar (0 a 1), define peso do giroscópio vs acelerômetro
// fc: frequência de corte do filtro Butterworth (Hz)
// fs: frequência de amostragem (Hz)
// L: comprimento físico do braço de medição para cálculo de desvio



//complementar
FILTER_MOVING_AVERAGE_T filterAvg;
FILTER_BUTTERWORTH_ANGLES_T filterButter;
FILTER_COMPLEMENTARY_T filterComp;

//definição dos vetores dos filtros a serem utilizados



void filter_complementary_init(FILTER_COMPLEMENTARY_PTR filterComp) { //inicialização do filtro complementarm coms os valores em zero
    filterComp->pitch = 0.0f;
    filterComp->roll  = 0.0f;
    filterComp->yaw = 0.0f;
}

void filter_complementary_update(FILTER_COMPLEMENTARY_PTR filterComp) { //função de atualização do filtro complementar

    
    adxl_measurements(&sensorControl);
    // mpu_measurements(&sensorControl);
    //aquisição das medidas do giroscópio e do acelerômetro
    float dt = 0.01f; // intervalo fixo entre amostras, assumido como 10 ms

    float gyro_ox = (filterComp->roll);  //(sensorControl.mpu_gyro_x*dt)*RAD_TO_DEG;
    float gyro_oy  = (filterComp->pitch);  //(sensorControl.mpu_gyro_y*dt)*RAD_TO_DEG;
    float gyro_oz = (filterComp->yaw); //(sensorControl.mpu_gyro_z*dt)*RAD_TO_DEG;

    //definição das variáveis calculadas a partir dos valores de roll, pitch e yaw, e (quando funciona), do MPU

    filterComp->roll = ALPHA * gyro_ox + (1.0f - ALPHA) * sensorControl.adxl_angl_x;
    filterComp->pitch  = ALPHA * gyro_oy  + (1.0f - ALPHA) * sensorControl.adxl_angl_y;
    filterComp->yaw = ALPHA * gyro_oz + (1.0f - ALPHA) * sensorControl.adxl_angl_z;

    //cálculo dos valores de roll, pitch e yaw.

}

void filter_channel_init(FILTER_BUTTERWORTH_PTR f) { //inicialização do canal do filtro de butterworth: foi feito assim para poder filtrar o roll, pitch e yaw sem eles se afetarem no cálculo

    float k = tanf(M_PI * fc / fs);
    float norm = 1.0f + sqrtf(2.0f) * k + (k * k);
    f->b0 = (k * k) / norm;
    f->b1 = (2 * (k * k)) / norm;
    f->b2 = (k * k) / norm;
    f->a1 = 2 * ((k * k) - 1) / norm;
    f->a2 = (1 - (sqrtf(2) * k) + (k * k)) / norm;
    f->x1 = 0.0f;
    f->x2 = 0.0f;
    f->y1 = 0.0f;
    f->y2 = 0.0f;
    //definição das constantes do filtro, a partir de fc e fs, definidas previamente
}


void filter_butterworth_init(FILTER_BUTTERWORTH_ANGLES_PTR filterButter) { //inicialização do filtro para roll, pitch e yaw separadamente
    //
    filter_channel_init(&filterButter->bw_roll);
    filter_channel_init(&filterButter->bw_pitch);
    filter_channel_init(&filterButter->bw_yaw);
}



float filter_butterworth_update(FILTER_BUTTERWORTH_PTR filterButter, float input){ //atualização e geração do output de cada um dos parâmetros
    if(filterButter->x1 == 0.0f && filterButter->x2 == 0.0f &&
       filterButter->y1 == 0.0f && filterButter->y2 == 0.0f) {
        filterButter->x1 = filterButter->x2 = input;
        filterButter->y1 = filterButter->y2 = input;
        return input;
    }
    
    float output = filterButter->b0 * input + 
                   filterButter->b1 * filterButter->x1 + 
                   filterButter->b2 * filterButter->x2 -
                   filterButter->a1 * filterButter->y1 -
                   filterButter->a2 * filterButter->y2;
    
    filterButter->x2 = filterButter->x1;
    filterButter->x1 = input;
    filterButter->y2 = filterButter->y1;
    filterButter->y1 = output;
    return output;
}



void filter_moving_avg_init(FILTER_MOVING_AVERAGE_PTR filterAvg){ //inicialização do filtro de média móvel, com buffers para cada um dos parâmetros
    for(int i=0; i<MOVING_AVG_SIZE;i++){
        filterAvg->pitch_buffer[i]=0.0f;
        filterAvg->roll_buffer[i]=0.0f;
        filterAvg->yaw_buffer[i]=0.0f;
    }
    filterAvg->index=0;
    filterAvg->count=0;
}

void filter_moving_avg_update(FILTER_MOVING_AVERAGE_PTR filterAvg, float new_roll, float new_pitch, float new_yaw){ //atualização constante do index e da contagem do filtro média móvel
    filterAvg->pitch_buffer[filterAvg->index]=new_pitch;
    filterAvg->roll_buffer[filterAvg->index]=new_roll;
    filterAvg->yaw_buffer[filterAvg->index]=new_yaw;
   

    filterAvg->index = (filterAvg->index+1) % MOVING_AVG_SIZE;

    if(filterAvg->count<MOVING_AVG_SIZE){
        filterAvg->count++;
    }
}

void filter_moving_avg_calculate(FILTER_MOVING_AVERAGE_PTR filterAvg) { //cálculo do filtro média móvel
    if (filterAvg->count == 0) return;  

    float soma_pitch = 0.0f, soma_roll = 0.0f, soma_yaw = 0.0f;
    int num_samples = filterAvg->count;  
    for (int i = 0; i < num_samples; i++) {
        int index = (filterAvg->index - 1 - i + MOVING_AVG_SIZE) % MOVING_AVG_SIZE;  
        soma_pitch += filterAvg->pitch_buffer[index];
        soma_roll += filterAvg->roll_buffer[index];
        soma_yaw += filterAvg->yaw_buffer[index];
    }
    filterAvg->avg_pitch = soma_pitch / num_samples;
    filterAvg->avg_roll = soma_roll / num_samples;
    filterAvg->avg_yaw = soma_yaw / num_samples;

    filterAvg->deviation =(filterAvg->avg_roll*L);
    filterAvg->sum_deviation += filterAvg->deviation;
}



void filter_init(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg){ //função para facilitar a inicialização de todos os filtros juntos 
    filter_complementary_init(filterComp);
  
    filter_butterworth_init(filterButter);

    filter_moving_avg_init(filterAvg);
}

void filter_apply(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg){//função para facilitar a aplicação de todos os filtros juntos
    filter_complementary_update(filterComp);

    #ifdef DEBUG
    Serial.print("Complementary x angle: "); Serial.println(filterComp->roll, 6);
    Serial.print("Complementary y angle: "); Serial.println(filterComp->pitch, 6);
    Serial.print("Complementary z angle: "); Serial.println(filterComp->yaw, 6);
    #endif
    // Passa para o filtro Butterworth
    float filtered_roll = filter_butterworth_update(&(filterButter->bw_roll), filterComp->roll);
    float filtered_pitch = filter_butterworth_update(&(filterButter->bw_pitch), filterComp->pitch);
    float filtered_yaw = filter_butterworth_update(&(filterButter->bw_yaw), filterComp->yaw);

    #ifdef DEBUG
    Serial.print("Butterworth x angle: "); Serial.println(filtered_roll, 6);
    Serial.print("Butterworth y angle: "); Serial.println(filtered_pitch, 6);
    Serial.print("Butterworth z angle: "); Serial.println(filtered_yaw, 6);
    #endif
    
    // Atualiza o filtro de média móvel
    filter_moving_avg_update(filterAvg, filtered_roll, filtered_pitch, filtered_yaw);
    filter_moving_avg_calculate(filterAvg);

    #ifdef DEBUG
    Serial.print("Moving Avg x angle: "); Serial.println(filterAvg->avg_roll, 6);
    Serial.print("Moving Avg y angle: "); Serial.println(filterAvg->avg_pitch, 6);
    Serial.print("Moving Avg z angle: "); Serial.println(filterAvg->avg_yaw, 6);
    #endif
    
    delay(10);
}


void filter_clear(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg){ //limpa todos os filtros juntos, reinicializando eles.
    filterComp->roll=0;
    filterComp->pitch=0;
    filterComp->yaw=0;
    filterButter->bw_roll.x1 = 0.0f;
    filterButter->bw_roll.x2 = 0.0f;
    filterButter->bw_roll.y1 = 0.0f;
    filterButter->bw_roll.y2 = 0.0f;
    filterButter->bw_pitch.x1 = 0.0f;
    filterButter->bw_pitch.x2 = 0.0f;
    filterButter->bw_pitch.y1 = 0.0f;
    filterButter->bw_pitch.y2 = 0.0f;
    filterButter->bw_yaw.x1 = 0.0f;
    filterButter->bw_yaw.x2 = 0.0f;
    filterButter->bw_yaw.y1 = 0.0f;
    filterButter->bw_yaw.y2 = 0.0f;
    filterAvg->count=0;
    filterAvg->index=0;
}



    
