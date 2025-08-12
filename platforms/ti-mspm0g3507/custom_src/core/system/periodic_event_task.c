#include "periodic_event_task.h"
#include "common_include.h"
//#include "log_config.h"
#include "log.h"
#include "systick.h"  // 包含get_ms()

static period_task_t *period_tasks = NULL;
static uint8_t task_count = 0;

/**
 * @brief 初始化任务调度器
 * @param table 任务数组指针
 * @param count 任务数量
 */
void init_task_scheduler(period_task_t *table, uint8_t count) {
    period_tasks = table;
    task_count = count;

    uint32_t current_time = get_ms();
    
    // 初始化所有任务的上次执行时间
    for (int i = 0; i < task_count; i++) {
        period_tasks[i].last_run_time_ms = current_time;
    }
    
    log_i("Task scheduler initialized with %d tasks", count);
}

/**
 * @brief 初始化周期性任务调度器
 */
void create_periodic_event_task(void) {
    uint32_t current_time = get_ms();
    
    // 初始化所有任务的上次执行时间
    for (int i = 0; i < task_count; i++) {
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
    for (int i = 0; i < task_count; i++) {
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
    for (int i = 0; i < task_count; i++) {
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
    for (int i = 0; i < task_count; i++) {
        if (period_tasks[i].id == event_id) {
            period_tasks[i].is_running = IDLE;
            log_i("Task %d disabled", event_id);
            break;
        }
    }
}