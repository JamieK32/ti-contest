#ifndef __iq_pid_h__
#define __iq_pid_h__
#include "IQmathLib.h"

// PID控制器类型定义
typedef enum {
    PID_TYPE_POSITION = 0,  // 位置式PID
    PID_TYPE_INCREMENT      // 增量式PID
} PID_Type_e;

// 积分抗饱和方法定义
typedef enum {
    ANTI_WINDUP_NONE = 0,       // 无抗饱和
    ANTI_WINDUP_CLAMPING,       // 积分夹紧法
    ANTI_WINDUP_BACK_CALC,      // 反向计算法
    ANTI_WINDUP_CONDITIONAL     // 条件积分法
} AntiWindup_Method_e;

// PID控制器结构体 - 所有成员都可以直接访问和修改
typedef struct {
    // PID参数 - 可直接设置
    _iq Kp;                 // 比例系数
    _iq Ki;                 // 积分系数  
    _iq Kd;                 // 微分系数
    
    // 控制目标和反馈 - 可直接设置
    _iq target;             // 目标值
    _iq feedback;           // 反馈值
    _iq error;              // 当前误差 (自动计算)
    _iq last_error;         // 上次误差 (自动更新)
    _iq last_last_error;    // 上上次误差 (自动更新)
    
    // 积分相关 - 可直接设置
    _iq integral;           // 积分累积值 (自动累积)
    _iq integral_max;       // 积分限幅最大值
    _iq integral_min;       // 积分限幅最小值
    _iq integral_separation_threshold; // 积分分离阈值
    
    // 积分抗饱和参数 - 新增
    _iq anti_windup_threshold;    // 积分抗饱和阈值
    _iq anti_windup_gain;         // 抗饱和增益系数(反向计算法用)
    AntiWindup_Method_e anti_windup_method; // 抗饱和方法
    
    // 输出相关 - 可直接设置
    _iq output;             // PID输出 (自动计算)
    _iq output_before_limit; // 限幅前的输出值 (用于抗饱和计算)
    _iq output_offset;      // 输出偏移量
    _iq output_max;         // 输出限幅最大值
    _iq output_min;         // 输出限幅最小值
    _iq last_output;        // 上次输出 (自动更新)
    
    // 死区处理 - 可直接设置
    _iq deadzone;           // 输入死区大小
    
    // 控制标志 - 可直接设置
    PID_Type_e type;        // PID类型
    uint16_t enable_integral_separation; // 积分分离使能 (1=开启, 0=关闭)
    uint16_t enable_integral_limit;      // 积分限幅使能 (1=开启, 0=关闭)
    uint16_t enable_output_limit;        // 输出限幅使能 (1=开启, 0=关闭)
    uint16_t enable_deadzone;            // 死区使能 (1=开启, 0=关闭)
    uint16_t enable_anti_windup;         // 积分抗饱和使能 (1=开启, 0=关闭)
    
} PID_Controller_t;

// iqmath与float转换宏定义
#define IQ_TO_FLOAT(iq_val)    _IQtoF(iq_val)     // _iq转float
#define FLOAT_TO_IQ(float_val) _IQ(float_val)     // float转_iq

// iqmath兼容性宏定义 - 解决缺失函数问题
#define IQ_ADD(a, b)           ((a) + (b))        // _iq加法
#define IQ_SUB(a, b)           ((a) - (b))        // _iq减法
#define IQ_CMP_GT(a, b)        ((a) > (b))        // _iq大于比较
#define IQ_CMP_LT(a, b)        ((a) < (b))        // _iq小于比较
#define IQ_CMP_GE(a, b)        ((a) >= (b))       // _iq大于等于比较
#define IQ_CMP_LE(a, b)        ((a) <= (b))       // _iq小于等于比较
#define IQ_ABS(a)              (((a) < 0) ? (-(a)) : (a))  // _iq绝对值

// 函数声明
void PID_Init(PID_Controller_t *pid, PID_Type_e type);
void PID_Reset(PID_Controller_t *pid);
_iq PID_Calculate(_iq target, _iq feedback, PID_Controller_t *pid);
_iq PID_PositionCalculate(PID_Controller_t *pid);
_iq PID_IncrementCalculate(PID_Controller_t *pid);

// 辅助函数
float PID_GetOutputFloat(PID_Controller_t *pid);
float PID_GetErrorFloat(PID_Controller_t *pid);
void PID_SetTargetFloat(PID_Controller_t *pid, float target);
void PID_SetFeedbackFloat(PID_Controller_t *pid, float feedback);

// 积分抗饱和相关函数
void PID_AntiWindupProcess(PID_Controller_t *pid);
uint16_t PID_CheckOutputSaturation(PID_Controller_t *pid);
void PID_SetAntiWindupMethod(PID_Controller_t *pid, AntiWindup_Method_e method, _iq threshold, _iq gain);

#endif