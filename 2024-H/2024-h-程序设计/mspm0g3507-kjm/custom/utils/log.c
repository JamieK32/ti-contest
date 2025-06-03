#include "log.h"

static void log_print(int level, const char *format, va_list args) {
    if (level <= MODULE_LOG_LEVEL) {
        const char *level_tag = "";
        switch (level) {
            case LOG_LEVEL_INFO:
                level_tag = LOG_TAG_INFO;
                break;
            case LOG_LEVEL_WARN:
                level_tag = LOG_TAG_WARN;
                break;
            case LOG_LEVEL_ERROR:
                level_tag = LOG_TAG_ERROR;
                break;
            default:
                level_tag = "[UNKNOWN]";
                break;
        }

        char buffer[MAX_LOG_SIZE];
        vsnprintf(buffer, sizeof(buffer), format, args);
        debug_uart_printf("%s %s\r\n", level_tag, buffer);
    }
}

void LOG_I(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void LOG_W(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_LEVEL_WARN, format, args);
    va_end(args);
}

void LOG_E(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}

void LOG_HEX(const unsigned char *data, int len) {
    if (LOG_LEVEL_INFO <= MODULE_LOG_LEVEL) {
        debug_uart_printf("%s HEX Data (%d bytes):\r\n", LOG_TAG_INFO, len);
        for (int i = 0; i < len; i++) {
            debug_uart_printf("%02X ", data[i]);
            if ((i + 1) % 16 == 0) {
                debug_uart_printf("\r\n");
            }
        }
        if (len % 16 != 0) {
            debug_uart_printf("\r\n");
        }
    }
}