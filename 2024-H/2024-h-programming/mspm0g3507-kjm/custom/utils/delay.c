#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

// 定义是否在中断上下文的宏
#define IS_IN_INTERRUPT() (__get_IPSR() != 0)

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
static void busy_wait_ms(uint32_t ms)
{
    while (ms--) {
        busy_wait_cycles(CYCLES_PER_MS);
    }
}

/**
 * @brief 处理长时间延时，避免单次延时过长
 * @param ms_total 总延时时间（单位：毫秒）
 * @param max_once 单次最大延时时间（单位：毫秒）
 */
static void busy_wait_long_ms(uint32_t ms_total, uint32_t max_once)
{
    while (ms_total > 0) {
        uint32_t delay_this_time = (ms_total > max_once) ? max_once : ms_total;
        busy_wait_ms(delay_this_time);
        ms_total -= delay_this_time;
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

/**
 * @brief 毫秒级延时
 * @param ms 延时时间（单位：毫秒）
 */
void delay_ms(uint32_t ms)
{
    if (ms == 0) {
        return;
    }

    if (IS_IN_INTERRUPT()) {
        // 中断上下文，使用忙等待延时
        busy_wait_ms(ms);
    }
    else if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        // 任务上下文，调度器运行中，使用任务延时
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
    else {
        // 调度器未运行（如初始化阶段），使用忙等待延时
        busy_wait_ms(ms);
    }
}

/**
 * @brief 秒级延时
 * @param s 延时时间（单位：秒）
 */
void delay_s(uint32_t s)
{
    if (s == 0) {
        return;
    }

    uint32_t total_ms = s * MS_PER_SECOND;

    if (IS_IN_INTERRUPT()) {
        // 中断上下文，使用忙等待延时
        busy_wait_long_ms(total_ms, MS_PER_SECOND);
    }
    else if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        // 任务上下文，调度器运行中，使用任务延时
        vTaskDelay(pdMS_TO_TICKS(total_ms));
    }
    else {
        // 调度器未运行，使用忙等待延时
        busy_wait_long_ms(total_ms, MS_PER_SECOND);
    }
}