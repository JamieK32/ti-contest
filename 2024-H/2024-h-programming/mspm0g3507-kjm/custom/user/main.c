#include "common_include.h"

//#include "log_config.h" // 日志配置
#include "log.h"

void system_init(void) 
{
    SYSCFG_DL_init();
		beep_init();
		systick_init();
}

void main_task_init(void) 
{
		jy901s.init();
    menu_init_and_create();
    car_init();
		gray_detection_init();
		create_periodic_event_task(); // 初始化任务调度器
}

void test_task(void) 
{
		play_music(music_example_2, music_example_2_size);
}

int main(void) 
{
    system_init();
    
#if UNIT_TEST_MODE    // 是否使能单元测试功能
		test_task();
		while (1) {
		
		}
#else
		main_task_init();
#endif

    while (1) {
        periodic_event_task_process(); // 处理所有任务
    }

    return 0;
}


void HardFault_Handler(void) 
{
    log_e("!!! Unhandled Interrupt HardFault_Handler !!!\n");
    
    play_alert_blocking(3, COLOR_RED);
    
    while (1)
    {
    }
}

