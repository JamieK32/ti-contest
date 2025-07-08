#ifndef PERIODIC_EVENT_TASK_H
#define PERIODIC_EVENT_TASK_H

#include <stdint.h>

// 定义事件 ID 枚举
typedef enum {
    EVENT_NONE = 0,
    EVENT_IMU_UPDATE,
    EVENT_TEMP_CONTROL,
    EVENT_KEY_STATE_UPDATE,
    EVENT_MENU_VAR_UPDATE,
    EVENT_PERIOD_PRINT,
    EVENT_ALERT,
    EVENT_CAR,
    EVENT_CAR_STATE_MACHINE,
		EVENT_MUSIC_PLAYER,
    NUM_PERIOD_TASKS
} EVENT_IDS;

// 定义运行状态枚举
typedef enum {
    RUN,
    IDLE,
} TASK_STATE;

// 简化的任务结构体
typedef struct {
    EVENT_IDS id;
    TASK_STATE is_running;
    void (*task_handler)(void);
    uint32_t period_ms;
    uint32_t last_run_time_ms;      // 上次执行时间
} period_task_t;

// 基础API函数
void create_periodic_event_task(void);
void periodic_event_task_process(void);
void enable_periodic_task(EVENT_IDS event_id);
void disable_periodic_task(EVENT_IDS event_id);

#endif // PERIODIC_EVENT_TASK_H