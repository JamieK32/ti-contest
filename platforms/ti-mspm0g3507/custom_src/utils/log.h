// log.h
#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include "hal_uart.h"

// 日志级别定义（数值越小，优先级越高）
#define LOG_LEVEL_EMPTY 5
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 1

// 日志标签
#define LOG_TAG_DEBUG "[DEBUG]"
#define LOG_TAG_INFO  "[INFO]"
#define LOG_TAG_WARN  "[WARN]"
#define LOG_TAG_ERROR "[ERROR]"

// 缓冲区大小
#ifndef MAX_LOG_SIZE
#define MAX_LOG_SIZE 256
#endif

#ifndef MODULE_LOG_LEVEL
#define MODULE_LOG_LEVEL LOG_LEVEL_EMPTY
#endif

#ifndef MODULE_LOG_ENABLED
#define MODULE_LOG_ENABLED 0
#endif

// 使用宏控制是否调用日志函数
#if MODULE_LOG_ENABLED
#define log_d(format, ...) LOG_D(format, ##__VA_ARGS__)
#define log_i(format, ...) LOG_I(format, ##__VA_ARGS__)
#define log_w(format, ...) LOG_W(format, ##__VA_ARGS__)
#define log_e(format, ...) LOG_E(format, ##__VA_ARGS__)
#else
#define log_d(format, ...) do {} while (0)
#define log_i(format, ...) do {} while (0)
#define log_w(format, ...) do {} while (0)
#define log_e(format, ...) do {} while (0)
#endif

// 函数声明
void LOG_D(const char *format, ...);
void LOG_I(const char *format, ...);
void LOG_W(const char *format, ...);
void LOG_E(const char *format, ...);

#endif // LOG_H