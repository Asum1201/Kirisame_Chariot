#ifndef KIRISAME_CHARIOT_PITCH_H
#define KIRISAME_CHARIOT_PITCH_H
#include <cstdint>

extern float pitch_full_value;
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

extern volatile float pitch_target_position;
extern float yaw_target_position;

extern volatile float pitch_actual_position;
extern volatile float pitch_actual_current;


extern volatile float pitch_cmd;

float pitch_add_value_calculate();
float yaw_add_value_calculate();

#ifdef __cplusplus
extern "C" {
#endif

void pitch_yaw_task(void *argument);

#ifdef __cplusplus
} // extern "C"
#endif
#endif //KIRISAME_CHARIOT_PITCH_H
