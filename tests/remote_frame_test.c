#include "remote.h"
#include "remote_frame.h"

#include <assert.h>
#include <stdint.h>

static void send_line(const char *line, uint32_t time)
{
    while (*line != '\0') {
        RemoteFrame_Byte((uint8_t)*line++, time);
    }
}

int main(void)
{
    DriveCommand submitted_command;
    ArmAction submitted_arm;
    static const char *keys[] = { "-", "1", "2", "3", "4", "5", "6", "7", "8" };
    static const ArmAction actions[] = {
        ARM_HOLD, ARM_BASE_POS, ARM_BASE_NEG, ARM_WRIST_POS, ARM_WRIST_NEG,
        ARM_LIFT_UP, ARM_LIFT_DOWN, ARM_CLOSE, ARM_OPEN
    };
    unsigned i;

    Remote_Init(1000U);
    RemoteFrame_Reset();
    send_line("M,-32768,+32767,0,-\n", 100U);
    assert(Remote_ReadWithArm(100U, &submitted_command, &submitted_arm));
    assert(submitted_command.mode == MANUAL);
    assert(submitted_command.vx_mm_s == -32768.0f);
    assert(submitted_command.vy_mm_s == 32767.0f);
    assert(submitted_command.yaw_deg_s == 0.0f && submitted_arm == ARM_HOLD);

    for (i = 0U; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        char line[] = "M,1,2,3,-\r\n";
        line[8] = keys[i][0];
        send_line(line, 200U + i);
        assert(Remote_ReadWithArm(200U + i, &submitted_command, &submitted_arm));
        assert(submitted_arm == actions[i]);
    }
    send_line("S\n", 300U);
    assert(Remote_ReadWithArm(300U, &submitted_command, &submitted_arm));
    assert(submitted_command.mode == STOP && submitted_arm == ARM_HOLD);
    send_line("A\r\n", 301U);
    assert(Remote_ReadWithArm(301U, &submitted_command, &submitted_arm));
    assert(submitted_command.mode == AUTO && submitted_arm == ARM_HOLD);

    /* Invalid lines and partial lines never refresh or submit. */
    Remote_Init(100U);
    send_line("M,10,0,0,-\n", 400U);
    send_line("M,32768,0,0,-\n", 400U);
    send_line("M,1,2,3,9\n", 401U);
    send_line("M,1,2,3,-,extra\n", 402U);
    send_line("M,1,2", 403U);
    assert(!Remote_ReadWithArm(500U, &submitted_command, &submitted_arm));

    /* Explicit reset drops a partial line without poisoning the next one. */
    Remote_Init(1000U);
    RemoteFrame_Reset();
    send_line("M,4,5,6,-\n", 404U);
    assert(Remote_ReadWithArm(404U, &submitted_command, &submitted_arm));
    assert(submitted_command.vx_mm_s == 4.0f);

    /* Overflow discards through LF, then accepts the next full line. */
    for (i = 0U; i < 60U; ++i) {
        RemoteFrame_Byte('x', 500U);
    }
    RemoteFrame_Byte('\n', 500U);
    send_line("S\n", 501U);
    assert(Remote_ReadWithArm(501U, &submitted_command, &submitted_arm));
    assert(submitted_command.mode == STOP);
    RemoteFrame_Reset();
    send_line("M,0,0,0,5", 600U);
    RemoteFrame_Byte(0, 600U);
    send_line("tail\n", 600U);
    assert(!Remote_ReadWithArm(600U, &submitted_command, &submitted_arm));
    send_line("M,0,0,0,5", 700U);
    RemoteFrame_Byte('\n', 1800U);
    assert(!Remote_ReadWithArm(1800U, &submitted_command, &submitted_arm));
    return 0;
}
