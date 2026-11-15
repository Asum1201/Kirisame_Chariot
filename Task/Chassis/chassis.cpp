//
// Created by Legion on 2026/11/9.
//
#include "chassis.h"

#include "cmsis_os.h"
#include "DJI_motor_driver.h"
#include "remote_ctrl.h"
#include "Protection.h"
#include "message_center.h"

#define CAN_BUS     (&hcan2)
#define ID_3508_L   (1)
#define ID_3508_R   (2)
#define ID_6020     (1)

#define ZERO_ROUND  (-0.1f)

#define ROUND_MIN   (-0.4f)
#define ROUND_MAX   (0.4f)

#define PM_LIMIT_POWER   (35.0f)
#define PM_LOOP_KI       (0.0004f)
#define PM_SCALE_MIN     (0.0f)

#define DIFF_GAIN        (0.6f)   /* 满舵时内外轮速度差比例：0.45 -> 外轮 1.45v / 内轮 0.55v */

#define CHASSIS_SPEED_MAX (0.3f)  /* 满杆对应的最高速（占 3508 满速 9600rpm 的比例）：0.3 ≈ 2880rpm */

typedef struct
{
    float max_out;
    float integral_limit;
    float kp;
    float ki;
    float kd;
} pid_cfg_t;

static const pid_cfg_t pid_6020_pos   = {0.2f, 0.1f, 1.0f,  0.0f,   0.0f};
static const pid_cfg_t pid_6020_vel   = {0.5f, 0.1f, 1.0f, 0.002f, 0.1f};
static const pid_cfg_t pid_3508_l_vel = {0.3f, 0.1f, 10.0f, 0.002f, 0.1f};
static const pid_cfg_t pid_3508_r_vel = {0.3f, 0.1f, 10.0f, 0.002f, 0.1f};

static DJI_motor *m6020;
static DJI_motor *m3508_l;
static DJI_motor *m3508_r;

static Subscriber_t *pm_sub;
static float pm_value[2];
static float chassis_power;

/* ===== FreeMASTER 观测变量：非 static + volatile，符号不被修饰，按 obs_ 搜即可 ===== */
volatile uint32_t obs_stage;    /* 0=未开始 1=构造6020 2=构造3508_L 3=构造3508_R
                                   4=初始化完成 5=循环中(闸门关) 6=循环中(闸门开) */
volatile uint32_t obs_loop;     /* 循环计数：持续在涨说明任务活着 */

volatile float obs_6020_pos;    /* 6020 实际位置（圈，含 round_cnt 的多圈累计） */
volatile float obs_6020_target; /* 6020 目标位置（圈，已含零偏，就是下发给驱动的值） */

volatile float obs_l_pos;       /* 左 3508 实际位置（圈） */
volatile float obs_l_speed;     /* 左 3508 实际速度（归一化 -1~1，与 velPid.get 同口径） */
volatile float obs_l_target;    /* 左 3508 目标速度（归一化 -1~1） */

volatile float obs_r_pos;       /* 右 3508 实际位置（圈） */
volatile float obs_r_speed;     /* 右 3508 实际速度（归一化 -1~1） */
volatile float obs_r_target;    /* 右 3508 目标速度（归一化 -1~1） */

volatile uint16_t obs_rc_ch2;   /* 遥控通道 2 原始值（364~1684，中位约 1024） */
volatile uint16_t obs_rc_ch3;   /* 遥控通道 3 原始值 */
volatile uint8_t obs_prot;      /* protection_state 镜像（0=正常，1=保护） */
volatile float obs_power;       /* 功率计实测功率 W（验证功率计有没有喂数据） */
volatile float obs_scale;       /* 功率缩放系数 0~1：1.0=没限速，0.5=输出减半 */

static void set_pid(pid *p, const pid_cfg_t *cfg)
{
    p->pid_reset(cfg->max_out, cfg->integral_limit, cfg->kp, cfg->ki, cfg->kd);
}

static float rc_norm(uint16_t ch)
{
    return ((float)ch - RC_CH_VALUE_OFFSET) / ((float)RC_CH_VALUE_MAX - RC_CH_VALUE_OFFSET);
}

/* 差速混控：前进速度 + 转向量 -> 左右轮归一化速度
 *   steer > 0 视为右转：左轮（外侧）加速、右轮（内侧）减速
 *   差速量与 v 相乘 -> 速度越快转向越明显，静止时不会自己转
 *   倒车时 v 为负，差速方向自动翻过来，不用特判
 *   超量程时等比缩小，保持内外轮比例（转向半径不变）
 */
static void chassis_diff_mix(float v, float steer, float *out_l, float *out_r)
{
    float l = v * (1.0f + DIFF_GAIN * steer);
    float r = v * (1.0f - DIFF_GAIN * steer);
    float m;
    float t;

    m = l < 0.0f ? -l : l;
    t = r < 0.0f ? -r : r;
    if (t > m)
        m = t;

    if (m > 1.0f)
    {
        l /= m;
        r /= m;
    }

    *out_l = l;
    *out_r = r;
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

static float power_scale = 1.0f;

static void chassis_power_loop(void)
{
    power_scale += PM_LOOP_KI * (PM_LIMIT_POWER - chassis_power);

    if (power_scale > 1.0f)
        power_scale = 1.0f;
    if (power_scale < PM_SCALE_MIN)
        power_scale = PM_SCALE_MIN;
}

static void chassis_pm_update(void)
{
    if (sub_getmessage(pm_sub, pm_value))
    {
        chassis_power = pm_value[0];
        chassis_power_loop();
    }
}

void StartChassisTask(void *argument)
{
    obs_stage = 1;
    m6020 = new DJI_motor(CAN_BUS, nullptr, ID_6020, DJI_GM6020, 0);
    m6020->motor_control(DJI_MOTOR_DISABLE_OFFSET);
    set_pid(&m6020->posPid, &pid_6020_pos);
    set_pid(&m6020->velPid, &pid_6020_vel);

    obs_stage = 2;
    m3508_l = new DJI_motor(CAN_BUS, nullptr, ID_3508_L, DJI_M3508, 0);
    set_pid(&m3508_l->velPid, &pid_3508_l_vel);

    obs_stage = 3;
    m3508_r = new DJI_motor(CAN_BUS, nullptr, ID_3508_R, DJI_M3508, 1);
    set_pid(&m3508_r->velPid, &pid_3508_r_vel);

    chassis_pm_init();
    obs_stage = 4;

    for (;;)
    {
        uint32_t tick = osKernelGetTickCount();
        float target_round;
        float target_l;
        float target_r;

        obs_loop++;
        obs_stage = 5;

        chassis_pm_update();
        obs_power = chassis_power;
        obs_scale = power_scale;

        obs_rc_ch2 = RC_Ctl.rc.ch2;
        obs_rc_ch3 = RC_Ctl.rc.ch3;
        obs_prot = (uint8_t)protection_state;

        obs_6020_pos = m6020->motor_get_rounds();
        obs_l_pos = m3508_l->motor_get_rounds();
        obs_l_speed = m3508_l->motor_get_speed() / MOTOR_3508_MAX_SPEED;
        obs_r_pos = m3508_r->motor_get_rounds();
        obs_r_speed = m3508_r->motor_get_speed() / MOTOR_3508_MAX_SPEED;

        if (rc_last_rx_tick != 0 && protection_state == STATE_SAFE)
        {
            obs_stage = 6;

            target_round = rc_target_round(RC_Ctl.rc.ch2);
            set_6020_position(target_round);
            obs_6020_target = target_round + ZERO_ROUND;

            float throttle = rc_norm(RC_Ctl.rc.ch3) * CHASSIS_SPEED_MAX * power_scale;
            float steer    = rc_norm(RC_Ctl.rc.ch2);

            chassis_diff_mix(throttle, steer, &target_l, &target_r);

            m3508_l->motor_set_speed(target_l);
            m3508_r->motor_set_speed(target_r);
            obs_l_target = target_l;
            obs_r_target = target_r;
        }
        else
        {
            m6020->motor_set_current(0.0f);
            m3508_l->motor_set_current(0.0f);
            m3508_r->motor_set_current(0.0f);
        }

        tick += 1;
        osDelayUntil(tick);
    }
}
