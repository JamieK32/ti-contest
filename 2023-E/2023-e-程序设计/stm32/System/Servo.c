#include "stm32f10x.h"                  // Device header
#include "PWM.h" 
#include "servo.h"
#include "delay.h"

void Servo_Init()
{
	PWM_Init();
}

void Servo_SetAngle1(float Angle1)
{
	Angle1 = (Angle1 > MAX_ANGLE_X) ? MAX_ANGLE_X : ((Angle1 < MIN_ANGLE_X) ? MIN_ANGLE_X : Angle1);  
	PWM_SetCompare2(Angle1 / 180 * 2000 + 500);
	
}
void Servo_SetAngle2(float Angle2)
{
	Angle2 = (Angle2 > MAX_ANGLE_Y) ? MAX_ANGLE_Y : ((Angle2 < MIN_ANGLE_Y) ? MIN_ANGLE_Y : Angle2);  
	PWM_SetCompare3(Angle2 / 180 * 2000 + 500);
}

uint16_t PWM_GetCompare2()  
{  
    return (uint16_t)(TIM2->CCR2);  
}
uint16_t PWM_GetCompare3()  
{  
    return (uint16_t)(TIM2->CCR3);  
}

float ReadServoAngle1(void)  
{  
    uint16_t pwmValue = PWM_GetCompare2();; // 假设这是获取TIM2的CH1当前PWM值的函数  
    float angle = (pwmValue - 500) / 2000.0f * 180; // 反向计算角度  
    return angle;  
}  

float ReadServoAngle2(void)  
{  
    uint16_t pwmValue = PWM_GetCompare3(); // 假设这是获取TIM2的CH2当前PWM值的函数  
    float angle = (pwmValue - 500) / 2000.0f * 180; // 反向计算角度  
    return angle;  
}
