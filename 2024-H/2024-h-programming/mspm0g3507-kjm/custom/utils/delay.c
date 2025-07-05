#include "delay.h"

// 定义毫秒和秒的转换常量
#define MS_PER_SECOND     (1000U)
#define US_PER_MS         (1000U)

// 从系统配置推算，MSPM0G350X 主频应该是 80MHz
#ifndef CPUCLK_FREQ
#define CPUCLK_FREQ       (80000000U)  // 80MHz
#endif

// 定义CPU时钟频率相关的延时周期计算
#define CYCLES_PER_US     (CPUCLK_FREQ / 1000000U)  // 80 cycles per microsecond
#define CYCLES_PER_MS     (CPUCLK_FREQ / 1000U)     // 80000 cycles per millisecond

/**
 * @brief 基于CPU周期的忙等待延时（防止编译器优化）
 * @param cycles 要等待的CPU周期数
 */
static void __attribute__((noinline)) busy_wait_cycles(uint32_t cycles)
{
    delay_cycles(cycles);  
}

/**
 * @brief 高精度微秒级延时
 * @param us 延时时间（单位：微秒）
 * @note 对于小于10us的延时，直接使用cycles计算更准确
 */
void delay_us(uint32_t us)
{
    if (us == 0) return;
    
    if (us < 10) {
        // 小延时直接用周期数，减少循环开销
        busy_wait_cycles(us * CYCLES_PER_US);
    } else {
        // 大延时用循环，每次1us
        while (us--) {
            busy_wait_cycles(CYCLES_PER_US);
        }
    }
}

/**
 * @brief 毫秒级延时
 * @param ms 延时时间（单位：毫秒）
 */
void delay_ms(uint32_t ms)
{
    if (ms == 0) return;
    
    while (ms--) {
        busy_wait_cycles(CYCLES_PER_MS);
    }
}

/**
 * @brief 获取当前CPU时钟频率（用于调试）
 */
uint32_t get_cpu_frequency(void)
{
    return CPUCLK_FREQ;
}


// 编译时检查
#if CYCLES_PER_US < 1
#error "CPU frequency too low for accurate microsecond delay"
#endif

#if CYCLES_PER_US > 1000
#warning "Very high CPU frequency, consider using timer-based delays for long delays"
#endif