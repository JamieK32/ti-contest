#ifndef _CHUANKO_H
#define _CHUANKO_H

void USART1_Init(void);
void USART1_IRQHandler(void); 
void processReceivedData(void); 
void sendString(const char *str); 

#endif
