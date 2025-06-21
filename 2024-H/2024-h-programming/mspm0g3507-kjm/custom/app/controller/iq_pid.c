#include "iq_pid.h"

// float转换辅助函数
float PID_GetOutputFloat(PID_Controller_t *pid) {
    return IQ_TO_FLOAT(pid->output);
}

float PID_GetErrorFloat(PID_Controller_t *pid) {
    return IQ_TO_FLOAT(pid->error);
}

void PID_SetTargetFloat(PID_Controller_t *pid, float target) {
    pid->target = FLOAT_TO_IQ(target);
}

void PID_SetFeedbackFloat(PID_Controller_t *pid, float feedback) {
    pid->feedback = FLOAT_TO_IQ(feedback);
}

// PID控制器初始化 - 设置默认值
void PID_Init(PID_Controller_t *pid, PID_Type_e type) {
    // 清零所有参数
    pid->Kp = _IQ(0.0);
    pid->Ki = _IQ(0.0);
    pid->Kd = _IQ(0.0);
    
    pid->target = _IQ(0.0);
    pid->feedback = _IQ(0.0);
    pid->error = _IQ(0.0);
    pid->last_error = _IQ(0.0);
    pid->last_last_error = _IQ(0.0);
    
    pid->integral = _IQ(0.0);
    pid->integral_max = _IQ(1000.0);        // 默认积分限幅
    pid->integral_min = _IQ(-1000.0);
    pid->integral_separation_threshold = _IQ(100.0); // 默认积分分离阈值
    
    // 积分抗饱和参数初始化
    pid->anti_windup_threshold = _IQ(50.0);  // 默认抗饱和阈值
    pid->anti_windup_gain = _IQ(0.1);        // 默认抗饱和增益
    pid->anti_windup_method = ANTI_WINDUP_CLAMPING; // 默认使用积分夹紧法
    
    pid->output = _IQ(0.0);
    pid->output_before_limit = _IQ(0.0);    // 限幅前输出
    pid->output_offset = _IQ(0.0);          // 默认无输出偏移
    pid->output_max = _IQ(3000.0);          // 默认输出限幅
    pid->output_min = _IQ(-3000.0);
    pid->last_output = _IQ(0.0);
    
    pid->deadzone = _IQ(0.0);               // 默认无死区
    
    pid->type = type;
    pid->enable_integral_separation = 0;     // 默认关闭积分分离
    pid->enable_integral_limit = 1;          // 默认开启积分限幅
    pid->enable_output_limit = 1;            // 默认开启输出限幅
    pid->enable_deadzone = 0;                // 默认关闭死区
    pid->enable_anti_windup = 1;             // 默认开启积分抗饱和
}

// 死区处理函数
_iq PID_DeadzoneProcess(_iq error, PID_Controller_t *pid) {
    if (!pid->enable_deadzone) {
        return error;
    }
    
    _iq abs_error = IQ_ABS(error);
    if (IQ_CMP_LT(abs_error, pid->deadzone)) {
        return _IQ(0.0);
    }
    
    // 去除死区后的误差
    if (IQ_CMP_GT(error, _IQ(0.0))) {
        return IQ_SUB(error, pid->deadzone);
    } else {
        return IQ_ADD(error, pid->deadzone);
    }
}

// 积分分离处理
uint16_t PID_IntegralSeparationCheck(_iq error, PID_Controller_t *pid) {
    if (!pid->enable_integral_separation) {
        return 1; // 不使用积分分离，允许积分
    }
    
    _iq abs_error = IQ_ABS(error);
    return IQ_CMP_LT(abs_error, pid->integral_separation_threshold) ? 1 : 0;
}

// 积分限幅处理
void PID_IntegralLimitProcess(PID_Controller_t *pid) {
    if (!pid->enable_integral_limit) {
        return;
    }
    
    if (IQ_CMP_GT(pid->integral, pid->integral_max)) {
        pid->integral = pid->integral_max;
    } else if (IQ_CMP_LT(pid->integral, pid->integral_min)) {
        pid->integral = pid->integral_min;
    }
}

// 检查输出是否饱和
uint16_t PID_CheckOutputSaturation(PID_Controller_t *pid) {
    if (!pid->enable_output_limit) {
        return 0; // 未开启输出限幅，不会饱和
    }
    
    // 检查限幅前的输出是否超出限制
    if (IQ_CMP_GT(pid->output_before_limit, pid->output_max) ||
        IQ_CMP_LT(pid->output_before_limit, pid->output_min)) {
        return 1; // 输出饱和
    }
    return 0; // 输出未饱和
}

// 积分抗饱和处理
void PID_AntiWindupProcess(PID_Controller_t *pid) {
    if (!pid->enable_anti_windup) {
        return;
    }
    
    switch (pid->anti_windup_method) {
        case ANTI_WINDUP_CLAMPING:
            // 积分夹紧法：当输出饱和时，停止积分累积
            if (PID_CheckOutputSaturation(pid)) {
                // 如果误差与积分项同号，则停止积分
                _iq integral_term = _IQmpy(pid->Ki, pid->integral);
                if ((IQ_CMP_GT(pid->error, _IQ(0.0)) && IQ_CMP_GT(integral_term, _IQ(0.0))) ||
                    (IQ_CMP_LT(pid->error, _IQ(0.0)) && IQ_CMP_LT(integral_term, _IQ(0.0)))) {
                    // 撤销本次积分累积
                    pid->integral = IQ_SUB(pid->integral, pid->error);
                }
            }
            break;
            
        case ANTI_WINDUP_BACK_CALC:
            // 反向计算法：根据输出饱和量反向修正积分项
            if (PID_CheckOutputSaturation(pid)) {
                _iq saturation_error = IQ_SUB(pid->output, pid->output_before_limit);
                _iq correction = _IQmpy(pid->anti_windup_gain, saturation_error);
                pid->integral = IQ_SUB(pid->integral, correction);
            }
            break;
            
        case ANTI_WINDUP_CONDITIONAL:
            // 条件积分法：当误差小于阈值时才进行积分
            if (IQ_CMP_GT(IQ_ABS(pid->error), pid->anti_windup_threshold)) {
                // 误差较大时，减少积分累积
                pid->integral = IQ_SUB(pid->integral, pid->error);
            }
            break;
            
        default:
            // ANTI_WINDUP_NONE 或其他情况不做处理
            break;
    }
}

// 输出限幅处理
void PID_OutputLimitProcess(PID_Controller_t *pid) {
    // 保存限幅前的输出值，用于抗饱和计算
    pid->output_before_limit = pid->output;
    
    if (!pid->enable_output_limit) {
        return;
    }
    
    if (IQ_CMP_GT(pid->output, pid->output_max)) {
        pid->output = pid->output_max;
    } else if (IQ_CMP_LT(pid->output, pid->output_min)) {
        pid->output = pid->output_min;
    }
}

// 位置式PID计算
_iq PID_PositionCalculate(PID_Controller_t *pid) {
    _iq proportional, integral, differential;
    
    // 计算误差并进行死区处理
    pid->error = IQ_SUB(pid->target, pid->feedback);
    pid->error = PID_DeadzoneProcess(pid->error, pid);
    
    // 比例项
    proportional = _IQmpy(pid->Kp, pid->error);
    
    // 积分项 - 检查积分分离
    if (PID_IntegralSeparationCheck(pid->error, pid)) {
        pid->integral = IQ_ADD(pid->integral, pid->error);
        PID_IntegralLimitProcess(pid); // 积分限幅
    }
    integral = _IQmpy(pid->Ki, pid->integral);
    
    // 微分项
    _iq error_diff = IQ_SUB(pid->error, pid->last_error);
    differential = _IQmpy(pid->Kd, error_diff);
    
    // 计算输出
    pid->output = IQ_ADD(IQ_ADD(proportional, integral), differential);
    pid->output = IQ_ADD(pid->output, pid->output_offset);
    
    // 输出限幅
    PID_OutputLimitProcess(pid);
    
    // 积分抗饱和处理
    PID_AntiWindupProcess(pid);
    
    // 更新历史误差
    pid->last_error = pid->error;
    
    return pid->output;
}

// 增量式PID计算
_iq PID_IncrementCalculate(PID_Controller_t *pid) {
    _iq delta_output;
    _iq proportional_delta, integral_delta, differential_delta;
    
    // 计算误差并进行死区处理
    pid->error = IQ_SUB(pid->target, pid->feedback);
    pid->error = PID_DeadzoneProcess(pid->error, pid);
    
    // 增量式PID算法：Δu(k) = Kp[e(k)-e(k-1)] + Ki*e(k) + Kd[e(k)-2e(k-1)+e(k-2)]
    
    // 比例增量
    proportional_delta = _IQmpy(pid->Kp, IQ_SUB(pid->error, pid->last_error));
    
    // 积分增量 - 检查积分分离
    if (PID_IntegralSeparationCheck(pid->error, pid)) {
        integral_delta = _IQmpy(pid->Ki, pid->error);
    } else {
        integral_delta = _IQ(0.0);
    }
    
    // 微分增量
    _iq error_diff2 = IQ_SUB(IQ_SUB(pid->error, _IQmpy(_IQ(2.0), pid->last_error)), pid->last_last_error);
    differential_delta = _IQmpy(pid->Kd, error_diff2);
    
    // 计算输出增量
    delta_output = IQ_ADD(IQ_ADD(proportional_delta, integral_delta), differential_delta);
    
    // 计算新的输出
    pid->output = IQ_ADD(pid->last_output, delta_output);
    pid->output = IQ_ADD(pid->output, pid->output_offset);
    
    // 输出限幅
    PID_OutputLimitProcess(pid);
    
    // 对于增量式PID，抗饱和主要通过限制增量来实现
    if (pid->enable_anti_windup && PID_CheckOutputSaturation(pid)) {
        // 如果输出饱和，限制本次的积分增量
        switch (pid->anti_windup_method) {
            case ANTI_WINDUP_CLAMPING:
                // 撤销造成饱和的积分增量
                if ((IQ_CMP_GT(pid->error, _IQ(0.0)) && 
                     IQ_CMP_GT(pid->output_before_limit, pid->output_max)) ||
                    (IQ_CMP_LT(pid->error, _IQ(0.0)) && 
                     IQ_CMP_LT(pid->output_before_limit, pid->output_min))) {
                    // 重新计算输出，不包含积分增量
                    pid->output = IQ_ADD(pid->last_output, 
                                        IQ_ADD(proportional_delta, differential_delta));
                    pid->output = IQ_ADD(pid->output, pid->output_offset);
                    PID_OutputLimitProcess(pid);
                }
                break;
                
            case ANTI_WINDUP_CONDITIONAL:
                // 当误差过大时不使用积分增量
                if (IQ_CMP_GT(IQ_ABS(pid->error), pid->anti_windup_threshold)) {
                    pid->output = IQ_ADD(pid->last_output, 
                                        IQ_ADD(proportional_delta, differential_delta));
                    pid->output = IQ_ADD(pid->output, pid->output_offset);
                    PID_OutputLimitProcess(pid);
                }
                break;
                
            default:
                break;
        }
    }
    
    // 更新历史值
    pid->last_last_error = pid->last_error;
    pid->last_error = pid->error;
    pid->last_output = pid->output;
    
    return pid->output;
}

// PID主计算函数
_iq PID_Calculate(_iq target, _iq feedback, PID_Controller_t *pid) {
    pid->target = target;
    pid->feedback = feedback;
    
    if (pid->type == PID_TYPE_POSITION) {
        return PID_PositionCalculate(pid);
    } else {
        return PID_IncrementCalculate(pid);
    }
}

// 设置积分抗饱和方法和参数
void PID_SetAntiWindupMethod(PID_Controller_t *pid, AntiWindup_Method_e method, 
                            _iq threshold, _iq gain) {
    pid->anti_windup_method = method;
    pid->anti_windup_threshold = threshold;
    pid->anti_windup_gain = gain;
    pid->enable_anti_windup = (method != ANTI_WINDUP_NONE) ? 1 : 0;
}

// 重置PID控制器
void PID_Reset(PID_Controller_t *pid) {
    pid->error = _IQ(0.0);
    pid->last_error = _IQ(0.0);
    pid->last_last_error = _IQ(0.0);
    pid->integral = _IQ(0.0);
    pid->output = _IQ(0.0);
    pid->output_before_limit = _IQ(0.0);
    pid->last_output = _IQ(0.0);
}