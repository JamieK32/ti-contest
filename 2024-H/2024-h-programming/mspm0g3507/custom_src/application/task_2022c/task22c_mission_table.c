#include "common_defines.h"
#if (CURRENT_TASK_TYPE == TASK_TYPE_22C)
#include "common_include.h"
#include "periodic_event_task.h"
#include "task24h_config.h"

static void debug_task(void) {
    car_debug_tick();
}

period_task_t task_table[] = {
   { EVENT_KEY_STATE_UPDATE,  RUN,  button_ticks,         20,  0 },    // 20ms
   { EVENT_MENU_VAR_UPDATE,   RUN,  oled_menu_tick,       20,  0 },    // 20ms
   { EVENT_PERIOD_PRINT,      IDLE, debug_task,           500, 0 },    // 500ms
   { EVENT_ALERT,             RUN,  alert_ticks,          10,  0 },    // 10ms
   { EVENT_CAR_STATE_MACHINE, IDLE, car_state_machine,    20,  0 },    // 20ms
   { EVENT_CAR,               RUN,  car_task,             20,  0 },    // 20ms
	 { EVENT_MUSIC_PLAYER,      RUN,  music_player_update,  5,   0 }, 	 // 5ms
	 { EVENT_TOF, 							RUN, 	VL53L1_Process,       5,   0 }, 	 // 5ms
};

void init_task_table(void) {
	init_task_scheduler(task_table, sizeof(task_table) / sizeof(task_table[0]));
}

#endif 
