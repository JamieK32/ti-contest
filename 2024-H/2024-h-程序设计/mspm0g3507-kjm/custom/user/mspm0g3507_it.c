#include "freertos.h"
#include "queue.h"
#include "task.h"
#include "mspm0g3507_it.h"
#include "ti_msp_dl_config.h"
#include "uart_debug.h"


extern TaskHandle_t uart_tx_task_handle;
extern QueueHandle_t uart_rx_queue;
/**
 * @brief UART 中断处理函数
 */
void UART_DEBUG_INST_IRQHandler(void) {
    uint8_t uart_data;
    DL_UART_IIDX idx = DL_UART_getPendingInterrupt(UART_DEBUG_INST);

    switch (idx) {
        case DL_UART_IIDX_RX: // 如果是接收中断
            uart_data = DL_UART_Main_receiveData(UART_DEBUG_INST);
            // 将接收到的数据放入队列
            xQueueSendFromISR(uart_rx_queue, &uart_data, NULL);
            break;

        case DL_UART_MAIN_IIDX_EOT_DONE:
            // 通知发送任务传输完成
            vTaskNotifyGiveFromISR(uart_tx_task_handle, NULL);
            break;

        case DL_UART_MAIN_IIDX_DMA_DONE_TX:

            break;

        default:
            break;
    }

    DL_UART_clearInterruptStatus(UART_DEBUG_INST, idx);
		portYIELD_FROM_ISR(uart_tx_task_handle);
}
