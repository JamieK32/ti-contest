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
0;
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "encoder.h"
#include <stddef.h> // For NULL
#include <stdlib.h> // For malloc/free if dynamically allocating
#include "log.h"

// 编码器更新函数 (由中断服务程序或轮询调用)
void encoder_update(void *arg) {
    encoder_instance_t *instance = (encoder_instance_t *)arg;
    encoder_manager_t* manager = instance->manager; // 获取所属的管理器

    // 使用用户提供的 GPIO 读取函数 (通过管理器访问)
    uint8_t p1val = manager->gpio_read_func(instance->pin1_gpio_handle, instance->pin1_bitmask) ? 1 : 0;
    uint8_t p2val = manager->gpio_read_func(instance->pin2_gpio_handle, instance->pin2_bitmask) ? 1 : 0;

    uint8_t current_state = instance->state & 3;

    if (p1val) current_state |= 4;
    if (p2val) current_state |= 8;

    instance->state = (current_state >> 2);

    switch (current_state) {
        case 1: case 7: case 8: case 14:
            instance->position++;
            return;
        case 2: case 4: case 11: case 13:
            instance->position--;
            return;
        case 3: case 12:
            instance->position += 2;
            return;
        case 6: case 9:
            instance->position -= 2;
            return;
        default:
             // No change or invalid state
             return;
    }
}

bool encoder_manager_init(
    encoder_manager_t* manager,
    const encoder_config_t* configs, uint8_t num_encoders,
    encoder_gpio_read_func_t gpio_read_func,
    encoder_attach_interrupt_func_t attach_interrupt_func,
    encoder_enter_critical_func_t enter_critical_func,
    encoder_exit_critical_func_t exit_critical_func
) {
    if (manager == NULL || configs == NULL || num_encoders == 0) {
        log_e("Invalid arguments for encoder_manager_init.");
        return false;
    }

    // 可以选择动态分配内存，或者使用静态全局数组
    // 这里为了简单，假设 manager->encoders 已经指向一个足够大的静态数组
    // 如果需要动态分配，请取消注释下面的代码
    /*
    manager->encoders = (encoder_instance_t*)malloc(num_encoders * sizeof(encoder_instance_t));
    if (manager->encoders == NULL) {
        log_e("Failed to allocate memory for encoder instances.");
        return false;
    }
    */

    manager->num_encoders = num_encoders;
    manager->gpio_read_func = gpio_read_func;
    manager->attach_interrupt_func = attach_interrupt_func;
    manager->enter_critical_func = enter_critical_func;
    manager->exit_critical_func = exit_critical_func;

    for (uint8_t i = 0; i < num_encoders; ++i) {
        manager->encoders[i].manager = manager; // 设置指向所属的管理器
        manager->encoders[i].pin1_gpio_handle = configs[i].pin1_gpio_handle;
        manager->encoders[i].pin1_bitmask = configs[i].pin1_bitmask;
        manager->encoders[i].pin2_gpio_handle = configs[i].pin2_gpio_handle;
        manager->encoders[i].pin2_bitmask = configs[i].pin2_bitmask;
        manager->encoders[i].pin1_handle = configs[i].pin1_handle;
        manager->encoders[i].pin2_handle = configs[i].pin2_handle;
        manager->encoders[i].position = 0;
        manager->encoders[i].interrupts_in_use = 0;

        // 在初始化时读取初始状态
        // 考虑添加一个延时，让外部电路稳定
        // delayMicroseconds(2000); // 如果需要，请实现一个平台无关的延时函数

        uint8_t s = 0;
        if (manager->gpio_read_func(manager->encoders[i].pin1_gpio_handle, manager->encoders[i].pin1_bitmask)) s |= 1;
        if (manager->gpio_read_func(manager->encoders[i].pin2_gpio_handle, manager->encoders[i].pin2_bitmask)) s |= 2;
        manager->encoders[i].state = s;

        // 尝试为每个编码器实例挂载中断
        if (manager->attach_interrupt_func != NULL) {
            // 为 A 相引脚挂载中断
            if (manager->attach_interrupt_func(manager->encoders[i].pin1_handle, encoder_update, &(manager->encoders[i]))) {
                manager->encoders[i].interrupts_in_use++;
            }
            // 为 B 相引脚挂载中断
            if (manager->attach_interrupt_func(manager->encoders[i].pin2_handle, encoder_update, &(manager->encoders[i]))) {
                manager->encoders[i].interrupts_in_use++;
            }
            if (manager->encoders[i].interrupts_in_use < 2) {
                 log_w("Encoder instance %u: Failed to attach interrupt to both pins. Will be polled.", i);
            } else {
                 log_i("Encoder instance %u: Interrupts attached to both pins.", i);
            }
        } else {
             log_w("Encoder instance %u: No interrupt attach function provided. Will be polled.", i);
        }
    }
    log_i("Encoder manager initialized with %u encoders.", num_encoders);
    return true;
}

int32_t encoder_manager_read(encoder_manager_t* manager, uint8_t index) {
    int32_t ret = 0;

    if (manager == NULL || index >= manager->num_encoders) {
        log_e("Invalid encoder index %u for read operation.", index);
        return 0;
    }

    encoder_instance_t* instance = &manager->encoders[index];

    // 如果中断未使用（例如 attach_interrupt_func 为 NULL 或只挂载成功一根线），则需要手动更新
    if (instance->interrupts_in_use < 2) {
        // 进入临界区保护共享资源
        if (manager->enter_critical_func != NULL) {
            manager->enter_critical_func();
        }
        // 手动调用更新函数
        encoder_update(instance);
        ret = instance->position;
        // 退出临界区
        if (manager->exit_critical_func != NULL) {
            manager->exit_critical_func();
        }
    } else {
        // 如果使用了中断，只需要保护对 position 的访问
        // 进入临界区保护共享资源
        if (manager->enter_critical_func != NULL) {
            manager->enter_critical_func();
        }
        ret = instance->position;
        // 退出临界区
        if (manager->exit_critical_func != NULL) {
            manager->exit_critical_func();
        }
    }
    return ret;
}

int32_t encoder_manager_read_and_reset(encoder_manager_t* manager, uint8_t index) {
    int32_t ret = 0;

    if (manager == NULL || index >= manager->num_encoders) {
        log_e("Invalid encoder index %u for read_and_reset operation.", index);
        return 0;
    }

    encoder_instance_t* instance = &manager->encoders[index];

    // 进入临界区保护共享资源
    if (manager->enter_critical_func != NULL) {
        manager->enter_critical_func();
    }

    // 如果中断未使用，则需要手动更新
    if (instance->interrupts_in_use < 2) {
        encoder_update(instance);
    }

    ret = instance->position;
    instance->position = 0;

    // 退出临界区
    if (manager->exit_critical_func != NULL) {
        manager->exit_critical_func();
    }
    return ret;
 }

void encoder_manager_write(encoder_manager_t* manager, uint8_t index, int32_t position) {
    if (manager == NULL || index >= manager->num_encoders) {
        log_e("Invalid encoder index %u for write operation.", index);
        return;
    }

    encoder_instance_t* instance = &manager->encoders[index];

    // 进入临界区保护共享资源
    if (manager->enter_critical_func != NULL) {
        manager->enter_critical_func();
    }
    instance->position = position;
    // 退出临界区
    if (manager->exit_critical_func != NULL) {
        manager->exit_critical_func();
    }
}
