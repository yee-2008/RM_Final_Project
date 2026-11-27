/* Move the declarations to CubeMX main.c (USER CODE Includes/PV/PFP),
 * and call App_Init / App_Loop from USER CODE 2 / WHILE loop body.
 */
#include "main.h"
#include "orient.h"

#include "wheel.h"
#include "drive.h"
#include "arm.h"
#include "remote.h"
#include "remote_frame.h"

static DriveMode mode;
static uint32_t last_control_ms;

static void App_Init(uint32_t remote_timeout_ms)
{
    Remote_Init(remote_timeout_ms);
    RemoteFrame_Reset();
    mode = STOP;
    last_control_ms = HAL_GetTick();
    Motor_Stop();
}

/* Call continuously from the foreground on a dedicated Bluetooth UART, 8N1.
 * Read before the 10 ms control gate so the F407 receive register is drained.
 */
static void App_PollBluetooth(UART_HandleTypeDef *bluetooth)
{
    static int discard_line;
    uint32_t status = bluetooth->Instance->SR;
    if (status & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) {
        /* STM32F4 clears receive errors with an SR read followed by a DR read. */
        (void)bluetooth->Instance->DR;
        RemoteFrame_Reset();
        discard_line = 1;
    } else if (status & USART_SR_RXNE) {
        uint8_t byte = (uint8_t)bluetooth->Instance->DR;
        if (discard_line) {
            if (byte == '\n') discard_line = 0;
        } else {
            RemoteFrame_Byte(byte, HAL_GetTick());
        }
    }

}

static void App_Loop(void)
{
    uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - last_control_ms) < 10U) {
        return;
    }
    last_control_ms = now;

    DriveCommand command = {0};
    ArmAction arm_action = ARM_HOLD;
    if (!Remote_ReadWithArm(now, &command, &arm_action)) {
        mode = STOP;
        Motor_Stop();
        return;
    }

    if (command.mode == AUTO) {
        if (mode != AUTO) {
            OrientInit();
        }
        mode = AUTO;
        /* Keep AUTO latched at END/LOSE until the operator leaves AUTO. */
        if (orient_state != END && orient_state != LOSE) {
            autoControl();
        }
        if (orient_state == END || orient_state == LOSE) {
            Motor_Stop();
            return;
        }
    } else if (command.mode == MANUAL) {
        mode = MANUAL;
        Arm_Tick(arm_action);
        Robot_To_Wheel(command.vx_mm_s, command.vy_mm_s, command.yaw_deg_s);
    } else {
        mode = STOP;
        Motor_Stop();
        return;
    }

    Motor_ApplyWheelTargets();
}
