#include "hal_uart.h"

__attribute__((aligned(8))) static char buffer[MAX_TX_BUFFER_SIZE];


// 发送字节数组
void usart_send_bytes(UART_Regs* uart, const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        DL_UART_Main_transmitDataBlocking(uart, data[i]);
    }
}

// 格式化并发送字符串
void usart_printf(UART_Regs* uart, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    usart_send_bytes(uart, (uint8_t*)buffer, strlen(buffer));
}
