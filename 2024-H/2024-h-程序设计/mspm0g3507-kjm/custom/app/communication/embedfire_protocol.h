#ifndef EMBEDFIRE_PROTOCOL_H
#define EMBEDFIRE_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <string.h> // For memcpy and memmove

// 包含你的 UART 驱动头文件
#include "uart_debug.h" // 假设这个头文件提供了 usart_send_bytes 和 usart_send_byte 函数

//==============================================================================
// 协议常量定义
//==============================================================================
#define PROTOCOL_HEADER 0x59485A53 // 包头

// 下位机 -> 上位机 指令码
#define CMD_TX_SET_TARGET_VALUE    0x01 // 设置上位机通道的目标值 (int)
#define CMD_TX_SET_ACTUAL_VALUE    0x02 // 设置上位机通道实际值 (int)
#define CMD_TX_SET_PID_PARAMS      0x03 // 设置上位机PID值 (float, float, float)
#define CMD_TX_SYNC_START          0x04 // 同步上位机启动按钮状态
#define CMD_TX_SYNC_STOP           0x05 // 同步上位机停止按钮状态
#define CMD_TX_SET_PERIOD          0x06 // 设置上位机周期 (uint)

// 上位机 -> 下位机 指令码
#define CMD_RX_SET_PID_PARAMS      0x10 // 设置下位机的PID值 (float, float, float)
#define CMD_RX_SET_TARGET_VALUE    0x11 // 设置下位机的目标值 (int)
#define CMD_RX_START               0x12 // 启动指令
#define CMD_RX_STOP                0x13 // 停止指令
#define CMD_RX_RESET               0x14 // 复位指令
#define CMD_RX_SET_PERIOD          0x15 // 设置下位机周期 (uint)

//==============================================================================
// 通道定义
//==============================================================================
typedef enum {
    EMBEDFIRE_CHANNEL_1 = 1,
    EMBEDFIRE_CHANNEL_2,
    EMBEDFIRE_CHANNEL_3,
    EMBEDFIRE_CHANNEL_4,
    EMBEDFIRE_CHANNEL_5,
    MAX_EMBEDFIRE_CHANNELS // 用于通道范围检查
} EMBEDFIRE_CHANNELS;

//==============================================================================
// 数据包结构定义
//==============================================================================
// 所有数据包的通用头部
// 使用 __attribute__((packed)) 来确保结构体成员紧密排列，没有填充字节
typedef struct {
    uint32_t head;      // 4 bytes, 包头 0x59485A53
    uint8_t  channel;   // 1 byte, 通道地址 (EMBEDFIRE_CHANNELS)
    uint32_t length;    // 4 bytes, 包长度 (从包头到校验)
    uint8_t  command;   // 1 byte, 指令码
} __attribute__((packed)) ProtocolHeader;

// 下位机 -> 上位机 数据包结构
typedef struct { ProtocolHeader header; int32_t target_value;  uint8_t  checksum; } __attribute__((packed)) TxSetTargetValuePacket;
typedef struct { ProtocolHeader header; int32_t actual_value;  uint8_t  checksum; } __attribute__((packed)) TxSetActualValuePacket;
typedef struct { ProtocolHeader header; float p_param; float i_param; float d_param; uint8_t checksum; } __attribute__((packed)) TxSetPIDParamsPacket;
typedef struct { ProtocolHeader header; uint8_t  checksum; } __attribute__((packed)) TxSyncStatePacket;
typedef struct { ProtocolHeader header; uint32_t period; uint8_t  checksum; } __attribute__((packed)) TxSetPeriodPacket;

// 上位机 -> 下位机 数据包结构 (用于接收)
// 设置下位机PID值 (0x10)
typedef struct {
    ProtocolHeader header;
    float    p_param;      // 4 bytes, P参数
    float    i_param;      // 4 bytes, I参数
    float    d_param;      // 4 bytes, D参数
    uint8_t  checksum;     // 1 byte, 校验和
} __attribute__((packed)) RxSetPIDParamsPacket;

// 设置下位机的目标值 (0x11)
typedef struct {
    ProtocolHeader header;
    int32_t  target_value; // 4 bytes, 目标值 (int 类型)
    uint8_t  checksum;     // 1 byte, 校验和
} __attribute__((packed)) RxSetTargetValuePacket;

// 启动指令 (0x12), 停止指令 (0x13), 复位指令 (0x14) - 这些指令只有头部和校验和
typedef struct {
    ProtocolHeader header;
    uint8_t  checksum;     // 1 byte, 校验和
} __attribute__((packed)) RxSimpleCommandPacket;

// 设置下位机周期 (0x15)
typedef struct {
    ProtocolHeader header;
    uint32_t period;       // 4 bytes, 周期 (unsigned int 类型)
    uint8_t  checksum;     // 1 byte, 校验和
} __attribute__((packed)) RxSetPeriodPacket;

//==============================================================================
// 串口初始化
//==============================================================================
void embedfire_protocol_receive_init(void);

//==============================================================================
// 校验和计算函数声明
//==============================================================================
/**
 * @brief 计算数据的校验和 (8位累加).
 * @param data: 数据缓冲区指针.
 * @param length: 数据长度.
 * @retval 校验和.
 */
uint8_t protocol_calculate_checksum(const uint8_t* data, size_t length);

//==============================================================================
// 协议组包和发送函数声明 (TX方向)
//==============================================================================
/**
 * @brief 向上位机发送目标值.
 * @param channel: 通道 (EMBEDFIRE_CHANNELS 枚举值).
 * @param target_value: 目标值 (int32_t).
 * @retval 无.
 */
void protocol_send_set_target_value(EMBEDFIRE_CHANNELS channel, int32_t target_value);

/**
 * @brief 向上位机发送实际值.
 * @param channel: 通道 (EMBEDFIRE_CHANNELS 枚举值).
 * @param actual_value: 实际值 (int32_t).
 * @retval 无.
 */
void protocol_send_set_actual_value(EMBEDFIRE_CHANNELS channel, int32_t actual_value);

/**
 * @brief 向上位机发送PID参数.
 * @param channel: 通道 (EMBEDFIRE_CHANNELS 枚举值).
 * @param p_param: P 参数 (float).
 * @param i_param: I 参数 (float).
 * @param d_param: D 参数 (float).
 * @retval 无.
 */
void protocol_send_set_pid_params(EMBEDFIRE_CHANNELS channel, float p_param, float i_param, float d_param);

/**
 * @brief 向上位机发送同步启动指令.
 * @param channel: 通道 (EMBEDFIRE_CHANNELS 枚举值).
 * @retval 无.
 */
void protocol_send_sync_start(EMBEDFIRE_CHANNELS channel);

/**
 * @brief 向上位机发送同步停止指令.
 * @param channel: 通道 (EMBEDFIRE_CHANNELS 枚举值).
 * @retval 无.
 */
void protocol_send_sync_stop(EMBEDFIRE_CHANNELS channel);

/**
 * @brief 向上位机发送周期值.
 * @param channel: 通道 (EMBEDFIRE_CHANNELS 枚举值).
 * @param period: 周期 (uint32_t).
 * @retval 无.
 */
void protocol_send_set_period(EMBEDFIRE_CHANNELS channel, uint32_t period);

//==============================================================================
// 协议接收和解析函数声明 (RX方向)
//==============================================================================

// 定义接收缓冲区大小
#define RX_BUFFER_SIZE 256 // 根据你的需求调整

/**
 * @brief 处理从UART接收到的一个字节数据.
 *        这个函数通常在UART接收中断服务程序中被调用，或者在主循环中
 *        轮询接收缓冲区并调用.
 * @param received_byte: 从UART接收到的一个字节.
 * @retval 无.
 */
void protocol_receive_byte(uint8_t received_byte);

// 回调函数类型定义
typedef void (*SetPIDParamsCallback)(uint8_t channel, float p, float i, float d);
typedef void (*SetTargetValueCallback)(uint8_t channel, int32_t target);
typedef void (*SimpleCommandCallback)(uint8_t channel); // 用于启动、停止、复位
typedef void (*SetPeriodCallback)(uint8_t channel, uint32_t period);

// 接收指令回调函数结构体
typedef struct {
    SetPIDParamsCallback   set_pid_params;   // CMD_RX_SET_PID_PARAMS (0x10)
    SetTargetValueCallback set_target_value; // CMD_RX_SET_TARGET_VALUE (0x11)
    SimpleCommandCallback  start;            // CMD_RX_START (0x12)
    SimpleCommandCallback  stop;             // CMD_RX_STOP (0x13)
    SimpleCommandCallback  reset;            // CMD_RX_RESET (0x14)
    SetPeriodCallback      set_period;       // CMD_RX_SET_PERIOD (0x15)
    // 如果有其他接收指令，在这里添加对应的回调函数指针
} ProtocolRxCallbacks;

/**
 * @brief 注册接收指令回调函数集合.
 * @param callbacks: 包含所有回调函数指针的结构体.
 * @retval 无.
 */
void protocol_register_rx_callbacks(const ProtocolRxCallbacks* callbacks);

#endif // EMBEDFIRE_PROTOCOL_H
