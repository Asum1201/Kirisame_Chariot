//
// Created by 33269 on 2026/11/5.
//
#include "ins_task.h"
#include "cmsis_os.h"

void StartINSTask(void)
{
    INS_Init();
    for (;;)
    {
        INS_Task();
        osDelay(1);
    }
}