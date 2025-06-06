#ifndef FILTERS_H_
#define FILTERS_H_


#include "mpu.h"
#include "app.h"
#include "adxl.h"
#include "stdio.h"
#include "math.h"

//bibliotecas utilizadas durante o funcionamento do circuito

#define ALPHA 0.0f         //define o quanto que o giroscópio influencia nas medidas
#define MOVING_AVG_SIZE 65 //define o tamanho da média móvel
#define L 0.5   // define o intervalo físico das medidas
#define fs 100 //100Hz - frequência de amostragem definida, pode ser mudada
#define fc 5//Hz - frequência de corte definida, pode ser mudada

typedef struct{
    float pitch_buffer[MOVING_AVG_SIZE];
    float roll_buffer[MOVING_AVG_SIZE];
    float yaw_buffer[MOVING_AVG_SIZE];
    float avg_pitch;
    float avg_roll;
    float avg_yaw;
    float deviation;
    //float sum_deviation;
    int index;
    int count;
}FILTER_MOVING_AVERAGE_T, *FILTER_MOVING_AVERAGE_PTR; //parâmetros necessários para o funcionamento do filtro média móvel




typedef struct {
    float pitch;
    float roll;
    float yaw;
} FILTER_COMPLEMENTARY_T, *FILTER_COMPLEMENTARY_PTR ; //parâmetros esperados para o filtro complementar

typedef struct{
    float b0, b1, b2;
    float a1, a2;
    float x1, x2;
    float y1, y2;
} FILTER_BUTTERWORTH_T, *FILTER_BUTTERWORTH_PTR; //Constantes do filtro de butterworth de segunda ordem

typedef struct {
    FILTER_BUTTERWORTH_T bw_roll;
    FILTER_BUTTERWORTH_T bw_pitch;
    FILTER_BUTTERWORTH_T bw_yaw;
} FILTER_BUTTERWORTH_ANGLES_T, *FILTER_BUTTERWORTH_ANGLES_PTR; //Parâmetros retirados do filtro de butterworth de segunda ordem

extern FILTER_COMPLEMENTARY_T filterComp;
extern FILTER_BUTTERWORTH_ANGLES_T filterButter;
extern FILTER_MOVING_AVERAGE_T filterAvg;
// os filtros estão em externo, pois seus resultados são utilizados em outras partes do código

void filter_complementary_init(FILTER_COMPLEMENTARY_PTR filterComp); //inicialização do filtro complementar

void filter_complementary_update(FILTER_COMPLEMENTARY_PTR filterComp); //atualização do filtro complementar
 
void filter_moving_avg_init(FILTER_MOVING_AVERAGE_PTR filterAvg);   //inicialização do filtro média móvel

void filter_moving_avg_update(FILTER_MOVING_AVERAGE_PTR filterAvg, float new_roll, float new_pitch, float new_yaw); //atualização do filtro média móvel 

void filter_moving_avg_calculate(FILTER_MOVING_AVERAGE_PTR filterAvg); //cálculo do filtro média móvel

void filter_channel_init(FILTER_BUTTERWORTH_PTR f); //inicialização do canal do filtro butterworth

void filter_butterworth_init(FILTER_BUTTERWORTH_ANGLES_PTR filterButter); //inicialização do filtro de butterworth

float filter_butterworth_update(FILTER_BUTTERWORTH_PTR filterButter, float input); //atualização do filtro de butterworth

void filter_init(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg); //inicialização de todos os filtros
void filter_apply(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg);  //aplicação de todos os filtros

void filter_clear(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg); //limpeza de todos os filtros

#endif