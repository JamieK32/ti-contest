/* Encoder Library, for measuring quadrature encoded signals
 * Based on the Arduino Encoder Library by Paul Stoffregen
 *
 * Copyright (c) 2011,2013 PJRC.COM, LLC - Paul Stoffregen <paul@pjrc.com>
 *
 * This version adapted for multi-platform C by Your Name Here
 *
 * Version 2.1 - Fix gpio_read_func access in encoder_update
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>
#include <stdbool.h>

// 前向声明 encoder_manager_t
typedef struct encoder_manager_t encoder_manager_t;

// 定义GPIO读取函数类型
typedef uint8_t (*encoder_gpio_read_func_t)(void *gpio_handle, uint32_t pin_mask);

// 定义中断挂载函数类型
typedef bool (*encoder_attach_interrupt_func_t)(void *pin_handle, void (*isr_handler)(void *arg), void *arg);

// 定义保护资源函数类型
typedef void (*encoder_enter_critical_func_t)(void);
typedef void (*encoder_exit_critical_func_t)(void);

// 单个编码器实例的内部状态
typedef struct {
    encoder_manager_t* manager;          // 指向所属的编码器管理器
    void*           pin1_gpio_handle;   // GPIO外设句柄 for pin1
    void*           pin2_gpio_handle;   // GPIO外设句柄 for pin2
    uint32_t        pin1_bitmask;       // pin1 位掩码
    uint32_t        pin2_bitmask;       // pin2 位掩码
    uint8_t         state;              // 编码器状态
    int32_t         position;           // 编码器位置
    void*           pin1_handle;        // pin1 硬件句柄，用于中断挂载
    void*           pin2_handle;        // pin2 硬件句柄，用于中断挂载
    uint8_t         interrupts_in_use;  // 记录成功挂载中断的引脚数量
} encoder_instance_t;

// 编码器配置结构体
typedef struct {
    void* pin1_gpio_handle;
    uint32_t pin1_bitmask;
    void* pin2_gpio_handle;
    uint32_t pin2_bitmask;
    void* pin1_handle; // 平台相关句柄
    void* pin2_handle; // 平台相关句柄
} encoder_config_t;

// 编码器管理器句柄
struct encoder_manager_t { // 定义 encoder_manager_t 结构体
    encoder_instance_t* encoders;       // 编码器实例数组
    uint8_t             num_encoders;   // 编码器数量

    // 硬件抽象层接口 (这些接口在初始化时只设置一次)
    encoder_gpio_read_func_t        gpio_read_func;
    encoder_attach_interrupt_func_t attach_interrupt_func;
    encoder_enter_critical_func_t   enter_critical_func;
    encoder_exit_critical_func_t    exit_critical_func;
};

/**
 * @brief 初始化编码器管理器和所有编码器实例
 *
 * @param manager 编码器管理器句柄
 * @param configs 编码器配置数组
 * @param num_encoders 编码器数量
 * @param gpio_read_func 读取GPIO状态的函数指针
 * @param attach_interrupt_func 挂载中断的函数指针 (用户提供平台相关实现)
 * @param enter_critical_func 进入临界区的函数指针
 * @param exit_critical_func 退出临界区的函数指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool encoder_manager_init(
    encoder_manager_t* manager,
    const encoder_config_t* configs, uint8_t num_encoders,
    encoder_gpio_read_func_t gpio_read_func,
    encoder_attach_interrupt_func_t attach_interrupt_func,
    encoder_enter_critical_func_t enter_critical_func,
    encoder_exit_critical_func_t exit_critical_func
);

/**
 * @brief 读取指定索引的编码器位置
 *
 * @param manager 编码器管理器句柄
 * @param index 编码器索引 (从 0 开始)
 * @return int32_t 编码器位置，如果索引无效则返回 0
 */
int32_t encoder_manager_read(encoder_manager_t* manager, uint8_t index);

/**
 * @brief 读取指定索引的编码器位置并清零
 *
 * @param manager 编码器管理器句柄
 * @param index 编码器索引 (从 0 开始)
 * @return int32_t 编码器位置，如果索引无效则返回 0
 */
int32_t encoder_manager_read_and_reset(encoder_manager_t* manager, uint8_t index);

/**
 * @brief 设置指定索引的编码器位置
 *
 * @param manager 编码器管理器句柄
 * @param index 编码器索引 (从 0 开始)
 * @param position 要设置的位置
 */
void encoder_manager_write(encoder_manager_t* manager, uint8_t index, int32_t position);

/**
 * @brief 编码器更新函数 (通常由中断服务程序调用)
 *
 * @param arg 指向 encoder_instance_t 结构的指针
 */
void encoder_update(void *arg);

#endif // ENCODER_H_
