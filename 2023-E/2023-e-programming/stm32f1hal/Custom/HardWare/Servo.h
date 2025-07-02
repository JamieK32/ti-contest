#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f1xx_hal.h"

// 舵机角度范围定义（以度为单位）
#define MIN_ANGLE_X    -180.0f
#define MAX_ANGLE_X     180.0f
#define MIN_ANGLE_Y    	-30.0f
#define MAX_ANGLE_Y     180.0f

// 初始位置定义
#define INIT_ANGLE_X     95.0f
#define INIT_ANGLE_Y  	 50.0f

#ifdef __cplusplus
extern "C" {
#endif

void Servo_Init(void);
void Servo_SetAngleX(float AngleX);
void Servo_SetAngleY(float AngleY);
void Servo_MoveIncrement(float x_increment, float y_increment);
void Servo_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __SERVO_H */
