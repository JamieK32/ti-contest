
#include "common_include.h"

#include "log_config.h" // 日志配置
#include "log.h"

// 初始化任务句柄
static TaskHandle_t init_task_handle = NULL;

void system_init(void) 
{
    SYSCFG_DL_init();
		beep_init();
#if DEBUG_MODE
		debug_uart_init();
#endif
}

void user_init(void) 
{
		jy901s.init();
    menu_init_and_create();
    car_init();
		gray_detection_init();
    create_periodic_event_task();
}

// 初始化任务函数
static void init_task(void *param)
{
    user_init();
    // 初始化完成后可以删除自身任务，释放资源
    vTaskDelete(NULL);
}

/**
 * @brief 创建初始化任务
 */
static void create_init_task(void)
{
    BaseType_t result = xTaskCreate(
        init_task,              		// 任务函数
        "InitTask",            			// 任务名称
        configMINIMAL_STACK_SIZE * 2,   // 堆栈大小（单位：word）
        NULL,                 		 	// 任务参数
        tskIDLE_PRIORITY + 2,  			// 优先级（比空闲任务高）
        &init_task_handle      			// 任务句柄
    );
    
    if (result != pdPASS) {
        log_e("Failed to create initialization task!");
        play_alert_blocking(3, COLOR_RED);
        while (1) {
            // 创建失败，进入死循环
        }
    }
}

void test_task_init_and_create(void) 
{
	motor_task_create();
}

int main(void) 
{
    system_init();
    
#if UNIT_TEST_MODE    // 是否使能单元测试功能
    test_task_init_and_create();
#else
    create_init_task();
#endif

    vTaskStartScheduler(); // 启动调度器

    // 如果能来到这里，说明调度器启动失败
    while (1) {
        log_e("Scheduler Start Failed");
        play_alert_blocking(3, COLOR_YELLOW);
        break;
    }

    return 0;
}

// 堆栈溢出钩子。置顶函数名，不可修改。
// rgb 紫色
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    log_e("!!! STACK OVERFLOW: %s !!!\n", pcTaskName); // 错误日志输出
    
		play_alert_blocking(3, COLOR_PURPLE);
    
    taskDISABLE_INTERRUPTS();
    while (1)
    {
    }
}

// 发生严重错误时进入
// rgb 红色
void HardFault_Handler(void) 
{
    log_e("!!! Unhandled Interrupt HardFault_Handler !!!\n");
    
    play_alert_blocking(3, COLOR_RED);
    
    taskDISABLE_INTERRUPTS();
    while (1)
    {
    }
}

// 内存分配失败钩子
void vApplicationMallocFailedHook(void)
{
    log_e("!!! MALLOC FAILED !!! FreeRTOS heap allocation failed.\n");

    play_alert_blocking(3, COLOR_RED);

    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}
