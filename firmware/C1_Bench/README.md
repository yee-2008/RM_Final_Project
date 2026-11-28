# C1 chassis bench glue

`c1_bench_glue.c` connects the shared application template to the CubeMX
handles and motor direction macros. It does not initialize peripherals and is
not called automatically. The normal startup remains stopped until a caller
explicitly supplies measured geometry, four calibrated full-duty wheel speeds,
and a nonzero remote timeout to `C1_Bench_Init()`.

Call order in the CubeMX `main()` is:

1. `HAL_Init()` and the HSI `SystemClock_Config()`.
2. Initialize the configured peripherals. The motor glue needs
   `MX_GPIO_Init()`, `MX_TIM5_Init()`, `MX_TIM9_Init()`, and
   `MX_USART3_UART_Init()`.
3. Call `C1_Bench_Init(&config)` only for an explicitly enabled bench build.
4. In the foreground loop, call `App_PollBluetooth(&huart3)` continuously and
   `App_Loop()` continuously; `App_Loop()` enforces the 10 ms control interval.

`C1_BenchConfig.geometry` must contain nonzero wheelbase, track, and radius in
millimetres, with `Sign` set to `+1` or `-1`. `max_rad_s[4]` is the measured
wheel angular speed at 1000 permille duty, in FL, FR, RL, RR order. Direction
polarity is set by each wheel's IN1/IN2 wiring and must be checked with the
chassis raised before floor operation.

## L298N 接线

| 轮位 | 驱动板通道 | EN（PWM） | 第一方向脚 | 第二方向脚 | 编码器 A / B |
| --- | --- | --- | --- | --- | --- |
| FL 左前 | 板 1 A | PA0（J2-23），TIM5_CH1 | PC0（J2-17） | PC1（J2-18） | PA5（J2-28）/ PA1（J2-24），TIM2 |
| FR 右前 | 板 1 B | PE5（J2-14），TIM9_CH1 | PC2（J2-19） | PD3（J3-23） | PC6（J3-39）/ PC7（J3-38），TIM3 |
| RL 左后 | 板 2 A | PA2（J2-25），TIM5_CH3 | PC4（J2-31） | PC5（J2-32） | PD12（J3-43）/ PD13（J3-42），TIM4 |
| RR 右后 | 板 2 B | PA3（J2-26），TIM5_CH4 | PD5（J3-21） | PD6（J3-20） | PE9（J2-37）/ PE11（J2-39），TIM1 |

A 路方向脚为 IN1/IN2，B 路方向脚为 IN3/IN4。拔掉驱动板 ENA/ENB 跳帽后接 PWM。
编码器 A/B 直接接 STM32，供电电压和信号电平按实际编码器核对。
MC520P30 标称堵转 3 A，高于 L298 每桥 2 A 的直流绝对最大额定值，台架点动应限流。
[ST L298 数据手册](https://www.st.com/resource/en/datasheet/l298.pdf)

系统与两个 APB 定时器时钟都是 16 MHz。TIM5、TIM9 的 PSC=0、ARR=799，输出 20 kHz。
四个编码器定时器在启动时开始正交计数。当前电机控制按标定转速换算占空比，速度闭环尚未接入。

## 舵机、循迹、蓝牙和音频

| 外设 | STM32 接线 |
| --- | --- |
| 底座舵机 | PB8（J3-14），TIM10_CH1 |
| 手腕舵机 | PB9（J3-13），TIM11_CH1 |
| 升降舵机 | PB14（J2-48），TIM12_CH1 |
| 夹爪舵机 | PB15（J3-48），TIM12_CH2 |
| 前排循迹 OUT1–8 | PE0、PE7、PE8、PE10、PE12、PE13、PE14、PE15（J3-12、J2-35、J2-36、J2-38、J2-40、J2-41、J2-42、J2-43） |
| 后排循迹 OUT1–8 | PD0、PD1、PB6、PD7、PD8、PD9、PD10、PD11（J3-26、J3-25、J3-16、J3-19、J3-47、J3-46、J3-45、J3-44） |
| HC-05 | RXD 接 PB10（J2-44），TXD 接 PB11（J2-45），USART3 9600 8N1。PE2（J2-11）预留为输入，按模块额外脚的用途确认方向。 |
| 板载 TF 卡 | SDIO D0–D3 为 PC8–PC11，CLK 为 PC12，CMD 为 PD2。 |
| I²S2 功放 | BCLK 接 PB13（J2-47），WS 接 PB12（J2-46），DIN 接 PC3（J2-20）。三线输出，MCLK 禁用。 |

舵机计数步长 1 µs，PSC=15、ARR=19999，输出 50 Hz。舵机脉宽、底盘速度和几何参数在实车标定后填入
`firmware/cubemx/Core/Inc/c1_settings.h`，再启用对应子系统。

SDIO 时钟来自主 PLL 的 48 MHz 输出，FatFs 在尝试挂载时初始化卡并切换到 4 位模式。
板载卡座没有独立的卡检测信号，PB7 配内部下拉供 CubeMX 的 FatFs 检测逻辑使用，实物悬空即可。
I²S2 配置为 48 kHz、16 位主机发送，发送 DMA 为 DMA1 Stream4 Channel0 环形模式。
工程启动时初始化存储和音频接口，现有 `audio.c` 播放逻辑尚未接入运行流程。
