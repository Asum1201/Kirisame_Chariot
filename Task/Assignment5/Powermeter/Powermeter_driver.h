//
// Created by 33269 on 2026/11/6.
//

#ifndef DX_ASSIGNMENT5_FREERTOS_POWERMETER_DRIVER_H
#define DX_ASSIGNMENT5_FREERTOS_POWERMETER_DRIVER_H
#include <sys/types.h>

#include "CAN_bsp.h"
struct pm_data
{
    int16_t raw_power;
    int16_t raw_energy;
    float  chassis_power;
    float buffer_energy;
};

class PowerMeter{

public:
    PowerMeter(CAN_HandleTypeDef *hcan_=&hcan1,can_rx_callback *callback_=nullptr,
        uint32_t id_pm = 3);
    friend void pm_data_update_callback(can_device_receive *device,uint8_t *data);
protected:
    uint32_t id;
    pm_data meter_data;
    can_device_receive can_rx;

public:
    uint8_t can_rx_data[8];
};
#ifdef __cplusplus
extern "C"
{
void PowerMeter_Init(void);
void StartPMTask(void);
}
#endif
#endif //DX_ASSIGNMENT5_FREERTOS_POWERMETER_DRIVER_H
