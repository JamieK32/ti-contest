// encoder_app.c

#include "encoder.h"
#include "ti_msp_dl_config.h" // 包含您的 Sysconfig 生成的头文件
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
#include "encoder_app.h"

// 如果需要使用log，包含log_config.h
// #include "log_config.h"
// 定义您的机器人需要的编码器数量
#define NUM_ROBOT_ENCODERS 2 // 两个轮子
// 存储编码器实例数据的数组
// 确保这个数组在中断服务程序中可以访问到
encoder_instance_t robot_encoders[NUM_ROBOT_ENCODERS];
// 编码器管理器实例
encoder_manager_t robot_encoder_manager = {
    .encoders = robot_encoders,
    .num_encoders = NUM_ROBOT_ENCODERS,
    .gpio_read_func = NULL,         // 在 init 中设置
    .attach_interrupt_func = NULL,  // 在 init 中设置
    .enter_critical_func = NULL,    // 在 init 中设置
    .exit_critical_func = NULL,     // 在 init 中设置
};
// 您需要一种方法来根据触发中断的引脚找到对应的 encoder_instance_t 指针
// 由于您所有编码器引脚都在 GROUP1 中断分组且在同一个 GPIO 端口 (GPIOB)，
// 我们可以使用一个简单的映射数组，索引可以是您定义的引脚句柄或者 Sysconfig 生成的 IIDX
// 用于存储 encoder_instance_t 指针的数组，供中断服务程序使用
// 为每个编码器引脚（A相和B相）预留空间 (2个编码器 * 2个引脚 = 4个引脚)
// 使用 Sysconfig 生成的 IIDX 作为索引的一部分，可以简化映射
// MSPM0 的 GPIO IIDX 从 DIO0 开始
#define MAX_GPIO_IIDX_IN_USE (DL_GPIO_IIDX_DIO7 + 1) // 最大 IIDX + 1
static encoder_instance_t* interrupt_iidx_to_encoder_instance[MAX_GPIO_IIDX_IN_USE];
// 您的 MSPM0 GPIO 读取函数封装
uint8_t mspm0_gpio_read(void *gpio_handle, uint32_t pin_mask) {
    GPIO_Regs* gpio_regs = (GPIO_Regs*)gpio_handle;
    return DL_GPIO_readPins(gpio_regs, pin_mask) ? 1 : 0;
}
// 您的 MSPM0 中断挂载函数
bool mspm0_attach_interrupt(void *pin_handle, void (*isr_handler)(void *arg), void *arg) {
    // pin_handle 是您在 encoder_config_t 中提供的 pin_handle (例如 Sysconfig 生成的 IIDX)
    // arg 是 encoder_instance_t*
    // 在这里实现 MSPM0 的 GPIO 外部中断配置和挂载
    // 1. 根据 pin_handle (IIDX) 将 arg 存储到 interrupt_iidx_to_encoder_instance 数组中
    // 注意：GPIO 引脚的配置 (输入、上拉、双边沿中断) 应该已经在 Sysconfig 中完成
    // 您只需要在这里存储中断触发和 encoder_update 调用之间的关联
    uint8_t map_index = (uint8_t)(uintptr_t)pin_handle; // 使用 IIDX 作为索引
    if (map_index < MAX_GPIO_IIDX_IN_USE) {
        interrupt_iidx_to_encoder_instance[map_index] = (encoder_instance_t*)arg;
        // 在 Sysconfig 中已经完成了中断使能和配置，这里只需要存储映射关系
        return true; // 假设存储映射关系成功
    } else {
        log_e("Invalid pin handle index (IIDX) for interrupt attachment: %u", map_index);
        return false; // 配置失败
    }
}
// 您的 MSPM0 GPIO 中断服务程序
// 这个 ISR 需要检查哪些引脚触发了中断，并找到对应的编码器实例
void GROUP1_IRQHandler(void)
{
    // 检查是否是 GPIOB 的中断
    if (DL_Interrupt_getStatusGroup(DL_INTERRUPT_GROUP_1, ENCODER_INT_IIDX)) {
        // 检查并处理右轮 A 相中断
        if (DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_M1_PIN)) {
             DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_M1_PIN); // 清除对应引脚的中断标志
             uint8_t map_index = (uint8_t)(uintptr_t)ENCODER_M1_IIDX;
             if (map_index < MAX_GPIO_IIDX_IN_USE && interrupt_iidx_to_encoder_instance[map_index] != NULL) {
                encoder_update(interrupt_iidx_to_encoder_instance[map_index]);
             }
        }
        // 检查并处理右轮 B 相中断
        if (DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_M2_PIN)) {
             DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_M2_PIN);
             uint8_t map_index = (uint8_t)(uintptr_t)ENCODER_M2_IIDX;
             if (map_index < MAX_GPIO_IIDX_IN_USE && interrupt_iidx_to_encoder_instance[map_index] != NULL) {
                encoder_update(interrupt_iidx_to_encoder_instance[map_index]);
             }
        }
        // 检查并处理左轮 A 相中断
        if (DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_M3_PIN)) {
             DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_M3_PIN);
             uint8_t map_index = (uint8_t)(uintptr_t)ENCODER_M3_IIDX;
             if (map_index < MAX_GPIO_IIDX_IN_USE && interrupt_iidx_to_encoder_instance[map_index] != NULL) {
                encoder_update(interrupt_iidx_to_encoder_instance[map_index]);
             }
        }
        // 检查并处理左轮 B 相中断
        if (DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_M4_PIN)) {
             DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_M4_PIN);
             uint8_t map_index = (uint8_t)(uintptr_t)ENCODER_M4_IIDX;
             if (map_index < MAX_GPIO_IIDX_IN_USE && interrupt_iidx_to_encoder_instance[map_index] != NULL) {
                encoder_update(interrupt_iidx_to_encoder_instance[map_index]);
             }
        }
    }
}

// 封装 FreeRTOS taskENTER_CRITICAL 宏的函数
void freertos_enter_critical(void) {
    taskENTER_CRITICAL();
}
// 封装 FreeRTOS taskEXIT_CRITICAL 宏的函数
void freertos_exit_critical(void) {
    taskEXIT_CRITICAL();
}

// 在您的系统初始化函数中 (例如 application_init)
void encoder_application_init(void) {
    // 1. 初始化 MSPM0 外设 (时钟、GPIO 等)
    // 2. 配置编码器引脚为输入，使能上拉，并配置为双边沿触发外部中断
    //    这个步骤应该已经在 Sysconfig 中完成
    //    确保您在 Sysconfig 中为 M1-M4 的所有 4 个引脚配置了正确的 GPIO 端口、引脚、模式和中断触发类型 (CHANGE)
    // 3. 定义编码器配置数组
    encoder_config_t encoder_configs[NUM_ROBOT_ENCODERS] = {
        // Encoder 0 (右轮)
        {
            .pin1_gpio_handle = GPIOB, .pin1_bitmask = ENCODER_M2_PIN, // 右轮 A 相
            .pin1_handle = (void*)(uintptr_t)ENCODER_M2_IIDX, // 使用 IIDX 作为 pin_handle
            .pin2_gpio_handle = GPIOB, .pin2_bitmask = ENCODER_M1_PIN, // 右轮 B 相 (假设)
            .pin2_handle = (void*)(uintptr_t)ENCODER_M1_IIDX  // 使用 IIDX 作为 pin_handle
        },
        // Encoder 1 (左轮)
        {
            .pin1_gpio_handle = GPIOB, .pin1_bitmask = ENCODER_M3_PIN, // 左轮 A 相 (假设)
            .pin1_handle = (void*)(uintptr_t)ENCODER_M3_IIDX,
            .pin2_gpio_handle = GPIOB, .pin2_bitmask = ENCODER_M4_PIN, // 左轮 B 相 (假设)
            .pin2_handle = (void*)(uintptr_t)ENCODER_M4_IIDX
        },
    };
    // 4. 初始化编码器管理器
    encoder_manager_init(
        &robot_encoder_manager,
        encoder_configs, NUM_ROBOT_ENCODERS,
        mspm0_gpio_read,         // 您的 GPIO 读取函数
        mspm0_attach_interrupt,  // 您的中断挂载函数 (用于建立映射关系)
        freertos_enter_critical,      // FreeRTOS 临界区函数
        freertos_exit_critical        // FreeRTOS 临界区函数
    );
    // 5. 启用相关的 GPIO 中断向量
    NVIC_EnableIRQ(ENCODER_INT_IRQN);
}