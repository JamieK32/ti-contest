#include "mspm0g3507_it.h"
#include "ti_msp_dl_config.h"
#include "encoder_app.h"

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
        default:
            break;
    }
    
    // 清除中断标志
    DL_UART_clearInterruptStatus(UART_0_INST, idx);
}

void GROUP1_IRQHandler(void) {
   if (DL_Interrupt_getStatusGroup(DL_INTERRUPT_GROUP_1, PORTB_INT_IIDX)) {
			encoder_group1_irq_handler();
    }

}