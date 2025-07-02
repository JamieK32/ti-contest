#ifndef UART_DMA_H__
#define UART_DMA_H__

#include "stdint.h"
#include "openmv.h"

void UART_DMA_Init(void);
void UART_Process_DMA_Data(void);
void Process_Received_Byte(uint8_t byte, uint16_t *index);

extern uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
extern volatile uint8_t uart_data_ready;


#endif 
