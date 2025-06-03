#include "periodic_event_task.h"
#include "common_include.h" // 确保 TIMER_5MS_PERIOD_MS 在这里定义为 5
//#include "log_config.h"
#include "log.h"

static TaskHandle_t PeriodicEventTaskHandle;

void debug_task(void) {
	log_i("wit: roll = %.2lf, yaw = %.2lf, pitch = %.2lf", jy901s.roll, jy901s.yaw, jy901s.pitch);
}

// Define periodic tasks array
// {ID, RUNNING_FLAG, task_handler, period_ms}
period_task_t period_tasks[] = {
    { EVENT_KEY_STATE_UPDATE,  RUN,  	button_ticks, 		 				 20   }, // 20ms
    { EVENT_MENU_VAR_UPDATE,   IDLE, 	NotifyMenuFromISR, 				 2000 }, // 2000ms (2s)
		{ EVENT_PERIOD_PRINT,  		 RUN,   debug_task, 							 500  }, // 500ms
		{ EVENT_ALERT, 						 RUN,  	alert_ticks, 							 10,  }, // 10ms
		{ EVENT_CAR_STATE_MACHINE, IDLE,   car_state_machine, 				 20,  }, // 20ms
		{ EVENT_CAR, 				   		 RUN,  car_task, 				         20,  }, // 20ms
		
};


void PeriodicEventTask(void *pvParameters)
{	
    // 跟踪每个子任务的上次执行时间
    static uint32_t last_execution_tick[NUM_PERIOD_TASKS];
    
    TickType_t xLastWakeTime;
    uint32_t ulCurrentTick = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TIMER_5MS_PERIOD_MS);
    
    // 预计算所有任务的period_ticks并初始化last_execution_tick
    for (int i = 0; i < NUM_PERIOD_TASKS; i++)
    {
        // 计算ticks并存储，避免后续重复计算
        period_tasks[i].period_ticks = (period_tasks[i].period_ms > 0) 
            ? fmax(1, period_tasks[i].period_ms / TIMER_5MS_PERIOD_MS) 
            : 1;  // 处理0ms和小于5ms的特殊情况
        
        // 对于短周期任务(<=5ms)，设置为下一个tick运行
        if (period_tasks[i].period_ms <= TIMER_5MS_PERIOD_MS) {
            last_execution_tick[i] = ulCurrentTick - 1;
        } else {
            // 对于长周期任务，设置为一个周期后运行
            last_execution_tick[i] = ulCurrentTick - period_tasks[i].period_ticks;
        }
    }
    
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        // 记录本周期开始时间
        TickType_t cycle_start_time = xTaskGetTickCount();
        uint32_t total_execution_time_ms = 0;
        uint8_t tasks_executed_this_cycle = 0;
        
        // 等待下一个5ms周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        uint32_t current_tick = xTaskGetTickCount();
        
        // 检查并执行到期的任务
        for (int i = 0; i < NUM_PERIOD_TASKS; i++)
        {
            if (period_tasks[i].is_running == RUN && period_tasks[i].task_handler != NULL)
            {
                // 使用预计算的period_ticks，避免重复计算
                if ((int32_t)(current_tick - last_execution_tick[i]) >= (int32_t)period_tasks[i].period_ticks)
                {
                    // 记录任务开始执行的时间
                    TickType_t task_start_time = xTaskGetTickCount();
                    
                    // 执行任务
                    period_tasks[i].task_handler();
                    
                    // 计算任务执行时间
                    TickType_t task_end_time = xTaskGetTickCount();
                    period_tasks[i].execution_time_ms = (task_end_time - task_start_time) * portTICK_PERIOD_MS;
                    
                    // 累加执行时间
                    total_execution_time_ms += period_tasks[i].execution_time_ms;
                    tasks_executed_this_cycle++;
                    
                    // 检查任务是否超时（执行时间超过任务周期的一半）
                    if (period_tasks[i].execution_time_ms > (period_tasks[i].period_ms / 2)) {
                        period_tasks[i].has_timeout = 1;
                        log_e("Task[%d] execution time too long: %lu ms (Task period: %lu ms)", 
                              period_tasks[i].id, 
                              (unsigned long)period_tasks[i].execution_time_ms, 
                              (unsigned long)period_tasks[i].period_ms);
                    }
                    
                    // 更新下次执行时间
                    last_execution_tick[i] += period_tasks[i].period_ticks;
                }
            }
        }
        
        // 计算整个周期的总执行时间
        TickType_t cycle_end_time = xTaskGetTickCount();
        uint32_t cycle_total_time_ms = (cycle_end_time - cycle_start_time) * portTICK_PERIOD_MS;
        
        // 如果累计任务时间超过了周期时间，输出所有任务的运行时间
        if (total_execution_time_ms > TIMER_5MS_PERIOD_MS) {

            log_w("Total execution time exceeded cycle time: %lu ms", (unsigned long)total_execution_time_ms);
            log_w("Tasks executed in this cycle: %d", tasks_executed_this_cycle);
            
            // 输出本周期内执行的所有任务的运行时间
            for (int i = 0; i < NUM_PERIOD_TASKS; i++) {
                if ((int32_t)(current_tick - last_execution_tick[i]) >= (int32_t)period_tasks[i].period_ticks
                    && period_tasks[i].is_running == RUN && period_tasks[i].task_handler != NULL) {
                    log_i("  Task[%d]: %lu ms (Period: %lu ms)", 
                          period_tasks[i].id,
                          (unsigned long)period_tasks[i].execution_time_ms,
                          (unsigned long)period_tasks[i].period_ms);
                }
            }
            
            // 输出整个周期的时间信息
            log_i("Cycle total time: %lu ms (Expected: %d ms)", 
                   (unsigned long)cycle_total_time_ms, TIMER_5MS_PERIOD_MS);
        }
    }
}


// 动态启用任务
void enable_periodic_task(EVENT_IDS event_id)
{
    uint32_t ulCurrentTick = xTaskGetTickCount();
    
    for (int i = 0; i < NUM_PERIOD_TASKS; i++)
    {
        if (period_tasks[i].id == event_id)
        {
            period_tasks[i].is_running = RUN;
            break;
        }
    }
}
void disable_periodic_task(EVENT_IDS event_id)
{
    for (int i = 0; i < NUM_PERIOD_TASKS; i++)
    {
        if (period_tasks[i].id == event_id)
        {
            period_tasks[i].is_running = IDLE;
            break;
        }
    }
}
void create_periodic_event_task(void) {
    BaseType_t xReturn = xTaskCreate(
        PeriodicEventTask,
        "PeriodicEventTask",
        configMINIMAL_STACK_SIZE * 4,
        NULL,
        tskIDLE_PRIORITY + 2,
        &PeriodicEventTaskHandle);
    if (xReturn == pdPASS) {
        log_i("PeriodicEventTask created successfully!");
    } else {
        log_e("Failed to create PeriodicEventTask! Error code: %d", xReturn);
    }
}
