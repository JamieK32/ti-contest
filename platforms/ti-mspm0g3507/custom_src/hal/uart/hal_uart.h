#ifndef __UART_H__
#define __UART_H__


#include "ti_msp_dl_config.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"


#define MAX_TX_BUFFER_SIZE 256

void usart_printf(UART_Regs* uart, const char* format, ...);
void usart_send_bytes(UART_Regs* uart, const uint8_t* data, size_t length);

#endif
