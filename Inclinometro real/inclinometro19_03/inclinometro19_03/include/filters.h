#ifndef FILTERS_H_
#define FILTERS_H_


#include "mpu.h"
#include "app.h"
#include "adxl.h"
#include "stdio.h"
#include "math.h"

#define ALPHA 0.3f         
#define MOVING_AVG_SIZE 50
#define L 0.5
#define fs 100 //100Hz
#define fc 5//Hz

typedef struct{
    float pitch_buffer[MOVING_AVG_SIZE];
    float roll_buffer[MOVING_AVG_SIZE];
    float yaw_buffer[MOVING_AVG_SIZE];
    float avg_pitch;
    float avg_roll;
    float avg_yaw;
    float deviation;
    float sum_deviation;
    int index;
    int count;
}FILTER_MOVING_AVERAGE_T, *FILTER_MOVING_AVERAGE_PTR;




typedef struct {
    float pitch;
    float roll;
    float yaw;
} FILTER_COMPLEMENTARY_T, *FILTER_COMPLEMENTARY_PTR ;

typedef struct{
    float b0, b1, b2;
    float a1, a2;
    float x1, x2;
    float y1, y2;
} FILTER_BUTTERWORTH_T, *FILTER_BUTTERWORTH_PTR;

typedef struct {
    FILTER_BUTTERWORTH_T bw_roll;
    FILTER_BUTTERWORTH_T bw_pitch;
    FILTER_BUTTERWORTH_T bw_yaw;
} FILTER_BUTTERWORTH_ANGLES_T, *FILTER_BUTTERWORTH_ANGLES_PTR;

extern FILTER_COMPLEMENTARY_T filterComp;
extern FILTER_BUTTERWORTH_ANGLES_T filterButter;
extern FILTER_MOVING_AVERAGE_T filterAvg;

void filter_complementary_init(FILTER_COMPLEMENTARY_PTR filterComp);

void filter_complementary_update(FILTER_COMPLEMENTARY_PTR filterComp);

void filter_moving_avg_init(FILTER_MOVING_AVERAGE_PTR filterAvg);

void filter_moving_avg_update(FILTER_MOVING_AVERAGE_PTR filterAvg, float new_roll, float new_pitch, float new_yaw);

void filter_moving_avg_calculate(FILTER_MOVING_AVERAGE_PTR filterAvg);

void filter_channel_init(FILTER_BUTTERWORTH_PTR f);

void filter_butterworth_init(FILTER_BUTTERWORTH_ANGLES_PTR filterButter);

float filter_butterworth_update(FILTER_BUTTERWORTH_PTR filterButter, float input);

void filter_init(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg);
void filter_apply(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg);

void filter_clear(FILTER_COMPLEMENTARY_PTR filterComp, FILTER_BUTTERWORTH_ANGLES_PTR filterButter, FILTER_MOVING_AVERAGE_PTR filterAvg);

#endif