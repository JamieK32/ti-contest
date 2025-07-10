#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"



void wit_test(void) {
		wit_imu_init();
		wit_imu_set_yaw_zero();
    for ( ; ; )  {
				log_i("wit: roll = %.2lf, yaw = %.2lf, pitch = %.2lf", jy61p.roll, jy61p.yaw, jy61p.pitch);
				delay_ms(200);
    }
}


