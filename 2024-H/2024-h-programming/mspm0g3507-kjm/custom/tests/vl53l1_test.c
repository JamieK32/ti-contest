#include "vl53l1_read.h"
#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"

void vl53l1_test(void)
{
    VL53L1_Read_Init();    // 初始化一次
    
    while(1) {
        uint16_t distance;
        
        VL53L1_Process();    // 处理状态机
        
        if (VL53L1_GetDistance(&distance) == VL53L1_OK) {
            log_i("距离: %d mm\n", distance);
        }
        
        delay_ms(10);
    }
}