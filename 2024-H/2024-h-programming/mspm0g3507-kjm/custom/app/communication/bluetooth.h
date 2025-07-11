/**
 * @file bluetooth.h
 * @brief 蓝牙通信模块头文件
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "lwpkt.h"
#include "ti_msp_dl_config.h"

enum {
	BLUETOOTH_CMD_START_QUESTION1 = 0x01,
	BLUETOOTH_CMD_STOP,
	BLUETOOTH_CMD_START_QUESTION2,
	BLUETOOTH_CMD_START_QUESTION3,
};

// ====================  返回值定义  ====================

typedef enum {
    BLUETOOTH_OK = 0,
    BLUETOOTH_ERROR = -1,
    BLUETOOTH_TIMEOUT = -2,
    BLUETOOTH_INVALID_PARAM = -3,
    BLUETOOTH_BUFFER_FULL = -4,
    BLUETOOTH_NO_DATA = -5
} bluetooth_result_t;

// ====================  函数声明  ====================

/**
 * @brief 初始化蓝牙模块
 * @return 初始化结果
 */
bluetooth_result_t bluetooth_init(void);

/**
 * @brief 发送单字节数据
 * @param byte 要发送的字节
 * @return 发送结果
 */
bluetooth_result_t bluetooth_send_byte(uint8_t byte);

/**
 * @brief 发送多字节数据
 * @param data 数据指针
 * @param length 数据长度
 * @return 发送结果
 */
bluetooth_result_t bluetooth_send_data(const uint8_t* data, size_t length);

/**
 * @brief 处理蓝牙接收（在主循环中调用）
 */
void bluetooth_process(void);

/**
 * @brief 数据包回调函数（用户需要实现）
 * @param data 数据指针
 * @param length 数据长度
 */
void bluetooth_data_received(const uint8_t* data, size_t length);

void bluetooth_irq_handler(DL_UART_IIDX idx);

#endif // BLUETOOTH_H