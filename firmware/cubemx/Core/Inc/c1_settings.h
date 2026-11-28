#ifndef C1_SETTINGS_H
#define C1_SETTINGS_H

/* Replace the zero calibration values with bench measurements before enabling
 * the matching subsystem. Wheel order is FL, FR, RL, RR. */
#define C1_ENABLE_DRIVE 0
#define C1_ENABLE_ARM   0
#define C1_ENABLE_LINE  0
#define C1_REMOTE_TIMEOUT_MS 250U  /* Tablet repeats frames every 50 ms. */

#if C1_ENABLE_DRIVE
#include "c1_bench_glue.h"
static const C1_BenchConfig chassis = {
    .geometry = {.Wheelbase = 190, .Track = 0, .Radius = 0, .Sign = 0},
    .max_rad_s = {0, 0, 0, 0},
    .remote_timeout_ms = C1_REMOTE_TIMEOUT_MS
};
#endif

#if C1_ENABLE_ARM
#include "arm.h"
#include "tim.h"
/* timer/channel, min_us, max_us, initial_us, signed step_us per 10 ms */
static const ArmJoint arm_joints[ARM_JOINT_COUNT] = {
    [ARM_BASE]  = {{&htim10, TIM_CHANNEL_1, 0, 0}, 0, 0},
    [ARM_WRIST] = {{&htim11, TIM_CHANNEL_1, 0, 0}, 0, 0},
    [ARM_LIFT]  = {{&htim12, TIM_CHANNEL_1, 0, 0}, 0, 0},
    [ARM_GRIP]  = {{&htim12, TIM_CHANNEL_2, 0, 0}, 0, 0}
};
#endif

#if C1_ENABLE_LINE
#if !C1_ENABLE_DRIVE
#error "Line tracking requires a calibrated drive"
#endif
#define C1_LINE_FORWARD_MM_S 0.0f
#define C1_LINE_TURN_DEG_S   0.0f
#define C1_LINE_LATERAL_GAIN 0.0f
#define C1_LINE_YAW_GAIN     0.0f
#endif

#endif
