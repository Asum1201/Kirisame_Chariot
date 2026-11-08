//
// Created by 33269 on 2026/11/6.
//

#include "Motor3508Assign.h"
#include "DJI_motor_driver.h"
#include "arm_math.h"
float Freq = 1.0f;
float Amplitude = 1.0f;
DJI_motor * M3508;
int16_t M3508_speed;
float M3508_speed_norm;
float now_rounds,offset,target_rounds,omega_t;
float time_s;
void M3508Control()
{
    //M3508->motor_set_speed(0.15f);
    omega_t = 2.0f * PI * Freq * time_s;
    offset  = Amplitude * arm_sin_f32(omega_t);
    target_rounds = 0.0f + offset;
    M3508->motor_set_rounds(target_rounds);
    M3508_speed = M3508->motor_get_speed();
    M3508_speed_norm = float(M3508_speed) / MOTOR_3508_MAX_SPEED;
}

void StartM3508Task()
{
    uint32_t tick = osKernelGetTickCount();
    M3508 = new DJI_motor(&hcan1,nullptr,1,DJI_M3508,0);
    for (;;)
    {
        M3508Control();
        time_s += 0.001f;
        tick+=1;
        osDelayUntil(tick);
    }
}
