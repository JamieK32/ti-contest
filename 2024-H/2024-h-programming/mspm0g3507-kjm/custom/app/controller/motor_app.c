#include "motor_app.h"

extern motorHardWareInterface l298n_interface; // 具体的 L298N 实现接口
extern motorHardWareInterface tb6612_interface;

/*
MotorSystemConfig l298n_cfg = {
    .motor_count = MOTOR_TYPE_TWO_WHEEL, // 默认两轮
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
*/

MotorSystemConfig tb6612_cfg = {
	.motor_count = MOTOR_TYPE_FOUR_WHEEL,
	.max_pwm_value = 3000,
	
	.motors = {
		[MOTOR_FRONT_LEFT] = {
				.enabled = true, // 默认启用
				.timer_instance = (GPTIMER_Regs*) Motor_PWM1_INST, // 定时器实例指针
				.pwm_cc_index = DL_TIMER_CC_0_INDEX,
				.polarity = false
		},
		
		[MOTOR_FRONT_RIGHT] = {
				.enabled = true, // 默认启用
				.timer_instance = (GPTIMER_Regs*) Motor_PWM1_INST,
				.pwm_cc_index = DL_TIMER_CC_1_INDEX,
				.polarity = true
		},
	
		[MOTOR_BACK_LEFT] = {
				.enabled = true, // 默认启用
				.timer_instance = (GPTIMER_Regs*) Motor_PWM2_INST,
				.pwm_cc_index = DL_TIMER_CC_0_INDEX,
				.polarity = true
		},
		[MOTOR_BACK_RIGHT] = {
				.enabled = true, // 默认启用
				.timer_instance = (GPTIMER_Regs*) Motor_PWM2_INST,
				.pwm_cc_index = DL_TIMER_CC_1_INDEX,
				.polarity = false
		},
	}
};


void motor_init(void) {
	tb6612_interface.enable_all_motor(&tb6612_cfg);
}

void motor_set_pwms(int *pwms) {
	tb6612_interface.set_pwms(&tb6612_cfg, pwms);
}

void motor_stop(void) {
	tb6612_interface.disable_all_motor(&tb6612_cfg);
}