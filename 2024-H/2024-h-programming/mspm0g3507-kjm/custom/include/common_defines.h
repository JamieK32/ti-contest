// application/include/common_defines.h

#ifndef COMMON_DEFINES
#define COMMON_DEFINES

#include "stdint.h"

#define UNIT_TEST_MODE 1								//单元测试模式 1 打开 0 关闭

#define ENCODER_PERIOD_MS          20             // 20ms 采样周期
#define WHEEL_RADIUS_CM            3.3f           // 轮胎半径，单位：cm
#define PULSE_NUM_PER_CIRCLE       1470           // 轮胎一圈的编码器计数

#endif // common_defines_H
