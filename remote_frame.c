#include "remote_frame.h"

#include "remote.h"

#include <stddef.h>

#define REMOTE_FRAME_MAX 48U

static char frame[REMOTE_FRAME_MAX];
static size_t frame_length;
static uint32_t first_byte_ms;
static int discard_line;

void RemoteFrame_Reset(void)
{
    frame_length = 0U;
    discard_line = 0;
    Remote_Disconnect();
}

static int parse_int16(const char **cursor, int16_t *value, char delimiter)
{
    const char *p = *cursor;
    int negative = 0;
    int32_t parsed = 0;
    int digits = 0;

    if (*p == '-') {
        negative = 1;
        ++p;
    } else if (*p == '+') {
        ++p;
    }
    while (*p >= '0' && *p <= '9') {
        parsed = parsed * 10 + (*p - '0');
        if (parsed > 32768) {
            return 0;
        }
        ++p;
        ++digits;
    }
    if (digits == 0 || *p != delimiter) {
        return 0;
    }
    parsed = negative ? -parsed : parsed;
    if (parsed < -32768 || parsed > 32767) {
        return 0;
    }
    *value = (int16_t)parsed;
    *cursor = p + 1;
    return 1;
}

static int parse_frame(char *line, DriveCommand *command, ArmAction *arm)
{
    const char *p = line;
    int16_t vx;
    int16_t vy;
    int16_t yaw;

    if (line[0] == 'S' && line[1] == '\0') {
        command->mode = STOP;
        command->vx_mm_s = 0.0f;
        command->vy_mm_s = 0.0f;
        command->yaw_deg_s = 0.0f;
        *arm = ARM_HOLD;
        return 1;
    }
    if (line[0] == 'A' && line[1] == '\0') {
        command->mode = AUTO;
        command->vx_mm_s = 0.0f;
        command->vy_mm_s = 0.0f;
        command->yaw_deg_s = 0.0f;
        *arm = ARM_HOLD;
        return 1;
    }
    if (*p++ != 'M' || *p++ != ',' ||
        !parse_int16(&p, &vx, ',') ||
        !parse_int16(&p, &vy, ',') ||
        !parse_int16(&p, &yaw, ',')) {
        return 0;
    }

    command->mode = MANUAL;
    command->vx_mm_s = (float)vx;
    command->vy_mm_s = (float)vy;
    command->yaw_deg_s = (float)yaw;
    switch (*p++) {
    case '-': *arm = ARM_HOLD; break;
    case '1': *arm = ARM_BASE_POS; break;
    case '2': *arm = ARM_BASE_NEG; break;
    case '3': *arm = ARM_WRIST_POS; break;
    case '4': *arm = ARM_WRIST_NEG; break;
    case '5': *arm = ARM_LIFT_UP; break;
    case '6': *arm = ARM_LIFT_DOWN; break;
    case '7': *arm = ARM_CLOSE; break;
    case '8': *arm = ARM_OPEN; break;
    default: return 0;
    }
    return *p == '\0';
}

static void submit_line(uint32_t received_ms)
{
    DriveCommand command;
    ArmAction arm;

    frame[frame_length] = '\0';
    if (frame_length > 0U && frame[frame_length - 1U] == '\r') {
        frame[--frame_length] = '\0';
    }
    if (parse_frame(frame, &command, &arm)) {
        (void)Remote_SubmitWithArm(&command, arm, received_ms);
    }
    frame_length = 0U;
}

void RemoteFrame_Byte(uint8_t byte, uint32_t received_ms)
{
    if (byte == '\n') {
        if (!discard_line) {
            submit_line(first_byte_ms);
        } else {
            frame_length = 0U;
            discard_line = 0;
        }
        return;
    }
    if (discard_line) {
        return;
    }
    if (byte == 0U || frame_length >= sizeof(frame) - 1U) {
        frame_length = 0U;
        discard_line = 1;
        return;
    }
    if (frame_length == 0U) first_byte_ms = received_ms;
    frame[frame_length++] = (char)byte;
}
