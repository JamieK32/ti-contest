#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"
/**
 * @file cam_protocol_example.c
 * @brief 摄像头协议库使用示例
 */

#if CAM_TEST

#include <stdio.h>

// ====================  回调函数实现  ====================

/**
 * @brief 循迹数据回调函数
 */
static void on_track_data(uint8_t track_value) {
    log_i("Track: 0x%02X (%d)\n", track_value, track_value);
}

/**
 * @brief 数字数据回调函数
 */
static void on_number_data(int32_t number_value) {
    log_i("Number: %d\n", number_value);
}

/**
 * @brief 命令数据回调函数
 */
static void on_command_data(uint8_t command_code) {
    log_i("Command: 0x%02X (%d)\n", command_code, command_code);
}

// ====================  摄像头数据接收处理  ====================

/**
 * @brief 在camera.c中的回调函数里调用
 */
void camera_data_received(const uint8_t* data, size_t length) {
    // 使用协议库处理数据
    cam_parse_result_t result = cam_protocol_process(data, length);
		camera_send_byte(0x41);
    if (result != CAM_PARSE_OK) {
        log_i("Parse error: %s\n", cam_protocol_get_error_string(result));
    }
}

// ====================  初始化示例  ====================

static void setup_cam_protocol(void) {
    // 初始化协议库
    cam_protocol_init();
    
    // 设置回调函数
    cam_protocol_set_track_callback(on_track_data);
    cam_protocol_set_number_callback(on_number_data);
    cam_protocol_set_command_callback(on_command_data);
    
    log_i("Camera protocol initialized\n");
}

// ====================  主函数示例  ====================

int cam_test(void) {
    // 设置协议库
		camera_init();
    setup_cam_protocol();

		while (1) {
				camera_process();
				delay_ms(1);
		}
   
    return 0;
}

#endif 