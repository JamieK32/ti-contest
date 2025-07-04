#include "tests.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"



void wit_test(void) {
		jy901s.init();
		jy901s.reset();
    for ( ; ; )  {
				log_i("wit: roll = %.2lf, yaw = %.2lf, pitch = %.2lf", jy901s.roll, jy901s.yaw, jy901s.pitch);
				delay_ms(200);
    }
}


