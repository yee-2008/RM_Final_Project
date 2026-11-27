"""Build the C1 manual-control integration test using GCC and a minimal HAL."""
import subprocess
import tempfile
from pathlib import Path

with tempfile.TemporaryDirectory() as temp:
    path = Path(temp)
    (path / "stm32f4xx_hal.h").write_text("""
#ifndef MOCK_HAL_H
#define MOCK_HAL_H
#include <stdint.h>
typedef struct { int unused; } TIM_HandleTypeDef, GPIO_TypeDef;
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
typedef struct { uint32_t SR, DR; } USART_TypeDef;
typedef struct { USART_TypeDef *Instance; } UART_HandleTypeDef;
#define USART_SR_PE 1
#define USART_SR_FE 2
#define USART_SR_NE 4
#define USART_SR_ORE 8
#define USART_SR_RXNE 32
uint32_t HAL_GetTick(void);
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *timer, uint32_t channel);
#endif
""")
    (path / "main.h").write_text('#include "stm32f4xx_hal.h"\n')
    exe = path / "arm_test.exe"
    subprocess.run(["gcc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "-I.", "-I" + temp, "arm.c", "remote.c", "remote_frame.c",
                    "tests/arm_test.c", "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
