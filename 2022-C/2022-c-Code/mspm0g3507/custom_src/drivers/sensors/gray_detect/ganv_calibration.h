// sensor_calibration.h
#ifndef __SENSOR_CALIBRATION_H__
#define __SENSOR_CALIBRATION_H__

#include "no_mcu_ganv.h"

// 校准状态枚举
typedef enum {
    CALIB_IDLE = 0,        // 空闲状态
    CALIB_BLACK,           // 黑色校准中
    CALIB_WAIT,            // 等待状态
    CALIB_WHITE,           // 白色校准中
    CALIB_SUCCESS,         // 校准成功
    CALIB_FAILED           // 校准失败
} calib_state_t;

// 校准结果结构体
typedef struct {
    unsigned short black[8];    // 黑色校准值
    unsigned short white[8];    // 白色校准值
    calib_state_t state;        // 当前状态
} sensor_calib_t;

// API函数
void calib_init(sensor_calib_t* calib);
calib_state_t calib_process(sensor_calib_t* calib, No_MCU_Sensor* sensor);
unsigned char calib_get_result(sensor_calib_t* calib, unsigned short* black, unsigned short* white);

#endif