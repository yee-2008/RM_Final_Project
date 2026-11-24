/* Move the declarations to CubeMX main.c (USER CODE Includes/PV/PFP),
 * and call App_Init / App_Loop from USER CODE 2 / WHILE loop body.
 */
#include "main.h"
#include "orient.h"

typedef enum { STOP, MANUAL, AUTO } DriveMode;

typedef struct {
    DriveMode mode;
    float vx_mm_s;
    float vy_mm_s;
    float yaw_deg_s;
} DriveCommand;

/* Implement in the hardware modules. Return 0 if no valid, fresh command
 * is available. After a timeout, require an explicit new drive command.
 */
int Remote_Read(DriveCommand *command);
void Motor_Stop(void);
void Motor_ApplyWheelTargets(void); /* Consume FL/FR/RL/RR_AngleVelDes. */
void Robot_To_Wheel(float vx, float vy, float yaw);

static DriveMode mode;
static uint32_t last_control_ms;

static void App_Init(void)
{
    mode = STOP;
    last_control_ms = HAL_GetTick();
    Motor_Stop();
}

static void App_Loop(void)
{
    uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - last_control_ms) < 10U) {
        return;
    }
    last_control_ms = now;

    DriveCommand command = {0};
    if (!Remote_Read(&command)) {
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
        Robot_To_Wheel(command.vx_mm_s, command.vy_mm_s, command.yaw_deg_s);
    } else {
        mode = STOP;
        Motor_Stop();
        return;
    }

    Motor_ApplyWheelTargets();
}
