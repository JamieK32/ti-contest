#ifndef __MOTOR_HARDWARE_H__
#define __MOTOR_HARDWARE_H__

#include "ti_msp_dl_config.h" // 包含你的单片机 DL 库头文件
#include <stdint.h>
#include <stdbool.h>
#include <math.h> // 为了使用 fmin 和 fmax，如果想避免浮点运算可以使用整型实现
#include "stdlib.h"

// 定义电机编号
typedef enum {
    MOTOR_FRONT_LEFT = 0,
    MOTOR_FRONT_RIGHT = 1,
    MOTOR_BACK_LEFT = 2,
    MOTOR_BACK_RIGHT = 3,
    NUM_MOTORS // 电机总数 (用于数组大小)
} MotorID;

// 定义电机类型 (两轮或四轮)
typedef enum {
    MOTOR_TYPE_TWO_WHEEL = 2,
    MOTOR_TYPE_FOUR_WHEEL = 4
} MotorType;

// 电机配置结构体
// 包含每个电机所需的硬件信息
typedef struct {
    bool enabled;           // 该电机是否启用 (用于两轮/四轮适配)
    GPTIMER_Regs* timer_instance;   // 定时器实例地址 (明确为 GPTIMER_Regs*)
		
		// L298N INTERFACE
    uint32_t cc_reverse_pwm_index; // 反向 PWM 通道索引 (例如 DL_TIMER_CC_0_INDEX)
    uint32_t cc_forward_pwm_index; // 正向 PWM 通道索引 (例如 DL_TIMER_CC_1_INDEX)
		// L298N INTERFACE
	
		// TB6612 INTERFACE
		// PWM 输出通道索引 (如果方向由 GPIO 控制，只需要一个 PWM 通道)
		uint32_t pwm_cc_index;
	
		GPIO_Regs* dir_gpio;
    uint32_t dir_pin1;
    uint32_t dir_pin2;
	
		// Standby 引脚
		GPIO_Regs* stby_gpio;  // Standby GPIO 端口
    uint32_t stby_pin; 		 // Standby 引脚
		// TB6612 INTERFACE
	
} MotorConfig;

// 系统级电机参数配置结构体
typedef struct {
    MotorType motor_type;       // 电机系统类型 (两轮或四轮)
    int max_pwm_value;          // 最大 PWM 值 (PWM 值是 uint32_t，但这里用 int 方便限幅处理正负)
    MotorConfig motors[NUM_MOTORS]; // 每个电机的配置
} MotorSystemConfig;

// 兼容原有接口 (内部调用新的函数)
typedef struct motorHardWareInterface {
    void (*enable_all_motor)(const MotorSystemConfig* config);
    void (*disable_all_motor)(const MotorSystemConfig* config);
    void (*set_pwms)(const MotorSystemConfig* sys_config, uint8_t, int); // 原有的按电机编号设置 PWM
} motorHardWareInterface;

extern motorHardWareInterface l298n_interface; // 具体的 L298N 实现接口
extern motorHardWareInterface tb6612_interface;

// ------------- 函数实现 (通常放在 .c 文件中，但为了示例方便放在这里声明) -------------

// 函数实现 AMPLITUDE_LIMIT
// 改为 static inline 函数，避免外部链接问题，并在编译时展开
static inline int amplitude_limit(int speed, int limit) {
    if (speed > limit) {
        return limit;
    } else if (speed < -limit) {
        return -limit;
    } else {
        return speed;
    }
    // 如果要使用浮点运算版本并链接数学库，需要包含 <math.h>
    // return (int)fmin(fmax((float)speed, (float)-limit), (float)limit);
}


#endif // __MOTOR_HARDWARE_H__
