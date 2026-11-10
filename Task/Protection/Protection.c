//
// Created by 33269 on 2026/11/9.
//

#include "Protection.h"

#include "cmsis_os2.h"
#include "message_center.h"
#include "remote_ctrl.h"
#include "ins_task.h"

typedef enum
{
    IDLE = 0,
    ON = 1,
    SAFE =2
}rc_state_e;//对遥控器状态的定义
uint32_t protection_now;

volatile protection_state_e protection_state = STATE_SAFE; //保护状态，唯一的定义在这里

rc_state_e rc_state;
Publisher_t *RC_State_pub;
/**
 * @brief 管理遥控器接收状态
 */
void set_rc_state()
{
    if (protection_now - rc_last_rx_tick >21) //和遥控器断连
    {
        switch (rc_state)
        {
            case SAFE: break;; //处在关控 状态
            case ON:
                rc_state = SAFE; //由开到关 状态
                break;;
            case IDLE: return; //还没启动
        } //失联都不允许对电机控制
    }
    else //存在连接
    {
        switch (rc_state)
        {
            case SAFE: //断线重连 状态
                //<@note 要不要对电机进行输出限幅防止突变？
                //<@note遥控器有数据的情况下重启的业务逻辑是什么？ //对位置环不断设置初始点直至凯控，对于速度环 一直设置为0
                rc_state = ON;
                break;
            case ON:
                break;
            case IDLE:
                rc_state = ON;
                break;
        }
    }
  //  pub_sendmessage(RC_State_pub,&rc_state);
}

void Protection_Init(void)
{
    rc_state = IDLE;
   // RC_State_pub = pub_register("RC_state",sizeof(rc_state_e));

}
void StartProTask(void)
{
    for (;;)
    {
        protection_now = osKernelGetTickCount();
        if (rc_last_rx_tick != 0)
        {
            set_rc_state();
        }
        if (rc_state == SAFE || INS.zn[2] < 0.342f)
             protection_state = NEED_PROTECTION;
        else protection_state = STATE_SAFE;

        osDelay(1);
    }
}