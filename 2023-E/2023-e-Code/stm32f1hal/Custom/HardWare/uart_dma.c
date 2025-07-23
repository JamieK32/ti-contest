#include "uart_dma.h"
#include "stm32f1xx_hal.h"            // Device header
#include "usart.h"

#define DMA_BUFFER_SIZE 256

uint8_t dma_rx_buffer[DMA_BUFFER_SIZE];
uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_data_ready = 0;
volatile uint16_t old_pos = 0;

// 初始化
void UART_DMA_Init(void)
{
    HAL_UART_Receive_DMA(&huart1, dma_rx_buffer, DMA_BUFFER_SIZE);
}

// 主循环中调用（推荐每1-5ms调用一次）
void UART_Process_DMA_Data(void)
{
    static uint16_t uart_rx_index = 0;
    uint16_t current_pos;
    
    // 获取DMA当前写入位置
    current_pos = DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
    
    if (current_pos != old_pos)
    {
        uint16_t length;
        
        if (current_pos > old_pos)
        {
            // 正常情况：没有回绕
            length = current_pos - old_pos;
            for (uint16_t i = 0; i < length; i++)
            {
                Process_Received_Byte(dma_rx_buffer[old_pos + i], &uart_rx_index);
            }
        }
        else
        {
            // 缓冲区回绕情况
            // 先处理末尾数据
            length = DMA_BUFFER_SIZE - old_pos;
            for (uint16_t i = 0; i < length; i++)
            {
                Process_Received_Byte(dma_rx_buffer[old_pos + i], &uart_rx_index);
            }
            // 再处理开头数据
            for (uint16_t i = 0; i < current_pos; i++)
            {
                Process_Received_Byte(dma_rx_buffer[i], &uart_rx_index);
            }
        }
        
        old_pos = current_pos;
    }
}

void Process_Received_Byte(uint8_t byte, uint16_t *index)
{
    if (*index < (UART_BUFFER_SIZE - 1))
    {
        uart_rx_buffer[(*index)++] = byte;
        
        // 检测帧结束
        if (byte == '}')
        {
            if (uart_rx_buffer[0] == '{')
            {
                uart_rx_buffer[*index] = '\0';
                uart_data_ready = 1;
            }
            *index = 0;  // 重置索引
        }
    }
    else
    {
        *index = 0;  // 缓冲区溢出，重置
    }
}
