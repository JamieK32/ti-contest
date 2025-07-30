#include "Emm_V5_Receive.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"

#define printf(fmt, ...) do {} while (0)

// 接收缓冲区大小增加
#define RX_BUFFER_SIZE 64
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_index = 0;
static volatile uint8_t frame_ready = 0;
static uint8_t motor_reached_flag[2] = {0, 0};  // 到位标志
// 电机状态存储
static int32_t motor_position[2] = {0, 0};
static int16_t motor_speed[2] = {0, 0};
static uint8_t motor_status[2] = {0, 0};

// 调试计数器
static uint32_t debug_frame_count = 0;
static uint32_t debug_parse_count = 0;
static uint32_t debug_error_count = 0;

void ParseSingleFrame(uint8_t* data, uint8_t len);

// 串口中断处理 - 改进的数据接收
void USART2_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE))
    {
        uint8_t data = (uint8_t)(huart2.Instance->DR & 0xFF);
        
        // 检测帧头 - 如果收到地址字节（1或2），且前面有数据，可能是新帧开始
        if((data == 1 || data == 2) && rx_index > 0)
        {
            // 检查当前缓冲区是否是完整帧（以0x6B结尾）
            if(rx_buffer[rx_index - 1] == 0x6B)
            {
                frame_ready = 1; // 标记当前帧完成
                debug_frame_count++;
                // 开始新帧
                rx_index = 0;
            }
        }
        
        // 存储数据
        if(rx_index < RX_BUFFER_SIZE - 1)
        {
            rx_buffer[rx_index] = data;
            rx_index++;
            
            // 检测到帧尾0x6B
            if(data == 0x6B && rx_index >= 4) // 最小帧长度为4
            {
                frame_ready = 1;
                debug_frame_count++;
            }
        }
        else
        {
            // 缓冲区满，重置
            rx_index = 0;
            debug_error_count++;
        }
    }
    
    // 清除错误标志
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE))
    {
        __HAL_UART_CLEAR_OREFLAG(&huart2);
        debug_error_count++;
    }
    
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_NE))
    {
        __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_NE);
        debug_error_count++;
    }
    
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_FE))
    {
        __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_FE);
        debug_error_count++;
    }
}

// 打印调试信息
void PrintDebugData(uint8_t* data, uint8_t len)
{
    printf("RX[%d]: ", len);
    for(int i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}

// 改进的帧解析 - 支持多帧分离
void ParseMultipleFrames(uint8_t* data, uint8_t len)
{
    uint8_t i = 0;
    
    while(i < len)
    {
        // 查找帧头（地址1或2）
        while(i < len && data[i] != 1 && data[i] != 2)
        {
            i++;
        }
        
        if(i >= len) break;
        
        // 查找帧尾0x6B
        uint8_t frame_start = i;
        uint8_t frame_end = frame_start;
        
        for(uint8_t j = frame_start + 1; j < len; j++)
        {
            if(data[j] == 0x6B)
            {
                frame_end = j;
                break;
            }
        }
        
        // 如果找到完整帧
        if(frame_end > frame_start)
        {
            uint8_t frame_len = frame_end - frame_start + 1;
            ParseSingleFrame(&data[frame_start], frame_len);
            i = frame_end + 1;
        }
        else
        {
            break; // 没有找到完整帧
        }
    }
}

// 解析单个响应帧
void ParseSingleFrame(uint8_t* data, uint8_t len)
{
    if(len < 4) 
    {
        printf("Frame too short: %d\r\n", len);
        return;
    }
    
    // 打印接收到的数据用于调试
    PrintDebugData(data, len);
    
    uint8_t addr = data[0];
    uint8_t func = data[1];
    
    printf("Addr: %d, Func: 0x%02X\r\n", addr, func);
    
    // 地址检查
    if(addr < 1 || addr > 2) 
    {
        printf("Invalid addr: %d\r\n", addr);
        return;
    }
    
    uint8_t motor_id = addr - 1; // 转换为数组索引
    debug_parse_count++;
    
    // 检查是否是错误响应
    if(func == 0x00)
    {
        printf("Error response from motor %d\r\n", addr);
        debug_error_count++;
        return;
    }
    
    switch(func)
    {
        case 0x36: // 读取实时位置
            if(len == 7)
            {
                uint8_t sign = data[2];
                int32_t pos = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6];
                if(sign == 0x01) pos = -pos;
                motor_position[motor_id] = pos;
                printf("Motor %d position: %d\r\n", addr, pos);
            }
            else
            {
                printf("Position frame length error: %d\r\n", len);
            }
            break;
            
        case 0x35: // 读取实时转速  
            if(len == 5)
            {
                uint8_t sign = data[2];
                int16_t speed = (data[3] << 8) | data[4];
                if(sign == 0x01) speed = -speed;
                motor_speed[motor_id] = speed;
                printf("Motor %d speed: %d RPM\r\n", addr, speed);
            }
            else
            {
                printf("Speed frame length error: %d\r\n", len);
            }
            break;
            
        case 0x3A: // 读取状态标志
            if(len == 4)
            {
                motor_status[motor_id] = data[2];
                printf("Motor %d status: 0x%02X\r\n", addr, data[2]);
                printf("  Enabled: %s\r\n", (data[2] & 0x01) ? "Yes" : "No");
                printf("  In Position: %s\r\n", (data[2] & 0x02) ? "Yes" : "No");
                printf("  Stalled: %s\r\n", (data[2] & 0x04) ? "Yes" : "No");
            }
            else
            {
                printf("Status frame length error: %d\r\n", len);
            }
            break;
            
        // 处理命令确认回复
        case 0xF3: // 使能控制回复
            if(len == 4)
            {
                printf("Enable cmd response: 0x%02X\r\n", data[2]);
                if(data[2] == 0x02)
                {
                    printf("Enable command successful\r\n");
                }
                else if(data[2] == 0xE2)
                {
                    printf("Enable command failed - conditions not met\r\n");
                }
            }
            break;
            
        case 0xF6: // 速度模式回复
            if(len == 4)
            {
                printf("Velocity cmd response: 0x%02X\r\n", data[2]);
                if(data[2] == 0x02)
                {
                    printf("Velocity command successful\r\n");
                }
                else if(data[2] == 0xE2)
                {
                    printf("Velocity command failed - conditions not met\r\n");
                }
            }
            break;
            
        case 0xFD: // 位置模式回复
            if(len == 4)
            {
                printf("Position cmd response: 0x%02X\r\n", data[2]);
                if(data[2] == 0x02)
                {
                    printf("Position command successful\r\n");
                }
                else if(data[2] == 0xE2)
                {
                    printf("Position command failed - conditions not met\r\n");
                }
                else if(data[2] == 0x9F)
                {
                    printf("Position reached\r\n");
                }
            }
            break;
            
        case 0xFE: // 立即停止回复
            if(len == 4)
            {
                printf("Stop cmd response: 0x%02X\r\n", data[2]);
                if(data[2] == 0x02)
                {
                    printf("Stop command successful\r\n");
                }
            }
            break;
            
        default:
            printf("Unknown function code: 0x%02X\r\n", func);
            break;
    }
}

// 主循环中调用此函数处理接收数据
void ProcessEmmReceive(void)
{
    if(frame_ready)
    {
        // 关中断，快速复制数据
        __disable_irq();
        uint8_t temp_buffer[RX_BUFFER_SIZE];
        uint8_t temp_len = rx_index;
        memcpy(temp_buffer, rx_buffer, temp_len);
        
        // 重置接收状态
        rx_index = 0;
        frame_ready = 0;
        __enable_irq();
        
        // 解析数据 - 支持多帧
        ParseMultipleFrames(temp_buffer, temp_len);
    }
}

// 改进的初始化
void InitEmmReceive(void)
{
    // 关闭接收中断
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
    
    rx_index = 0;
    frame_ready = 0;
    debug_frame_count = 0;
    debug_parse_count = 0;
    debug_error_count = 0;
    
    // 清空状态数据
    memset(motor_position, 0, sizeof(motor_position));
    memset(motor_speed, 0, sizeof(motor_speed));
    memset(motor_status, 0, sizeof(motor_status));
    
    // 清空接收缓冲区
    memset(rx_buffer, 0, sizeof(rx_buffer));
    

    
    // 使能串口接收中断
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    
    printf("EMM Receive initialized (improved)\r\n");
}

// 其他函数保持不变...
int32_t GetMotorPosition(uint8_t motor_id)
{
    if(motor_id >= 1 && motor_id <= 2)
    {
        return motor_position[motor_id - 1];
    }
    return 0;
}

int16_t GetMotorSpeed(uint8_t motor_id)
{
    if(motor_id >= 1 && motor_id <= 2)
    {
        return motor_speed[motor_id - 1];
    }
    return 0;
}

uint8_t GetMotorStatus(uint8_t motor_id)
{
    if(motor_id >= 1 && motor_id <= 2)
    {
        return motor_status[motor_id - 1];
    }
    return 0;
}

uint8_t IsMotorEnabled(uint8_t motor_id)
{
    uint8_t status = GetMotorStatus(motor_id);
    return (status & 0x01) ? 1 : 0;
}

uint8_t IsMotorInPosition(uint8_t motor_id)
{
    uint8_t status = GetMotorStatus(motor_id);
    return (status & 0x02) ? 1 : 0;
}

