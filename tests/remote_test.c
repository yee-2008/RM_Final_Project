#include "remote.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>

int main(void)
{
    DriveCommand out;
    DriveCommand manual = { MANUAL, 120.0f, -30.0f, 45.0f };
    DriveCommand stop = { STOP, 0.0f, 0.0f, 0.0f };
    DriveCommand invalid = { (DriveMode)99, 1.0f, 2.0f, 3.0f };

    Remote_Init(100U);
    assert(!Remote_Read(0U, &out));
    assert(!Remote_Submit(&invalid, 0U));
    assert(Remote_Submit(&manual, 0U));
    assert(Remote_Read(99U, &out));
    assert(out.mode == MANUAL && out.vx_mm_s == 120.0f);
    assert(!Remote_Read(100U, &out));
    assert(!Remote_Read(101U, &out));

    /* Link recovers only after an explicitly submitted fresh command. */
    assert(Remote_Submit(&manual, 102U));
    assert(Remote_Read(102U, &out));

    /* STOP is accepted and returned as a live command, distinct from loss. */
    assert(Remote_Submit(&stop, 103U));
    assert(Remote_Read(103U, &out));
    assert(out.mode == STOP);
    assert(!Remote_Read(203U, &out));

    /* Unsigned elapsed time handles one tick counter wrap. */
    assert(Remote_Submit(&manual, UINT32_MAX - 10U));
    assert(Remote_Read(20U, &out));
    assert(!Remote_Read(90U, &out));

    invalid.mode = MANUAL;
    invalid.vx_mm_s = NAN;
    assert(!Remote_Submit(&invalid, 300U));

    Remote_Init(0U);
    assert(!Remote_Submit(&manual, 0U));
    assert(!Remote_Read(0U, &out));
    return 0;
}
