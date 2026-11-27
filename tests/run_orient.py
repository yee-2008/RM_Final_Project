"""Run with Python and GCC from the repository root."""
from pathlib import Path
import subprocess
import tempfile

with tempfile.TemporaryDirectory() as temp:
    root = Path(temp)
    (root / "main.h").write_text("#include <stdint.h>\n")
    pins = "".join(f"#define GPIO_PIN_{i} {1 << i}\n" for i in range(16))
    (root / "gpio.h").write_text(
        "#define GPIOE 0\n#define GPIOB 1\n#define GPIOD 2\n"
        "int HAL_GPIO_ReadPin(int port, int mask);\n" + pins
    )
    exe = root / "orient_test.exe"
    subprocess.run(["gcc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "-I.", "-I" + temp, "orient.c", "pid.c", "Mecanum.c",
                    "tests/orient_test.c", "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
