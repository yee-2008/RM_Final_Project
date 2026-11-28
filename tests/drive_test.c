#include <assert.h>
#include "drive.h"
#include "wheel.h"

float FL_AngleVelDes, FR_AngleVelDes, RL_AngleVelDes, RR_AngleVelDes;
static uint32_t now;
static int32_t duty[4];

uint32_t HAL_GetTick(void) { return now; }
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *t, uint32_t c)
{
    (void)t; (void)c;
    return HAL_OK;
}
HAL_StatusTypeDef MotorDriver_Start(const MotorDriver *m)
{
    (void)m;
    return HAL_OK;
}
void MotorDriver_SetDuty(const MotorDriver *m, int32_t value)
{
    duty[m->channel] = value;
}

int main(void)
{
    GPIO_TypeDef gpio = {0};
    TIM_HandleTypeDef timer[4] = {{0}};
    DriveMotorConfig config[4] = {0};
    now = UINT32_MAX - 4;
    for (unsigned i = 0; i < 4; ++i) {
        timer[i].arr = i == 0 ? UINT32_MAX : UINT16_MAX;
        timer[i].cnt = i < 2 ? timer[i].arr - 4 : 4;
        config[i].motor = (MotorDriver){&timer[i], i, &gpio, 1, &gpio, 2};
        config[i].encoder = &timer[i];
        config[i].max_rad_s = 20;
        config[i].feedback_gain = 1;
        config[i].counts_per_turn = i == 2 ? -1000 : 1000;
    }
    assert(Motor_Init(config) == HAL_OK);
    FL_AngleVelDes = FR_AngleVelDes = RL_AngleVelDes = 10;
    RR_AngleVelDes = -10;
    for (unsigned i = 0; i < 4; ++i)
        timer[i].cnt = i < 2 ? 5 : timer[i].arr - 5;
    now += 10;
    Motor_ApplyWheelTargets();
    assert(duty[0] == 685 && duty[1] == 685 && duty[2] == 685 && duty[3] == -685);
    Motor_ApplyWheelTargets();
    assert(duty[0] == 685);
    now += 21;
    Motor_ApplyWheelTargets();
    for (unsigned i = 0; i < 4; ++i) assert(duty[i] == 0);
    now += 10;
    FL_AngleVelDes = 40;
    Motor_ApplyWheelTargets();
    assert(duty[0] == 1000 && duty[1] == 0);
    Motor_Stop();
    assert(duty[0] == 0 && FL_AngleVelDes == 0);
    config[1].counts_per_turn = 0;
    assert(Motor_Init(config) == HAL_ERROR);
    for (unsigned i = 0; i < 4; ++i) config[i].feedback_gain = 0;
    assert(Motor_Init(config) == HAL_OK);
    FL_AngleVelDes = 10;
    Motor_ApplyWheelTargets();
    assert(duty[0] == 500);
    return 0;
}
