// application/include/common_defines.h

#ifndef COMMON_DEFINES
#define COMMON_DEFINES

#include "stdint.h"

#define UNIT_TEST_MODE 0 									//单元测试模式 1 打开 0 关闭
#define DEBUG_MODE 		 1												//调试模式		 1 打开 0 关闭


#define ENCODER_PERIOD_MS          20             // 20ms 采样周期
#define WHEEL_RADIUS_CM            3.3f           // 轮胎半径，单位：cm
#define PULSE_NUM_PER_CIRCLE       1470           // 轮胎一圈的编码器计数


#define MAX_TX_BUFFER_SIZE 256 										//最大TX缓冲区大小
#define MAX_RX_BUFFER_SIZE MAX_TX_BUFFER_SIZE			//最大RX缓冲区大小

#define UART_RX_QUEUE_SIZE MAX_TX_BUFFER_SIZE			//最大接收队列大小
#define UART_TX_QUEUE_SIZE 2											//最大发送队列大小（一个队列占MAX_TX_BUFFER_SIZE）


#endif // common_defines_H
