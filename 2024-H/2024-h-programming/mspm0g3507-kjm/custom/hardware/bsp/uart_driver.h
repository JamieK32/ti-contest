#ifndef __UART_H__
#define __UART_H__


#include "ti_msp_dl_config.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "common_defines.h"

typedef struct UsartUtil 
{
	uint8_t data;
	uint8_t rx_buffer[MAX_RX_BUFFER_SIZE];
	uint16_t rx_write_index;
	uint16_t rx_read_index ;
	uint8_t rx_data_ready;
} UsartUtil;


void usart_printf(UART_Regs* uart, const char* format, ...);
void usart_send_bytes(UART_Regs* uart, const uint8_t* data, size_t length);

#endif
