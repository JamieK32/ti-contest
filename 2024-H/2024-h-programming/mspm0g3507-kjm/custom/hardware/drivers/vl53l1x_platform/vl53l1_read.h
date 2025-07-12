#ifndef __VL53L1_READ_H__
#define __VL53L1_READ_H__

#include "vl53l1_platform.h"
#include "vl53l1_api.h"
#include "sw_i2c.h"
#include "common_include.h"

// 简化的测量结果
typedef enum {
    VL53L1_OK = 0,          // 测量成功
    VL53L1_NO_DATA,         // 暂无数据
    VL53L1_ERROR            // 测量错误
} VL53L1_Status_t;

// 内部状态（用户不需要关心）
typedef enum {
    STATE_INIT = 0,
    STATE_IDLE,
    STATE_MEASURING,
    STATE_ERROR
} VL53L1_InternalState_t;

typedef struct {
    VL53L1_InternalState_t state;
    uint16_t distance_mm;
    uint32_t tick_count;        // 添加这个字段
    uint32_t last_measure_time;
    uint8_t data_ready;
    uint8_t init_done;
} VL53L1_Context_t;

extern VL53L1_Dev_t sensor;

// 最简单的API - 只需要这3个函数
void VL53L1_Read_Init(void);                              // 初始化
void VL53L1_Process(void);                                 // 处理函数(10ms调用)
VL53L1_Status_t VL53L1_GetDistance(uint16_t *distance);   // 获取距离

#endif /* __VL53L1_READ_H__ */