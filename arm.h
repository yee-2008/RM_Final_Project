#ifndef ARM_H
#define ARM_H

#include "actuators.h"
#include "remote.h"

enum { ARM_BASE, ARM_WRIST, ARM_LIFT, ARM_GRIP, ARM_JOINT_COUNT };

typedef struct {
    ServoDriver pwm;
    uint16_t initial_us;
    /* Signed pulse increment per 10 ms tick for +roll, lift up or close. */
    int16_t positive_step_us;
} ArmJoint;

HAL_StatusTypeDef Arm_Init(const ArmJoint joints[ARM_JOINT_COUNT]);
void Arm_Tick(ArmAction action);

#endif
