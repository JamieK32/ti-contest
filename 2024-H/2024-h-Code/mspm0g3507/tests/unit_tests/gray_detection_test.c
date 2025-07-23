#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"


static uint8_t gray_datas[12];

void gd_test(void) {
		gray_detection_init();
    for ( ; ; )  {
				uint16_t temp_data = gray_read_byte();
			   for (int i = 0; i < 12; i++) {
					gray_datas[i] = (temp_data >> i) & 0x01; // 提取每一位并存入数组
				}
				log_i("Gray Data Array: %d %d %d %d %d %d %d %d %d %d %d %d",
						gray_datas[0], gray_datas[1], gray_datas[2], gray_datas[3],
						gray_datas[4], gray_datas[5], gray_datas[6], gray_datas[7],
						gray_datas[8], gray_datas[9], gray_datas[10], gray_datas[11]);
				delay_ms(200);
    }
}