/**
 * @file system_time.c
 * @brief MSP M0G3507 简单毫秒时间管理
 */
#include "systick.h"
#include "ti_msp_dl_config.h"

// 宏定义
#define SYSTICK_LOAD_VALUE    (CPUCLK_FREQ / 1000)        // SysTick重装载值
#define TICKS_PER_US          (CPUCLK_FREQ / 1000000)     // 每微秒的tick数

// 全局变量
static volatile uint32_t system_ms_count = 0;    // 毫秒计数器

/**
 * @brief 初始化系统时间模块
 */
void systick_init(void) {
    // 配置SysTick定时器，每1ms产生一次中断
    SysTick_Config(SYSTICK_LOAD_VALUE);
    
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
uint32_t get_ms(void) {
    return system_ms_count;
}

uint32_t get_us(void) {
    uint32_t ms, ticks;
    __disable_irq();
    ms = system_ms_count;
    ticks = SysTick->VAL;
    __enable_irq();
    
    // 使用64位计算避免精度损失
    uint32_t elapsed_ticks = SYSTICK_LOAD_VALUE - ticks;
    uint32_t elapsed_us = ((uint64_t)elapsed_ticks * 1000000ULL) / CPUCLK_FREQ;
    
    return ms * 1000 + elapsed_us;
}