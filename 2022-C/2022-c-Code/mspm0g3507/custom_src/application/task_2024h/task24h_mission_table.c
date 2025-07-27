#include "common_defines.h"
#if (CURRENT_TASK_TYPE == TASK_TYPE_24H)
#include "common_include.h"
#include "periodic_event_task.h"

static void debug_task(void) {
    car_debug_tick();
}

#if CURRENT_IMU == MPU6050_GYRO
float yaw, roll, pitch;
void mpu6050_dmp_update(void) {
	mpu_dmp_get_data(&pitch, &roll, &yaw);
}
#endif

period_task_t task_table[] = {
   { EVENT_KEY_STATE_UPDATE,  RUN,  button_ticks,         20,  0 },    // 20ms
   { EVENT_MENU_VAR_UPDATE,   RUN,  oled_menu_tick,       20,  0 },    // 20ms
   { EVENT_PERIOD_PRINT,      IDLE, debug_task,           500, 0 },    // 500ms
   { EVENT_ALERT,             RUN,  alert_ticks,          10,  0 },    // 10ms
   { EVENT_CAR_STATE_MACHINE, IDLE, car_state_machine,    20,  0 },    // 20ms
   { EVENT_CAR,               RUN,  car_task,             20,  0 },    // 20ms
	 { EVENT_MUSIC_PLAYER,      RUN,  music_player_update,  5,   0 }, 		// 5ms
#if CURRENT_IMU == WIT_GYRO
	 { EVENT_IMU_UPDATE,			  RUN,  wit_imu_process, 			 10,   0 }, 	  // 2ms
#elif CURRENT_IMU == MPU6050_GYRO
	 { EVENT_IMU_UPDATE,			  RUN,  mpu6050_dmp_update, 	 10,   0 }, 	  // 2ms
#endif 
};

void init_task_table(void) {
	init_task_scheduler(task_table, sizeof(task_table) / sizeof(task_table[0]));
}
#endif 