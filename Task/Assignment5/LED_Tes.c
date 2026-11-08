//
// Created by 33269 on 2026/11/5.
//

#include "LED_Tes.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"
#include <stdbool.h>
//1.神秘流水灯（能量就行）
bool init_r = false,init_g = false,init_b = false;
typedef enum
{
    ON = 1,
    OFF = 0
}led_state_e;
led_state_e r_state,g_state,b_state;
void led_r()
{
    init_r = true;
    r_state = OFF;
    for (;;)
    {
        if (r_state == OFF)
        {
            HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);
            r_state = ON;
        }
        else
        {
            HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
            r_state = OFF;
        }
        osDelay(500);
    }
}

void led_g()
{
    while (init_r == false)
    {
        osDelay(1);
    }
    init_g = true;
    g_state = OFF;
    osDelay(200);
    for (;;)
    {
        if (g_state == OFF)
         {
              HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
              g_state = ON;
         }
         else
         {
               HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
              g_state = OFF;
         }
        osDelay(500);
    }
}
void led_b()
{
    while (init_g == false)
    {
        osDelay(1);
    }
    osDelay(400);
    init_b = true;
    b_state = OFF;
    for (;;)
    {
        if (b_state == OFF)
        {
            HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
            b_state = ON;
        }
        else
        {
            HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
            b_state = OFF;
        }
        osDelay(500);
    }
}

