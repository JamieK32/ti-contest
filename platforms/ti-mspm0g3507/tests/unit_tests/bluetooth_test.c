#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"
#include "bluetooth.h"

void bluetooth_test(void) {
    if (bluetooth_init() != BLUETOOTH_OK) {
        while (1);
    }
		bluetooth_send_byte(0x13);
		while (1) {
		
		}
}