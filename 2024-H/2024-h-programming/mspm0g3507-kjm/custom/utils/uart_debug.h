#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ti_msp_dl_config.h"

// 函数声明
void debug_uart_init(void);
void debug_uart_send_bytes(const uint8_t *data, size_t length);
void debug_uart_send_string(const char *str);
void debug_uart_printf(const char *format, ...);

// 内部回调函数（由中断调用）
void debug_uart_dma_tx_complete_callback(void);

#endif // UART_DEBUG_H