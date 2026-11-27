#include "drive.h"

#include <float.h>

#include "wheel.h"

static DriveMotorConfig motors[DRIVE_WHEEL_COUNT];
static uint8_t motor_initialized;

static uint8_t valid_speed(float speed)
{
    return speed > 0.0f && speed <= FLT_MAX;
}

HAL_StatusTypeDef Motor_Init(const DriveMotorConfig config[DRIVE_WHEEL_COUNT])
{
    Motor_Stop();
    if (motor_initialized != 0U) {
        for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
            HAL_TIM_PWM_Stop(motors[i].motor.timer, motors[i].motor.channel);
        }
        motor_initialized = 0U;
    }

    if (config == NULL) {
        return HAL_ERROR;
    }

    for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
        if (config[i].motor.timer == NULL ||
            config[i].motor.in1_port == NULL ||
            config[i].motor.in2_port == NULL ||
            !valid_speed(config[i].max_rad_s)) {
            return HAL_ERROR;
        }
    }

    for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
        motors[i] = config[i];
    }

    for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
        if (MotorDriver_Start(&motors[i].motor) != HAL_OK) {
            for (uint32_t j = 0; j < DRIVE_WHEEL_COUNT; ++j) {
                MotorDriver_SetDuty(&motors[j].motor, 0);
                if (j < i) {
                    (void)HAL_TIM_PWM_Stop(motors[j].motor.timer,
                                           motors[j].motor.channel);
                }
            }
            motor_initialized = 0U;
            return HAL_ERROR;
        }
    }

    motor_initialized = 1U;
    Motor_Stop();
    return HAL_OK;
}

void Motor_Stop(void)
{
    FL_AngleVelDes = 0.0f;
    FR_AngleVelDes = 0.0f;
    RL_AngleVelDes = 0.0f;
    RR_AngleVelDes = 0.0f;

    if (motor_initialized != 0U) {
        for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
            MotorDriver_SetDuty(&motors[i].motor, 0);
        }
    }
}

void Motor_ApplyWheelTargets(void)
{
    if (motor_initialized == 0U) {
        Motor_Stop();
        return;
    }

    const float targets[DRIVE_WHEEL_COUNT] = {
        FL_AngleVelDes, FR_AngleVelDes, RL_AngleVelDes, RR_AngleVelDes
    };
    double peak = 0.0;

    for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
        float target = targets[i];
        if (!(target == target) || target > FLT_MAX || target < -FLT_MAX) {
            Motor_Stop();
            return;
        }
        double demand = (double)target / (double)motors[i].max_rad_s;
        if (demand < 0.0) {
            demand = -demand;
        }
        if (demand > peak) {
            peak = demand;
        }
    }

    double scale = peak > 1.0 ? 1.0 / peak : 1.0;
    for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
        double duty = (double)targets[i] / (double)motors[i].max_rad_s * scale * 1000.0;
        MotorDriver_SetDuty(&motors[i].motor, (int32_t)duty);
    }
}
