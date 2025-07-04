#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"

uint32_t  now_time = 0, lastime = 0;


void mpu_test(void) {
		mpu6050_hardware_init();
		while (mpu_dmp_init()) {}
    for ( ; ; )  {
			float pitch, yaw, roll; 
			mpu_dmp_get_data(&pitch, &roll, &yaw);
			now_time = system_time_get_ms();
			if (now_time - lastime >= 1000) {
				lastime = now_time;
				log_i("pitch = %.2f, roll = %.2f, yaw = %.2f", pitch, roll, yaw);
			}
			delay_ms(10);
    }
}

