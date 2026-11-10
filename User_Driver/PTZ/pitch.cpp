#include "pitch.h"
#include "DJI_motor_driver.h"
#include "remote_ctrl.h"
#include "Protection.h"
float pitch_full_value = 0.5f;

volatile float pitch_position_kp = 0.0f;
volatile float pitch_position_ki = 0.0f;
volatile float pitch_position_kd = 0.0f;
volatile float pitch_position_max_output = 0.15f;
volatile float pitch_position_integral_limit = 0.1f;

volatile float pitch_velocity_kp = 0.0f;
volatile float pitch_velocity_ki = 0.0f;
volatile float pitch_velocity_kd = 0.0f;
volatile float pitch_velocity_max_output = 0.25f;
volatile float pitch_velocity_integral_limit = 0.1f;

volatile float pitch_feed = 0.0f;
volatile float pitch_target_position = 0;
volatile float pitch_actual_position = 0.0f;
volatile float pitch_actual_current = 0.0f;

float yaw_full_value = 2.0f;


volatile float pitch_cmd = 0.0f;

float pitch_add_value_calculate()
{
    float rc_value = 2.0f * ((float)RC_Ctl.rc.ch0 - (float)RC_CH_VALUE_OFFSET) / ((float)RC_CH_VALUE_MAX - (float)RC_CH_VALUE_MIN);

    if (rc_value <= 0.05f && rc_value >= -0.05f)
    {
        return 0.0f;
    }

    return rc_value * pitch_full_value * 0.001f;
}

float yaw_add_value_calculate()
{
    float rc_value = 2.0f * ((float)RC_Ctl.rc.ch0 - (float)RC_CH_VALUE_OFFSET) / ((float)RC_CH_VALUE_MAX - (float)RC_CH_VALUE_MIN);

    if (rc_value <= 0.05f && rc_value >= -0.05f)
    {
        return 0.0f;
    }

    return rc_value * yaw_full_value * 0.001f;
}

void pitch_yaw_task(void *argument)
{
     DJI_motor *motor_GM6020_pitch = new DJI_motor(&hcan1, nullptr, 1, DJI_GM6020, 0);

    for (;;)
    {
        pitch_actual_position = motor_GM6020_pitch->motor_get_rounds();
        /*uint32_t now_tick = osKernelGetTickCount();
        bool rc_valid =
            (rc_last_rx_tick != 0U) &&
            ((now_tick - rc_last_rx_tick) <= 21U); */

         if ((protection_state == NEED_PROTECTION) /* || (!rc_valid) */)
        {
            pitch_target_position = motor_GM6020_pitch->motor_get_rounds();

            motor_GM6020_pitch->motor_control(DJI_MOTOR_CLEAR_PID);
            motor_GM6020_pitch->motor_set_current(0.0f);
        }
        else
        {
            motor_GM6020_pitch->posPid.pid_reset(
                pitch_position_max_output,
                pitch_position_integral_limit,
                pitch_position_kp,
                pitch_position_ki,
                pitch_position_kd
            );

            motor_GM6020_pitch->velPid.pid_reset(
                pitch_velocity_max_output,
                pitch_velocity_integral_limit,
                pitch_velocity_kp,
                pitch_velocity_ki,
                pitch_velocity_kd
            );

            //pitch_target_position += pitch_add_value_calculate();

            /* if (pitch_target_position < 0.0f)
            {
                pitch_target_position = 0.0f;
            }
            else if (pitch_target_position > 0.3f)
            {
                pitch_target_position = 0.3f;
            }*/

            motor_GM6020_pitch->motor_set_rounds(
                pitch_target_position
            );

            float voltage_cmd =
                motor_GM6020_pitch->velPid.out + pitch_feed;


            float voltage_limit = pitch_velocity_max_output;

            if (voltage_limit < 0.0f)
            {
                voltage_limit = -voltage_limit;
            }

            if (voltage_limit > 1.0f)
            {
                voltage_limit = 1.0f;
            }

            if (voltage_cmd > voltage_limit)
            {
                voltage_cmd = voltage_limit;
            }
            else if (voltage_cmd < -voltage_limit)
            {
                voltage_cmd = -voltage_limit;
            }

            motor_GM6020_pitch->motor_set_current(voltage_cmd);
                    pitch_cmd = voltage_cmd;
        }

        osDelay(1);
    }
}