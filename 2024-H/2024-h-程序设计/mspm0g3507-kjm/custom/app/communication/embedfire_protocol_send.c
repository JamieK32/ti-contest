#include "embedfire_protocol.h"
#include "log_config.h"
#include "log.h"

//==============================================================================
// 校验和计算函数实现
//==============================================================================
// 协议中提到是 8 位校验和方式，通常是累加求和后取低8位
uint8_t protocol_calculate_checksum(const uint8_t* data, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum += data[i];
    }
    return sum;
}

//==============================================================================
// 内部函数: 发送通用数据包
//==============================================================================
/**
 * @brief 内部函数，用于发送数据包数据和校验和.
 * @param data: 数据缓冲区指针 (不包含校验和).
 * @param data_length: 数据长度 (不包含校验和).
 * @retval 无.
 */
void protocol_send_packet(const uint8_t* data, size_t data_length) {
    uint8_t checksum = protocol_calculate_checksum(data, data_length);
    debug_uart_send_bytes(data, data_length);
    debug_uart_send_byte(checksum);
}

//==============================================================================
// 协议组包和发送函数实现 (TX方向)
//==============================================================================
// 辅助函数：检查通道是否有效
static int is_valid_channel(EMBEDFIRE_CHANNELS channel) {
    return (channel >= EMBEDFIRE_CHANNEL_1 && channel < MAX_EMBEDFIRE_CHANNELS);
}

void protocol_send_set_target_value(EMBEDFIRE_CHANNELS channel, int32_t target_value) {
    if (!is_valid_channel(channel)) return; // 无效通道，不发送
    TxSetTargetValuePacket packet;
    packet.header.head = PROTOCOL_HEADER;
    packet.header.channel = channel;
    // 计算数据包长度 (包头 + 通道 + 长度 + 指令 + 参数 + 校验和)
    // 包长度是整个数据包的大小，包括校验和，但计算校验和时是不包含校验和自身的
    packet.header.length = sizeof(TxSetTargetValuePacket);
    packet.header.command = CMD_TX_SET_TARGET_VALUE;
    packet.target_value = target_value;
    // 发送数据包 (不包含最后的校验和字节，校验和由 protocol_send_packet 计算并发送)
    protocol_send_packet((const uint8_t*)&packet, sizeof(TxSetTargetValuePacket) - sizeof(packet.checksum));
}

void protocol_send_set_actual_value(EMBEDFIRE_CHANNELS channel, int32_t actual_value) {
     if (!is_valid_channel(channel)) return; // 无效通道，不发送
    TxSetActualValuePacket packet;
    packet.header.head = PROTOCOL_HEADER;
    packet.header.channel = channel;
    packet.header.length = sizeof(TxSetActualValuePacket);
    packet.header.command = CMD_TX_SET_ACTUAL_VALUE;
    packet.actual_value = actual_value;
    protocol_send_packet((const uint8_t*)&packet, sizeof(TxSetActualValuePacket) - sizeof(packet.checksum));
}

void protocol_send_set_pid_params(EMBEDFIRE_CHANNELS channel, float p_param, float i_param, float d_param) {
     if (!is_valid_channel(channel)) return; // 无效通道，不发送
    TxSetPIDParamsPacket packet;
    packet.header.head = PROTOCOL_HEADER;
    packet.header.channel = channel;
    // 包长度应包含3个float参数的长度，即整个结构体大小
    packet.header.length = sizeof(TxSetPIDParamsPacket);
    packet.header.command = CMD_TX_SET_PID_PARAMS;
    packet.p_param = p_param;
    packet.i_param = i_param;
    packet.d_param = d_param;
    protocol_send_packet((const uint8_t*)&packet, sizeof(TxSetPIDParamsPacket) - sizeof(packet.checksum));
}

void protocol_send_sync_start(EMBEDFIRE_CHANNELS channel) {
     if (!is_valid_channel(channel)) return; // 无效通道，不发送
    TxSyncStatePacket packet;
    packet.header.head = PROTOCOL_HEADER;
    packet.header.channel = channel;
    packet.header.length = sizeof(TxSyncStatePacket);
    packet.header.command = CMD_TX_SYNC_START;
    protocol_send_packet((const uint8_t*)&packet, sizeof(TxSyncStatePacket) - sizeof(packet.checksum));
}

void protocol_send_sync_stop(EMBEDFIRE_CHANNELS channel) {
     if (!is_valid_channel(channel)) return; // 无效通道，不发送
    TxSyncStatePacket packet;
    packet.header.head = PROTOCOL_HEADER;
    packet.header.channel = channel;
    packet.header.length = sizeof(TxSyncStatePacket);
    packet.header.command = CMD_TX_SYNC_STOP;
    protocol_send_packet((const uint8_t*)&packet, sizeof(TxSyncStatePacket) - sizeof(packet.checksum));
}

void protocol_send_set_period(EMBEDFIRE_CHANNELS channel, uint32_t period) {
     if (!is_valid_channel(channel)) return; // 无效通道，不发送
    TxSetPeriodPacket packet;
    packet.header.head = PROTOCOL_HEADER;
    packet.header.channel = channel;
    packet.header.length = sizeof(TxSetPeriodPacket);
    packet.header.command = CMD_TX_SET_PERIOD;
    packet.period = period;
    protocol_send_packet((const uint8_t*)&packet, sizeof(TxSetPeriodPacket) - sizeof(packet.checksum));
}