#include "periodic_event_task.h"
#include "common_include.h"
#include "log_config.h"
#include "log.h"

static TaskHandle_t PeriodicEventTaskHandle;

void debug_task(void) {
	debug_speed_pid();
}



// Define periodic tasks array - 基于1ms主循环重新计算
// {ID, RUNNING_FLAG, task_handler, period_ms}
period_task_t period_tasks[] = {
    { EVENT_KEY_STATE_UPDATE,  RUN, 	button_ticks, 		 				 20   }, // 20ms (20个tick)
    { EVENT_MENU_VAR_UPDATE,   RUN, 	oled_menu_tick, 				 	 20 }, // 2000ms (2000个tick)
		{ EVENT_PERIOD_PRINT,  		 IDLE,  debug_task, 							 500  }, // 500ms (500个tick)
		{ EVENT_ALERT, 						 RUN,  	alert_ticks, 							 10,  }, // 10ms (10个tick)
		{ EVENT_CAR_STATE_MACHINE, IDLE,  car_state_machine, 	   		 20,  }, // 20ms (20个tick)
		{ EVENT_CAR, 				   		 RUN,   car_task, 				         20,  }, // 20ms (20个tick)
};


void PeriodicEventTask(void *pvParameters)
{	
    // 跟踪每个子任务的上次执行时间
    static uint32_t last_execution_tick[NUM_PERIOD_TASKS];
    
    TickType_t xLastWakeTime;
    uint32_t ulCurrentTick = xTaskGetTickCount();
    
    // 使用1ms作为主循环周期（实际情况）
    const TickType_t xFrequency = pdMS_TO_TICKS(ACTUAL_LOOP_PERIOD_MS);
    
    // 预计算所有任务的period_ticks - 基于1ms主循环
    for (int i = 0; i < NUM_PERIOD_TASKS; i++)
    {
        // 直接按毫秒计算ticks
        period_tasks[i].period_ticks = period_tasks[i].period_ms;
        
        
        // 初始化：让所有任务在第一个周期就可以执行
        last_execution_tick[i] = ulCurrentTick - period_tasks[i].period_ticks;
    }
    
    xLastWakeTime = xTaskGetTickCount();
    
    // 简化监控
    static uint32_t cycle_count = 0;
    
    while (1)
    {
        // 等待下一个1ms周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        uint32_t current_tick = xTaskGetTickCount();

        // 检查并执行到期的任务
        for (int i = 0; i < NUM_PERIOD_TASKS; i++)
        {
            if (period_tasks[i].is_running == RUN && period_tasks[i].task_handler != NULL)
            {
                uint32_t ticks_since_last = current_tick - last_execution_tick[i];
                
                // 检查是否到了执行时间
                if (ticks_since_last >= period_tasks[i].period_ticks)
                {
                    // 记录任务开始执行的时间
                    TickType_t task_start_time = xTaskGetTickCount();
                    
                    // 执行任务
                    period_tasks[i].task_handler();
                    
                    // 计算任务执行时间
                    TickType_t task_end_time = xTaskGetTickCount();
                    period_tasks[i].execution_time_ms = (task_end_time - task_start_time) * portTICK_PERIOD_MS;
                    
                    // 检查任务执行时间
                    if (period_tasks[i].execution_time_ms > 5) {  // 超过5ms报警
                        log_w("Task[%d] execution time: %lu ms", 
                              period_tasks[i].id, 
                              (unsigned long)period_tasks[i].execution_time_ms);
                    }
                    
                    // 更新下次执行时间 - 不重置，只增加周期
                    last_execution_tick[i] = current_tick;
                }
            }
        }
    }
}

// 动态启用任务
void enable_periodic_task(EVENT_IDS event_id)
{
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
        configMINIMAL_STACK_SIZE * 8,
        NULL,
        tskIDLE_PRIORITY + 2,
        &PeriodicEventTaskHandle);
    if (xReturn == pdPASS) {
        log_i("PeriodicEventTask created successfully!");
    } else {
        log_e("Failed to create PeriodicEventTask! Error code: %d", xReturn);
    }
}