//
// Created by 33269 on 2026/10/11.
//

#include "remote_ctrl.h"

#include <string.h>

#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"



//RC上次收到数据的Tick用于关控保护，为了减少延迟采用全局变量的方式
volatile uint32_t rc_last_rx_tick;
/* ----------------------- Internal Data ----------------------------------- */
// volatile unsigned char sbus_rx_buffer[25];
// static
RC_Ctl_t RC_Ctl;
/* ----------------------- Function Implements  ---------------------------- */

extern UART_HandleTypeDef huart3;
/* Larger than one frame: IDLE defines the packet boundary, including malformed
 * packets. Normal DMA restarts at each gap and recovers byte alignment. */
uint8_t dbus_rx_buffer[36];
static volatile uint8_t restart_required;

void BSP_DBUS_Start_Receive(void)
{
    HAL_StatusTypeDef result=HAL_UARTEx_ReceiveToIdle_DMA(
        &huart3,dbus_rx_buffer,sizeof(dbus_rx_buffer));
    restart_required=(uint8_t)(result!=HAL_OK);
    if (result==HAL_OK) __HAL_DMA_DISABLE_IT(huart3.hdmarx,DMA_IT_HT);
}
void BSP_DBUS_Init(void)
{
    restart_required=1;
    BSP_DBUS_Start_Receive();
}

uint8_t dbg_recv_buff[18];
uint32_t rc_cb_count;

void remote_ctrl_recvcallback(void)
{
    //rc_cb_count++;
    //对遥控器DBUS_buffer数据进行移位解析
    const uint8_t *d=dbus_rx_buffer;
    RC_Ctl.rc.ch0 = (d[0]| (d[1] << 8)) & 0x07ff;           //!< Channel 0
    RC_Ctl.rc.ch1 = ((d[1] >> 3) | (d[2] << 5)) & 0x07ff;   //!< Channel 1
    RC_Ctl.rc.ch2 = ((d[2] >> 6) | (d[3] << 2) |            //!< Channel 2
                     (d[4] << 10)) & 0x07ff;
    RC_Ctl.rc.ch3 = ((d[4] >> 1) | (d[5] << 7)) & 0x07ff;   //!< Channel 3
    RC_Ctl.rc.s1  = ((d[5] >> 4)& 0x000C) >> 2;                          //!< Switch left
    RC_Ctl.rc.s2  = ((d[5] >> 4)& 0x0003);                               //!< Switch right

    RC_Ctl.mouse.x = (int16_t) ( d[6] | (d[7] << 8) );
    RC_Ctl.mouse.y =(int16_t)  ( d[8] | (d[9] << 8) );
    RC_Ctl.mouse.z = (int16_t) ( d[10] | (d[11] << 8) );
    RC_Ctl.mouse.press_l = d[12];
    RC_Ctl.mouse.press_r = d[13];
    RC_Ctl.key.v = d[14] | (d[15] << 8);
    rc_cb_count = (rc_cb_count+1) % 10000;
    rc_last_rx_tick = xTaskGetTickCountFromISR();
}

static uint8_t valid_frame(void)
{
    const uint8_t *d=dbus_rx_buffer;
    uint16_t c0=(uint16_t)((d[0]|(d[1]<<8))&0x7ff);
    uint16_t c1=(uint16_t)(((d[1]>>3)|(d[2]<<5))&0x7ff);
    uint16_t c2=(uint16_t)(((d[2]>>6)|(d[3]<<2)|(d[4]<<10))&0x7ff);
    uint16_t c3=(uint16_t)(((d[4]>>1)|(d[5]<<7))&0x7ff);
    uint8_t s1=(d[5]>>4)&3U, s2=(d[5]>>6)&3U;
    return (uint8_t)(c0>=364U && c0<=1684U && c1>=364U && c1<=1684U &&
        c2>=364U && c2<=1684U && c3>=364U && c3<=1684U && s1 && s2);
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart->Instance!=USART3) return;
    if (size==18U && valid_frame()) {
        remote_ctrl_recvcallback();
    }
    BSP_DBUS_Start_Receive(); /* Re-arm even after truncated or invalid data. */
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance!=USART3) return;
    (void)HAL_UART_AbortReceive(huart);
    BSP_DBUS_Start_Receive();
}