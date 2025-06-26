#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"


static uint8_t gray_datas[12];

void gd_task(void *arg) {
		gray_detection_init();
    for ( ; ; )  {
				gray_read_data();
				uint16_t temp_data = pca9555_read_bit12(&pca9555_i2c, PCA9555_ADDR);
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


void gd_task_create(void) {
    xTaskCreate(gd_task, "gd_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
}
