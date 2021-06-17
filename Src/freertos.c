/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include "user_mb_app.h"
#include <PWMServoDriver.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern SPI_HandleTypeDef hspi1;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t MasterTaskHandle;
const osThreadAttr_t MasterTask_attributes = {
    .name = "MasterTask",
    .priority = (osPriority_t)osPriorityNormal,
    .stack_size = 128 * 4};
osThreadId_t SlaveTaskHandle;
const osThreadAttr_t SlaveTask_attributes = {
    .name = "SlaveTask",
    .priority = (osPriority_t)osPriorityNormal,
    .stack_size = 128 * 4};
/* USER CODE END Variables */
/* Definitions for SYSTask */
osThreadId_t SYSTaskHandle;
const osThreadAttr_t SYSTask_attributes = {
  .name = "SYSTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void MasterTask(void *argument);
void SlaveTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartSYSTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of SYSTask */
  SYSTaskHandle = osThreadNew(StartSYSTask, NULL, &SYSTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  MasterTaskHandle = osThreadNew(MasterTask, NULL, &MasterTask_attributes);
  SlaveTaskHandle = osThreadNew(SlaveTask, NULL, &SlaveTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartSYSTask */
/**
  * @brief  Function implementing the SYSTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSYSTask */
void StartSYSTask(void *argument)
{
  /* USER CODE BEGIN StartSYSTask */
  uint16_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  /* Infinite loop */
  for (;;)
  {
    // eMBMasterReqReadHoldingRegister(1, 0, 10, 100);
    // eMBMasterReqWriteMultipleHoldingRegister(1, 0, 10, data, 100);
    HAL_GPIO_TogglePin(LED_2_GPIO_Port, LED_2_Pin);
    osDelay(500);
  }
  /* USER CODE END StartSYSTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void MasterTask(void *argument)
{
  eMBMasterInit(MB_RTU, 3, 115200, MB_PAR_NONE);
  eMBMasterEnable();
  while (1)
  {
    eMBMasterPoll();
  }
}
void SlaveTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  eMBInit(MB_RTU, 0x01, 2, 115200, MB_PAR_NONE);
  eMBEnable();
  /* Infinite loop */
  for (;;)
  {
    eMBPoll();
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
