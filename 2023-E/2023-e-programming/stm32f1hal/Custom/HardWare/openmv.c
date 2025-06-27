#include "openmv.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"

// 保存解析出的变量
float x_value = 0;
float y_value = 0;

/*
{
"x" : 20,
"y" : 15
}
*/

// 解析函数
void parse_openmv_data(const char *data)
{
    // 将 JSON 字符串解析为 JSON 对象
    cJSON *json = cJSON_Parse(data);  // data 是接收到的 JSON 数据字符串
    if (json == NULL)
    {
        return;
    }

    // 在 JSON 对象中提取 "x" 和 "y" 字段
    cJSON *x_item = cJSON_GetObjectItem(json, "x");  // 解析 "x"
    cJSON *y_item = cJSON_GetObjectItem(json, "y");  // 解析 "y"

    if (cJSON_IsNumber(x_item))
    {
        x_value = x_item->valueint;  // 如果解析为数字，存储值到变量中
    }

    if (cJSON_IsNumber(y_item))
    {
        y_value = y_item->valueint;  // 如果解析为数字，存储值到变量中
    }

    // 释放 JSON 对象的内存
    cJSON_Delete(json);
}


extern uint8_t uart_rx_buffer[UART_BUFFER_SIZE];    // 接收缓冲区
extern volatile uint8_t uart_data_ready;        // 数据接收完成标志

void process_received_data(void)
{
    // 检查是否接收到了完整 JSON 数据
    if (uart_data_ready)
    {
        uart_data_ready = 0;  // 清除标志位
        parse_openmv_data((const char *)uart_rx_buffer);
    }
}
