//
// Created by 33269 on 2026/11/9.
//

#ifndef KIRISAME_CHARIOT_PROTECTION_H
#define KIRISAME_CHARIOT_PROTECTION_H
#include <stdint.h>

typedef enum
{
    STATE_SAFE = 0,
    NEED_PROTECTION = 1
}protection_state_e;
protection_state_e protection_state = 0; //保护的状态 0为正常，1为保护状态

#endif //KIRISAME_CHARIOT_PROTECTION_H
