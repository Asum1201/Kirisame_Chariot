//
// Created by 33269 on 2026/11/10.
//

#include "Shoot_Task.h"

#include <cmath>

#include "arm_math.h"
#include "Protection.h"
#include "remote_ctrl.h"
#include "cmsis_os2.h"
#include "DJI_motor_driver.h"

//friction pulley
float fspeed_R = 0,fspeed_L = 0;
float Pluck_p= 0.2f,Pluck_i = 0.3f,Pluck_d= 0.3f,P_Max = 0.2f,P_Ilimit = 0.1f;

const int16_t SHOOT_BLOCK_THRED = 7000;
const float GEAR_RATIO = 4.5f;
/*---------------------Plucking Definition--------------------------*/

uint32_t blockedcount = 0,
        shoottaskcount = 0;
float shoot_nowpos = 0.0f;
int16_t Plucking_current = 0;
float current_ref = 0.0f;
float motor_rounds= 0.0f;
//---------------------------------------------------

/*--------------------Friction Definition --------------------------*/
DJI_motor *Plucking_Motor,*Friction_Motor_L,*Friction_Motor_R;
const float FWHEEL_D = 0.06f; //摩擦轮径
const float FMOTOR_SPEED_FORWARD_FAST = 24.0f /(1.0f / 2.0f * FWHEEL_D * MOTOR_3508_MAX_SPEED *2.0f *PI/ 60.0f),
            FMOTOR_SPEED_FORWARD_MEDIUM = 16.0f /(1.0f / 2.0f * FWHEEL_D * MOTOR_3508_MAX_SPEED *2.0f *PI/ 60.0f),
            FMOTOR_SPEED_FORWARD_SLOW = 8.0f /(1.0f / 2.0f * FWHEEL_D * MOTOR_3508_MAX_SPEED *2.0f *PI/ 60.0f);
uint32_t cpunt;
//----------
void Friction_Pulley_Task()
{
    if (protection_state == NEED_PROTECTION)
    {
        Friction_Motor_L->motor_set_speed(0);
        Friction_Motor_R->motor_set_speed(0);
    }
    else
    {
        if (RC_Ctl.rc.s1 != 1)
        {
            Friction_Motor_L->motor_set_speed(0);
            Friction_Motor_R->motor_set_speed(0);
        }
        else
        {
            switch (RC_Ctl.rc.s2)
            {
                case 1:
                    Friction_Motor_L->motor_set_speed(FMOTOR_SPEED_FORWARD_FAST);
                    Friction_Motor_R->motor_set_speed(FMOTOR_SPEED_FORWARD_FAST);
                    break;
                case 3:
                    Friction_Motor_L->motor_set_speed(FMOTOR_SPEED_FORWARD_MEDIUM);
                    Friction_Motor_R->motor_set_speed(FMOTOR_SPEED_FORWARD_MEDIUM);
                    break;
                case 2:
                    Friction_Motor_L->motor_set_speed(FMOTOR_SPEED_FORWARD_SLOW);
                    Friction_Motor_R->motor_set_speed(FMOTOR_SPEED_FORWARD_SLOW);
                    break;
            }
        }
    }
    //Friction_Motor_L->motor_set_speed(0);
    // Friction_Motor_R->motor_set_speed(0);
    fspeed_R = Friction_Motor_R->motor_get_speed();
    fspeed_L = Friction_Motor_L->motor_get_speed();
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
       Friction_Motor_L = new DJI_motor(&hcan1,nullptr,2,DJI_M3508, 1);
       Friction_Motor_R = new DJI_motor(&hcan1,nullptr,3,DJI_M3508,0);
        Plucking_Motor = new DJI_motor(&hcan1,nullptr,1,DJI_M2006,1);
  //  Plucking_Motor->posPid.param.p = Pluck_p;
   // Plucking_Motor->posPid.param.i = Pluck_i;
   // Plucking_Motor->posPid.param.d = Pluck_d;
  //  Plucking_Motor->posPid.param.max_out = P_Max;
   // Plucking_Motor->posPid.param.integral_higher_limit = P_Ilimit;
  //  Plucking_Motor->posPid.param.integral_lower_limit = -P_Ilimit;

}
void StartShootTask()
{
    osDelay(500);
    Shoot_Init();
    for (;;)
    {
        Friction_Pulley_Task();
        fspeed_R = Friction_Motor_R->motor_get_speed();
        fspeed_L = Friction_Motor_L->motor_get_speed();
        Plucking_Motor_Task();
        //Plucking_Motor->motor_set_rounds(current_ref);
       // motor_rounds = Plucking_Motor->motor_get_rounds();
        osDelay(1);
    }
}
