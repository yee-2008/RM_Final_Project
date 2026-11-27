# C1 chassis bench glue

`c1_bench_glue.c` connects the shared application template to the CubeMX
handles and motor direction macros. It does not initialize peripherals and is
not called automatically. The normal startup remains stopped until a caller
explicitly supplies measured geometry, four calibrated full-duty wheel speeds,
and a nonzero remote timeout to `C1_Bench_Init()`.

Call order in the CubeMX `main()` is:

1. `HAL_Init()` and the HSI `SystemClock_Config()`.
2. `MX_GPIO_Init()`, `MX_TIM5_Init()`, and `MX_USART3_UART_Init()`.
3. Call `C1_Bench_Init(&config)` only for an explicitly enabled bench build.
4. In the foreground loop, call `App_PollBluetooth(&huart3)` continuously and
   `App_Loop()` continuously; `App_Loop()` enforces the 10 ms control interval.

`C1_BenchConfig.geometry` must contain nonzero wheelbase, track, and radius in
millimetres, with `Sign` set to `+1` or `-1`. `max_rad_s[4]` is the measured
wheel angular speed at 1000 permille duty, in FL, FR, RL, RR order. Direction
polarity is set by each wheel's IN1/IN2 wiring and must be checked with the
chassis raised before floor operation.

## Proposed pin map

| Function | Proposed STM32 pin |
| --- | --- |
| FL/FR/RL/RR PWM | TIM5 CH1/CH2/CH3/CH4: PA0/PA1/PA2/PA3, AF2 |
| FL direction IN1/IN2 | PC0/PC1 |
| FR direction IN1/IN2 | PC2/PC3 |
| RL direction IN1/IN2 | PC4/PC5 |
| RR direction IN1/IN2 | PD12/PD13 |
| Servo 1–4 | TIM8 CH1/CH2/CH3/CH4: PC6/PC7/PC8/PC9, AF3 |
| Bluetooth UART | USART3 TX/RX: PB10/PB11, AF7, 9600 8N1 proposed |

The installed CubeMX MCU database confirms these signals exist on the
STM32F407V(E-G)Tx LQFP100 device. This is a proposed MCU pin assignment, not a
confirmation of the C1 board connector wiring. Servo output is not started by
this glue. With HSI at 16 MHz and APB1 divider 1, the CubeMX motor timer setup
PSC=0 and ARR=799 yields 20 kHz; USART3 is configured for 9600 8N1. Verify the
actual module baud rate and board wiring before enabling the bench call.
