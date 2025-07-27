#include "vl53l1_read.h"
#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"

void vl53l1_test(void)
{
		log_i("TOF模块测试开始");
    VL53L1_Read_Init();    // 初始化一次

    while(1) {
       static  uint16_t distance = 0;
        
        VL53L1_Process();    // 处理状态机
        
        VL53L1_GetDistance(&distance) ;
         if (distance) {
					 log_i("距离: %d mm\n", distance);
				 }
        
        delay_ms(1);
    }
}