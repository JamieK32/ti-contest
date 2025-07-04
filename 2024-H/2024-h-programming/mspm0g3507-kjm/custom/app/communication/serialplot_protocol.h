#ifndef SERIALPLOT_PROTOCOL_H
#define SERIALPLOT_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

// 定义一个宏，用于指定格式化浮点数的小数位数
// 根据你的需求调整
#define SERIALPLOT_FLOAT_PRECISION 2

/**
 * @brief 向 hyOzd 的 serialplot 发送多个数据点（在一行），使用逗号分隔。
 *
 * 这个函数允许你在同一行发送多个数据点，每个数据点之间使用逗号分隔。
 * 所有数据点发送完毕后，会自动发送一个换行符。
 * 数据格式: <value1>,<value2>,...,<valueN>\n
 * Serialplot 将根据逗号解析出多个数据点，并按顺序对应通道。
 *
 * @param num_channels 要发送的数据通道数量。
 * @param ... 可变参数列表，包含 num_channels 个 float (数据值)。
 *            例如: serialplot_send_multi_data(3, 1.23f, 4.56f, 7.89f);
 */
void serialplot_send_multi_data(size_t num_channels, ...);


#endif // SERIALPLOT_PROTOCOL_H
