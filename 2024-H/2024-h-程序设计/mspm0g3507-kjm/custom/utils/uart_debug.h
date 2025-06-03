#ifndef __UART_H__
#define __UART_H__

#include "ti_msp_dl_config.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "common_defines.h"
#include "freertos.h"
#include "task.h"
#include "queue.h"

typedef struct {
    uint8_t buf[MAX_TX_BUFFER_SIZE];
    size_t  len;
} uart_tx_packet_t;


typedef struct UsartUtil 
{
	uint8_t data;
	uint8_t rx_buffer[MAX_RX_BUFFER_SIZE];
	uint16_t rx_write_index;
	uint16_t rx_read_index ;
	uint8_t rx_data_ready;
} UsartUtil;

void debug_uart_init(void);
void debug_uart_send_byte(uint8_t byte);
void debug_uart_send_bytes(const uint8_t *data, size_t length);
void debug_uart_printf(const char *format, ...);

#endif
