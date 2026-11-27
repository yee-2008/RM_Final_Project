#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "stm32f4xx_hal.h"

typedef struct {
    TIM_HandleTypeDef *timer;
    uint32_t channel;
    GPIO_TypeDef *in1_port;
    uint16_t in1_pin;
    GPIO_TypeDef *in2_port;
    uint16_t in2_pin;
} MotorDriver;

typedef struct {
    TIM_HandleTypeDef *timer;
    uint32_t channel;
    uint16_t min_us;
    uint16_t max_us;
} ServoDriver;

/* Duty in permille, from -1000 to 1000. Zero disables the L298N output. */
HAL_StatusTypeDef MotorDriver_Start(const MotorDriver *motor);
void MotorDriver_SetDuty(const MotorDriver *motor, int32_t duty);

/* Configure the servo timer for 1 us per tick and a 20 ms period. */
HAL_StatusTypeDef ServoDriver_Start(const ServoDriver *servo, uint16_t initial_us);
void ServoDriver_SetPulse(const ServoDriver *servo, uint16_t pulse_us);

#endif
