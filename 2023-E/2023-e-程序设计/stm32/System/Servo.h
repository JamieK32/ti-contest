#ifndef _SERVO_H
#define _SERVO_H

#define MIN_ANGLE_X 0 
#define MAX_ANGLE_X 180
#define MIN_ANGLE_Y 0  
#define MAX_ANGLE_Y 180 

void Servo_Init();
void Servo_SetAngle1(float Angle1);
void Servo_SetAngle2(float Angle2);
uint16_t PWM_GetCompare2(); 
uint16_t PWM_GetCompare3();  

#endif
