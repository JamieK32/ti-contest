#include "stm32f10x.h"                  // Device header
#include "Servo.h"
#include "OLED.h"
#include "KEY.h"
#include "Delay.h"
#include "chuanko.h"
#include "PID.h"
#include "PWM.h"

// 接收到的数据缓冲区
extern uint8_t rx_buffer[10];
// 接收数据长度
extern volatile uint16_t rx_length;
// 接收标志
extern volatile uint8_t rx_flag;
float Angle1;
float Angle2;

int x = 0,y = 0,x_err = 0,y_err = 0;

// 主函数
int main(void) 
{
	Servo_Init();
	OLED_Init();
	OLED_Clear();
  USART1_Init();
	Servo_SetAngle1(90);
	Servo_SetAngle2(90);
	OLED_ShowString(1,1,"Angle1:");
	OLED_ShowString(2,1,"Angle2:");
	Delay_ms(500);
    while (1) 
		{
        // 检查是否有数据接收完成
			if (rx_flag) 
			{
				processReceivedData();
				x=(uint8_t)rx_buffer[2];
				y=(uint8_t)rx_buffer[3];
			}
			OLED_ShowSignedNum(1,1,x,5);
			OLED_ShowSignedNum(2,1,y,5);
			if(x != 0)
			{
				pid_S_X(x,80);
			}
			if(y != 0)
			{
				pid_S_Y(y,60);
			}
		
	}
}


