#include "openmv.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "stm32f1xx_it.h"
#include "ServoPid.h"
#include "main.h"
#include "usart.h"
#include "uart_dma.h"

float err_x = 0;
float err_y = 0;

void openmv_init(void) {
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}

void parse_xy(char *json_data, float *x, float *y) {
    cJSON *json = cJSON_Parse(json_data);
    if (json == NULL) return;
    cJSON *x_item, *y_item;
		x_item = cJSON_GetObjectItem(json, "x");
		y_item = cJSON_GetObjectItem(json, "y");
    if (cJSON_IsNumber(x_item)) *x = x_item->valueint;
    if (cJSON_IsNumber(y_item)) *y = y_item->valueint;
    cJSON_Delete(json);
}

void parse_openmv_data(void) {
	parse_xy((char *)uart_rx_buffer, &err_x, &err_y);
}

void process_received_data(void) {
    if (uart_data_ready) {
        uart_data_ready = 0;
        parse_openmv_data();
    }
}