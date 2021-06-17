/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#define Flash_CS_Pin GPIO_PIN_4
#define Flash_CS_GPIO_Port GPIOA
#define RS4851_Pin GPIO_PIN_13
#define RS4851_GPIO_Port GPIOB
#define RS4852_Pin GPIO_PIN_14
#define RS4852_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_0
#define LED_2_GPIO_Port GPIOE
#define LED_3_Pin GPIO_PIN_1
#define LED_3_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */
  // #define RS485_TX_MODE HAL_GPIO_WritePin(RS4851_GPIO_Port,RS4851_Pin,GPIO_PIN_SET)
  // #define RS485_RX_MODE HAL_GPIO_WritePin(RS4851_GPIO_Port,RS4851_Pin,GPIO_PIN_RESET)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
