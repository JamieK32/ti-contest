#include "mspm0g3507_it.h"
#include "ti_msp_dl_config.h"

#include "uart_debug.h"

// 外部函数声明
extern void debug_uart_dma_tx_complete_callback(void);

/**
 * @brief UART 中断处理函数
 */
void UART_0_INST_IRQHandler(void) {
    uint8_t uart_data;
    DL_UART_IIDX idx = DL_UART_getPendingInterrupt(UART_0_INST);
    
    switch (idx) {
        case DL_UART_IIDX_RX: // 接收中断
            uart_data = DL_UART_Main_receiveData(UART_0_INST);
            break;
            
        case DL_UART_MAIN_IIDX_EOT_DONE: // 发送完成中断
            // 通知DMA发送完成
            debug_uart_dma_tx_complete_callback();
            break;
            
        case DL_UART_MAIN_IIDX_DMA_DONE_TX: // DMA发送完成中断
            // 通知DMA发送完成
            debug_uart_dma_tx_complete_callback();
            break;
            
        default:
            break;
    }
    
    // 清除中断标志
    DL_UART_clearInterruptStatus(UART_0_INST, idx);
}