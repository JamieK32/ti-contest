/**
 * @file system_time.h
 * @brief MSP M0G3507 简单毫秒时间管理
 */

#ifndef SYSTICK_H__
#define SYSTICK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化系统时间模块
 */
void systick_init(void);

/**
 * @brief 获取系统运行时间（毫秒）
 * @return 系统运行时间（毫秒）
 */
uint32_t get_ms(void);

/**
 * @brief 毫秒延时函数
 * @param ms 延时时间（毫秒）
 */
void system_time_delay_ms(uint32_t ms);

/**
 * @brief 获取系统运行时间（微秒）
 * @return 系统运行时间（微秒）
 */
uint32_t get_us(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_TIME_H */