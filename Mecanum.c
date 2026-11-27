/*这是把底盘速度解算到四个轮子转速的代码
约定前进方向为x轴，使用右手系，绕z正向旋转的角速度约定为正（就是逆时针转）
由此得到底盘四个轮子相对中心的坐标
姑且按照线下培训的代码格式，规定全局结构体变量Wheel_Data，解算函数可直接访问
Whell_Data可以像Motor_Data一样用函数初始化
注意方向！这里的AngelVelDes一律取底盘向前为正，实际进行PID解算时应该根据电机朝向和正反转规定取符号*/

/*底盘物理参数由工图数据填写，单位mm，速度暂定mm/s，角速度deg/s*/ 
#include "wheel.h"

#define PI 3.1416f

float FL_AngleVelDes;   // 左前轮期望角速度，rad/s
float FR_AngleVelDes;   // 右前轮期望角速度，rad/s
float RL_AngleVelDes;   // 左后轮期望角速度，rad/s
float RR_AngleVelDes;   // 右后轮期望角速度，rad/s

float Robot_Vx;
float Robot_Vy;
float Robot_AngleVel; 
Wheel_DataTypedef Wheel_Data;

void Robot_To_Wheel(float Vx, float Vy, float AngleVel)
{
    /* 几何参数：半轴距 a、半轮距 b、轮半径 R，统一为 float */
    float a = Wheel_Data.Wheelbase / 2.0f;   // mm
    float b = Wheel_Data.Track     / 2.0f;   // mm
    float R = (float)Wheel_Data.Radius;      // mm

    if (!Wheel_Data.Radius || !Wheel_Data.Wheelbase || !Wheel_Data.Track ||
        (Wheel_Data.Sign != 1 && Wheel_Data.Sign != -1)) {
        FL_AngleVelDes = FR_AngleVelDes = RL_AngleVelDes = RR_AngleVelDes = 0;
        return;
    }

    /* 输入速度转为 float，角速度 deg/s -> rad/s */
    float w  = (float)AngleVel * PI / 180.0f; // rad/s，逆时针为正

    /* 归一化符号：s_FL = s_RR = Sign，s_FR = s_RL = -Sign */
    float s = (float)Wheel_Data.Sign;

    /* 单轮线速度（mm/s）：v_i = vx + s_i*vy + w*(s_i*x_i - y_i)
     * FL: x=+a, y=+b, s_i=+s
     * FR: x=+a, y=-b, s_i=-s
     * RL: x=-a, y=+b, s_i=-s
     * RR: x=-a, y=-b, s_i=+s
     */
    float v_FL = Vx + ( s)*Vy + w*(( s)*( a) - ( b));
    float v_FR = Vx + (-s)*Vy + w*((-s)*( a) - (-b));
    float v_RL = Vx + (-s)*Vy + w*((-s)*(-a) - ( b));
    float v_RR = Vx + ( s)*Vy + w*(( s)*(-a) - (-b));

    /* 线速度 -> 轮角速度（rad/s），保存到全局变量 */
    FL_AngleVelDes = v_FL / R;
    FR_AngleVelDes = v_FR / R;
    RL_AngleVelDes = v_RL / R;
    RR_AngleVelDes = v_RR / R;
}

void Wheel_To_Robot(float FL_AngleVelFB, float FR_AngleVelFB, float RL_AngleVelFB, float RR_AngleVelFB)
{
    float a = Wheel_Data.Wheelbase / 2.0f;   // mm
    float b = Wheel_Data.Track     / 2.0f;   // mm
    float R = (float)Wheel_Data.Radius;      // mm
    float s = (float)Wheel_Data.Sign;        // O型=+1，X型=-1

    /* 旋转项分母：sa - b。O型且 a=b 时为0，退化，需防止除以0 */
    float rot_den = s * a - b;

    /* 车体线速度，mm/s */
    Robot_Vx = R * 0.25f * (FL_AngleVelFB + FR_AngleVelFB + RL_AngleVelFB + RR_AngleVelFB);

    Robot_Vy = R * s * 0.25f * (FL_AngleVelFB - FR_AngleVelFB - RL_AngleVelFB + RR_AngleVelFB);

    /* 角速度：先算 rad/s，再转 deg/s */
    if (rot_den > 1e-6f || rot_den < -1e-6f)
	{
        float w_rad = R * 0.25f / rot_den * (FL_AngleVelFB - FR_AngleVelFB + RL_AngleVelFB - RR_AngleVelFB);
        Robot_AngleVel = w_rad * 180.0f / PI;   // deg/s
    }
	else{
        /* 退化：旋转不可由轮速唯一确定 */
        Robot_AngleVel = 0.0f;
    }
}
