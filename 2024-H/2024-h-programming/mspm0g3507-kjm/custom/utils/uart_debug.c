#include "uart_debug.h"
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

// DMA发送完成标志
static volatile bool dma_tx_complete = false;

/**
 * @brief 初始化 UART 调试功能
 */
void debug_uart_init(void) {
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    dma_tx_complete = true; // 初始状态为完成
}

/**
 * @brief 使用 DMA 发送多个字节（阻塞）
 * @param data 数据指针
 * @param length 数据长度
 */
void debug_uart_send_bytes(const uint8_t *data, size_t length) {
    if (length == 0) return;
    
    // 等待上一次DMA传输完成
    while (!dma_tx_complete);
    
    // 标记DMA传输开始
    dma_tx_complete = false;
    
    // 设置源地址
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(data));
    // 设置目标地址
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(&UART_0_INST->TXDATA));
    // 设置要搬运的字节数
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, length);
    // 使能 DMA 通道
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
    
    // 阻塞等待DMA传输完成
    while (!dma_tx_complete);
}

/**
 * @brief 格式化并发送字符串（阻塞）
 * @param format 格式化字符串
 * @param ... 格式化参数
 */
void debug_uart_printf(const char *format, ...) {
    static char printf_buffer[256];
    va_list args;
    
    va_start(args, format);
    int len = vsnprintf(printf_buffer, sizeof(printf_buffer), format, args);
    va_end(args);
    
    if (len > 0) {
        debug_uart_send_bytes((uint8_t *)printf_buffer, len);
    }
}

/**
 * @brief 发送字符串（阻塞）
 * @param str 字符串指针
 */
void debug_uart_send_string(const char *str) {
    if (str != NULL) {
        debug_uart_send_bytes((uint8_t *)str, strlen(str));
    }
}

/**
 * @brief DMA发送完成回调（由中断调用）
 */
void debug_uart_dma_tx_complete_callback(void) {
    dma_tx_complete = true;
}