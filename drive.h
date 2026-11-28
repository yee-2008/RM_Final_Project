#ifndef DRIVE_H
#define DRIVE_H

#include "actuators.h"

#define DRIVE_WHEEL_COUNT 4U

typedef struct {
    MotorDriver motor;
    /* Wheel angular speed in rad/s at 1000 permille duty. */
    float max_rad_s;
    TIM_HandleTypeDef *encoder;
    int32_t counts_per_turn; /* Signed quadrature counts per wheel revolution. */
    float feedback_gain; /* Dimensionless proportional correction, 0 = open loop. */
} DriveMotorConfig;

/* Config order: front-left, front-right, rear-left, rear-right. */
HAL_StatusTypeDef Motor_Init(const DriveMotorConfig config[DRIVE_WHEEL_COUNT]);
void Motor_Stop(void);
void Motor_ApplyWheelTargets(void);

#endif
