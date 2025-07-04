#include "_74hc595.h"
#include "delay.h"
#include "motor_app.h"
#include "encoder_app.h"
#include "tests.h"
#include "log_config.h"
#include "log.h"

void motor_test(void) {
		motor_init();
		encoder_application_init();
		int pwms[4] = {1000, 1000, 1000, 1000};
		motor_set_pwms(pwms);
		int a = 0, b = 0, c = 0, d = 0;
    for ( ; ; )  {
				a = encoder_manager_read_and_reset(&robot_encoder_manager, 0);
				b = encoder_manager_read_and_reset(&robot_encoder_manager, 1);
				c = encoder_manager_read_and_reset(&robot_encoder_manager, 2);
				d = encoder_manager_read_and_reset(&robot_encoder_manager, 3);
				log_i("%d %d %d %d", a, b, c, d);
				delay_ms(200);
    }
}


