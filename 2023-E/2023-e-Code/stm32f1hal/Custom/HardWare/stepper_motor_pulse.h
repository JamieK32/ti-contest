#ifndef STEPPER_MOTOR_PULSE_H
#define STEPPER_MOTOR_PULSE_H
#include "main.h"
#include "stm32f1xx_hal.h"

typedef struct {
    GPIO_TypeDef* step_port;        // 脉冲引脚端口
    uint16_t step_pin;              // 脉冲引脚
    GPIO_TypeDef* dir_port;         // 方向引脚端口
    uint16_t dir_pin;               // 方向引脚
    GPIO_TypeDef* en_port;          // 使能引脚端口（可选）
    uint16_t en_pin;                // 使能引脚（可选）

    uint32_t steps_per_rev;         // 每圈步数
    uint32_t current_position;      // 当前位置
    uint8_t direction;              // 当前方向 (0: CW, 1: CCW)
    uint8_t enabled;                // 使能状态
		uint16_t step_delay;
} StepperMotor_t;

#define STEPPER_DIR_CW      0       // 顺时针
#define STEPPER_DIR_CCW     1       // 逆时针

void Stepper_Init(StepperMotor_t* motor, 
                  GPIO_TypeDef* step_port, uint16_t step_pin,
                  GPIO_TypeDef* dir_port, uint16_t dir_pin,
                  GPIO_TypeDef* en_port, uint16_t en_pin,
                  uint32_t steps_per_rev, uint16_t step_delay);
void Stepper_Enable(StepperMotor_t* motor);
void Stepper_Disable(StepperMotor_t* motor);
void Stepper_SetDirection(StepperMotor_t* motor, uint8_t direction);
void Stepper_Step(StepperMotor_t* motor);
void Stepper_Steps(StepperMotor_t* motor, uint32_t steps);
void Stepper_RotateRelative(StepperMotor_t* motor, float angle);

void Delay_us(uint32_t us);
#endif
