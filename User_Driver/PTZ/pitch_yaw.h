#ifndef KIRISAME_CHARIOT_PITCH_H
#define KIRISAME_CHARIOT_PITCH_H
#include <cstdint>

extern float pitch_full_spd_value;
extern float yaw_full_spd_value;

extern volatile float pitch_position_kp;
extern volatile float pitch_position_ki;
extern volatile float pitch_position_kd;
extern volatile float pitch_position_max_output;
extern volatile float pitch_position_integral_limit;
extern volatile float pitch_velocity_kp;
extern volatile float pitch_velocity_ki;
extern volatile float pitch_velocity_kd;
extern volatile float pitch_velocity_max_output;
extern volatile float pitch_velocity_integral_limit;
extern volatile float pitch_feed;
extern volatile float pitch_position_low;
extern volatile float pitch_position_high;

extern volatile float yaw_position_kp;
extern volatile float yaw_position_ki;
extern volatile float yaw_position_kd;
extern volatile float yaw_position_max_output;
extern volatile float yaw_position_integral_limit;
extern volatile float yaw_velocity_kp;
extern volatile float yaw_velocity_ki;
extern volatile float yaw_velocity_kd;
extern volatile float yaw_velocity_max_output;
extern volatile float yaw_velocity_integral_limit;
extern volatile float yaw_feed;
extern volatile float yaw_feed;

extern volatile float pitch_target_position;
extern volatile float yaw_target_position;

extern volatile float pitch_actual_position;
extern volatile float pitch_cmd;
extern volatile float yaw_actual_position;
extern volatile float yaw_cmd;

float pitch_add_value_calculate();
float yaw_add_value_calculate();
float target_limit(float raw_target_value);

#ifdef __cplusplus
extern "C" {
#endif

void pitch_yaw_task(void *argument);

#ifdef __cplusplus
} // extern "C"
#endif
#endif //KIRISAME_CHARIOT_PITCH_H
