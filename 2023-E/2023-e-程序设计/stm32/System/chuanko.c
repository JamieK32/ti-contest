#include "stm32f10x.h"

// 接收到的数据缓冲区
uint8_t rx_buffer[10];
// 接收数据长度
volatile uint16_t rx_length = 0;
// 接收标志
volatile uint8_t rx_flag = 0;

// 串口1初始化函数
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能GPIOA和USART1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 配置USART1的TX和RX引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART1
    USART_InitStructure.USART_BaudRate = 115200; // 波特率115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 使能USART1
    USART_Cmd(USART1, ENABLE);

    // 配置中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// 串口1中断服务函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // 接收到数据
        uint8_t temp = USART_ReceiveData(USART1);
        if (rx_length < sizeof(rx_buffer)) {
            rx_buffer[rx_length++] = temp;
        }
        // 检查是否接收完成（可以根据协议定义的结束符判断）
        if (temp == 0x5B) { // 假设数据以0x5B结束
            rx_flag = 1;
        }
    }
}

// 处理接收到的数据
void processReceivedData(void) {
    if (rx_length >= 7 && rx_flag) {
        // 检查数据格式是否正确
        if (rx_buffer[0] == 0x2C && rx_buffer[6] == 0x5B) {
            // 提取X和Y坐标
            uint8_t x = rx_buffer[2];
            uint8_t y = rx_buffer[3];
            // 打印接收到的坐标
            char log[50];
            sprintf(log, "Received X: %d, Y: %d\r\n", x, y);
            // 通过串口发送日志（可选）
            // sendString(log);
        }
        // 重置接收缓冲区
        rx_length = 0;
        rx_flag = 0;
    }
}


// 辅助函数：发送字符串
void sendString(const char *str) {
    while (*str) {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}