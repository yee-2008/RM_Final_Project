/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BT_AUX_Pin GPIO_PIN_2
#define BT_AUX_GPIO_Port GPIOE
#define FL_IN1_Pin GPIO_PIN_0
#define FL_IN1_GPIO_Port GPIOC
#define FL_IN2_Pin GPIO_PIN_1
#define FL_IN2_GPIO_Port GPIOC
#define FR_IN1_Pin GPIO_PIN_2
#define FR_IN1_GPIO_Port GPIOC
#define RL_IN1_Pin GPIO_PIN_4
#define RL_IN1_GPIO_Port GPIOC
#define RL_IN2_Pin GPIO_PIN_5
#define RL_IN2_GPIO_Port GPIOC
#define LINE_F1_Pin GPIO_PIN_7
#define LINE_F1_GPIO_Port GPIOE
#define LINE_F2_Pin GPIO_PIN_8
#define LINE_F2_GPIO_Port GPIOE
#define LINE_F3_Pin GPIO_PIN_10
#define LINE_F3_GPIO_Port GPIOE
#define LINE_F4_Pin GPIO_PIN_12
#define LINE_F4_GPIO_Port GPIOE
#define LINE_F5_Pin GPIO_PIN_13
#define LINE_F5_GPIO_Port GPIOE
#define LINE_F6_Pin GPIO_PIN_14
#define LINE_F6_GPIO_Port GPIOE
#define LINE_F7_Pin GPIO_PIN_15
#define LINE_F7_GPIO_Port GPIOE
#define LINE_R4_Pin GPIO_PIN_8
#define LINE_R4_GPIO_Port GPIOD
#define LINE_R5_Pin GPIO_PIN_9
#define LINE_R5_GPIO_Port GPIOD
#define LINE_R6_Pin GPIO_PIN_10
#define LINE_R6_GPIO_Port GPIOD
#define LINE_R7_Pin GPIO_PIN_11
#define LINE_R7_GPIO_Port GPIOD
#define LINE_R0_Pin GPIO_PIN_0
#define LINE_R0_GPIO_Port GPIOD
#define LINE_R1_Pin GPIO_PIN_1
#define LINE_R1_GPIO_Port GPIOD
#define FR_IN2_Pin GPIO_PIN_3
#define FR_IN2_GPIO_Port GPIOD
#define RR_IN1_Pin GPIO_PIN_5
#define RR_IN1_GPIO_Port GPIOD
#define RR_IN2_Pin GPIO_PIN_6
#define RR_IN2_GPIO_Port GPIOD
#define LINE_R3_Pin GPIO_PIN_7
#define LINE_R3_GPIO_Port GPIOD
#define LINE_R2_Pin GPIO_PIN_6
#define LINE_R2_GPIO_Port GPIOB
#define SD_DETECT_Pin GPIO_PIN_7
#define SD_DETECT_GPIO_Port GPIOB
#define LINE_F0_Pin GPIO_PIN_0
#define LINE_F0_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
