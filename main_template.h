#ifndef MAIN_TEMPLATE_H
#define MAIN_TEMPLATE_H

#include "main.h"

void App_Init(uint32_t remote_timeout_ms);
void App_PollBluetooth(UART_HandleTypeDef *bluetooth);
void App_Loop(void);

#endif
