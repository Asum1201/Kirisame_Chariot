//
// Created by 33269 on 2026/10/8.
//

#include "bsp_usart.h"
#include<stdlib.h>
#include<memory.h>

static uint8_t idx = 0; /**记录共有多少实例*/
static USARTInstance * usart_instance[DEVICE_USART_CNT] = {NULL};/**用一个指针数组保存每个实例(便于回调函数判断是谁触发了回调)*/

void usart_service_init(USARTInstance * _instance)
{
    HAL_UARTEx_ReceiveToIdle_DMA(_instance->usart_handle, _instance->recv_buff, _instance->recv_buff_size);
    __HAL_DMA_DISABLE_IT(_instance->usart_handle->hdmarx, DMA_IT_HT);
}

USARTInstance * usart_register(USART_Init_config_s * init_config)
{
    if (idx == DEVICE_USART_CNT)
        while (1)
            ; //
    for (uint8_t i = 0; i < idx; i++) // 检查是否已经注册过串口
        if (usart_instance[i]->usart_handle == init_config->usart_handle)
            while (1)
                ;//
    USARTInstance *instance = (USARTInstance *)malloc(sizeof(USARTInstance));
    memset(instance, 0, sizeof(USARTInstance));

    instance->usart_handle = init_config->usart_handle;
    instance->recv_buff_size = init_config->recv_buff_size;
    instance->module_callback = init_config->module_callback; //配置参数到实例中

    usart_instance[idx++] = instance; //指针注册到数组里
    usart_service_init(instance);
    return instance;
}

void usart_send(USARTInstance *_instance, uint8_t *send_buf, uint16_t send_size,USART_TRANSFER_MODE mode)
{
    switch (mode)
    {
        case USART_TRANSFER_BLOCKING:
            HAL_UART_Transmit(_instance->usart_handle, send_buf, send_size,100);
            break;
        case USART_TRANSFER_IT:
            HAL_UART_Transmit_IT(_instance->usart_handle, send_buf, send_size);
            break;
        case USART_TRANSFER_DMA:
            HAL_UART_Transmit_DMA(_instance->usart_handle, send_buf, send_size);
            break;
        default:
            while (1)
                ;
            break;
    }
}

uint8_t usart_is_ready(USARTInstance *_instance)
{
    if (_instance->usart_handle->gState == HAL_UART_STATE_BUSY_TX)
        return 0;
    else
        return 1;
}

/**
 * @brief 每次dma/idle中断发生时，都会调用此函数.对于每个uart实例会调用对应的回调进行进一步的处理
 *
 * @param huart 发生中断的串口
 * @param Size 此次接收到的总数居量,暂时没用
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    for (uint8_t i = 0; i < idx; i++)
        if (usart_instance[i]->usart_handle == huart)
        {
            // call the callback function if it is not NULL
           /* if (usart_instance[i]->module_callback != NULL)
            {
                usart_instance[i]->module_callback();
                memset(usart_instance[i]->recv_buff, 0, Size); // 接收结束后清空buffer,对于变长数据是必要的
            } */

            /* 只有收到完整一帧才解析 */
            if (Size == usart_instance[i]->recv_buff_size)
            {
                if (usart_instance[i]->module_callback != NULL)
                    usart_instance[i]->module_callback();
            }

            /* 清空整个缓冲区，不是只清 Size */
            memset(usart_instance[i]->recv_buff, 0,usart_instance[i]->recv_buff_size);

            HAL_UARTEx_ReceiveToIdle_DMA(usart_instance[i]->usart_handle, usart_instance[i]->recv_buff, usart_instance[i]->recv_buff_size);
            __HAL_DMA_DISABLE_IT(usart_instance[i]->usart_handle->hdmarx, DMA_IT_HT);
            return;
        }
}