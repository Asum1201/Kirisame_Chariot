//
// Created by Legion on 2026/11/9.
//
#include "chassis.h"

#include "cmsis_os.h"
#include "DJI_motor_driver.h"
#include "remote_ctrl.h"
#include "Protection.h"
#include "message_center.h"

#include <math.h>

#define CAN_BUS     (&hcan1)
#define ID_3508_L   (7)
#define ID_3508_R   (8)
#define ID_6020     (2)

#define ZERO_ROUND  (1.0f)
#define ROUND_MIN   (-1.0f)
#define ROUND_MAX   (1.0f)

#define PM_LIMIT_POWER     (50.0f)
#define PM_BASE_POWER      (5.0f)
#define PM_R_EQUIV         (0.10f)
#define PM_K_EQUIV         (0.0012f)
#define M3508_CURRENT_MAX  (16384.0f)
#define M3508_AMPERE_MAX   (20.0f)

typedef struct
{
    float max_out;
    float integral_limit;
    float kp;
    float ki;
    float kd;
} pid_cfg_t;

static const pid_cfg_t pid_6020_pos   = {0.2f, 0.1f, 0.3f,  0.0f,   0.0f};
static const pid_cfg_t pid_6020_vel   = {0.5f, 0.1f, 10.0f, 0.002f, 0.1f};
static const pid_cfg_t pid_3508_l_vel = {0.5f, 0.1f, 10.0f, 0.002f, 0.1f};
static const pid_cfg_t pid_3508_r_vel = {0.5f, 0.1f, 10.0f, 0.002f, 0.1f};

static DJI_motor *m6020;
static DJI_motor *m3508_l;
static DJI_motor *m3508_r;

static Subscriber_t *pm_sub;
static float pm_value[2];
static float chassis_power;

static void set_pid(pid *p, const pid_cfg_t *cfg)
{
    p->pid_reset(cfg->max_out, cfg->integral_limit, cfg->kp, cfg->ki, cfg->kd);
}

static float rc_norm(uint16_t ch)
{
    return ((float)ch - RC_CH_VALUE_OFFSET) / ((float)RC_CH_VALUE_MAX - RC_CH_VALUE_OFFSET);
}

static float rc_target_round(uint16_t ch)
{
    float ratio = ((float)ch - RC_CH_VALUE_MIN) / ((float)RC_CH_VALUE_MAX - RC_CH_VALUE_MIN);

    if (ratio < 0.0f)
        ratio = 0.0f;
    if (ratio > 1.0f)
        ratio = 1.0f;

    return ROUND_MIN + ratio * (ROUND_MAX - ROUND_MIN);
}

static void set_6020_position(float rounds)
{
    m6020->motor_set_rounds(rounds + ZERO_ROUND);
}

static void chassis_pm_init(void)
{
    static char pm_topic[] = "pm_data";

    pm_sub = sub_register(pm_topic, sizeof(pm_value));
}

static void chassis_pm_update(void)
{
    if (sub_getmessage(pm_sub, pm_value))
        chassis_power = pm_value[0];
}

static float chassis_wheel_current(DJI_motor *motor)
{
    int16_t raw = (int16_t)((motor->can_rx_data[4] << 8) | motor->can_rx_data[5]);

    return (float)raw * (M3508_AMPERE_MAX / M3508_CURRENT_MAX);
}

static float chassis_power_scale(void)
{
    float il = chassis_wheel_current(m3508_l);
    float ir = chassis_wheel_current(m3508_r);
    float nl = m3508_l->motor_get_speed();
    float nr = m3508_r->motor_get_speed();
    float A = PM_R_EQUIV * (il * il + ir * ir);
    float B = PM_K_EQUIV * (il * nl + ir * nr);
    float C = PM_LIMIT_POWER - PM_BASE_POWER;
    float k;

    if (A + B + PM_BASE_POWER <= PM_LIMIT_POWER)
        return 1.0f;

    if (C <= 0.0f)
        return 0.0f;

    if (A <= 1e-6f)
        k = C / B;
    else
        k = (-B + sqrtf(B * B + 4.0f * A * C)) / (2.0f * A);

    if (k > 1.0f)
        k = 1.0f;
    if (k < 0.0f)
        k = 0.0f;

    if (chassis_power > PM_LIMIT_POWER)
        k *= PM_LIMIT_POWER / chassis_power;

    return k;
}

// void StartChassisTask(void *argument)
// {
//     m6020 = new DJI_motor(CAN_BUS, nullptr, ID_6020, DJI_GM6020, 0);
//     m6020->motor_control(DJI_MOTOR_DISABLE_OFFSET);
//     set_pid(&m6020->posPid, &pid_6020_pos);
//     set_pid(&m6020->velPid, &pid_6020_vel);
//
//     m3508_l = new DJI_motor(CAN_BUS, nullptr, ID_3508_L, DJI_M3508, 0);
//     set_pid(&m3508_l->velPid, &pid_3508_l_vel);
//
//     m3508_r = new DJI_motor(CAN_BUS, nullptr, ID_3508_R, DJI_M3508, 0);
//     set_pid(&m3508_r->velPid, &pid_3508_r_vel);
//
//     chassis_pm_init();
//
//     for (;;)
//     {
//         uint32_t tick = osKernelGetTickCount();
//         if (rc_last_rx_tick != 0 && protection_state == STATE_SAFE)
//         {
//             float k;
//
//             chassis_pm_update();
//             k = chassis_power_scale();
//
//             set_6020_position(rc_target_round(RC_Ctl.rc.ch2));
//             m3508_l->motor_set_speed(k * rc_norm(RC_Ctl.rc.ch3));
//             m3508_r->motor_set_speed(k * rc_norm(RC_Ctl.rc.ch3));
//         }
//         else
//         {
//             m6020->motor_set_current(0.0f);
//             m3508_l->motor_set_current(0.0f);
//             m3508_r->motor_set_current(0.0f);
//         }
//
//         tick += 1;
//         osDelayUntil(tick);
//     }
// }
