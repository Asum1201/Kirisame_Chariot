//
// Created by 33269 on 2026/11/10.
//

#include "Shoot_Task.h"

#include <cmath>

#include "Protection.h"
#include "remote_ctrl.h"
#include "cmsis_os2.h"
#include "DJI_motor_driver.h"

//friction pulley

const int16_t SHOOT_BLOCK_THRED = 1000;
const float GEAR_RATIO = 1.9206f;
/*---------------------Plucking Definition--------------------------*/

uint32_t blockedcount = 0,
        shoottaskcount = 0;
float shoot_nowpos = 0.0f;
int16_t Plucking_current = 0;
DJI_motor *Plucking_Motor;
//---------------------------------------------------
void Friction_Pulley_Task()
{

}

void Plucking_Motor_Task()
{
    if (protection_state == NEED_PROTECTION)
    {
       Plucking_Motor->motor_set_speed(0);
        shoot_nowpos = Plucking_Motor->motor_get_rounds();
    }
    else
    {
        if (RC_Ctl.rc.s1 != 1)//停止拨弹的位置环
        {
            //do nothing
        }
        else
        {
            Plucking_current = Plucking_Motor->motor_get_current();
            if (abs(Plucking_current) > SHOOT_BLOCK_THRED)
            {
                blockedcount++;
                if (blockedcount >= 200)
                {
                    shoot_nowpos -=2.0f * GEAR_RATIO;
                    blockedcount = 0;
                }
            }
            else
            {
                blockedcount = 0;
                shoottaskcount = (shoottaskcount + 1)%10000;
                if (shoottaskcount %1000 == 0)shoot_nowpos +=1.0f * GEAR_RATIO;
            }
        }
        Plucking_Motor->motor_set_rounds(shoot_nowpos);
    }
}

void Shoot_Init()
{
        Plucking_Motor = new DJI_motor(&hcan1,nullptr,8,DJI_M2006,0);
}
void StartShootTask()
{
    osDelay(500);
    Shoot_Init();
    for (;;)
    {
        Friction_Pulley_Task();
        Plucking_Motor_Task();
        osDelay(1);
    }
}
