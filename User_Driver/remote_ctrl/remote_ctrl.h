//
// Created by 33269 on 2026/10/11.
//

#ifndef REMOTE_CTRL_H
#define REMOTE_CTRL_H


#include "usart.h"
#include <stdint.h>

#define RC_DATA_SIZE sizeof(RC_Ctl_t)
//必须等于其大小，不然memcpy会拷贝越界，拷到一坨垃圾

extern  uint8_t dbg_recv_buff[18];
extern uint32_t rc_cb_count;

#pragma pack(1)

typedef union
{
    struct
    {
        struct
        {
            uint8_t  ch0_h:8;             //!< Byte 0

            uint8_t  ch0_l:3;             //!< Byte 1
            uint8_t  ch1_h:5;

            uint8_t  ch1_l:6;             //!< Byte 2
            uint8_t  ch2_h:2;

            uint8_t  ch2_m:8;             //!< Byte 3

            uint8_t  ch2_l:1;             //!< Byte 4
            uint8_t  ch3_h:7;

            uint8_t  ch3_l:4;             //!< Byte 5

            uint8_t  s1:2;
            uint8_t  s2:2;
        }rc;

        struct
        {
            int16_t x;                    //!< Byte 6-7
            int16_t y;                    //!< Byte 8-9
            int16_t z;                    //!< Byte 10-11
            uint8_t press_l;              //!< Byte 12
            uint8_t press_r;              //!< Byte 13
        }mouse;

        struct
        {
            uint16_t v;                   //!< Byte 14-15
        }key;

        uint16_t resv;                    //!< Byte 16-17
    };
    uint8_t buf[18];                      //!< Union --> Byte<0-17>
}RC_Ctl_Define_t;

/* ----------------------- RC Channel Definition---------------------------- */
#define RC_CH_VALUE_MIN              ((uint16_t)364 )
#define RC_CH_VALUE_OFFSET           ((uint16_t)1024)
#define RC_CH_VALUE_MAX              ((uint16_t)1684)

/* ----------------------- RC Switch Definition----------------------------- */
#define RC_SW_UP                     ((uint16_t)1)
#define RC_SW_MID                    ((uint16_t)3)
#define RC_SW_DOWN                   ((uint16_t)2)

/* ----------------------- PC Key Definition-------------------------------- */
#define KEY_PRESSED_OFFSET_W         ((uint16_t)0x01<<0)
#define KEY_PRESSED_OFFSET_S         ((uint16_t)0x01<<1)
#define KEY_PRESSED_OFFSET_A         ((uint16_t)0x01<<2)
#define KEY_PRESSED_OFFSET_D         ((uint16_t)0x01<<3)
#define KEY_PRESSED_OFFSET_Q         ((uint16_t)0x01<<4)
#define KEY_PRESSED_OFFSET_E         ((uint16_t)0x01<<5)
#define KEY_PRESSED_OFFSET_SHIFT     ((uint16_t)0x01<<6)
#define KEY_PRESSED_OFFSET_CTRL      ((uint16_t)0x01<<7)

/* ----------------------- Data Struct ------------------------------------- */
typedef struct
{

    struct
    {
        uint16_t ch0;
        uint16_t ch1;
        uint16_t ch2;
        uint16_t ch3;
        uint8_t  s1;
        uint8_t  s2;
    }rc;

    struct
    {
        int16_t x;
        int16_t y;
        int16_t z;
        uint8_t press_l;
        uint8_t press_r;
    }mouse;

    struct
    {
        uint16_t v;
    }key;
}RC_Ctl_t;
#pragma pack()

extern RC_Ctl_t RC_Ctl;
extern volatile uint32_t rc_last_rx_tick;
/**
 * @brief 配置RC的USART的参数，调用bsp_usart的init函数 ，得到一个USART实例指针
 *        设置一个Publisher
 */
void remote_ctrl_init(void);

/**
 * @在DMA空闲中断时调用，进行DBUS的数据拼接解析，并且作为Publisher发布该数据
 */
void remote_ctrl_recvcallback();
#endif //REMOTE_CTRL_H
