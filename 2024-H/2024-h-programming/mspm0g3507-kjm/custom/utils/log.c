#include "log.h"
#include "common_defines.h"
#include "uart_debug.h"

static void log_print(int level, const char *format, va_list args) {
		#if DEBUG_MODE
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
		#endif
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