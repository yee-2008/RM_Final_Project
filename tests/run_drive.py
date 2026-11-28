"""Exercise the C1 wheel speed controller with simulated encoder registers."""
import subprocess
import tempfile
from pathlib import Path

with tempfile.TemporaryDirectory() as tmp:
    path = Path(tmp)
    (path / "stm32f4xx_hal.h").write_text("""
#include <stdint.h>
#include <stddef.h>
typedef struct { uint32_t cnt, arr; } TIM_HandleTypeDef;
typedef struct { int unused; } GPIO_TypeDef;
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
#define __HAL_TIM_GET_COUNTER(t) ((t)->cnt)
#define __HAL_TIM_GET_AUTORELOAD(t) ((t)->arr)
uint32_t HAL_GetTick(void);
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *, uint32_t);
""")
    exe = path / "drive_test.exe"
    subprocess.run(["gcc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "-I.", "-I" + tmp, "drive.c", "tests/drive_test.c",
                    "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
