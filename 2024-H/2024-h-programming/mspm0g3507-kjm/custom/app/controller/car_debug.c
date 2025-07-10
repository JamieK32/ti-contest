#include "car_debug.h"
#include "car_pid.h"
#include "log_config.h"
#include "log.h"
#include "serialplot_protocol.h"
#include "gray_detection_app.h"

#define SPEED_TEST 0
#define MILEAGE_TEST 0
#define ANGLE_TEST 0
#define TRACK_TEST 1

void car_debug_init(void) {
#if SPEED_TEST
	for (int i = 0; i < motor_count; i++) {
			car.target_speed[i] = 45;
	}
#endif
	
#if MILEAGE_TEST
	car.target_mileage_cm = 100;
#endif
	
#if ANGLE_TEST
	car.target_angle = 90;
#endif
}

void debug_speed_pid(void) {
#if SPEED_TEST
	serialplot_send_multi_data(5, speedPid[0].target, 
					speedPid[0].feedback, speedPid[1].feedback, speedPid[2].feedback, speedPid[3].feedback);
#endif
	
#if MILEAGE_TEST
	serialplot_send_multi_data(6, mileagePid.target, mileagePid.feedback, encoder.distance_cm[0], encoder.distance_cm[1], encoder.distance_cm[2], encoder.distance_cm[3]);
#endif 
	
#if TRACK_TEST
	gray_byte = gray_get_position();
	log_i("%d", gray_byte);
#endif 
}

void update_debug_information(void) {
	gray_get_position();
}