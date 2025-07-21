#include "common_defines.h"
#if (CURRENT_TASK_TYPE == TASK_TYPE_22C)
#include "task22c_config.h"
#include "common_include.h"
#include "log.h"

typedef struct bluetooth_cmd_handler_t {
	uint8_t cmd;
	void (*callback)(void);
} bluetooth_cmd_handler_t;

extern float track_default_speed;

void question1_start_cb(void) {
		car_path_init();
		car_add_float(&track_default_speed, C22_BASE_SPEED - 5);                           //初始速度
		car_add_bool(&is_outer_track,true);                  //执行外圈
		car_add_track(100);                                  //超出黑线部分
		car_add_move_until_stop_mark(CAR_STATE_TRACK);       //循迹一圈 
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);
		car_start();          
}

void question2_start_cb(void) {
		car_path_init();
		car_add_float(&track_default_speed, C22_BASE_SPEED);    //初始速度设置
		car_add_bool(&is_outer_track,true);                    //执行外圈
		car_add_track(100);                
		car_add_move_until_stop_mark(CAR_STATE_TRACK);         //第一圈
		car_add_track(100);
		car_add_move_until_stop_mark(CAR_STATE_TRACK);         //第二圈
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);
		car_start();       
}

void question3_start_cb(void) {
	  car_path_init();
		car_add_float(&track_default_speed,C22_BASE_SPEED ); //初始速度设置
		car_add_bool(&is_outer_track, true);                 //设置第一圈为外圈
		car_add_track(50);
		car_add_move_until_stop_mark(CAR_STATE_TRACK);      //执行第一圈	
		car_add_float(&track_default_speed,C22_BASE_SPEED + 14);                 //修改第二圈速度反超主车
	  car_add_bool(&is_outer_track, false);                //设置第二圈为外圈
		car_add_track(150);           
		car_add_float(&track_default_speed,C22_BASE_SPEED-1);                
		car_add_move_until_stop_mark(CAR_STATE_TRACK);      //继续执行第二圈
    car_add_bool(&is_outer_track, true);
		car_add_track(50);           
		car_add_move_until_stop_mark(CAR_STATE_TRACK);      //执行第三圈
    enable_periodic_task(EVENT_CAR_STATE_MACHINE);		
		car_start();            
}

void question4_start_cb(void) {
	  car_path_init();
		car_add_float(&track_default_speed, 60);                 //设置初始速度
		car_add_bool(&is_outer_track,true);                 //执行外外圈
		car_add_track(100);
		car_add_move_until_stop_mark(CAR_STATE_TRACK);      //跟跑外圈
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);		
		car_start();        
}


void question4_reconnect_cb(void) {
		car_path_init();
		car_add_float(&track_default_speed, 60);                 //设置初始速度
		car_add_track(100);
		car_add_move_until_stop_mark(CAR_STATE_TRACK);      //跟跑外圈
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);		
		car_start();                                        // 启动状态机
}

void test_connect_cb(void) {
		set_alert_count(2);
		start_alert();
}

void stop_cb(void) {
		car_stop();
}

bluetooth_cmd_handler_t bluetooth_cmd_handler[BLUETOOTH_CMD_COUNT] = {
	{BLUETOOTH_CMD_START_QUESTION1, 		question1_start_cb},
	{BLUETOOTH_CMD_START_QUESTION2, 		question2_start_cb},
	{BLUETOOTH_CMD_START_QUESTION3, 		question3_start_cb},
	{BLUETOOTH_CMD_START_QUESTION4,		  question4_start_cb},
	{BLUETOOTH_CMD_RECONNECT_QUESTION4, question4_reconnect_cb},
	{BLUETOOTH_CMD_TEST_CONNECT, 				test_connect_cb},
	{BLUETOOTH_CMD_STOP, 								stop_cb},
};


void bluetooth_data_received(const uint8_t* data, size_t length){
	if (length == 1) {
		for (int i = 0; i < BLUETOOTH_CMD_COUNT; i++) {
			if (data[0] == bluetooth_cmd_handler[i].cmd) {
				 bluetooth_cmd_handler[i].callback();
			}
		}
	} else {
		log_e("data length error");
	}
}
#endif 
