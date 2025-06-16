#include "motor_app.h"

extern motorHardWareInterface l298n_interface; // 具体的 L298N 实现接口
extern motorHardWareInterface tb6612_interface;

// 定义外部可访问的配置结构体实例
// 在应用层初始化时填充这些配置
MotorSystemConfig g_motor_system_config = {
    .motor_type = MOTOR_TYPE_TWO_WHEEL, // 默认两轮
    .max_pwm_value = 3000, // 默认最大 PWM 值

    .motors = {
        // 配置前左电机 (根据你的实际硬件连接和 DL 库配置)
        [MOTOR_FRONT_RIGHT] = {
            .enabled = true, // 默认启用
            .timer_instance = (GPTIMER_Regs*) Motor_PWM1_INST, // 定时器实例指针
            .cc_reverse_pwm_index = DL_TIMER_CC_0_INDEX, // 反向 PWM 通道索引
            .cc_forward_pwm_index = DL_TIMER_CC_1_INDEX, // 正向 PWM 通道索引
        },
        // 配置前右电机 (根据你的实际硬件连接和 DL 库配置)
        [MOTOR_FRONT_LEFT] = {
            .enabled = true, // 默认启用
            .timer_instance = (GPTIMER_Regs*) Motor_PWM2_INST,
            .cc_reverse_pwm_index = DL_TIMER_CC_1_INDEX,
            .cc_forward_pwm_index = DL_TIMER_CC_0_INDEX,
        },
    }
};


void motor_init(void) {
	l298n_interface.enable_all_motor(&g_motor_system_config);
}

void motor_set_pwm(MotorID id, int pwm) {
	l298n_interface.set_pwms(&g_motor_system_config, id, pwm);
}

void motor_stop(void) {
	l298n_interface.enable_all_motor(&g_motor_system_config);
}