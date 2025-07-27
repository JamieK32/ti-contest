#include "task21f_config.h"
#include "common_include.h"
//#include "log_config.h"
#include "log.h"


maixCam_t maix_cam;

//     "TRACK"   : do_tracking,                                               #循迹
//     "NUMBER"  : do_digiting,                                               #数字识别
//     "SQUARE"  : lambda img: detector.detect(),                            #停车点识别
//     "TRACK_SQ": do_tracking_with_detector,                                 #循迹加停车识别
//     "STOP"    : lambda img: None                                           #停止所有任务



/**
 * @brief 循迹数据回调函数
 */
static void on_track_data(uint8_t track_value) {
    maix_cam.track_data = track_value;
    log_i("Track data received: 0x%02X", track_value);
}

/**
 * @brief 数字数据回调函数
 */
static void on_number_data(int32_t number_value) {
		maix_cam.num = number_value;
    log_i("Number data received: %d", number_value);
}

/**
 * @brief 命令数据回调函数
 */
static void on_command_data(uint8_t command_code) {
		maix_cam.cmd = command_code;
    log_i("Command data received: 0x%02X", command_code);
}

// ====================  摄像头数据接收处理  ====================
/**
 * @brief 在camera.c中的回调函数里调用
 */
void camera_data_received(const uint8_t* data, size_t length) {

    // 使用协议库处理数据
    cam_parse_result_t result = cam_protocol_process(data, length);
    

    if (result != CAM_PARSE_OK) {
        log_i("Parse error: %s", cam_protocol_get_error_string(result));
    }
}

// ====================  初始化示例  ====================
void setup_cam_protocol(void) {
    // 初始化协议库
    cam_protocol_init();
    // 设置回调函数
    cam_protocol_set_track_callback(on_track_data);
    cam_protocol_set_number_callback(on_number_data);
    cam_protocol_set_command_callback(on_command_data);
    log_i("Camera protocol initialized");
}
