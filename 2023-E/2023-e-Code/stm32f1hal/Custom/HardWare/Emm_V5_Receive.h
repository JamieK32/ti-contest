#ifndef EMM_V5_RECEIVE_H
#define EMM_V5_RECEIVE_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "stdbool.h"

// 外部引用的串口句柄
extern UART_HandleTypeDef huart2;

// 初始化函数
void InitEmmReceive(void);

// 主循环调用的处理函数
void ProcessEmmReceive(void);

// 获取电机数据的接口函数
int32_t GetMotorPosition(uint8_t motor_id);  // 获取电机位置(脉冲数)
int16_t GetMotorSpeed(uint8_t motor_id);     // 获取电机速度(RPM)
uint8_t GetMotorStatus(uint8_t motor_id);    // 获取电机状态标志

// 状态检查函数
uint8_t IsMotorEnabled(uint8_t motor_id);    // 检查电机是否使能
uint8_t IsMotorInPosition(uint8_t motor_id); // 检查电机是否到位


#endif