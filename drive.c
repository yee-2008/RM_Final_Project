#include "drive.h"

#include <float.h>

#include "wheel.h"

static DriveMotorConfig motors[DRIVE_WHEEL_COUNT];
static uint8_t motor_initialized;
static uint32_t previous_count[DRIVE_WHEEL_COUNT];
static uint32_t previous_ms;

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
            !valid_speed(config[i].max_rad_s) ||
            ((config[i].feedback_gain == 0.0f) != (config[0].feedback_gain == 0.0f)) ||
            (config[i].feedback_gain != 0.0f &&
             (!valid_speed(config[i].feedback_gain) || config[i].encoder == NULL ||
              config[i].counts_per_turn == 0))) {
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
            if (motors[i].feedback_gain != 0.0f) {
                previous_count[i] = __HAL_TIM_GET_COUNTER(motors[i].encoder);
            }
        }
        previous_ms = HAL_GetTick();
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

    uint32_t now = HAL_GetTick();
    uint32_t elapsed_ms = now - previous_ms;
    if (motors[0].feedback_gain != 0.0f && elapsed_ms > 20U) {
        /* A missed control cycle can make the encoder delta ambiguous. */
        Motor_Stop();
        return;
    }
    if (motors[0].feedback_gain != 0.0f && elapsed_ms == 0U) return;
    previous_ms = now;
    double scale = peak > 1.0 ? 1.0 / peak : 1.0;
    for (uint32_t i = 0; i < DRIVE_WHEEL_COUNT; ++i) {
        double requested = targets[i] * scale;
        double corrected = requested;
        if (motors[i].feedback_gain != 0.0f) {
            TIM_HandleTypeDef *encoder = motors[i].encoder;
            uint32_t mask = __HAL_TIM_GET_AUTORELOAD(encoder);
            uint32_t count = __HAL_TIM_GET_COUNTER(encoder);
            uint32_t delta = (count - previous_count[i]) & mask;
            previous_count[i] = count;
            /* C1 uses the full 16- or 32-bit period on each encoder timer. */
            int64_t ticks = delta;
            if (delta > mask / 2U) ticks -= (int64_t)mask + 1;
            double measured = ticks * 6283.185307179586 /
                              ((double)motors[i].counts_per_turn * elapsed_ms);
            corrected += motors[i].feedback_gain * (requested - measured);
        }
        double duty = corrected / motors[i].max_rad_s * 1000.0;
        if (duty > 1000.0) duty = 1000.0;
        if (duty < -1000.0) duty = -1000.0;
        if (requested == 0.0) duty = 0.0;
        MotorDriver_SetDuty(&motors[i].motor, (int32_t)duty);
    }
}
