#include "remote.h"

#include <float.h>

static uint32_t timeout_ms;
static uint32_t last_received_ms;
static DriveCommand latest_command;
static int initialized;
static int have_command;

static int finite_float(float value)
{
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

void Remote_Init(uint32_t timeout)
{
    timeout_ms = timeout;
    last_received_ms = 0U;
    latest_command.mode = STOP;
    latest_command.vx_mm_s = 0.0f;
    latest_command.vy_mm_s = 0.0f;
    latest_command.yaw_deg_s = 0.0f;
    initialized = timeout != 0U;
    have_command = 0;
}

int Remote_Submit(const DriveCommand *command, uint32_t received_ms)
{
    if (!initialized || command == 0) {
        return 0;
    }
    if (command->mode != STOP && command->mode != MANUAL && command->mode != AUTO) {
        return 0;
    }
    if (!finite_float(command->vx_mm_s) || !finite_float(command->vy_mm_s) ||
        !finite_float(command->yaw_deg_s)) {
        return 0;
    }

    latest_command = *command;
    last_received_ms = received_ms;
    have_command = 1;
    return 1;
}

int Remote_Read(uint32_t now_ms, DriveCommand *command)
{
    if (!initialized || !have_command || command == 0) {
        return 0;
    }
    if ((uint32_t)(now_ms - last_received_ms) >= timeout_ms) {
        have_command = 0;
        latest_command.mode = STOP;
        latest_command.vx_mm_s = 0.0f;
        latest_command.vy_mm_s = 0.0f;
        latest_command.yaw_deg_s = 0.0f;
        return 0;
    }

    *command = latest_command;
    return 1;
}
