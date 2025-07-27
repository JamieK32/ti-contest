/**
 * @file cam_protocol.h  
 * @brief 摄像头私有协议库 - 只接收解析
 * @version 1.0
 * @author Your Name
 * @date 2025
 */

#ifndef CAM_PROTOCOL_H
#define CAM_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ====================  数据类型定义  ====================

/**
 * @brief 协议数据类型
 */
typedef enum {
    CAM_DATA_TRACK   = 'T',    // 循迹数据: T:0x33
    CAM_DATA_NUMBER  = 'N',    // 数字数据: N:123  
    CAM_DATA_COMMAND = 'C'     // 命令数据: C:0x01
} cam_data_type_t;

/**
 * @brief 协议解析结果
 */
typedef enum {
    CAM_PARSE_OK = 0,          // 解析成功
    CAM_PARSE_ERROR,           // 解析错误
    CAM_PARSE_INVALID_FORMAT,  // 格式无效
    CAM_PARSE_INVALID_LENGTH   // 长度无效
} cam_parse_result_t;

/**
 * @brief 解析后的数据结构
 */
typedef struct {
    cam_data_type_t type;      // 数据类型
    union {
        uint8_t track_value;   // 循迹数据值 (T类型)
        int32_t number_value;  // 数字数据值 (N类型)  
        uint8_t command_code;  // 命令代码值 (C类型)
    } data;
    bool valid;                // 数据是否有效
} cam_protocol_data_t;

// ====================  回调函数类型  ====================

/**
 * @brief 循迹数据回调函数类型
 * @param track_value 循迹数据值
 */
typedef void (*cam_track_callback_t)(uint8_t track_value);

/**
 * @brief 数字数据回调函数类型  
 * @param number_value 数字数据值
 */
typedef void (*cam_number_callback_t)(int32_t number_value);

/**
 * @brief 命令数据回调函数类型
 * @param command_code 命令代码值
 */
typedef void (*cam_command_callback_t)(uint8_t command_code);

// ====================  公共函数  ====================

/**
 * @brief 初始化协议库
 */
void cam_protocol_init(void);

/**
 * @brief 解析摄像头数据
 * @param data 原始数据
 * @param length 数据长度
 * @param parsed_data 解析结果输出
 * @return cam_parse_result_t 解析结果
 */
cam_parse_result_t cam_protocol_parse(const uint8_t* data, size_t length, cam_protocol_data_t* parsed_data);

/**
 * @brief 处理摄像头数据(自动调用回调)
 * @param data 原始数据
 * @param length 数据长度  
 * @return cam_parse_result_t 处理结果
 */
cam_parse_result_t cam_protocol_process(const uint8_t* data, size_t length);

/**
 * @brief 设置循迹数据回调函数
 * @param callback 回调函数指针
 */
void cam_protocol_set_track_callback(cam_track_callback_t callback);

/**
 * @brief 设置数字数据回调函数
 * @param callback 回调函数指针
 */
void cam_protocol_set_number_callback(cam_number_callback_t callback);

/**
 * @brief 设置命令数据回调函数
 * @param callback 回调函数指针
 */
void cam_protocol_set_command_callback(cam_command_callback_t callback);

/**
 * @brief 获取错误描述字符串
 * @param result 解析结果
 * @return 错误描述字符串
 */
const char* cam_protocol_get_error_string(cam_parse_result_t result);

#endif /* CAM_PROTOCOL_H */