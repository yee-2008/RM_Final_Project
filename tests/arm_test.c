#include <assert.h>
#include "../main_template.c"

static uint32_t ms;
static USART_TypeDef regs;
static UART_HandleTypeDef uart = { &regs };
static uint16_t target[4];
static int starts, stops, fail_at = -1;
static float chassis_x;
OrientState orient_state = STRAIGHT;
uint32_t HAL_GetTick(void) { return ms; }
void Motor_Stop(void) { chassis_x = 0; }
void Motor_ApplyWheelTargets(void) {}
void Robot_To_Wheel(float x, float y, float yaw) { chassis_x = x; (void)y; (void)yaw; }
void autoControl(void) {}
void OrientInit(void) {}
HAL_StatusTypeDef ServoDriver_Start(const ServoDriver *s, uint16_t us)
{
    if (starts++ == fail_at) return HAL_ERROR;
    target[s->channel] = us;
    return HAL_OK;
}
void ServoDriver_SetPulse(const ServoDriver *s, uint16_t us) { target[s->channel] = us; }
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *timer, uint32_t channel)
{
    (void)timer; (void)channel; ++stops;
    return HAL_OK;
}

static void frame(const char *line)
{
    while (*line) {
        regs.SR = USART_SR_RXNE;
        regs.DR = (uint8_t)*line++;
        App_PollBluetooth(&uart); App_Loop();
    }
    regs.SR = 0;
}
static void tick(void) { ms += 10; App_PollBluetooth(&uart); App_Loop(); }

int main(void)
{
    TIM_HandleTypeDef timer = {0};
    ArmJoint joints[4];
    for (unsigned i = 0; i < 4; ++i)
        joints[i] = (ArmJoint){{&timer, i, 1400, 1600}, 1500, i == ARM_GRIP ? -50 : 50};
    fail_at = 2;
    assert(Arm_Init(joints) == HAL_ERROR && stops == 2);
    fail_at = -1;
    assert(Arm_Init(joints) == HAL_OK);
    App_Init(40);
    frame("M,100,0,0,7\n"); tick();
    assert(chassis_x == 100 && target[ARM_GRIP] == 1450);
    tick(); tick();
    assert(target[ARM_GRIP] == 1400);
    tick();
    assert(chassis_x == 0 && target[ARM_GRIP] == 1400);
    frame("M,0,0,0,5\n"); tick();
    assert(target[ARM_LIFT] == 1550);
    frame("M,0,0,0,-\n"); tick();
    assert(target[ARM_LIFT] == 1550);
    frame("M,0,0,0,6\n"); tick();
    assert(target[ARM_LIFT] == 1500);
    frame("M,0,0,0,1\n"); tick();
    frame("M,0,0,0,3\n"); tick();
    assert(target[ARM_BASE] == 1550 && target[ARM_WRIST] == 1550);
    frame("A\n"); tick(); assert(target[ARM_WRIST] == 1550);
    frame("M,0,0,0,8\n"); tick(); assert(target[ARM_GRIP] == 1450);
    frame("S\n"); tick(); assert(target[ARM_GRIP] == 1450);
    frame("M,100,0,0,7\n"); tick();
    regs.SR = USART_SR_ORE; App_PollBluetooth(&uart); App_Loop(); regs.SR = 0;
    frame("M,100,0,0,8\n"); tick();
    assert(chassis_x == 0 && target[ARM_GRIP] == 1400);
}
