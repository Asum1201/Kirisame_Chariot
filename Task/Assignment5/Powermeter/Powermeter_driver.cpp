//
// Created by 33269 on 2026/11/6.
//

#include "Powermeter_driver.h"

float DeBug_energy,DeBug_power;
void pm_data_update_callback(can_device_receive *can_receive,uint8_t * data);

PowerMeter::PowerMeter(CAN_HandleTypeDef *hcan_,can_rx_callback *callback_,uint32_t id_pm)
                      :can_rx_data{},can_rx(hcan_,0x31,nullptr){
    callback_ == nullptr ? can_rx.can_set_callback(pm_data_update_callback) : can_rx.can_set_callback(callback_);

}

void pm_data_update_callback(can_device_receive *can_receive,uint8_t * data)
{
    PowerMeter *pm_ptr = container_of(can_receive,PowerMeter,can_rx);
    memcpy(pm_ptr->can_rx_data,data,8);
    pm_ptr->meter_data.raw_power = (int16_t)(data[1] <<8) | (data[0] & 0xFF);
    pm_ptr->meter_data.raw_energy = (int16_t)(data[3] <<8) | (data[2] & 0xFF);
    pm_ptr->meter_data.chassis_power = float(pm_ptr->meter_data.raw_power) * 0.1f;
    pm_ptr->meter_data.buffer_energy = float(pm_ptr->meter_data.raw_energy) * 0.1f;
    DeBug_energy = pm_ptr->meter_data.buffer_energy;
    DeBug_power = pm_ptr->meter_data.chassis_power;
}


PowerMeter *pm_debug;
void PowerMeter_Init(void)
{
    pm_debug = new PowerMeter(&hcan1,pm_data_update_callback,0x31);
}

void StartPMTask()
{
    osDelay(300);
   PowerMeter_Init();
    for (;;)
    {
        //把这个driverh放在main.c里就会编译错误，而写一个wrapper来externc太麻烦了，于是。。。
        osDelay(1);
    }
}
