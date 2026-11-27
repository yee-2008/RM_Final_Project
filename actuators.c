#include "actuators.h"

HAL_StatusTypeDef MotorDriver_Start(const MotorDriver *motor)
{
    MotorDriver_SetDuty(motor, 0);
    return HAL_TIM_PWM_Start(motor->timer, motor->channel);
}

void MotorDriver_SetDuty(const MotorDriver *motor, int32_t duty)
{
    if (duty > 1000) {
        duty = 1000;
    } else if (duty < -1000) {
        duty = -1000;
    }

    /* Request zero duty while updating the direction pins. */
    __HAL_TIM_SET_COMPARE(motor->timer, motor->channel, 0);
    HAL_GPIO_WritePin(motor->in1_port, motor->in1_pin,
                     duty > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->in2_port, motor->in2_pin,
                     duty < 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);

    uint32_t magnitude = (uint32_t)(duty < 0 ? -duty : duty);
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(motor->timer);
    uint32_t compare = (uint32_t)((uint64_t)period * magnitude / 1000U);
    __HAL_TIM_SET_COMPARE(motor->timer, motor->channel, compare);
}

HAL_StatusTypeDef ServoDriver_Start(const ServoDriver *servo, uint16_t initial_us)
{
    ServoDriver_SetPulse(servo, initial_us);
    return HAL_TIM_PWM_Start(servo->timer, servo->channel);
}

void ServoDriver_SetPulse(const ServoDriver *servo, uint16_t pulse_us)
{
    if (pulse_us < servo->min_us) {
        pulse_us = servo->min_us;
    } else if (pulse_us > servo->max_us) {
        pulse_us = servo->max_us;
    }

    __HAL_TIM_SET_COMPARE(servo->timer, servo->channel, pulse_us);
}
