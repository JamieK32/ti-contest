// motor_tb6612.c

#include "motor_hardware.h" // 包含头文件
// 不使用该功能 // #include "log_config.h"
#include "log.h"
#include <stdio.h> // 为了 log 输出
#include <stdlib.h> // 为了 abs() 函数

// 声明 TB6612 驱动内部函数 (static)
static void tb6612_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void tb6612_disable_all_motor_impl(const MotorSystemConfig* g_motor_system_config);
static void tb6612_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, uint8_t num, int pwm); // 原有的按电机编号设置 PWM

// 具体的 TB6612 驱动接口实现
motorHardWareInterface tb6612_interface = {
    .disable_all_motor = tb6612_disable_all_motor_impl,
    .enable_all_motor = tb6612_enable_all_motor_impl,
    .set_pwms = tb6612_set_pwms_impl, // 原有的按编号设置函数
};

// 函数实现设置单个电机 PWM 和方向
// 改为 static inline 函数，避免外部链接问题，并在编译时展开
static inline void set_motor_pwm_tb6612(const MotorConfig* config, int pwm) {
    // 确保配置和定时器实例有效
    if (config == NULL || config->timer_instance == NULL) {
        log_e("Invalid motor config or timer instance!\n");
        return;
    }

    uint32_t abs_pwm = (uint32_t)abs(pwm); // 获取 PWM 的绝对值

    // 在 FreeRTOS 任务中，考虑使用 RTOS 临界区保护对寄存器的访问
    // 在中断服务程序中，使用单片机特定的中断禁用函数保护
    // 与 L298N 类似，这里也面临在任务和中断中调用的可能性。
    // 考虑到低复杂度原则，如果需要保护，最简单的方式是禁用中断。
    // 但这会影响实时性，特别是在中断中调用时。
    // 理想情况下，应该根据调用环境来选择保护机制。
    // 在 FreeRTOS 任务中，可以使用 taskENTER_CRITICAL() / taskEXIT_CRITICAL() 或 mutex。
    // 在中断中，使用 __disable_irq() / __enable_irq() 或单片机特定的中断屏蔽寄存器。
    // 为了简化，这里暂时不添加保护，假设 DL 函数是中断安全的，或者只在任务中调用。
    // 如果需要在中断中安全调用，则需要更复杂的保护机制。

    // uint32_t primask;
    // __asm__ volatile("mrs %0, primask" : "=r" (primask)); // 保存中断状态
    // __asm__ volatile("cpsid i"); // 禁用全局中断

    if (pwm > 0) { // 正向
        // 设置方向 GPIOs
        if (config->dir_gpio != NULL) {
            // 通常一个高一个低表示方向
            DL_GPIO_setPins(config->dir_gpio, config->dir_pin1);
            DL_GPIO_clearPins(config->dir_gpio, config->dir_pin2);
        } else {
             log_w("Direction GPIO config is NULL for motor!\n");
        }
        // 设置 PWM 通道占空比
        DL_Timer_setCaptureCompareValue(config->timer_instance, abs_pwm, config->pwm_cc_index);
    } else if (pwm < 0) { // 反向
         // 设置方向 GPIOs
        if (config->dir_gpio != NULL) {
            // 反过来设置方向
            DL_GPIO_clearPins(config->dir_gpio, config->dir_pin1);
            DL_GPIO_setPins(config->dir_gpio, config->dir_pin2);
        } else {
            log_w("Direction GPIO config is NULL for motor!\n");
        }
        // 设置 PWM 通道占空比
        DL_Timer_setCaptureCompareValue(config->timer_instance, abs_pwm, config->pwm_cc_index);
    } else { // 停止 (PWM = 0)
        // 设置方向 GPIOs (通常两个引脚都拉低表示停止/刹车)
         if (config->dir_gpio != NULL) {
            DL_GPIO_clearPins(config->dir_gpio, config->dir_pin1 | config->dir_pin2); // 两个引脚都拉低表示停止
        } else {
             log_w("Direction GPIO config is NULL for motor!\n");
        }
        // 设置 PWM 通道占空比为 0
        DL_Timer_setCaptureCompareValue(config->timer_instance, 0, config->pwm_cc_index);
    }

    // 恢复中断状态
    // if ((primask & 1) == 0) {
    //      __asm__ volatile("cpsie i");
    // }
}

// TB6612 驱动内部函数实现
static void tb6612_enable_all_motor_impl(const MotorSystemConfig* g_motor_system_config) {
    // 遍历配置结构体，只对启用的电机进行操作
    for (int i = 0; i < NUM_MOTORS; i++) {
        if (g_motor_system_config->motors[i].enabled) {
             // 使能 Standby 引脚 (如果配置了)
             if (g_motor_system_config->motors[i].stby_gpio != NULL) {
                 DL_GPIO_setPins(g_motor_system_config->motors[i].stby_gpio, g_motor_system_config->motors[i].stby_pin);
             } else {
                 // 只有配置了 Standby 引脚才打印警告
                 if (g_motor_system_config->motors[i].stby_pin != 0) { // 假设 stby_pin 为 0 表示未配置
                    log_w("Standby GPIO config is NULL for motor %d enable!\n", i);
                 }
             }

            // 启动定时器 (如果配置了)
            GPTIMER_Regs* timer_instance = g_motor_system_config->motors[i].timer_instance;
            if (timer_instance != NULL) {
                DL_Timer_startCounter(timer_instance);
            } else {
                // 只有配置了 Timer 实例才打印警告
                // 如果电机是步进电机或其它不需要 PWM 的类型，timer_instance 可能为 NULL，这是正常的
                // 你可能需要在 MotorConfig 中添加一个类型字段来区分
                 log_w("Timer instance is NULL for motor %d enable!\n", i);
            }
        }
    }
}

static void tb6612_disable_all_motor_impl(const MotorSystemConfig* g_motor_system_config) {
     // 遍历配置结构体，只对启用的电机进行操作
     for (int i = 0; i < NUM_MOTORS; i++) {
        if (g_motor_system_config->motors[i].enabled) {
             // 停止定时器 (如果配置了)
             GPTIMER_Regs* timer_instance = g_motor_system_config->motors[i].timer_instance;
             if (timer_instance != NULL) {
                 DL_Timer_stopCounter(timer_instance);
             } else {
                 // 只有配置了 Timer 实例才打印警告
                 if (g_motor_system_config->motors[i].pwm_cc_index != 0xFF) { // 假设 0xFF 表示未配置 PWM CC Index
                      log_w("Timer instance is NULL for motor %d disable!\n", i);
                 }
             }

             // 禁用 Standby 引脚 (如果配置了)
             if (g_motor_system_config->motors[i].stby_gpio != NULL) {
                 DL_GPIO_clearPins(g_motor_system_config->motors[i].stby_gpio, g_motor_system_config->motors[i].stby_pin);
             } else {
                 // 只有配置了 Standby 引脚才打印警告
                 if (g_motor_system_config->motors[i].stby_pin != 0) {
                     log_w("Standby GPIO config is NULL for motor %d disable!\n", i);
                 }
             }

             // 将方向引脚也拉低，确保电机完全停止
             if (g_motor_system_config->motors[i].dir_gpio != NULL) {
                 DL_GPIO_clearPins(g_motor_system_config->motors[i].dir_gpio,
                                   g_motor_system_config->motors[i].dir_pin1 |
                                   g_motor_system_config->motors[i].dir_pin2);
             } else {
                 // 只有配置了方向引脚才打印警告
                  if (g_motor_system_config->motors[i].dir_pin1 != 0 || g_motor_system_config->motors[i].dir_pin2 != 0) {
                     log_w("Direction GPIO config is NULL for motor %d disable!\n", i);
                  }
             }
        }
    }
}

// 原有的按电机编号设置 PWM 函数，现在内部调用新的函数
static void tb6612_set_pwms_impl(const MotorSystemConfig* g_motor_system_config, uint8_t num, int pwm) {
     if (num >= NUM_MOTORS) {
        log_e("Invalid motor number: %d\n", num);
        return;
    }

    // 获取对应电机的配置
    const MotorConfig* config = &g_motor_system_config->motors[num];

    // 检查电机是否启用
    if (!config->enabled) {
        // 如果电机未启用，设置为停止状态
        set_motor_pwm_tb6612(config, 0); // 调用新的函数并设置为停止
        // log_i("Motor %d is disabled, setting to stop.\n", num); // 可以选择打印信息
        return; // 不处理禁用电机的 PWM 设置请求
    }

    // 应用限幅
    int limited_pwm = amplitude_limit(pwm, g_motor_system_config->max_pwm_value);

    // 调用新的函数设置单个电机的 PWM
    set_motor_pwm_tb6612(config, limited_pwm);
}
