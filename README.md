# RM_Final_Project
The iHiter RM final project

## STM32 主程序模板

`main_template.c` 是引脚未定时的控制流程模板。用 CubeMX 生成工程后，将模板接入 `main.c` 的 USER CODE 区域。在外设和底盘参数初始化后调用 `App_Init()`，在主循环调用 `App_Loop()`。

- `Remote_Read()` 提供有效且未超时的遥控指令。
- `Motor_Stop()` 停止底盘并清除旧轮速目标。
- `Motor_ApplyWheelTargets()` 将四轮角速度目标交给电机控制。
- 启用循迹前，需要统一 `Wheel_Cal()` 与 `Robot_To_Wheel()` 的接口，确认循迹状态更新和低速参数。
