#include "delay.h"

// 定义毫秒和秒的转换常量
#define MS_PER_SECOND     (1000U)
#define US_PER_MS         (1000U)

// 定义CPU时钟频率相关的延时周期计算
#define CYCLES_PER_US     (CPUCLK_FREQ / 1000000U)
#define CYCLES_PER_MS     (CPUCLK_FREQ / 1000U)

/**
 * @brief 基于CPU周期的忙等待延时
 * @param cycles 要等待的CPU周期数
 */
static void busy_wait_cycles(uint32_t cycles)
{
    delay_cycles(cycles);
}

/**
 * @brief 基于毫秒的忙等待延时
 * @param ms 延时时间（单位：毫秒）
 */
void delay_ms(uint32_t ms)
{
    while (ms--) {
        busy_wait_cycles(CYCLES_PER_MS);
    }
}



/**
 * @brief 微秒级延时
 * @param us 延时时间（单位：微秒）
 */
void delay_us(uint32_t us)
{
    while (us--) {
        busy_wait_cycles(CYCLES_PER_US);
    }
}
