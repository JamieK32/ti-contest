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
						uint32_t channel = g_motor_system_config->motors[i].pwm_cc_index;
            if (timer_instance != NULL) {
                DL_Timer_startCounter(timer_instance);
								DL_Timer_setCaptureCompareValue(timer_instance, 0, channel);
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
                DL_Timer_stopCounter(timer_instance);
								DL_Timer_setCaptureCompareValue(timer_instance, 0, channel);
            } else {
                 log_w("Timer instance is NULL for motor %d enable!\n", i);
            }
        }
    }
}

static void tb6612_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, int *pwms) {
    uint16_t hc595_databyte = 0;  // 初始化为0
    
    for (int i = 0; i < g_motor_system_config->motor_count; i++) {
        // 检查电机是否启用
        if (!g_motor_system_config->motors[i].enabled) {
            // 禁用电机：IN1=0, IN2=0 (停止)
            hc595_databyte &= ~(1 << (i * 2));       // 清除 IN1
            hc595_databyte &= ~(1 << (i * 2 + 1));   // 清除 IN2
            continue;
        }
        
        bool polarity = g_motor_system_config->motors[i].polarity;
        
        if (pwms[i] > 0) {  // 正向运行
            if (polarity) {
                // 正向：IN1=1, IN2=0
                hc595_databyte |= (1 << (i * 2));       // 设置 IN1
                hc595_databyte &= ~(1 << (i * 2 + 1));   // 清除 IN2
            } else {
                // 正向（极性反转）：IN1=0, IN2=1
                hc595_databyte &= ~(1 << (i * 2));       // 清除 IN1
                hc595_databyte |= (1 << (i * 2 + 1));    // 设置 IN2
            }
        } else if (pwms[i] < 0) {  // 反向运行
            if (polarity) {
                // 反向：IN1=0, IN2=1
                hc595_databyte &= ~(1 << (i * 2));       // 清除 IN1
                hc595_databyte |= (1 << (i * 2 + 1));    // 设置 IN2
            } else {
                // 反向（极性反转）：IN1=1, IN2=0
                hc595_databyte |= (1 << (i * 2));        // 设置 IN1
                hc595_databyte &= ~(1 << (i * 2 + 1));   // 清除 IN2
            }
        } else {  // pwms[i] == 0，停止
            // 停止：IN1=0, IN2=0
            hc595_databyte &= ~(1 << (i * 2));       // 清除 IN1
            hc595_databyte &= ~(1 << (i * 2 + 1));   // 清除 IN2
        }
    }
    
    hc595_output_byte(hc595_databyte);
    
    // 设置PWM占空比
    for (int i = 0; i < g_motor_system_config->motor_count; i++) {
        if (!g_motor_system_config->motors[i].enabled) {
            DL_Timer_setCaptureCompareValue(
                g_motor_system_config->motors[i].timer_instance, 
                0, 
                g_motor_system_config->motors[i].pwm_cc_index
            );
            continue;
        }
        DL_Timer_setCaptureCompareValue(
            g_motor_system_config->motors[i].timer_instance, 
            abs(pwms[i]), 
            g_motor_system_config->motors[i].pwm_cc_index
        );
    }
}