#include "embedfire_protocol.h"
#include "ti_msp_dl_config.h"

// 接收状态机的枚举
typedef enum {
    STATE_WAITING_FOR_HEADER_BYTE0, // 等待包头第一个字节 0x53
    STATE_WAITING_FOR_HEADER_BYTE1, // 等待包头第二个字节 0x5A
    STATE_WAITING_FOR_HEADER_BYTE2, // 等待包头第三个字节 0x48
    STATE_WAITING_FOR_HEADER_BYTE3, // 等待包头第四个字节 0x59
    STATE_RECEIVING_HEADER_DATA,    // 接收包头以外的固定头部数据 (通道、长度、指令)
    STATE_RECEIVING_PAYLOAD,        // 接收数据负载和校验和
} ReceiveState;

static ReceiveState rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
// 接收缓冲区
// 缓冲区大小需要能容纳最大可能的接收数据包，包括校验和
#define MAX_RX_PACKET_SIZE sizeof(RxSetPIDParamsPacket) // 最大的接收包是 RxSetPIDParamsPacket
static uint8_t rx_buffer[MAX_RX_PACKET_SIZE];
static size_t rx_buffer_index = 0;
static uint32_t expected_packet_length = 0; // 使用 uint32_t 存储包长度
// 回调函数结构体变量
static ProtocolRxCallbacks rx_callbacks = {0}; // 初始化为零，所有指针为NULL

// 回调函数注册函数实现
void protocol_register_rx_callbacks(const ProtocolRxCallbacks* callbacks) {
    if (callbacks) {
        rx_callbacks = *callbacks; // 复制结构体内容
    } else {
        // 如果传入NULL，可以将所有回调函数指针置为NULL
        memset(&rx_callbacks, 0, sizeof(ProtocolRxCallbacks));
    }
}
void embedfire_protocol_receive_init(void) {
	NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}

/**
 * @brief 根据接收到的完整数据包处理指令.
 * @param packet_data: 完整数据包的起始地址.
 * @param packet_length: 完整数据包的长度.
 * @retval 无.
 */
void process_received_packet(const uint8_t* packet_data, size_t packet_length) {
    const ProtocolHeader* header = (const ProtocolHeader*)packet_data;
    uint8_t channel = header->channel;
    uint8_t command = header->command;
    // 检查通道是否有效 (只处理 1-5 的通道)
    if (channel < EMBEDFIRE_CHANNEL_1 || channel >= MAX_EMBEDFIRE_CHANNELS) {
        // 无效通道，丢弃数据包
        return;
    }
    // 根据指令码和注册的回调函数来处理数据
    switch (command) {
        case CMD_RX_SET_PID_PARAMS: {
            // 校验包长度
            if (packet_length == sizeof(RxSetPIDParamsPacket)) {
                 const RxSetPIDParamsPacket* packet = (const RxSetPIDParamsPacket*)packet_data;
                 // 调用注册的回调函数
                 if (rx_callbacks.set_pid_params) {
                     rx_callbacks.set_pid_params(channel, packet->p_param, packet->i_param, packet->d_param);
                 }
            } else {
                // 包长度错误，可以添加错误日志
            }
            break;
        }
        case CMD_RX_SET_TARGET_VALUE: {
            // 校验包长度
            if (packet_length == sizeof(RxSetTargetValuePacket)) {
                const RxSetTargetValuePacket* packet = (const RxSetTargetValuePacket*)packet_data;
                // 调用注册的回调函数
                if (rx_callbacks.set_target_value) {
                    rx_callbacks.set_target_value(channel, packet->target_value);
                }
            } else {
                 // 包长度错误，可以添加错误日志
            }
            break;
        }
        case CMD_RX_START: {
            // 校验包长度
             if (packet_length == sizeof(RxSimpleCommandPacket)) {
                 // 调用注册的回调函数
                 if (rx_callbacks.start) {
                     rx_callbacks.start(channel);
                 }
             } else {
                 // 包长度错误，可以添加错误日志
             }
            break;
        }
        case CMD_RX_STOP: {
            // 校验包长度
             if (packet_length == sizeof(RxSimpleCommandPacket)) {
                 // 调用注册的回调函数
                 if (rx_callbacks.stop) {
                     rx_callbacks.stop(channel);
                 }
             } else {
                 // 包长度错误，可以添加错误日志
             }
            break;
        }
        case CMD_RX_RESET: {
            // 校验包长度
             if (packet_length == sizeof(RxSimpleCommandPacket)) {
                 // 调用注册的回调函数
                 if (rx_callbacks.reset) {
                     rx_callbacks.reset(channel);
                 }
             } else {
                 // 包长度错误，可以添加错误日志
             }
            break;
        }
        case CMD_RX_SET_PERIOD: {
            // 校验包长度
            if (packet_length == sizeof(RxSetPeriodPacket)) {
                const RxSetPeriodPacket* packet = (const RxSetPeriodPacket*)packet_data;
                // 调用注册的回调函数
                if (rx_callbacks.set_period) {
                    rx_callbacks.set_period(channel, packet->period);
                }
            } else {
                 // 包长度错误，可以添加错误日志
            }
            break;
        }
        default:
            // 未知指令，忽略，可以添加警告日志
            break;
    }
}
// 接收状态机处理函数
void protocol_receive_byte(uint8_t received_byte) {
    switch (rx_state) {
        case STATE_WAITING_FOR_HEADER_BYTE0:
            if (received_byte == 0x53) {
                rx_buffer[rx_buffer_index++] = received_byte;
                rx_state = STATE_WAITING_FOR_HEADER_BYTE1;
            } else {
                // 错误字节，继续等待第一个字节
                rx_buffer_index = 0; // 复位缓冲区索引
            }
            break;
        case STATE_WAITING_FOR_HEADER_BYTE1:
            if (received_byte == 0x5A) {
                rx_buffer[rx_buffer_index++] = received_byte;
                rx_state = STATE_WAITING_FOR_HEADER_BYTE2;
            } else {
                // 错误字节，复位状态机
                rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
                rx_buffer_index = 0;
            }
            break;
        case STATE_WAITING_FOR_HEADER_BYTE2:
            if (received_byte == 0x48) {
                rx_buffer[rx_buffer_index++] = received_byte;
                rx_state = STATE_WAITING_FOR_HEADER_BYTE3;
            } else {
                // 错误字节，复位状态机
                rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
                rx_buffer_index = 0;
            }
            break;
        case STATE_WAITING_FOR_HEADER_BYTE3:
            if (received_byte == 0x59) {
                rx_buffer[rx_buffer_index++] = received_byte;
                // 成功接收到完整的包头，进入接收头部数据状态
                rx_state = STATE_RECEIVING_HEADER_DATA;
            } else {
                // 错误字节，复位状态机
                rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
                rx_buffer_index = 0;
            }
            break;
        case STATE_RECEIVING_HEADER_DATA:
            // 接收包头以外的固定头部数据 (通道、长度、指令)
            rx_buffer[rx_buffer_index++] = received_byte;
            if (rx_buffer_index >= sizeof(ProtocolHeader)) {
                 // 已经接收了完整的头部，现在可以读取包长度
                 const ProtocolHeader* header = (const ProtocolHeader*)rx_buffer;
                 expected_packet_length = header->length;
                 // 检查期望的包长度是否合理 (防止缓冲区溢出或接收到不合法的短包)
                 // 最小包长度是 包头(4) + 通道(1) + 长度(4) + 指令(1) + 校验和(1) = 11 bytes
                 if (expected_packet_length > MAX_RX_PACKET_SIZE || expected_packet_length < (sizeof(ProtocolHeader) + sizeof(uint8_t))) {
                     // 非法长度，复位状态机
                     rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
                     rx_buffer_index = 0;
                     expected_packet_length = 0;
                     // 可以添加错误日志
                     return;
                 }
                 // 进入接收数据负载状态
                 rx_state = STATE_RECEIVING_PAYLOAD;
            } else if (rx_buffer_index >= MAX_RX_PACKET_SIZE) {
                // 缓冲区溢出 (虽然不应该发生在这个状态，但作为安全措施)
                rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
                rx_buffer_index = 0;
                expected_packet_length = 0;
                // 可以添加错误日志
            }
            break;
        case STATE_RECEIVING_PAYLOAD:
            // 接收数据负载和校验和
            if (rx_buffer_index < MAX_RX_PACKET_SIZE) {
                rx_buffer[rx_buffer_index++] = received_byte;
                // 检查是否接收到完整的数据包 (总长度达到 expected_packet_length)
                if (rx_buffer_index == expected_packet_length) {
                    // 接收到一个完整的数据包，现在进行校验和检查
                    uint8_t received_checksum = rx_buffer[rx_buffer_index - 1]; // 最后一个字节是校验和
                    uint8_t calculated_checksum = protocol_calculate_checksum(rx_buffer, rx_buffer_index - 1); // 计算数据部分的校验和
                    if (received_checksum == calculated_checksum) {
                        // 校验和匹配，处理数据包
                        process_received_packet(rx_buffer, rx_buffer_index);
                    } else {
                        // 校验和不匹配，数据包错误
                        // 可以添加错误日志
                    }
                    // 数据包处理完毕或校验错误，复位状态机，准备接收下一个包
                    rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
                    rx_buffer_index = 0;
                    expected_packet_length = 0;
                }
            } else {
                // 缓冲区溢出
                rx_state = STATE_WAITING_FOR_HEADER_BYTE0;
                rx_buffer_index = 0;
                expected_packet_length = 0;
                // 可以添加错误日志
            }
            break;
    }
}