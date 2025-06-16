// motor_l298n.c

#include "motor_hardware.h" // 包含头文件
// 不使用该功能 // #include "log_config.h"
#include "log.h"
#include <stdio.h> // 为了 log 输出


// 声明 L298N 驱动内部函数 (static)
static void l298n_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void l298n_disable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void l298n_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, uint8_t num, int pwm); // 原有的按电机编号设置 PWM

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

    // 在 FreeRTOS 任务中，考虑使用 RTOS 临界区保护对寄存器的访问
    // 在中断服务程序中，使用单片机特定的中断禁用函数保护
    // 考虑到该函数可能在任务和中断中被调用，直接禁用中断是最简单的保护方式，但会影响实时性。
    // 如果底层 DL_Timer_setCaptureCompareValue 本身是中断安全的，或者你只在任务中调用此函数，则可能不需要额外的保护。
    // 在你的低复杂度要求下，如果需要保护，可以考虑在这里禁用中断。

    // uint32_t primask;
    // __asm__ volatile("mrs %0, primask" : "=r" (primask)); // 保存中断状态
    // __asm__ volatile("cpsid i"); // 禁用全局中断

    // 根据你的宏定义逻辑实现 PWM 和方向控制
    // 假设使用 DL_Timer_setCaptureCompareValue 设置两个通道的占空比来控制方向和速度
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

    // 恢复中断状态
    // if ((primask & 1) == 0) {
    //      __asm__ volatile("cpsie i");
    // }
}


// L298N 驱动内部函数实现
static void l298n_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config) {
    // 遍历配置结构体，只对启用的电机使能定时器
    for (int i = 0; i < NUM_MOTORS; i++) {
        if (g_motor_system_config->motors[i].enabled) {
            GPTIMER_Regs* timer_instance = g_motor_system_config->motors[i].timer_instance;
            if (timer_instance != NULL) {
                // 使用通用的定时器启动函数 (如果 DL 库提供的话)
                // 或者根据实例类型判断调用不同的启动函数
                // 你的原始代码使用了 DL_TimerG_startCounter 和 DL_TimerA_startCounter
                // MSPM0 的 GPTIMER 实际上是统一的 GPTIMER 外设，只是有些实例是 16/32 位，有些是 16 位。
                // DL 库可能提供了通用的 GPTIMER 函数。假设 DL_Timer_startCounter 适用于所有 GPTIMER 实例。
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
static void l298n_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, uint8_t num, int pwm) {
     if (num >= NUM_MOTORS) {
        log_e("Invalid motor number: %d\n", num);
        return;
    }

    // 获取对应电机的配置
    const MotorConfig* config = &g_motor_system_config->motors[num];

    // 检查电机是否启用
    if (!config->enabled) {
        // 如果电机未启用，设置为停止状态
        set_motor_pwm_l298n(config, 0); // 调用新的函数并设置为停止
        return; // 不处理禁用电机的 PWM 设置请求
    }

    // 应用限幅
    int limited_pwm = amplitude_limit(pwm, g_motor_system_config->max_pwm_value);

    // 调用新的函数设置单个电机的 PWM
    set_motor_pwm_l298n(config, limited_pwm);
}
