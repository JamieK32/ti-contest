#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"
#include "ganv_calibration.h"

uint16_t Anolog[8] = {0};
uint16_t white[8] = {1800, 1800, 1800, 1800, 1800, 1800, 1800, 1800};
uint16_t black[8] = {300, 300, 300, 300, 300, 300, 300, 300};
uint16_t Normal[8];

// 修复版本1：传递传感器对象
void sensor_auto_calibration_example(No_MCU_Sensor* sensor)  // 接收传感器指针
{
    sensor_calib_t calib;
    uint16_t black_calib[8], white_calib[8];
    calib_state_t state;
    
    // 使用传入的传感器对象
    calib_init(&calib);
    
    log_i("请将传感器放在黑色场上，开始校准...\r\n");
    delay_ms(3000); 
    
    // 校准循环
    do {
        state = calib_process(&calib, sensor);  // 使用传入的传感器
        
        // 状态提示
        static calib_state_t last_state = CALIB_IDLE;
        if(state != last_state) {
            switch(state) {
                case CALIB_BLACK: log_i("黑色校准中...\r\n"); break;
                case CALIB_WAIT:  log_i("请移动到白色场上...\r\n"); break;
                case CALIB_WHITE: log_i("白色校准中...\r\n"); break;
                case CALIB_SUCCESS: log_i("校准成功!\r\n"); break;
                case CALIB_FAILED:  log_i("校准失败!\r\n"); break;
                default: break;
            }
            last_state = state;
        }
        
        delay_ms(10);
        
    } while(state != CALIB_SUCCESS && state != CALIB_FAILED);
    
    // 获取结果并重新初始化同一个传感器
    if(calib_get_result(&calib, black_calib, white_calib)) {
        log_i("黑色值: {%d,%d,%d,%d,%d,%d,%d,%d}\r\n",
              black_calib[0], black_calib[1], black_calib[2], black_calib[3],
              black_calib[4], black_calib[5], black_calib[6], black_calib[7]);
        log_i("白色值: {%d,%d,%d,%d,%d,%d,%d,%d}\r\n",
              white_calib[0], white_calib[1], white_calib[2], white_calib[3],
              white_calib[4], white_calib[5], white_calib[6], white_calib[7]);
        
        // 用校准结果重新初始化传入的传感器
        No_MCU_Ganv_Sensor_Init(sensor, white_calib, black_calib);
        log_i("传感器初始化完成，开始正常工作\r\n");
        return; // 校准成功，返回
    } 
    
    // 校准失败，使用默认值
    uint16_t white[8] = {1800, 1800, 1800, 1800, 1800, 1800, 1800, 1800};
    uint16_t black[8] = {300, 300, 300, 300, 300, 300, 300, 300};
    No_MCU_Ganv_Sensor_Init(sensor, white, black);
    log_e("传感器校准失败，使用默认值\r\n");
}

void no_mcu_ganv_test(void)
{
    // 初始化
    No_MCU_Sensor sensor;  // 只创建一个传感器对象
    unsigned char Digital;
    
    // 基础初始化
    No_MCU_Ganv_Sensor_Init_Frist(&sensor);
    
    // 自动校准（传递传感器对象指针）
    sensor_auto_calibration_example(&sensor);
    
    delay_ms(100);
    
    // 主循环
    while (1) {
        No_Mcu_Ganv_Sensor_Task_Without_tick(&sensor);
        Digital = Get_Digtal_For_User(&sensor);
        log_i("Digital %d-%d-%d-%d-%d-%d-%d-%d\r\n", 
              (Digital >> 0) & 0x01, (Digital >> 1) & 0x01, (Digital >> 2) & 0x01,
              (Digital >> 3) & 0x01, (Digital >> 4) & 0x01, (Digital >> 5) & 0x01, 
              (Digital >> 6) & 0x01, (Digital >> 7) & 0x01);
        delay_ms(50);
    }
}