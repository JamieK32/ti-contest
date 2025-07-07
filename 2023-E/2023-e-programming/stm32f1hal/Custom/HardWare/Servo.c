#include "Servo.h"
#include "tim.h"
#include "main.h"
#include "stm32f1xx_hal_tim.h"
#include "math.h"

static float Angle_X = INIT_ANGLE_X;   // 当前X轴角度
static float Angle_Y = INIT_ANGLE_Y;   // 当前Y轴角度

void Servo_Init(void) {
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
}

/**
 * @brief 设置X轴舵机的角度（优化精度版本）
 * @param AngleX: 要设置的X轴角度
 */
void Servo_SetAngleX(float AngleX)
{
    // 限制角度范围
    AngleX = (AngleX > MAX_ANGLE_X) ? MAX_ANGLE_X : ((AngleX < MIN_ANGLE_X) ? MIN_ANGLE_X : AngleX);
    
    // 优化的角度转换公式（更精确的映射）
    // 将-180到180度映射到2000-4000个计数值（1ms-2ms）
    float normalized_angle = (AngleX + 180.0f) / 360.0f;  // 归一化到0-1
    uint32_t pulse = (uint32_t)(2000.0f + normalized_angle * 2000.0f + 0.5f); // 加0.5四舍五入
    
    // 限制脉宽范围
    if (pulse < 2000) pulse = 2000;
    if (pulse > 4000) pulse = 4000;
    
    // 设置对应PWM信号
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
    
    // ✅ 修复：更新当前角度
    Angle_X = AngleX;
}

/**
 * @brief 设置Y轴舵机的角度（优化精度版本）
 * @param AngleY: 要设置的Y轴角度
 */
void Servo_SetAngleY(float AngleY)
{
    // 限制角度范围
    AngleY = (AngleY > MAX_ANGLE_Y) ? MAX_ANGLE_Y : ((AngleY < MIN_ANGLE_Y) ? MIN_ANGLE_Y : AngleY);
    
    // 针对Y轴范围(-30到180度)的优化映射
    // 将-30到180度映射到合适的脉宽范围
    float angle_range = MAX_ANGLE_Y - MIN_ANGLE_Y;  // 210度
    float normalized_angle = (AngleY - MIN_ANGLE_Y) / angle_range;  // 归一化到0-1
    uint32_t pulse = (uint32_t)(2000.0f + normalized_angle * 2000.0f + 0.5f); // 四舍五入
    
    // 限制脉宽范围
    if (pulse < 2000) pulse = 2000;
    if (pulse > 4000) pulse = 4000;
    
    // 设置对应PWM信号
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
    
    // ✅ 修复：更新当前角度
    Angle_Y = AngleY;
}

/**
 * @brief 增量移动舵机
 * @param x_increment: X轴的增量
 * @param y_increment: Y轴的增量
 */
void Servo_MoveIncrement(float x_increment, float y_increment)
{
    // 限制单次增量大小，防止过大的跳跃
    float max_increment = 3.0f;  // 单次最大增量
    
    if (fabsf(x_increment) > max_increment) {
        x_increment = (x_increment > 0) ? max_increment : -max_increment;
    }
    if (fabsf(y_increment) > max_increment) {
        y_increment = (y_increment > 0) ? max_increment : -max_increment;
    }

   float new_X = Angle_X + x_increment;
   float new_Y = Angle_Y + y_increment;
    // 调用设置角度函数（包含角度范围限制）
    Servo_SetAngleX(new_X);
    Servo_SetAngleY(new_Y);
}

/**
 * @brief 将舵机复位到默认初始位置
 */
void Servo_Reset(void)
{
    // 复位X轴和Y轴角度到初始位置
    Servo_SetAngleX(INIT_ANGLE_X);
    Servo_SetAngleY(INIT_ANGLE_Y);
    // 稍作延迟确保舵机到达目标位置
    HAL_Delay(500);
}
