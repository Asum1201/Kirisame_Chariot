//
// Created by 33269 on 2026/10/8.
//

#ifndef BSP_USART_H
#define BSP_USART_H

#include<stdint.h>
#include "main.h"

#define DEVICE_USART_CNT 3
#define USART_RXBUFF_MAXSIZE 256

typedef void (* usart_module_callback)(void);

typedef enum
{
    USART_TRANSFER_NONE=0,
    USART_TRANSFER_BLOCKING,
    USART_TRANSFER_IT,
    USART_TRANSFER_DMA,
} USART_TRANSFER_MODE;

typedef struct
{
    uint8_t recv_buff[USART_RXBUFF_MAXSIZE];
    uint8_t recv_buff_size;//接收一包数据的大小
    UART_HandleTypeDef *usart_handle;
    usart_module_callback module_callback;
}USARTInstance;

typedef struct
{
    uint8_t recv_buff_size;
    UART_HandleTypeDef *usart_handle;
    usart_module_callback module_callback;
}USART_Init_config_s;

/**
 * @brief 初始化_instance USART实例的串口服务
 * @param _instance USART实例
 */
void usart_service_init(USARTInstance * _instance);

/**
 * @brief 利用初始config注册一个usart实例(并返回一个实例的指针)
 * @param Init_config 初始化USART实例的参数
 * @return 一个指向USART实例的指针用来给module层使用以便找到该实例
 */
USARTInstance * usart_register(USART_Init_config_s * init_config);

/**
 * @brief 通过调用该函数可以发送一帧数据,需要传入一个usart实例,发送buff以及这一帧的长度
 * @note 在短时间内连续调用此接口,若采用IT/DMA会导致上一次的发送未完成而新的发送取消.
 * @note 若希望连续使用DMA/IT进行发送,请配合USARTIsReady()使用,或自行为你的module实现一个发送队列和任务.
 * @todo 是否考虑为USARTInstance增加发送队列以进行连续发送?
 *
 * @param _instance 串口实例
 * @param send_buf 待发送数据的buffer
 * @param send_size how many bytes to send
 */
void usart_send(USARTInstance *_instance, uint8_t *send_buf, uint16_t send_size,USART_TRANSFER_MODE mode);

/**
 * @brief 判断串口是否准备好,用于连续或异步的IT/DMA发送
 *
 * @param _instance 要判断的串口实例
 * @return uint8_t ready 1, busy 0
 */
uint8_t usart_is_ready(USARTInstance *_instance);

#endif //_BSP_USART_H
