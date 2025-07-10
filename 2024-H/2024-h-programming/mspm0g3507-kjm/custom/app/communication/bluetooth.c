/**
 * @file bluetooth.c
 * @brief 蓝牙通信模块
 */

#include "bluetooth.h"
#include "lwpkt.h"
#include "lwrb.h" 

#include "uart_driver.h"

// ====================  配置定义  ====================

#define BLUETOOTH_UART          UART_0_INST
#define BLUETOOTH_UART_IRQ      UART_0_INST_INT_IRQN

#define BT_RX_BUFFER_SIZE       128
#define BT_TX_BUFFER_SIZE       128

// ====================  全局变量  ====================

static lwpkt_t bluetooth_pkt;
static uint8_t bt_rx_buffer[BT_RX_BUFFER_SIZE];
static uint8_t bt_tx_buffer[BT_TX_BUFFER_SIZE];
static lwrb_t bt_rx_rb, bt_tx_rb;
static volatile bool bluetooth_data_ready = false;

// ====================  内部函数声明  ====================

static bluetooth_result_t bluetooth_flush_tx(void);
static void bluetooth_packet_callback(lwpkt_t* pkt, lwpkt_evt_type_t evt);

// ====================  初始化函数  ====================

bluetooth_result_t bluetooth_init(void) {
    // 初始化环形缓冲区
    lwrb_init(&bt_rx_rb, bt_rx_buffer, sizeof(bt_rx_buffer));
    lwrb_init(&bt_tx_rb, bt_tx_buffer, sizeof(bt_tx_buffer));
    
    // 初始化lwpkt
    lwpktr_t lwpkt_result = lwpkt_init(&bluetooth_pkt, &bt_tx_rb, &bt_rx_rb);
    if (lwpkt_result != lwpktOK) {
        return BLUETOOTH_ERROR;
    }
    
    // 设置回调函数
    lwpkt_set_evt_fn(&bluetooth_pkt, bluetooth_packet_callback);
    
    // 启用UART中断
    NVIC_EnableIRQ(BLUETOOTH_UART_IRQ);
    
    bluetooth_data_ready = false;
    
    return BLUETOOTH_OK;
}

// ====================  中断处理  ====================

void bluetooth_irq_handler(DL_UART_IIDX idx) {
    if (idx == DL_UART_IIDX_RX) {
        uint8_t received_byte = DL_UART_Main_receiveData(BLUETOOTH_UART);
        
        if (lwrb_write(&bt_rx_rb, &received_byte, 1) == 1) {
            bluetooth_data_ready = true;
        }
    }
}

// ====================  发送函数  ====================

bluetooth_result_t bluetooth_send_byte(uint8_t byte) {
    lwpktr_t result = lwpkt_write(&bluetooth_pkt, &byte, 1);
    
    if (result != lwpktOK) {
        return BLUETOOTH_ERROR;
    }
    
    return bluetooth_flush_tx();
}

bluetooth_result_t bluetooth_send_data(const uint8_t* data, size_t length) {
    if (data == NULL || length == 0) {
        return BLUETOOTH_INVALID_PARAM;
    }
    
    lwpktr_t result = lwpkt_write(&bluetooth_pkt, data, length);
    
    if (result != lwpktOK) {
        return BLUETOOTH_ERROR;
    }
    
    return bluetooth_flush_tx();
}

static bluetooth_result_t bluetooth_flush_tx(void) {
    size_t available = lwrb_get_full(&bt_tx_rb);
    
    if (available == 0) {
        return BLUETOOTH_OK;
    }
    
    uint8_t temp_buffer[available];
    size_t read_bytes = lwrb_read(&bt_tx_rb, temp_buffer, available);
    
    if (read_bytes > 0) {
        usart_send_bytes(BLUETOOTH_UART, temp_buffer, read_bytes);
        return BLUETOOTH_OK;
    }
    
    return BLUETOOTH_ERROR;
}

// ====================  接收处理  ====================

void bluetooth_process(void) {
    if (bluetooth_data_ready) {
        lwpkt_process(&bluetooth_pkt, 128);
        bluetooth_data_ready = false;
    }
}

static void bluetooth_packet_callback(lwpkt_t* pkt, lwpkt_evt_type_t evt) {
    if (evt == LWPKT_EVT_PKT) {
        const uint8_t* data = lwpkt_get_data(pkt);
        size_t len = lwpkt_get_data_len(pkt);
        
        // 调用用户实现的接收处理函数
        bluetooth_data_received(data, len);
    }
}

// ====================  弱符号默认实现  ====================
__attribute__((weak)) void bluetooth_data_received(const uint8_t* data, size_t length) {
    (void)data;
    (void)length;
}