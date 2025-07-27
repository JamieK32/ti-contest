/**
 * @file camera.h
 * @brief 摄像头通信模块
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ti_msp_dl_config.h"

// ====================  返回值定义  ====================

typedef enum {
    CAMERA_OK = 0,
    CAMERA_ERROR,
    CAMERA_INVALID_PARAM,
    CAMERA_TIMEOUT,
    CAMERA_BUSY
} camera_result_t;

// ====================  公共函数声明  ====================

/**
 * @brief 初始化摄像头通信
 * @return camera_result_t 初始化结果
 */
camera_result_t camera_init(void);

/**
 * @brief 发送单个字节到摄像头
 * @param byte 要发送的字节
 * @return camera_result_t 发送结果
 */
camera_result_t camera_send_byte(uint8_t byte);

/**
 * @brief 发送数据到摄像头
 * @param data 数据指针
 * @param length 数据长度
 * @return camera_result_t 发送结果
 */
camera_result_t camera_send_data(const uint8_t* data, size_t length);

/**
 * @brief 发送字符串到摄像头
 * @param str 字符串指针
 * @return camera_result_t 发送结果
 */
camera_result_t camera_send_string(const char* str);

/**
 * @brief 处理摄像头通信数据
 * @note 需要在主循环中定期调用
 */
void camera_process(void);

/**
 * @brief 摄像头中断处理函数
 * @param idx 中断索引
 * @note 在UART中断服务程序中调用
 */
void camera_irq_handler(DL_UART_IIDX idx);

// ====================  回调函数（用户实现）  ====================

/**
 * @brief 摄像头数据接收回调函数
 * @param data 接收到的数据
 * @param length 数据长度
 * @note 用户需要实现此函数来处理接收到的数据
 */
void camera_data_received(const uint8_t* data, size_t length);

#endif /* CAMERA_H */