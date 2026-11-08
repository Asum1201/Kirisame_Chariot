/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    CAN_HandleTypeDef *hcan;
    uint32_t id;
    uint8_t len;
    uint8_t *buf_data;
}can_device_transmit_member;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for CAN1_Sending_Service */
osThreadId_t CAN1_Sending_ServiceHandle;
const osThreadAttr_t CAN1_Sending_Service_attributes = {
  .name = "CAN1_Sending_Service",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CAN2_Sending_Service */
osThreadId_t CAN2_Sending_ServiceHandle;
const osThreadAttr_t CAN2_Sending_Service_attributes = {
  .name = "CAN2_Sending_Service",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for DJI_Motor_Service */
osThreadId_t DJI_Motor_ServiceHandle;
const osThreadAttr_t DJI_Motor_Service_attributes = {
  .name = "DJI_Motor_Service",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for LED_R */
osThreadId_t LED_RHandle;
const osThreadAttr_t LED_R_attributes = {
  .name = "LED_R",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LED_G */
osThreadId_t LED_GHandle;
const osThreadAttr_t LED_G_attributes = {
  .name = "LED_G",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LED_B */
osThreadId_t LED_BHandle;
const osThreadAttr_t LED_B_attributes = {
  .name = "LED_B",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for INS_Task */
osThreadId_t INS_TaskHandle;
const osThreadAttr_t INS_Task_attributes = {
  .name = "INS_Task",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for MotorAssignment */
osThreadId_t MotorAssignmentHandle;
const osThreadAttr_t MotorAssignment_attributes = {
  .name = "MotorAssignment",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for PMTask */
osThreadId_t PMTaskHandle;
const osThreadAttr_t PMTask_attributes = {
  .name = "PMTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for can1_send_fifo */
osMessageQueueId_t can1_send_fifoHandle;
const osMessageQueueAttr_t can1_send_fifo_attributes = {
  .name = "can1_send_fifo"
};
/* Definitions for can2_send_fifo */
osMessageQueueId_t can2_send_fifoHandle;
const osMessageQueueAttr_t can2_send_fifo_attributes = {
  .name = "can2_send_fifo"
};
/* Definitions for CAN1CountingSem */
osSemaphoreId_t CAN1CountingSemHandle;
const osSemaphoreAttr_t CAN1CountingSem_attributes = {
  .name = "CAN1CountingSem"
};
/* Definitions for CAN2CountingSem */
osSemaphoreId_t CAN2CountingSemHandle;
const osSemaphoreAttr_t CAN2CountingSem_attributes = {
  .name = "CAN2CountingSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void can1_sending_service(void *argument);
void can2_sending_service(void *argument);
void DJI_motor_service(void *argument);
void led_r(void *argument);
void led_g(void *argument);
void led_b(void *argument);
void StartINSTask(void *argument);
void StartM3508Task(void *argument);
void StartPMTask(void *argument);

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

  /* Create the semaphores(s) */
  /* creation of CAN1CountingSem */
  CAN1CountingSemHandle = osSemaphoreNew(3, 3, &CAN1CountingSem_attributes);

  /* creation of CAN2CountingSem */
  CAN2CountingSemHandle = osSemaphoreNew(3, 3, &CAN2CountingSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of can1_send_fifo */
  can1_send_fifoHandle = osMessageQueueNew (16, sizeof(can_device_transmit_member), &can1_send_fifo_attributes);

  /* creation of can2_send_fifo */
  can2_send_fifoHandle = osMessageQueueNew (16, sizeof(can_device_transmit_member), &can2_send_fifo_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of CAN1_Sending_Service */
  CAN1_Sending_ServiceHandle = osThreadNew(can1_sending_service, NULL, &CAN1_Sending_Service_attributes);

  /* creation of CAN2_Sending_Service */
  CAN2_Sending_ServiceHandle = osThreadNew(can2_sending_service, NULL, &CAN2_Sending_Service_attributes);

  /* creation of DJI_Motor_Service */
  DJI_Motor_ServiceHandle = osThreadNew(DJI_motor_service, NULL, &DJI_Motor_Service_attributes);

  /* creation of LED_R */
  LED_RHandle = osThreadNew(led_r, NULL, &LED_R_attributes);

  /* creation of LED_G */
  LED_GHandle = osThreadNew(led_g, NULL, &LED_G_attributes);

  /* creation of LED_B */
  LED_BHandle = osThreadNew(led_b, NULL, &LED_B_attributes);

  /* creation of INS_Task */
  INS_TaskHandle = osThreadNew(StartINSTask, NULL, &INS_Task_attributes);

  /* creation of MotorAssignment */
  MotorAssignmentHandle = osThreadNew(StartM3508Task, NULL, &MotorAssignment_attributes);

  /* creation of PMTask */
  PMTaskHandle = osThreadNew(StartPMTask, NULL, &PMTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_can1_sending_service */
/**
  * @brief  Function implementing the CAN1_Sending_Service thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_can1_sending_service */
__weak void can1_sending_service(void *argument)
{
  /* USER CODE BEGIN can1_sending_service */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END can1_sending_service */
}

/* USER CODE BEGIN Header_can2_sending_service */
/**
* @brief Function implementing the CAN2_Sending_Service thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_can2_sending_service */
__weak void can2_sending_service(void *argument)
{
  /* USER CODE BEGIN can2_sending_service */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END can2_sending_service */
}

/* USER CODE BEGIN Header_DJI_motor_service */
/**
* @brief Function implementing the DJI_Motor_Service thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DJI_motor_service */
__weak void DJI_motor_service(void *argument)
{
  /* USER CODE BEGIN DJI_motor_service */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DJI_motor_service */
}

/* USER CODE BEGIN Header_led_r */
/**
* @brief Function implementing the LED_R thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_led_r */
__weak void led_r(void *argument)
{
  /* USER CODE BEGIN led_r */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END led_r */
}

/* USER CODE BEGIN Header_led_g */
/**
* @brief Function implementing the LED_G thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_led_g */
__weak void led_g(void *argument)
{
  /* USER CODE BEGIN led_g */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END led_g */
}

/* USER CODE BEGIN Header_led_b */
/**
* @brief Function implementing the LED_B thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_led_b */
__weak void led_b(void *argument)
{
  /* USER CODE BEGIN led_b */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END led_b */
}

/* USER CODE BEGIN Header_StartINSTask */
/**
* @brief Function implementing the INS_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartINSTask */
__weak void StartINSTask(void *argument)
{
  /* USER CODE BEGIN StartINSTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartINSTask */
}

/* USER CODE BEGIN Header_StartM3508Task */
/**
* @brief Function implementing the MotorAssignment thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartM3508Task */
__weak void StartM3508Task(void *argument)
{
  /* USER CODE BEGIN StartM3508Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartM3508Task */
}

/* USER CODE BEGIN Header_StartPMTask */
/**
* @brief Function implementing the PMTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPMTask */
__weak void StartPMTask(void *argument)
{
  /* USER CODE BEGIN StartPMTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartPMTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

