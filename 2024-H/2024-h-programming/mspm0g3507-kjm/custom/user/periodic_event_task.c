#include "periodic_event_task.h"
#include "common_include.h"
//#include "log_config.h"
#include "log.h"
#include "systick.h"  // 包含get_ms()

void debug_task(void) {
    debug_speed_pid();
}


// 定义周期性任务数组
period_task_t period_tasks[] = {
    { EVENT_KEY_STATE_UPDATE,  RUN,  button_ticks,         20,  0 },    // 20ms
    { EVENT_MENU_VAR_UPDATE,   RUN,  oled_menu_tick,       20,  0 },    // 20ms
    { EVENT_PERIOD_PRINT,      IDLE, debug_task,           500, 0 },    // 500ms
    { EVENT_ALERT,             RUN,  alert_ticks,          10,  0 },    // 10ms
    { EVENT_CAR_STATE_MACHINE, IDLE, car_state_machine,    20,  0 },    // 20ms
    { EVENT_CAR,               RUN,  car_task,             20,  0 },    // 20ms
		{ EVENT_MUSIC_PLAYER,      RUN,  music_player_update,  5,   0 }, 		// 5ms
		{ EVENT_WIT_IMU,					 RUN,  VL53L1_Process, 			 10,   0 }, 	  // 2ms
};

// 获取任务数组大小
#define PERIOD_TASKS_COUNT (sizeof(period_tasks) / sizeof(period_tasks[0]))

/**
 * @brief 初始化周期性任务调度器
 */
void create_periodic_event_task(void) {
    uint32_t current_time = get_ms();
    
    // 初始化所有任务的上次执行时间
    for (int i = 0; i < PERIOD_TASKS_COUNT; i++) {
        period_tasks[i].last_run_time_ms = current_time;
    }
    
    log_i("Periodic task scheduler initialized");
}

/**
 * @brief 周期性任务调度处理函数
 * @note 在主循环中调用
 */
void periodic_event_task_process(void) {
    uint32_t current_time = get_ms();
    
    // 遍历所有任务
    for (int i = 0; i < PERIOD_TASKS_COUNT; i++) {
        period_task_t *task = &period_tasks[i];
        
        // 检查任务是否需要执行
        if (task->is_running == RUN && task->task_handler != NULL) {
            // 计算距离上次执行的时间
            uint32_t time_since_last_run = current_time - task->last_run_time_ms;
            
            // 检查是否到了执行时间
            if (time_since_last_run >= task->period_ms) {
                // 执行任务
                task->task_handler();
                
                // 更新上次执行时间
                task->last_run_time_ms = current_time;
            }
        }
    }
}

/**
 * @brief 启用指定的周期性任务
 */
void enable_periodic_task(EVENT_IDS event_id) {
    for (int i = 0; i < PERIOD_TASKS_COUNT; i++) {
        if (period_tasks[i].id == event_id) {
            period_tasks[i].is_running = RUN;
            period_tasks[i].last_run_time_ms = get_ms();  // 重置执行时间
            log_i("Task %d enabled", event_id);
            break;
        }
    }
}

/**
 * @brief 禁用指定的周期性任务
 */
void disable_periodic_task(EVENT_IDS event_id) {
    for (int i = 0; i < PERIOD_TASKS_COUNT; i++) {
        if (period_tasks[i].id == event_id) {
            period_tasks[i].is_running = IDLE;
            log_i("Task %d disabled", event_id);
            break;
        }
    }
}