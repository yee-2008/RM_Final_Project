#include "main.h"
#include "gpio.h"
#include "orient.h"
#include "pid.h"
#include "wheel.h"

#define LIMIT 3     //状态转变的传感器触发数阈值
static float forwardSpeed; /* m/s */
static float turnSpeed;    /* deg/s */
static PID autoPosition, autoDirection;

void OrientConfigure(float forward_mm_s, float turn_deg_s,
                     float lateral_gain_mm_s, float yaw_gain_deg_s)
{
    forwardSpeed = forward_mm_s > 0 ? forward_mm_s / 1000.0f : 0;
    turnSpeed = turn_deg_s > 0 ? turn_deg_s : 0;
    PID_Init(&autoPosition, lateral_gain_mm_s / 1000.0f, 0, 0, 0,
             forwardSpeed, 0, 0, 0, forwardSpeed);
    PID_Init(&autoDirection, yaw_gain_deg_s, 0, 0, 0, turnSpeed, 0, 0, 0, turnSpeed);
}

OrientState orient_state=STRAIGHT;

// typedef enum OrientState {
//     STRAIGHT=0,         //直行
//     TURNLEFT,           //左转
//     TURNRIGHT,          //右转
//     CROSSLINE,          //前轮经过终点线
//     END,                //后轮经过终点线,寻迹结束
// } OrientState;

//读取前排红外循迹模块
void ReadForward(uint8_t position[]){
    position[0]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_0);
    position[1]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_7);
    position[2]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_8);
    position[3]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_10);
    position[4]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_12);
    position[5]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_13);
    position[6]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14);
    position[7]=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15);
}

//读取后排红外循迹模块
void ReadBack(uint8_t position[]){
    position[0]=HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_0);
    position[1]=HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_1);
    position[2]=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6);
    position[3]=HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_7);
    position[4]=HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_8);
    position[5]=HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_9);
    position[6]=HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_10);
    position[7]=HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_11);
}

//循迹初始化，寻迹状态重置为直行，PID参数初始化
void OrientInit(void){
    orient_state=STRAIGHT;
    autoPosition.error_all = autoPosition.error_last = 0;
    autoDirection.error_all = autoDirection.error_last = 0;
    return;
}

//计算小车相对轨迹的偏差，并且判断当前循迹状态
//状态为STRAIGHT时，返回两个值
//第一项为小车车头读取出的偏差，车偏左为正，第二项为车头和车尾读出的偏差之差（车头-车尾)
//偏差是根据权重算出的，从左往右(以前进方向区分左右)1~8号传感器权重为{-3.5,-2.5,-1,5,-0.5,0.5,1.5,2.5,3.5}
//算法为计算被触发的传感器的权重的平均值
void GetPosition(float receiver[]){
    uint8_t forward[8]={0};
    ReadForward(forward);
    uint8_t backward[8]={0};
    ReadBack(backward);
    int count_forward=0,count_backward=0;
    float total_forward=0,total_backward=0;
    for (int i=0;i<8;i++){
        if (forward[i]!=0){
            count_forward++;
            total_forward+=i-3.5;
        }
        if (backward[i]!=0){
            count_backward++;
            total_backward+=-i+3.5;
        }
    }
    receiver[0] = count_forward ? total_forward / count_forward : FAIL;
    receiver[1] = count_forward && count_backward
        ? receiver[0] - total_backward / count_backward : FAIL;
    if (orient_state != END && !count_forward && !count_backward) {
        orient_state = LOSE;
        return;
    }
    switch (orient_state){
        case STRAIGHT:
            if (count_forward>=LIMIT){
                if (forward[0]!=0&&forward[7]!=0){
                    orient_state=CROSSLINE;
                    return;
                }
                if (total_forward>0){
                    orient_state=TURNRIGHT;
                    return;
                }
                if (total_forward<0){
                    orient_state=TURNLEFT;
                    return;
                }
            }

            break;
        case CROSSLINE:
            if (count_backward>=LIMIT&&backward[0]!=0&&backward[7]!=0){
                orient_state=END;
                return;
            }
            break;


        case TURNRIGHT:
        case TURNLEFT:
            if (count_forward!=0&&count_forward<LIMIT){
                orient_state=STRAIGHT;
                return;
            }
            break;
        case END:
        case LOSE:
            break;
    }
}

//计算小车各个电机的目标速度，这里只负责Y方向和前进方向的位置环(方向定义与麦轮解算相同),电机速度环的控制就交给负责电机的同志了(^_^)
void autoControl(void){
    float VelocityY = 0;         //  m/s
    float AngelVelocity = 0;     //  deg/s
    float VelocityX = 0;                //  m/s
    float position[2];
    GetPosition(position);
    switch (orient_state)
    {
    case STRAIGHT:
        VelocityX=forwardSpeed;
        if (position[0]!=FAIL){
            VelocityY=calculate(&autoPosition,position[0]);
        }
        if (position[1]!=FAIL){
            AngelVelocity=calculate(&autoDirection,position[1]);
        }
        break;


    case TURNLEFT:
        VelocityX=0;
        VelocityY=0;
        AngelVelocity=turnSpeed;
        break;


    case TURNRIGHT:
        VelocityX=0;
        VelocityY=0;
        AngelVelocity=-turnSpeed;
        break;


    case CROSSLINE:
        VelocityX=forwardSpeed;
        VelocityY=0;
        AngelVelocity=0;
        break;


    case END:
    case LOSE:
        VelocityX=0;
        VelocityY=0;
        AngelVelocity=0;
        break;

    }
    Robot_To_Wheel(VelocityX*1000,VelocityY*1000,AngelVelocity);
    return;
}