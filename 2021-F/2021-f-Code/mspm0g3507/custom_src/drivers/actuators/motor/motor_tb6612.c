// motor_tb6612.c

#include "motor_hardware.h" 
#include "_74hc595.h"
#include "log.h"
#include <stdio.h> // 为了 log 输出
#include <stdlib.h> // 为了 abs() 函数

// 声明 TB6612 驱动内部函数 (static)
static void tb6612_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void tb6612_disable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void tb6612_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, int *pwms);

// 具体的 TB6612 驱动接口实现
motorHardWareInterface tb6612_interface = {
    .disable_all_motor = tb6612_disable_all_motor_impl,
    .enable_all_motor = tb6612_enable_all_motor_impl,
    .set_pwms = tb6612_set_pwms_impl, // 原有的按编号设置函数
};


static void tb6612_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config) {
    // 遍历配置结构体，只对启用的电机使能定时器
    for (int i = 0; i < NUM_MOTORS; i++) {
        if (g_motor_system_config->motors[i].enabled) {
            GPTIMER_Regs* timer_instance = g_motor_system_config->motors[i].timer_instance;
            if (timer_instance != NULL) {
                DL_Timer_startCounter(timer_instance);
            } else {
                 log_w("Timer instance is NULL for motor %d enable!\n", i);
            }
        }
    }
}

static void tb6612_disable_all_motor_impl(const MotorSystemConfig* g_motor_system_config) {
    // 遍历配置结构体，只对启用的电机使能定时器
    for (int i = 0; i < NUM_MOTORS; i++) {
        if (g_motor_system_config->motors[i].enabled) {
            GPTIMER_Regs* timer_instance = g_motor_system_config->motors[i].timer_instance;
						uint32_t channel = g_motor_system_config->motors[i].pwm_cc_index;
            if (timer_instance != NULL) {
								DL_Timer_setCaptureCompareValue(timer_instance, 0, channel);
                DL_Timer_stopCounter(timer_instance);
            } else {
                 log_w("Timer instance is NULL for motor %d enable!\n", i);
            }
        }
    }
}

static void tb6612_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, int *pwms) {
    if (pwms == NULL || g_motor_system_config == NULL) {
        log_e("Invalid parameters in tb6612_set_pwms_impl\n");
        return;
    }
    
    uint16_t hc595_databyte = 0;  
    
    // 确保不会数组越界
    int motor_count = ((int)g_motor_system_config->motor_count > NUM_MOTORS) ? 
                      NUM_MOTORS : g_motor_system_config->motor_count;
    
    // 设置方向控制
    for (int i = 0; i < motor_count; i++) {
        if (!g_motor_system_config->motors[i].enabled) {
            // 禁用电机：IN1=0, IN2=0 (停止)
            hc595_databyte &= ~(1 << (i * 2));       
            hc595_databyte &= ~(1 << (i * 2 + 1));   
            continue;
        }
        
        bool polarity = g_motor_system_config->motors[i].polarity;
        
        if (pwms[i] > 0) {  
            if (polarity) {
                hc595_databyte |= (1 << (i * 2));       
                hc595_databyte &= ~(1 << (i * 2 + 1));   
            } else {
                hc595_databyte &= ~(1 << (i * 2));       
                hc595_databyte |= (1 << (i * 2 + 1));    
            }
        } else if (pwms[i] < 0) {  
            if (polarity) {
                hc595_databyte &= ~(1 << (i * 2));       
                hc595_databyte |= (1 << (i * 2 + 1));    
            } else {
                hc595_databyte |= (1 << (i * 2));        
                hc595_databyte &= ~(1 << (i * 2 + 1));   
            }
        } else {  // pwms[i] == 0
            hc595_databyte &= ~(1 << (i * 2));       
            hc595_databyte &= ~(1 << (i * 2 + 1));   
        }
    }
    
    // 输出方向控制信号
    hc595_output_byte(hc595_databyte);
    
    // 设置PWM占空比
    for (int i = 0; i < motor_count; i++) {
        GPTIMER_Regs* timer_instance = g_motor_system_config->motors[i].timer_instance;
        uint32_t pwm_cc_index = g_motor_system_config->motors[i].pwm_cc_index;
        
        if (timer_instance == NULL) {
            log_e("Timer instance is NULL for motor %d\n", i);
            continue;
        }
        
        if (!g_motor_system_config->motors[i].enabled) {
            DL_Timer_setCaptureCompareValue(timer_instance, 0, pwm_cc_index);
        } else {
            // 使用绝对值设置PWM，方向已经由74HC595控制
            uint32_t pwm_value = (uint32_t)abs(pwms[i]);
            // 可以添加PWM值限制检查
            if (pwm_value > g_motor_system_config->max_pwm_value) {
                pwm_value = g_motor_system_config->max_pwm_value;
            }
            DL_Timer_setCaptureCompareValue(timer_instance, pwm_value, pwm_cc_index);
        }
    }
}