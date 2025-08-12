/**
 * @file camera.c
 * @brief 摄像头通信模块
 */

#include "maix_cam.h"
#include "lwpkt.h"
#include "lwrb.h"
#include "hal_uart.h"
#include <string.h>

// ====================  配置定义  ====================

#define CAMERA_UART             UART_1_INST    // 假设使用UART1与摄像头通信
#define CAMERA_UART_IRQ         UART_1_INST_INT_IRQN

#define CAMERA_RX_BUFFER_SIZE   256            // 摄像头可能发送较多数据
#define CAMERA_TX_BUFFER_SIZE   128

// ====================  全局变量  ====================

static lwpkt_t camera_pkt;
static uint8_t camera_rx_buffer[CAMERA_RX_BUFFER_SIZE];
static uint8_t camera_tx_buffer[CAMERA_TX_BUFFER_SIZE];
static lwrb_t camera_rx_rb, camera_tx_rb;
static volatile bool camera_data_ready = false;

// ====================  内部函数声明  ====================

static camera_result_t camera_flush_tx(void);
static void camera_packet_callback(lwpkt_t* pkt, lwpkt_evt_type_t evt);

// ====================  初始化函数  ====================

camera_result_t camera_init(void) {
    // 初始化环形缓冲区
    lwrb_init(&camera_rx_rb, camera_rx_buffer, sizeof(camera_rx_buffer));
    lwrb_init(&camera_tx_rb, camera_tx_buffer, sizeof(camera_tx_buffer));
    
    // 初始化lwpkt
    lwpktr_t lwpkt_result = lwpkt_init(&camera_pkt, &camera_tx_rb, &camera_rx_rb);
    if (lwpkt_result != lwpktOK) {
        return CAMERA_ERROR;
    }
    
    // 设置回调函数
    lwpkt_set_evt_fn(&camera_pkt, camera_packet_callback);
    
    // 启用UART中断
    NVIC_EnableIRQ(CAMERA_UART_IRQ);
    
    camera_data_ready = false;
    
    return CAMERA_OK;
}

// ====================  中断处理  ====================

void camera_irq_handler(DL_UART_IIDX idx) {
    if (idx == DL_UART_IIDX_RX) {
        uint8_t received_byte = DL_UART_Main_receiveData(CAMERA_UART);
        
        if (lwrb_write(&camera_rx_rb, &received_byte, 1) == 1) {
            camera_data_ready = true;
        }
    }
}

// ====================  发送函数  ====================

camera_result_t camera_send_byte(uint8_t byte) {
    lwpktr_t result = lwpkt_write(&camera_pkt, &byte, 1);
    
    if (result != lwpktOK) {
        return CAMERA_ERROR;
    }
    
    return camera_flush_tx();
}

camera_result_t camera_send_data(const uint8_t* data, size_t length) {
    if (data == NULL || length == 0) {
        return CAMERA_INVALID_PARAM;
    }
    
    lwpktr_t result = lwpkt_write(&camera_pkt, data, length);
    
    if (result != lwpktOK) {
        return CAMERA_ERROR;
    }
    
    return camera_flush_tx();
}

camera_result_t camera_send_string(const char* str) {
    if (str == NULL) {
        return CAMERA_INVALID_PARAM;
    }
    
    size_t length = strlen(str);
    return camera_send_data((const uint8_t*)str, length);
}

static camera_result_t camera_flush_tx(void) {
    size_t available = lwrb_get_full(&camera_tx_rb);
    
    if (available == 0) {
        return CAMERA_OK;
    }
    
    uint8_t temp_buffer[available];
    size_t read_bytes = lwrb_read(&camera_tx_rb, temp_buffer, available);
    
    if (read_bytes > 0) {
        usart_send_bytes(CAMERA_UART, temp_buffer, read_bytes);
        return CAMERA_OK;
    }
    
    return CAMERA_ERROR;
}

// ====================  接收处理  ====================

void camera_process(void) {
    if (camera_data_ready) {
        lwpkt_process(&camera_pkt, 256);  // 摄像头数据较多，处理更多字节
        camera_data_ready = false;
    }
}

static void camera_packet_callback(lwpkt_t* pkt, lwpkt_evt_type_t evt) {
    if (evt == LWPKT_EVT_PKT) {
        const uint8_t* data = lwpkt_get_data(pkt);
        size_t len = lwpkt_get_data_len(pkt);
        
        // 调用用户实现的接收处理函数
        camera_data_received(data, len);
    }
}

// ====================  弱符号默认实现  ====================

__attribute__((weak)) void camera_data_received(const uint8_t* data, size_t length) {
    // 默认空实现，用户可以重写这个函数
    (void)data;
    (void)length;
}