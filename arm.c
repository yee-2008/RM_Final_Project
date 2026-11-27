#include "arm.h"

static ArmJoint joints[ARM_JOINT_COUNT];
static uint16_t pulse[ARM_JOINT_COUNT];
static int ready;

HAL_StatusTypeDef Arm_Init(const ArmJoint config[ARM_JOINT_COUNT])
{
    if (!config || ready) {
        return HAL_ERROR;
    }
    for (unsigned i = 0; i < ARM_JOINT_COUNT; ++i) {
        const ArmJoint *joint = &config[i];
        if (!joint->pwm.timer || !joint->pwm.min_us ||
            joint->pwm.min_us >= joint->pwm.max_us ||
            joint->pwm.max_us >= 20000 || joint->positive_step_us == 0 ||
            joint->initial_us < joint->pwm.min_us ||
            joint->initial_us > joint->pwm.max_us) {
            return HAL_ERROR;
        }
        for (unsigned j = 0; j < i; ++j) {
            if (config[j].pwm.timer == joint->pwm.timer &&
                config[j].pwm.channel == joint->pwm.channel) {
                return HAL_ERROR;
            }
        }
    }
    for (unsigned i = 0; i < ARM_JOINT_COUNT; ++i) {
        joints[i] = config[i];
        pulse[i] = config[i].initial_us;
        if (ServoDriver_Start(&joints[i].pwm, pulse[i]) != HAL_OK) {
            while (i > 0) {
                --i;
                HAL_TIM_PWM_Stop(joints[i].pwm.timer, joints[i].pwm.channel);
            }
            return HAL_ERROR;
        }
    }
    ready = 1;
    return HAL_OK;
}

void Arm_Tick(ArmAction action)
{
    if (!ready || action == ARM_HOLD || (unsigned)action > ARM_OPEN) {
        return;
    }
    static const unsigned axis[] = {
        [ARM_BASE_POS] = ARM_BASE, [ARM_BASE_NEG] = ARM_BASE,
        [ARM_WRIST_POS] = ARM_WRIST, [ARM_WRIST_NEG] = ARM_WRIST,
        [ARM_LIFT_UP] = ARM_LIFT, [ARM_LIFT_DOWN] = ARM_LIFT,
        [ARM_CLOSE] = ARM_GRIP, [ARM_OPEN] = ARM_GRIP
    };
    unsigned i = axis[action];
    int step = joints[i].positive_step_us;
    if (action == ARM_BASE_NEG || action == ARM_WRIST_NEG ||
        action == ARM_LIFT_DOWN || action == ARM_OPEN) {
        step = -step;
    }
    int next = pulse[i] + step;
    if (next > joints[i].pwm.max_us) next = joints[i].pwm.max_us;
    if (next < joints[i].pwm.min_us) next = joints[i].pwm.min_us;
    pulse[i] = (uint16_t)next;
    ServoDriver_SetPulse(&joints[i].pwm, pulse[i]);
}
