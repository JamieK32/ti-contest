#include "openmv.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "stm32f1xx_it.h"

#define printf(fmt, ...) do {} while(0) //关闭调试功能

CAM_STATE cam_state = CAM_STATE_NONE;

// 保存解析出的变量
float x_value = 0; //当前激光点的X坐标
float y_value = 0; //当前激光点的y坐标
float center_point_x = 0;
float center_point_y = 0;
int points_array[MAX_POINTS][DIMENSIONS] = {0}; //方框路径坐标
int num_points = 0;	//方框路径数量

void parse_xy(const char *json_data, float *x, float *y) {
		cJSON *json = cJSON_Parse(json_data);  // data 是接收到的 JSON 数据字符串
		if (json == NULL)
		{
				return;
		}

		// 在 JSON 对象中提取 "x" 和 "y" 字段
		cJSON *x_item = cJSON_GetObjectItem(json, "x");  // 解析 "x"
		cJSON *y_item = cJSON_GetObjectItem(json, "y");  // 解析 "y"

		if (cJSON_IsNumber(x_item))
		{
				*x = x_item->valueint;  // 如果解析为数字，存储值到变量中
		}

		if (cJSON_IsNumber(y_item))
		{
				*y = y_item->valueint;  // 如果解析为数字，存储值到变量中
		}

		// 释放 JSON 对象的内存
		cJSON_Delete(json);
}

void parse_path_points(const char *json_data, int points_array[][DIMENSIONS], int *num_points) {
    /**
     * 解析 JSON 数据中的 path_points，并存储到二维数组 points_array。
     * 
     * 参数:
     *  - json_data: JSON 数据字符串。
     *  - points_array: 保存解析结果的二维数组。
     *  - num_points: 保存解析出来的点数。
     */
    // 初始化点数为 0
    *num_points = 0;
    // 1. 使用 cJSON 解析 JSON 字符串
    cJSON *json = cJSON_Parse(json_data);
    if (json == NULL) {
        printf("Error parsing JSON data!\n");
        return;
    }
    // 2. 提取 "path_points" 数组
    cJSON *path_points = cJSON_GetObjectItem(json, "path_points");
    if (!cJSON_IsArray(path_points)) {
        printf("Error: path_points is not a valid array!\n");
        cJSON_Delete(json);
        return;
    }
    // 3. 遍历 "path_points" 数组
    int path_count = cJSON_GetArraySize(path_points);
    if (path_count > MAX_POINTS) {
        printf("Warning: path_points contains more points than supported (%d > %d)!\n", path_count, MAX_POINTS);
        path_count = MAX_POINTS;  // 限制为最大点数
    }
    for (int i = 0; i < path_count; i++) {
        // 获取数组中的每个坐标点（它们本身是数组）
        cJSON *point = cJSON_GetArrayItem(path_points, i);
        if (cJSON_IsArray(point) && cJSON_GetArraySize(point) == 2) {
            int x = cJSON_GetArrayItem(point, 0)->valueint;  // 提取第一个元素 (x 坐标)
            int y = cJSON_GetArrayItem(point, 1)->valueint;  // 提取第二个元素 (y 坐标)
            // 将提取到的点存储到 points_array
            points_array[*num_points][0] = x;
            points_array[*num_points][1] = y;
            (*num_points)++;  // 增加点计数
        } else {
            printf("Invalid or incomplete point at index %d, skipping it.\n", i);
        }
    }
    // 4. 释放 JSON 对象占用的内存
    cJSON_Delete(json);
}

// 解析函数
void parse_openmv_data(const char *data)
{
		if (cam_state == CAM_STATE_NONE) return;
		
		switch((uint8_t)cam_state) {
			case RECEIVE_XY: 
					parse_xy(data, &x_value, &y_value);
					break;
			case RECEIVE_PATH_POINTS: 
					parse_path_points(data, points_array, &num_points);
					break;
			case RECEIVE_CENTER_POINT:
					parse_xy(data, &center_point_x, &center_point_y);
					break;
	}
}

void process_received_data(void)
{
    // 检查是否接收到了完整 JSON 数据
    if (uart_data_ready)
    {
        uart_data_ready = 0;  // 清除标志位
        parse_openmv_data((const char *)uart_rx_buffer);
    }
}
