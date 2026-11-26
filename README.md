# RM_Final_Project
The iHiter RM final project

## STM32 主程序模板

`main_template.c` 是引脚未定时的控制流程模板。用 CubeMX 生成工程后，将模板接入 `main.c` 的 USER CODE 区域。在外设和底盘参数初始化后调用 `App_Init()`，在主循环调用 `App_Loop()`。

- `Remote_Read()` 提供有效且未超时的遥控指令。
- `Motor_Stop()` 停止底盘并清除旧轮速目标。
- `Motor_ApplyWheelTargets()` 将四轮角速度目标交给电机控制。
- 启用循迹前，需要统一 `Wheel_Cal()` 与 `Robot_To_Wheel()` 的接口，确认循迹状态更新和低速参数。

## L298N 电机与 PWM 舵机驱动

将 `actuators.c` 加到 Keil 工程，并把 `actuators.h` 所在目录加入头文件搜索路径。在 CubeMX 中配置好 GPIO 和定时器，完成 MX 初始化后再启动各通道。每个电机和舵机各建一份配置，电机按照 FL、FR、RL、RR 对应到实车轮位。

- **电机**：L298N 的每路 EN 接一个定时器高有效 PWM 通道，IN1/IN2 接推挽输出 GPIO，初始均为低。若驱动板用 EN 跳帽将 EN 固定为高，接 PWM 前拔掉对应跳帽。调用 `MotorDriver_Start()` 启动并检查返回值。用 `MotorDriver_SetDuty()` 给出 −1000 至 +1000 的有符号占空比，0 关闭 EN，使轮子滑行停止。初次调试架空逐轮点动，按实测调整各轮方向。上层换向时先写 0，待轮子停稳后再给反向占空比。
- **舵机**：每路使用独立的高有效 PWM 通道。定时器计数频率设为 1 MHz，ARR 设为 19999。按实测为每个关节填写 `min_us`、`max_us` 和安全的初始脉宽，其中最小值应小于最大值。调用 `ServoDriver_Start()` 并检查返回值，运行时用 `ServoDriver_SetPulse()` 设置微秒脉宽，输出会限制在该关节的范围内。电机和舵机 PWM 如需不同频率，应分配到不同定时器。
- **主循环接入**：在现有 `Motor_Stop()` 中对四轮写 0，并清除上次轮速目标。`Motor_ApplyWheelTargets()` 的输入是 rad/s，由轮速控制层换算成有符号占空比后交给电机驱动。遥控失联时由主循环调用停车接口。
