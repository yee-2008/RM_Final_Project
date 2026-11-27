#ifndef WHEEL_H
#define WHEEL_H

#include <stdint.h>

typedef struct{
    uint16_t Wheelbase; // 轴距，mm
    uint16_t Track;     // 轮距，mm
    uint16_t Radius;    // 轮半径，mm
    int8_t   Sign;      // 归一化符号：O型=+1，X型=-1
} Wheel_DataTypedef;

extern Wheel_DataTypedef Wheel_Data;
extern float FL_AngleVelDes, FR_AngleVelDes, RL_AngleVelDes, RR_AngleVelDes;
extern float Robot_Vx, Robot_Vy, Robot_AngleVel;
void Robot_To_Wheel(float vx, float vy, float yaw);
void Wheel_To_Robot(float fl, float fr, float rl, float rr);

#endif
