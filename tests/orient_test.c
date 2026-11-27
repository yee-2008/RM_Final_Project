#include <assert.h>
#include "orient.h"
#include "wheel.h"

static unsigned pins[3];

int HAL_GPIO_ReadPin(int port, int mask)
{
    return (pins[port] & mask) != 0;
}

int main(void)
{
    Robot_To_Wheel(100, 0, 0);
    assert(FL_AngleVelDes == 0);
    Wheel_Data = (Wheel_DataTypedef){200, 180, 30, -1};
    OrientConfigure(120, 30, 10, 2);
    OrientInit();

    pins[0] = 7U << 13;
    autoControl();
    assert(orient_state == TURNRIGHT && FL_AngleVelDes * FR_AngleVelDes < 0);
    pins[0] = 1U << 8;
    autoControl();
    assert(orient_state == STRAIGHT && FL_AngleVelDes != FR_AngleVelDes);
    pins[0] = 1 | (1U << 7) | (1U << 15);
    autoControl();
    assert(orient_state == CROSSLINE && FL_AngleVelDes == 4);
    pins[2] = 3 | (1U << 11);
    autoControl();
    assert(orient_state == END && FL_AngleVelDes == 0);
    pins[0] = pins[2] = 0;
    autoControl();
    assert(orient_state == END);

    OrientInit();
    pins[2] = 1;
    float errors[2];
    GetPosition(errors);
    assert(errors[0] == FAIL && errors[1] == FAIL);
    pins[2] = 0;
    autoControl();
    assert(orient_state == LOSE && RR_AngleVelDes == 0);
}
