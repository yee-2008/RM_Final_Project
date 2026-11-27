#ifndef REMOTE_H
#define REMOTE_H

#include <stdint.h>

typedef enum { STOP, MANUAL, AUTO } DriveMode;

typedef enum {
    ARM_HOLD, ARM_BASE_POS, ARM_BASE_NEG, ARM_WRIST_POS, ARM_WRIST_NEG,
    ARM_LIFT_UP, ARM_LIFT_DOWN, ARM_CLOSE, ARM_OPEN
} ArmAction;

typedef struct {
    DriveMode mode;
    float vx_mm_s;
    float vy_mm_s;
    float yaw_deg_s;
} DriveCommand;

/* Configure a nonzero link timeout in milliseconds. */
void Remote_Init(uint32_t timeout_ms);

/* Submit one fully decoded command and its receive time. Returns 1 if accepted.
 * Call from the main thread; this module does not define a transport protocol.
 */
int Remote_Submit(const DriveCommand *command, uint32_t received_ms);

/* Returns 1 with the latest command while the link is fresh, otherwise 0.
 * STOP is a valid command; 0 means startup or link loss.
 */
int Remote_Read(uint32_t now_ms, DriveCommand *command);

int Remote_SubmitWithArm(const DriveCommand *command, ArmAction arm, uint32_t received_ms);
int Remote_ReadWithArm(uint32_t now_ms, DriveCommand *command, ArmAction *arm);
void Remote_Disconnect(void);

#endif
