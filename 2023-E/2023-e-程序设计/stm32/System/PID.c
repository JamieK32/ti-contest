#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "OLED.h"

int16_t Err_X = 0,Err_Y = 0;

int16_t x_pwm = 0,
		now_x;

int16_t y_pwm = 0,
		now_y,
		ks = 1500;
		
		
float Err_S_Y = 0,
	  last_Err_S_Y = 0,
	  integral = 0,
	  p_S = -1.35,
	  i_S = -0.34,
	  d_S = -0.25;

float Err_S_X = 0,
	  last_Err_S_X = 0,
      integral_X = 0,
	  p_S_X = -1.35,
	  i_S_X = -0.34,
	  d_S_X = -0.25;

void pid_S_Y(float true_S, float tar_S)
{
	
    Err_S_Y = tar_S - true_S;
	integral += Err_S_Y;
	y_pwm=p_S * Err_S_Y+d_S*(Err_S_Y-last_Err_S_Y)+i_S * integral;
    last_Err_S_Y = Err_S_Y;
	now_y = ks + y_pwm;
	if(now_y > 2500)
	{
		now_y = 2500;
	}
	else if(now_y < 500)
	{
		now_y = 500;
	}
    PWM_SetCompare2(now_y);
	OLED_ShowSignedNum(3,1,now_y,5);
			
}

void pid_S_X(float true_S, float tar_S)
{
	Err_S_X = tar_S - true_S;
	integral_X += Err_S_X;
	x_pwm=p_S_X * Err_S_X+d_S_X*(Err_S_X-last_Err_S_X)+i_S_X * integral_X;
    last_Err_S_X = Err_S_X;
	now_x = ks + x_pwm;
	if(now_x > 2500)
	{
		now_x = 2500;
	}
	else if(now_x < 500)
	{
		now_x = 500;
	}
    PWM_SetCompare3(now_x);
	OLED_ShowSignedNum(4,1,now_x,5);
}