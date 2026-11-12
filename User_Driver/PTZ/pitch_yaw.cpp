#include "pitch_yaw.h"
#include "DJI_motor_driver.h"
#include "remote_ctrl.h"
#include "Protection.h"
float pitch_full_spd_value = 0.3f;
float yaw_full_spd_value = 1.0f;

volatile float pitch_position_kp = 30.0f;
volatile float pitch_position_ki = 0.0f;
volatile float pitch_position_kd = 0.0f;
volatile float pitch_position_max_output = 0.8f;
volatile float pitch_position_integral_limit = 0.1f;
volatile float pitch_velocity_kp = 0.8f;
volatile float pitch_velocity_ki = 0.0f;
volatile float pitch_velocity_kd = 0.0f;
volatile float pitch_velocity_max_output = 0.3f;
volatile float pitch_velocity_integral_limit = 0.1f;
volatile float pitch_feed = 0.05f;
volatile float pitch_target_position = 0;
volatile float pitch_position_low = -0.78f;
volatile float pitch_position_high = -0.62f;

volatile float yaw_position_kp = 30.0f;
volatile float yaw_position_ki = 0.0f;
volatile float yaw_position_kd = 0.0f;
volatile float yaw_position_max_output = 0.8f;
volatile float yaw_position_integral_limit = 0.1f;
volatile float yaw_velocity_kp = 0.5f;
volatile float yaw_velocity_ki = 0.0f;
volatile float yaw_velocity_kd = 0.0f;
volatile float yaw_velocity_max_output = 0.3f;
volatile float yaw_velocity_integral_limit = 0.1f;
volatile float yaw_feed = 0.0f;
volatile float yaw_target_position = 0.0f;

volatile float pitch_actual_position = 0.0f;
volatile float pitch_cmd = 0.0f;
volatile float yaw_actual_position = 0.0f;
volatile float yaw_cmd = 0.0f;


volatile float  valid = 0.0f;
volatile float  ch1 = 0.0f;
volatile float  ch0 = 0.0f;

float pitch_add_value_calculate()
{
    float rc_value = 2.0f * ((float)RC_Ctl.rc.ch1 - (float)RC_CH_VALUE_OFFSET) / ((float)RC_CH_VALUE_MAX - (float)RC_CH_VALUE_MIN);

    if (rc_value <= 0.05f && rc_value >= -0.05f)
    {
        return 0.0f;
    }
    ch1 = rc_value;


    return -rc_value * pitch_full_spd_value * 0.001f;
}

float yaw_add_value_calculate()
{
    float rc_value = 2.0f * ((float)RC_Ctl.rc.ch0 - (float)RC_CH_VALUE_OFFSET) / ((float)RC_CH_VALUE_MAX - (float)RC_CH_VALUE_MIN);

    if (rc_value <= 0.05f && rc_value >= -0.05f)
    {
        return 0.0f;
    }
    ch0 = rc_value;

    return -rc_value * yaw_full_spd_value * 0.001f;
}

float target_limit(float raw_target_value)
{
    float target_value;
    target_value = raw_target_value;
    if (raw_target_value < pitch_position_low)
    {
        target_value = pitch_position_low;
    }
    else if (raw_target_value > pitch_position_high)
    {
        target_value = pitch_position_high;
    }
    return target_value;
}

void pitch_yaw_task(void *argument)
{
    DJI_motor *motor_GM6020_pitch = new DJI_motor(&hcan1, nullptr, 1, DJI_GM6020, 1);
    DJI_motor *motor_GM6020_yaw = new DJI_motor(&hcan2, nullptr, 2, DJI_GM6020, 0);

    for (;;)
    {
        motor_GM6020_pitch->motor_control(DJI_MOTOR_DISABLE_OFFSET);

        pitch_actual_position = motor_GM6020_pitch->motor_get_rounds();
        yaw_actual_position = motor_GM6020_yaw->motor_get_rounds();

        valid = protection_state;
         if (protection_state == NEED_PROTECTION)
      {
            pitch_target_position = motor_GM6020_pitch->motor_get_rounds();
            yaw_target_position = motor_GM6020_yaw->motor_get_rounds();

            motor_GM6020_pitch->motor_control(DJI_MOTOR_CLEAR_PID);
            motor_GM6020_yaw->motor_control(DJI_MOTOR_CLEAR_PID);

            motor_GM6020_pitch->motor_set_current(0.0f);
            motor_GM6020_yaw->motor_set_current(0.0f);
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
            motor_GM6020_yaw->posPid.pid_reset(
                yaw_position_max_output,
                yaw_position_integral_limit,
                yaw_position_kp,
                yaw_position_ki,
                yaw_position_kd
            );
            motor_GM6020_yaw->velPid.pid_reset(
                yaw_velocity_max_output,
                yaw_velocity_integral_limit,
                yaw_velocity_kp,
                yaw_velocity_ki,
                yaw_velocity_kd
            );

            float pitch_raw_target = pitch_target_position;
            pitch_raw_target += pitch_add_value_calculate();
            if (pitch_raw_target < pitch_position_low) { pitch_raw_target = pitch_position_low; }
            if (pitch_raw_target > pitch_position_high) {pitch_raw_target = pitch_position_high; }
            pitch_target_position = pitch_raw_target;
            yaw_target_position += yaw_add_value_calculate();

            motor_GM6020_pitch->motor_set_rounds(pitch_target_position);
            motor_GM6020_yaw->motor_set_rounds(yaw_target_position);

           float pitch_voltage_cmd = -motor_GM6020_pitch->velPid.out + pitch_feed;
             //float yaw_feed_x = (motor_GM6020_yaw->velPid.out > 0) ? yaw_feed : -yaw_feed;
           float yaw_voltage_cmd = motor_GM6020_yaw->velPid.out /*+ yaw_feed_x*/;

            if (pitch_voltage_cmd > pitch_velocity_max_output){pitch_voltage_cmd = pitch_velocity_max_output;}
            else if (pitch_voltage_cmd < -pitch_velocity_max_output){pitch_voltage_cmd = -pitch_velocity_max_output;}
            if (yaw_voltage_cmd > yaw_velocity_max_output){yaw_voltage_cmd = yaw_velocity_max_output;}
            else if (yaw_voltage_cmd < -yaw_velocity_max_output){yaw_voltage_cmd = -yaw_velocity_max_output;}

            motor_GM6020_pitch->motor_set_current(pitch_voltage_cmd);
            motor_GM6020_yaw->motor_set_current(yaw_voltage_cmd);
            pitch_cmd = pitch_voltage_cmd;
            yaw_cmd = yaw_voltage_cmd;
        }

        osDelay(1);
    }
}