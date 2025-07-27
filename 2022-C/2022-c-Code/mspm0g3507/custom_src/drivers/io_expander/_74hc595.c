#include "_74hc595.h"
#include "delay.h" // 确保包含了延迟函数库（平台相关）

void hc595_delay(uint32_t us) {
//	delay_us(us);
}

void hc595_output_byte(uint8_t byte_data) {
    for (int bit = 7; bit >= 0; bit--) { // 从MSB开始
        // 提取当前位
        if (byte_data & (1 << bit)) {
            DL_GPIO_setPins(PORTA_PORT, PORTA_HC595_DS_PIN); // 输出逻辑 1
        } else {
            DL_GPIO_clearPins(PORTA_PORT, PORTA_HC595_DS_PIN); // 输出逻辑 0
        }
        hc595_delay(1); // setup time
        
        // 时钟脉冲
        DL_GPIO_setPins(PORTA_PORT, PORTA_HC595_SHCP_PIN); // 时钟上升沿
        hc595_delay(1);
        DL_GPIO_clearPins(PORTA_PORT, PORTA_HC595_SHCP_PIN); // 时钟下降沿
        hc595_delay(1); // hold time
    }
    
    // 锁存阶段
    DL_GPIO_setPins(PORTA_PORT, PORTA_HC595_STCP_PIN); // 锁存上升沿
    hc595_delay(1);
    DL_GPIO_clearPins(PORTA_PORT, PORTA_HC595_STCP_PIN); // 锁存下降沿
}
