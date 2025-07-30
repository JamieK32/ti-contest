#include "stepper_motor_pulse.h"
#include <math.h>
#include <stdlib.h>

/**
 * @brief 初始化步进电机
 * @param motor: 步进电机结构体指针
 * @param step_port: 脉冲引脚端口
 * @param step_pin: 脉冲引脚
 * @param dir_port: 方向引脚端口
 * @param dir_pin: 方向引脚
 * @param en_port: 使能引脚端口（如不使用传入NULL）
 * @param en_pin: 使能引脚（如不使用传入0）
 * @param steps_per_rev: 每圈步数
 */
void Stepper_Init(StepperMotor_t* motor, 
                  GPIO_TypeDef* step_port, uint16_t step_pin,
                  GPIO_TypeDef* dir_port, uint16_t dir_pin,
                  GPIO_TypeDef* en_port, uint16_t en_pin,
                  uint32_t steps_per_rev, uint16_t step_delay)
{
    motor->step_port = step_port;
    motor->step_pin = step_pin;
    motor->dir_port = dir_port;
    motor->dir_pin = dir_pin;
    motor->en_port = en_port;
    motor->en_pin = en_pin;
    motor->steps_per_rev = steps_per_rev;
    motor->current_position = 0;
    motor->direction = STEPPER_DIR_CW;
    motor->enabled = 0;
    motor->step_delay = step_delay;
    // 初始化引脚状态
    HAL_GPIO_WritePin(motor->step_port, motor->step_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);
    
    if(motor->en_port != NULL) {
        HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_SET); // 默认禁用
    }
}

/**
 * @brief 使能步进电机
 * @param motor: 步进电机结构体指针
 */
void Stepper_Enable(StepperMotor_t* motor)
{
    if(motor->en_port != NULL) {
        HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_RESET);
    }
    motor->enabled = 1;
}

/**
 * @brief 禁用步进电机
 * @param motor: 步进电机结构体指针
 */
void Stepper_Disable(StepperMotor_t* motor)
{
    if(motor->en_port != NULL) {
        HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_SET);
    }
    motor->enabled = 0;
}

/**
 * @brief 设置步进电机方向
 * @param motor: 步进电机结构体指针
 * @param direction: 方向 (STEPPER_DIR_CW 或 STEPPER_DIR_CCW)
 */
void Stepper_SetDirection(StepperMotor_t* motor, uint8_t direction)
{
    motor->direction = direction;
    if(direction == STEPPER_DIR_CW) {
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_SET);
    }
}

/**
 * @brief 发送一个脉冲（一步）
 * @param motor: 步进电机结构体指针
 */
void Stepper_Step(StepperMotor_t* motor)
{
    if(!motor->enabled) return;
    
    // 产生脉冲
    HAL_GPIO_WritePin(motor->step_port, motor->step_pin, GPIO_PIN_SET);
    Delay_us(2); // 脉冲宽度2微秒
    HAL_GPIO_WritePin(motor->step_port, motor->step_pin, GPIO_PIN_RESET);
    
    // 更新位置
    if(motor->direction == STEPPER_DIR_CW) {
        motor->current_position++;
        if(motor->current_position >= motor->steps_per_rev) {
            motor->current_position = 0;
        }
    } else {
        if(motor->current_position == 0) {
            motor->current_position = motor->steps_per_rev - 1;
        } else {
            motor->current_position--;
        }
    }
}

/**
 * @brief 发送指定数量的脉冲
 * @param motor: 步进电机结构体指针
 * @param steps: 步数
 */
void Stepper_Steps(StepperMotor_t* motor, uint32_t steps)
{
    for(uint32_t i = 0; i < steps; i++) {
        Stepper_Step(motor);
        Delay_us(motor->step_delay); // 步进间隔，可根据需要调整速度
    }
}

/**
 * @brief 相对旋转指定角度
 * @param motor: 步进电机结构体指针
 * @param angle: 相对角度（正值顺时针，负值逆时针）
 */
void Stepper_RotateRelative(StepperMotor_t* motor, float angle)
{
    uint32_t steps = (uint32_t)(fabs(angle) * motor->steps_per_rev / 360.0f);
    
    if(angle >= 0) {
        Stepper_SetDirection(motor, STEPPER_DIR_CW);
    } else {
        Stepper_SetDirection(motor, STEPPER_DIR_CCW);
    }
    
    Stepper_Steps(motor, steps);
}

/**
 * @brief 获取当前位置
 * @param motor: 步进电机结构体指针
 * @return 当前位置
 */
uint32_t Stepper_GetPosition(StepperMotor_t* motor)
{
    return motor->current_position;
}

/**
 * @brief 微秒级延时函数
 * @param us: 延时微秒数
 * @note 需要根据具体的系统时钟频率调整
 */
void Delay_us(uint32_t us)
{
    uint32_t delay = us * 18;  // 72MHz下大约18个循环 = 1us
    while(delay--) {
        __NOP();  // 空操作，防止编译器优化
    }
}