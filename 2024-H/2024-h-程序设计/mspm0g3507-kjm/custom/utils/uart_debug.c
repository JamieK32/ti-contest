#include "uart_debug.h"
#include "embedfire_protocol.h"
#include <string.h>
#include <stdarg.h>

// 任务和队列句柄
TaskHandle_t uart_tx_task_handle = NULL;
TaskHandle_t uart_rx_task_handle = NULL;
QueueHandle_t uart_rx_queue = NULL;
QueueHandle_t uart_tx_queue = NULL;

void debug_uart_tx_task(void *param);
void debug_uart_rx_task(void *param); 

/**
 * @brief 初始化 UART 相关的 FreeRTOS 资源
 */
void debug_uart_init(void) {
		NVIC_EnableIRQ(UART_DEBUG_INST_INT_IRQN);
    uart_rx_queue = xQueueCreate(UART_RX_QUEUE_SIZE, sizeof(uint8_t));
    uart_tx_queue = xQueueCreate(UART_TX_QUEUE_SIZE, sizeof(uart_tx_packet_t)); // 修改队列元素类型为 uart_tx_packet_t
    xTaskCreate(debug_uart_rx_task, "UART_RX", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &uart_rx_task_handle);
    xTaskCreate(debug_uart_tx_task, "UART_TX", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &uart_tx_task_handle);
}

/**
 * @brief 使用 DMA 发送单个字节
 * @param uart UART 实例
 * @param byte 要发送的字节
 */
void debug_uart_send_byte(uint8_t byte) {
    UART_DEBUG_INST->TXDATA = byte;
}

/**
 * @brief 使用 DMA 发送多个字节
 * @param uart UART 实例
 * @param data 数据指针
 * @param length 数据长度
 */
void debug_uart_send_bytes(const uint8_t *data, size_t length) {
    // 设置源地址
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(data));
    // 设置目标地址
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(&UART_DEBUG_INST->TXDATA));
    // 设置要搬运的字节数
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, length);
    // 使能 DMA 通道
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
}

/**
 * @brief 格式化并通过队列发送字符串
 * @param uart UART 实例
 * @param format 格式化字符串
 * @param ... 格式化参数
 */
void debug_uart_printf(const char *format, ...) {
    uart_tx_packet_t tx_packet;
    va_list args;
    va_start(args, format);
    vsnprintf((char *)tx_packet.buf, sizeof(tx_packet.buf), format, args);
    va_end(args);
    tx_packet.len = strlen((char *)tx_packet.buf);

    // 发送到队列
    xQueueSend(uart_tx_queue, &tx_packet, portMAX_DELAY);
    // 通知发送任务有新数据
    xTaskNotifyGive(uart_tx_task_handle);
}

/**
 * @brief UART 发送任务
 * @param param 任务参数（未使用）
 */
void debug_uart_tx_task(void *param) {
    uart_tx_packet_t tx_packet;
    BaseType_t dma_transfer_active = pdFALSE;

    while (1) {
        // 等待任务通知（可能是有新数据或DMA传输完成）
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 如果没有正在进行的DMA传输，准备新的数据
        if (dma_transfer_active == pdFALSE) {
            // 从队列中读取数据
            if (xQueueReceive(uart_tx_queue, &tx_packet, 0) == pdPASS) {
                // 如果有数据，通过 DMA 发送
                if (tx_packet.len > 0) {
                    dma_transfer_active = pdTRUE;
                    debug_uart_send_bytes(tx_packet.buf, tx_packet.len);
                }
            }
        } else {
            // DMA 传输已完成，标记为未激活状态
            dma_transfer_active = pdFALSE;
        }
    }
}

/**
 * @brief UART 接收任务
 * @param param 任务参数（未使用）
 */
void debug_uart_rx_task(void *param) {
    uint8_t data;

    while (1) {
        // 从接收队列中读取数据
        if (xQueueReceive(uart_rx_queue, &data, portMAX_DELAY) == pdPASS) {
            // 处理接收到的数据（调用协议处理函数）
            protocol_receive_byte(data);
        }
    }
}
