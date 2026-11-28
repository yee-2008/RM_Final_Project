#include "c1_bench_glue.h"

#include "main.h"
#include "main_template.h"
#include "tim.h"
#include "usart.h"

static uint8_t bench_ready;

HAL_StatusTypeDef C1_Bench_Init(const C1_BenchConfig *config)
{
    if (config == NULL || bench_ready != 0U ||
        config->remote_timeout_ms == 0U ||
        config->geometry.Wheelbase == 0U ||
        config->geometry.Track == 0U ||
        config->geometry.Radius == 0U ||
        (config->geometry.Sign != 1 && config->geometry.Sign != -1) ||
        htim5.Instance != TIM5 || htim9.Instance != TIM9 ||
        huart3.Instance != USART3) {
        Motor_Stop();
        return HAL_ERROR;
    }

    DriveMotorConfig motors[DRIVE_WHEEL_COUNT] = {
        {{&htim5, TIM_CHANNEL_1, FL_IN1_GPIO_Port, FL_IN1_Pin,
          FL_IN2_GPIO_Port, FL_IN2_Pin}, config->max_rad_s[0], &htim2,
          config->counts_per_turn[0], config->feedback_gain},
        {{&htim9, TIM_CHANNEL_1, FR_IN1_GPIO_Port, FR_IN1_Pin,
          FR_IN2_GPIO_Port, FR_IN2_Pin}, config->max_rad_s[1], &htim3,
          config->counts_per_turn[1], config->feedback_gain},
        {{&htim5, TIM_CHANNEL_3, RL_IN1_GPIO_Port, RL_IN1_Pin,
          RL_IN2_GPIO_Port, RL_IN2_Pin}, config->max_rad_s[2], &htim4,
          config->counts_per_turn[2], config->feedback_gain},
        {{&htim5, TIM_CHANNEL_4, RR_IN1_GPIO_Port, RR_IN1_Pin,
          RR_IN2_GPIO_Port, RR_IN2_Pin}, config->max_rad_s[3], &htim1,
          config->counts_per_turn[3], config->feedback_gain}
    };

    if (Motor_Init(motors) != HAL_OK) {
        Motor_Stop();
        return HAL_ERROR;
    }

    Wheel_Data = config->geometry;
    App_Init(config->remote_timeout_ms);
    bench_ready = 1U;
    return HAL_OK;
}
