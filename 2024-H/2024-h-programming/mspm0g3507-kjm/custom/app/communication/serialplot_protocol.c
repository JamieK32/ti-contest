#include "serialplot_protocol.h"
#include "log_config.h"
#include "log.h"

void serialplot_send_multi_data(size_t num_channels, ...) {
    va_list args;
    va_start(args, num_channels);

    char buffer[MAX_TX_BUFFER_SIZE];
    int offset = 0;

    for (size_t i = 0; i < num_channels; ++i) {
        // 获取浮点数值
        float value = (float)va_arg(args, double); // va_arg for float promotes to double

        // 格式化单个数据点并写入缓冲区
        // 格式: <float_value>
        // 如果不是第一个数据，前面添加逗号
        int written;
        if (i == 0) {
            written = snprintf(buffer + offset, sizeof(buffer) - offset, "%.*f",
                               SERIALPLOT_FLOAT_PRECISION, value);
        } else {
            written = snprintf(buffer + offset, sizeof(buffer) - offset, ",%.*f",
                               SERIALPLOT_FLOAT_PRECISION, value);
        }


        if (written < 0 || written >= (int)(sizeof(buffer) - offset)) {
            // 格式化错误或缓冲区溢出，可以考虑错误处理
            // 例如，发送一个错误信息或者直接返回
            log_e("Serialplot buffer overflow or formatting error!\n"); // 使用log宏进行错误提示
            offset = sizeof(buffer); // 标记缓冲区已满，停止写入
            break;
        }
        offset += written;
    }

    va_end(args);

    // 在所有数据点之后发送换行符
    if (offset < sizeof(buffer)) {
        buffer[offset++] = '\n';
    } else {
        // 如果缓冲区满了，但没有空间放换行符，这是一个问题
        // 在 snprintf 循环中增加检查，避免这种情况
        log_e("Serialplot buffer full, no space for newline!\n"); // 使用log宏进行错误提示
    }
    debug_uart_send_bytes((uint8_t*)buffer, offset);
}
