#ifndef VL53L1_READ_H
#define VL53L1_READ_H

#include <stdint.h>
#include <string.h>
#include "vl53l1_api.h"
#include "vl53l1_platform.h"
#include "hal_soft_i2c.h"
#include "delay.h"
#include "systick.h"

// 优化测量配置参数 - 提高更新频率
#define VL53L1_MEASURE_INTERVAL_MS      15      // 测量间隔15ms (67Hz) - 提高频率
#define VL53L1_TIMING_BUDGET_US         30000   // 时间预算30ms - 缩短预算提高速度
#define VL53L1_TIMEOUT_MS               60      // 测量超时60ms - 缩短超时
#define VL53L1_MAX_DISTANCE_MM          600     // 最大有效距离60cm
#define VL53L1_MIN_DISTANCE_MM          10      // 最小有效距离1cm
#define VL53L1_INIT_DELAY_MS            30      // 初始化延时30ms
#define VL53L1_I2C_DELAY_TIME           1       // I2C延时1us - 恢复快速通信

// VL53L1X距离模式配置 - 使用中距离模式提高速度
#define VL53L1_DISTANCE_MODE            VL53L1_DISTANCEMODE_MEDIUM

// ROI区域配置 - 长距离测量使用全ROI获得最大光采集
#define VL53L1_ROI_TOP_LEFT_X           0       // 全区域左上角X
#define VL53L1_ROI_TOP_LEFT_Y           15      // 全区域左上角Y  
#define VL53L1_ROI_BOTTOM_RIGHT_X       15      // 全区域右下角X
#define VL53L1_ROI_BOTTOM_RIGHT_Y       0       // 全区域右下角Y

// 函数返回值定义
typedef enum {
    VL53L1_OK = 1,          // 有新数据
    VL53L1_NO_DATA = 0,     // 无新数据
    VL53L1_ERROR = -1       // 错误
} VL53L1_Status_t;

// 函数声明
void VL53L1_Read_Init(void);
void VL53L1_Process(void);
int VL53L1_GetDistance(uint16_t *distance);
uint8_t VL53L1_IsReady(void);
void VL53L1_Reset(void);

#endif // VL53L1_READ_H
