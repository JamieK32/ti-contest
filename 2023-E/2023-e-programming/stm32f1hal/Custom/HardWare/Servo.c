#include "Servo.h"
#include "tim.h"
#include "main.h"
#include "stm32f1xx_hal_tim.h"

static float Angle_X = INIT_ANGLE_X;   // 当前X轴角度
static float Angle_Y = INIT_ANGLE_Y;   // 当前Y轴角度

void Servo_Init(void) {
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
}

/**
 * @brief 设置X轴舵机的角度
 * @param AngleX: 要设置的X轴角度
  STM32F1的TIM2时钟是72MHz
	实际计数频率 = 72MHz ÷ (35+1) = 72MHz ÷ 36 = 2MHz
	PWM周期 = (39999+1) ÷ 2MHz = 40000 ÷ 2,000,000 = 0.02秒 = 20ms ✓
	每个计数值的时间 = 1 ÷ 2MHz = 0.5μs
	标准舵机脉宽：

	0° → 1ms → 1000μs ÷ 0.5μs = 2000个计数值
	90° → 1.5ms → 1500μs ÷ 0.5μs = 3000个计数值
	180° → 2ms → 2000μs ÷ 0.5μs = 4000个计数值
 */
void Servo_SetAngleX(float AngleX)
{
    // 限制角度范围
    AngleX = (AngleX > MAX_ANGLE_X) ? MAX_ANGLE_X : ((AngleX < MIN_ANGLE_X) ? MIN_ANGLE_X : AngleX);

    // 将角度值转换为PWM占空比
    uint32_t pulse = (uint32_t)((AngleX / 180.0f) * 2000.0f + 2000.0f);

    // 设置对应PWM信号
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);

    // 更新当前角度
    Angle_X = AngleX;
}

/**
 * @brief 设置Y轴舵机的角度
 * @param AngleY: 要设置的Y轴角度
 */
void Servo_SetAngleY(float AngleY)
{
    // 限制角度范围
    AngleY = (AngleY > MAX_ANGLE_Y) ? MAX_ANGLE_Y : ((AngleY < MIN_ANGLE_Y) ? MIN_ANGLE_Y : AngleY);

    // 将角度值转换为PWM占空比
    uint32_t pulse = (uint32_t)((AngleY / 180.0f) * 2000.0f + 2000.0f);

    // 设置对应PWM信号
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);

    // 更新当前角度
    Angle_Y = AngleY;
}

/**
 * @brief 增量移动舵机
 * @param x_increment: X轴的增量
 * @param y_increment: Y轴的增量
 */
void Servo_MoveIncrement(float x_increment, float y_increment)
{
    // 计算新的角度
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
		Angle_X = INIT_ANGLE_X;
		Angle_Y = INIT_ANGLE_Y;
    // 稍作延迟确保舵机到达目标位置
    HAL_Delay(500);
}
