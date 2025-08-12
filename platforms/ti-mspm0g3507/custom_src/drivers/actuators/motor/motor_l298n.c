// motor_l298n.c

#include "motor_hardware.h" // 包含头文件
// 不使用该功能 // #include "log_config.h"
#include "log.h"
#include <stdio.h> // 为了 log 输出


// 声明 L298N 驱动内部函数 (static)
static void l298n_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void l298n_disable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void l298n_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, int *pwms);

// 具体的 L298N 驱动接口实现
motorHardWareInterface l298n_interface = {
    .disable_all_motor = l298n_disable_all_motor_impl,
    .enable_all_motor = l298n_enable_all_motor_impl,
    .set_pwms = l298n_set_pwms_impl, // 原有的按编号设置函数
};

// 函数实现设置单个电机 PWM 和方向
// 这将取代 SET_MOTOR_PWM 宏
// 改为 static inline 函数，避免外部链接问题，并在编译时展开
static inline void set_motor_pwm_l298n(const MotorConfig* config, int pwm) {
    // 确保定时器实例有效
    if (config == NULL || config->timer_instance == NULL) {
        log_e("Invalid motor config or timer instance!\n"); // 使用 log 宏
        return;
    }
    if (pwm > 0) { // 正向
        // 设置反向通道占空比为 0
        DL_Timer_setCaptureCompareValue(config->timer_instance, 0, config->cc_reverse_pwm_index);
        // 设置正向通道占空比为 pwm (绝对值)
        DL_Timer_setCaptureCompareValue(config->timer_instance, (uint32_t)pwm, config->cc_forward_pwm_index);
    } else if (pwm < 0) { // 反向
        int abs_pwm = -pwm;
         // 设置正向通道占空比为 0
        DL_Timer_setCaptureCompareValue(config->timer_instance, 0, config->cc_forward_pwm_index);
        // 设置反向通道占空比为 pwm 的绝对值
        DL_Timer_setCaptureCompareValue(config->timer_instance, (uint32_t)abs_pwm, config->cc_reverse_pwm_index);
    } else { // 停止 (PWM = 0)
        // 两个通道占空比都设置为 0
        DL_Timer_setCaptureCompareValue(config->timer_instance, 0, config->cc_reverse_pwm_index);
        DL_Timer_setCaptureCompareValue(config->timer_instance, 0, config->cc_forward_pwm_index);
    }

}


// L298N 驱动内部函数实现
static void l298n_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config) {
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

static void l298n_disable_all_motor_impl(const MotorSystemConfig* g_motor_system_config) {
    // 遍历配置结构体，只对启用的电机禁用定时器
     for (int i = 0; i < NUM_MOTORS; i++) {
        if (g_motor_system_config->motors[i].enabled) {
             GPTIMER_Regs* timer_instance = g_motor_system_config->motors[i].timer_instance;
             if (timer_instance != NULL) {
                 // 使用通用的定时器停止函数 (如果 DL 库提供的话)
                 DL_Timer_stopCounter(timer_instance);
             } else {
                 log_w("Timer instance is NULL for motor %d disable!\n", i);
             }
        }
    }
}

// 原有的按电机编号设置 PWM 函数，现在内部调用新的函数
static void l298n_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, int *pwms) {
    for (int i = 0; i < g_motor_system_config->motor_count; i++) {
			MotorConfig config = g_motor_system_config->motors[i];
			if (!config.enabled) {
					set_motor_pwm_l298n(&config, 0);
					continue; // 不处理禁用电机的 PWM 设置请求
			}
			// 应用限幅
			int limited_pwm = amplitude_limit(pwms[i], g_motor_system_config->max_pwm_value);
			// 调用新的函数设置单个电机的 PWM
			set_motor_pwm_l298n(&config, limited_pwm);
	}
}