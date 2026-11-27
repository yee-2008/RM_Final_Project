#ifndef C1_BENCH_GLUE_H
#define C1_BENCH_GLUE_H

#include "wheel.h"
#include "drive.h"

typedef struct {
    Wheel_DataTypedef geometry;
    /* Full-duty calibrated wheel speed in rad/s, order FL, FR, RL, RR. */
    float max_rad_s[DRIVE_WHEEL_COUNT];
    uint32_t remote_timeout_ms;
} C1_BenchConfig;

/* Call only after CubeMX GPIO, TIM5, and USART3 initialization. */
HAL_StatusTypeDef C1_Bench_Init(const C1_BenchConfig *config);

#endif
