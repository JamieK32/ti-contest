#if GANV_SENSOR

// sensor_calibration.c
#include "ganv_calibration.h"
#include "no_mcu_ganv.h"
#include "log_config.h"
#include "log.h"
#include "delay.h"
#include "systick.h"

// sensor_calibration.c

// 校准参数配置
#define BLACK_TIME_MS    5000    // 黑色校准时间 10秒
#define WAIT_TIME_MS     3000     // 等待时间 5秒  
#define WHITE_TIME_MS    5000    // 白色校准时间 10秒
#define SAMPLE_INTERVAL  20       // 采样间隔 50ms

// 内部变量
static uint32_t start_time = 0;
static uint32_t last_sample = 0;
static uint32_t black_sum[8] = {0};
static uint32_t white_sum[8] = {0};
static uint32_t black_count = 0;
static uint32_t white_count = 0;

// 重置校准数据
static void reset_data(void)
{
    int i;
    for(i = 0; i < 8; i++) {
        black_sum[i] = 0;
        white_sum[i] = 0;
    }
    black_count = 0;
    white_count = 0;
}

// 初始化校准
void calib_init(sensor_calib_t* calib)
{
    calib->state = CALIB_BLACK;
    start_time = get_ms();
    last_sample = 0;
    reset_data();
    
    int i;
    for(i = 0; i < 8; i++) {
        calib->black[i] = 0;
        calib->white[i] = 0;
    }
}

// 校准处理函数
calib_state_t calib_process(sensor_calib_t* calib, No_MCU_Sensor* sensor)
{
    uint32_t now = get_ms();
    uint32_t elapsed = now - start_time;
    unsigned short values[8];
    int i;
    
    switch(calib->state) {
        case CALIB_BLACK:
            // 采样间隔控制
            if(now - last_sample >= SAMPLE_INTERVAL) {
                Get_Anolog_Value(sensor, values);
                
                // 累加黑色数据
                for(i = 0; i < 8; i++) {
                    black_sum[i] += values[i];
                }
                black_count++;
                last_sample = now;
            }
            
            // 10秒后进入等待状态
            if(elapsed >= BLACK_TIME_MS) {
                // 计算黑色平均值
                for(i = 0; i < 8; i++) {
                    if(black_count > 0) {
                        calib->black[i] = (unsigned short)(black_sum[i] / black_count);
                    }
                }
                
                log_i("黑色校准完成，采样%d次，平均值: %d-%d-%d-%d-%d-%d-%d-%d\r\n",
                      black_count,
                      calib->black[0], calib->black[1], calib->black[2], calib->black[3],
                      calib->black[4], calib->black[5], calib->black[6], calib->black[7]);
                
                calib->state = CALIB_WAIT;
                start_time = now;
            }
            break;
            
        case CALIB_WAIT:
            // 等待5秒
            if(elapsed >= WAIT_TIME_MS) {
                calib->state = CALIB_WHITE;
                start_time = now;
                last_sample = 0;
            }
            break;
            
        case CALIB_WHITE:
            // 采样间隔控制
            if(now - last_sample >= SAMPLE_INTERVAL) {
                Get_Anolog_Value(sensor, values);
                
                // 累加白色数据
                for(i = 0; i < 8; i++) {
                    white_sum[i] += values[i];
                }
                white_count++;
                last_sample = now;
            }
            
            // 10秒后完成校准
            if(elapsed >= WHITE_TIME_MS) {
                // 计算白色平均值
                for(i = 0; i < 8; i++) {
                    if(white_count > 0) {
                        calib->white[i] = (unsigned short)(white_sum[i] / white_count);
                    }
                }
                
                log_i("白色校准完成，采样%d次，平均值: %d-%d-%d-%d-%d-%d-%d-%d\r\n",
                      white_count,
                      calib->white[0], calib->white[1], calib->white[2], calib->white[3],
                      calib->white[4], calib->white[5], calib->white[6], calib->white[7]);
                
                // 验证校准结果
                unsigned char valid = 1;
                for(i = 0; i < 8; i++) {
                    if(calib->white[i] <= calib->black[i]) {
                        log_e("通道%d校准失败: 白色值(%d) <= 黑色值(%d)\r\n", 
                              i, calib->white[i], calib->black[i]);
                        valid = 0;
                    }
                }
                
                calib->state = valid ? CALIB_SUCCESS : CALIB_FAILED;
            }
            break;
            
        default:
            break;
    }
    
    return calib->state;
}

// 获取校准结果
unsigned char calib_get_result(sensor_calib_t* calib, unsigned short* black, unsigned short* white)
{
    if(calib->state == CALIB_SUCCESS) {
        int i;
        for(i = 0; i < 8; i++) {
            black[i] = calib->black[i];
            white[i] = calib->white[i];
        }
        return 1;
    }
    return 0;
}

#endif 