//
// Created by 33269 on 2026/10/11.
//

#include "remote_ctrl.h"

#include <string.h>

#include "bsp_usart.h"

#include "FreeRTOS.h"
#include "task.h"



//RC上次收到数据的Tick用于关控保护，为了减少延迟采用全局变量的方式
volatile uint32_t rc_last_rx_tick;
/* ----------------------- Internal Data ----------------------------------- */
// volatile unsigned char sbus_rx_buffer[25];
// static
RC_Ctl_t RC_Ctl;
static USART_Init_config_s RC_Init_Config;
static USARTInstance * RCInstance;
/* ----------------------- Function Implements  ---------------------------- */

void remote_ctrl_init(void)
{
      //RC参数的配置
       RC_Init_Config .usart_handle = &huart3;
       RC_Init_Config .recv_buff_size = 18; //18 为DBUS一帧数据大小
       RC_Init_Config .module_callback = remote_ctrl_recvcallback;

       RCInstance  = usart_register(&RC_Init_Config); //注册一个USART实例
}
uint8_t dbg_recv_buff[18];
uint32_t rc_cb_count;

void remote_ctrl_recvcallback(void)
{
    memcpy(dbg_recv_buff, RCInstance->recv_buff, 18);
    rc_cb_count++;
    //对遥控器DBUS_buffer数据进行移位解析
    RC_Ctl.rc.ch0 = (RCInstance->recv_buff[0]| (RCInstance->recv_buff[1] << 8)) & 0x07ff;           //!< Channel 0
    RC_Ctl.rc.ch1 = ((RCInstance->recv_buff[1] >> 3) | (RCInstance->recv_buff[2] << 5)) & 0x07ff;   //!< Channel 1
    RC_Ctl.rc.ch2 = ((RCInstance->recv_buff[2] >> 6) | (RCInstance->recv_buff[3] << 2) |            //!< Channel 2
                     (RCInstance->recv_buff[4] << 10)) & 0x07ff;
    RC_Ctl.rc.ch3 = ((RCInstance->recv_buff[4] >> 1) | (RCInstance->recv_buff[5] << 7)) & 0x07ff;   //!< Channel 3
    RC_Ctl.rc.s1  = ((RCInstance->recv_buff[5] >> 4)& 0x000C) >> 2;                          //!< Switch left
    RC_Ctl.rc.s2  = ((RCInstance->recv_buff[5] >> 4)& 0x0003);                               //!< Switch right

    RC_Ctl.mouse.x = (int16_t) ( RCInstance->recv_buff[6] | (RCInstance->recv_buff[7] << 8) );
    RC_Ctl.mouse.y =(int16_t)  ( RCInstance->recv_buff[8] | (RCInstance->recv_buff[9] << 8) );
    RC_Ctl.mouse.z = (int16_t) ( RCInstance->recv_buff[10] | (RCInstance->recv_buff[11] << 8) );
    RC_Ctl.mouse.press_l = RCInstance->recv_buff[12];
    RC_Ctl.mouse.press_r = RCInstance->recv_buff[13];
    RC_Ctl.key.v = RCInstance->recv_buff[14] | (RCInstance->recv_buff[15] << 8);
    rc_cb_count = (rc_cb_count+1) % 10000;
    rc_last_rx_tick = xTaskGetTickCountFromISR();
}