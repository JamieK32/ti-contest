#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

// PID控制器类型定义
typedef enum {
    PID_TYPE_POSITION = 0,  // 位置式PID
    PID_TYPE_INCREMENT      // 增量式PID
} PID_Type_e;

// PID控制器结构体 - 简化版本，只保留基本功能
typedef struct {
    // PID参数 - 可直接设置
    float Kp;               // 比例系数
    float Ki;               // 积分系数  
    float Kd;               // 微分系数
    
    // 控制目标和反馈 - 可直接设置
    float target;           // 目标值
    float feedback;         // 反馈值
    float error;            // 当前误差 (自动计算)
    float last_error;       // 上次误差 (自动更新)
    float last_last_error;  // 上上次误差 (自动更新)
    
    // 积分相关 - 可直接设置
    float integral;         // 积分累积值 (自动累积)
    float integral_max;     // 积分限幅最大值
    float integral_min;     // 积分限幅最小值
    float integral_separation_threshold; // 积分分离阈值
    
    // 输出相关 - 可直接设置
    float output;           // PID输出 (自动计算)
    float output_offset;    // 输出偏移量
    float output_max;       // 输出限幅最大值
    float output_min;       // 输出限幅最小值
    float last_output;      // 上次输出 (自动更新)
    
    // 死区处理 - 可直接设置
    float deadzone;         // 输入死区大小
    
    // 控制标志 - 可直接设置
    PID_Type_e type;        // PID类型
    uint16_t enable_integral_separation; // 积分分离使能 (1=开启, 0=关闭)
    uint16_t enable_integral_limit;      // 积分限幅使能 (1=开启, 0=关闭)
    uint16_t enable_output_limit;        // 输出限幅使能 (1=开启, 0=关闭)
    uint16_t enable_deadzone;            // 死区使能 (1=开启, 0=关闭)
    
} PID_Controller_t;

// 函数声明
void PID_Init(PID_Controller_t *pid, PID_Type_e type);
void PID_Reset(PID_Controller_t *pid);
float PID_Calculate(float target, float feedback, PID_Controller_t *pid);
float PID_PositionCalculate(PID_Controller_t *pid);
float PID_IncrementCalculate(PID_Controller_t *pid);

// 辅助函数
void PID_SetTarget(PID_Controller_t *pid, float target);
void PID_SetFeedback(PID_Controller_t *pid, float feedback);
float PID_GetOutput(PID_Controller_t *pid);
float PID_GetError(PID_Controller_t *pid);

// 参数设置函数
void PID_SetParams(PID_Controller_t *pid, float kp, float ki, float kd);
void PID_SetIntegralLimit(PID_Controller_t *pid, float max_val, float min_val);
void PID_SetOutputLimit(PID_Controller_t *pid, float max_val, float min_val);
void PID_SetDeadzone(PID_Controller_t *pid, float deadzone);
void PID_SetIntegralSeparation(PID_Controller_t *pid, float threshold);

#endif // __FLOAT_PID_H__
