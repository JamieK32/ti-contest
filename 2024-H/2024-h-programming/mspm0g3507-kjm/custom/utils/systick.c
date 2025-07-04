/**
 * @file system_time.c
 * @brief MSP M0G3507 简单毫秒时间管理
 */

#include "systick.h"
#include "ti_msp_dl_config.h"

// 全局变量
static volatile uint32_t system_ms_count = 0;    // 毫秒计数器

/**
 * @brief 初始化系统时间模块
 */
void system_time_init(void) {
    // 配置SysTick定时器，每1ms产生一次中断
    SysTick_Config(CPUCLK_FREQ / 1000);
    
    // 初始化计数器
    system_ms_count = 0;
}

/**
 * @brief SysTick中断服务程序
 */
void SysTick_Handler(void) {
    system_ms_count++;
}

/**
 * @brief 获取系统运行时间（毫秒）
 * @return 系统运行时间（毫秒）
 */
uint32_t system_time_get_ms(void) {
    return system_ms_count;
}