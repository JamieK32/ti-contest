/**
 * @file cam_protocol.c
 * @brief 摄像头私有协议库实现 - 只接收解析
 */

#include "cam_protocol.h"
#include <stdlib.h>
#include <string.h>

// ====================  内部变量  ====================

static cam_track_callback_t   track_callback = NULL;
static cam_number_callback_t  number_callback = NULL; 
static cam_command_callback_t command_callback = NULL;

// ====================  内部函数声明  ====================

static uint8_t hex_char_to_byte(char high, char low);
static uint8_t hex_char_to_nibble(char c);

// ====================  公共函数实现  ====================

void cam_protocol_init(void) {
    track_callback = NULL;
    number_callback = NULL;
    command_callback = NULL;
}

cam_parse_result_t cam_protocol_parse(const uint8_t* data, size_t length, cam_protocol_data_t* parsed_data) {
    if (data == NULL || parsed_data == NULL) {
        return CAM_PARSE_ERROR;
    }
    
    // 初始化结果
    parsed_data->valid = false;
    
    // 检查最小长度: "X:"
    if (length < 3) {
        return CAM_PARSE_INVALID_LENGTH;
    }
    
    // 检查格式: 第二个字符必须是':'
    if (data[1] != ':') {
        return CAM_PARSE_INVALID_FORMAT;
    }
    
    // 获取数据类型
    parsed_data->type = (cam_data_type_t)data[0];
    
    switch (parsed_data->type) {
        case CAM_DATA_TRACK:    // T:0x33
            if (length >= 6 && data[2] == '0' && data[3] == 'x') {
                parsed_data->data.track_value = hex_char_to_byte(data[4], data[5]);
                parsed_data->valid = true;
                return CAM_PARSE_OK;
            }
            return CAM_PARSE_INVALID_FORMAT;
            
        case CAM_DATA_NUMBER:   // N:123
            if (length > 2) {
                char number_str[16] = {0};
                size_t num_len = length - 2;
                if (num_len < sizeof(number_str)) {
                    memcpy(number_str, &data[2], num_len);
                    parsed_data->data.number_value = atoi(number_str);
                    parsed_data->valid = true;
                    return CAM_PARSE_OK;
                }
            }
            return CAM_PARSE_INVALID_FORMAT;
            
        case CAM_DATA_COMMAND:  // C:0x01
            if (length >= 6 && data[2] == '0' && data[3] == 'x') {
                parsed_data->data.command_code = hex_char_to_byte(data[4], data[5]);
                parsed_data->valid = true;
                return CAM_PARSE_OK;
            }
            return CAM_PARSE_INVALID_FORMAT;
            
        default:
            return CAM_PARSE_INVALID_FORMAT;
    }
}

cam_parse_result_t cam_protocol_process(const uint8_t* data, size_t length) {
    cam_protocol_data_t parsed_data;
    cam_parse_result_t result = cam_protocol_parse(data, length, &parsed_data);
    
    if (result == CAM_PARSE_OK && parsed_data.valid) {
        // 调用相应的回调函数
        switch (parsed_data.type) {
            case CAM_DATA_TRACK:
                if (track_callback != NULL) {
                    track_callback(parsed_data.data.track_value);
                }
                break;
                
            case CAM_DATA_NUMBER:
                if (number_callback != NULL) {
                    number_callback(parsed_data.data.number_value);
                }
                break;
                
            case CAM_DATA_COMMAND:
                if (command_callback != NULL) {
                    command_callback(parsed_data.data.command_code);
                }
                break;
        }
    }
    
    return result;
}

void cam_protocol_set_track_callback(cam_track_callback_t callback) {
    track_callback = callback;
}

void cam_protocol_set_number_callback(cam_number_callback_t callback) {
    number_callback = callback;
}

void cam_protocol_set_command_callback(cam_command_callback_t callback) {
    command_callback = callback;
}

const char* cam_protocol_get_error_string(cam_parse_result_t result) {
    switch (result) {
        case CAM_PARSE_OK:
            return "Success";
        case CAM_PARSE_ERROR:
            return "Parse error";
        case CAM_PARSE_INVALID_FORMAT:
            return "Invalid format";
        case CAM_PARSE_INVALID_LENGTH:
            return "Invalid length";
        default:
            return "Unknown error";
    }
}

// ====================  内部函数实现  ====================

static uint8_t hex_char_to_byte(char high, char low) {
    return (hex_char_to_nibble(high) << 4) | hex_char_to_nibble(low);
}

static uint8_t hex_char_to_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return 0;
}