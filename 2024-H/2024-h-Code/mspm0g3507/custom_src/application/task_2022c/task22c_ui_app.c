#include "common_defines.h"
#if (CURRENT_TASK_TYPE == TASK_TYPE_22C)
#include "ui.h"
#include "log_config.h"
#include "log.h"
#include "common_include.h"
#include "task22c_config.h"

/* =============================================================================
 * 任务配置和回调函数
 * ============================================================================= */
bool task_running_flag = false;
extern float track_default_speed;



// 22C赛任务5：第一题 - 单圈外圈循迹
static void setup_task1(void) {
    car_path_init();                                           // 初始化路径规划
    car_add_float(&track_default_speed, C22_BASE_SPEED - 5);      // 设置循迹速度为基准速度-5（35）
    car_add_bool(&is_outer_track, true);                      // 设置标志位：走外圈模式
    car_add_byte(BLUETOOTH_CMD_START_QUESTION1);              // 发送蓝牙指令：开始第一题
    car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动直到检测到停车标记
    car_add_byte(BLUETOOTH_CMD_STOP);                         // 发送蓝牙指令：停止
    car_set_loop(1);                                          // 设置循环次数为1次
}

// 22C赛任务6：第二题 - 双停车点外圈循迹
static void setup_task2(void) {
    car_path_init();                                          // 初始化路径规划
    car_add_float(&track_default_speed, C22_BASE_SPEED);      // 设置循迹速度为基准速度（40）
    car_add_bool(&is_outer_track, true);                      // 设置标志位：走外圈模式
    car_add_byte(BLUETOOTH_CMD_START_QUESTION2);              // 发送蓝牙指令：开始第二题
    car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动直到检测到第一个停车标记
    car_add_track(STOP_MARK_CLEARANCE_CM);                    // 继续循迹40cm（消除停车点检测误差）
    car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动直到检测到第二个停车标记
    car_add_byte(BLUETOOTH_CMD_STOP);                         // 发送蓝牙指令：停止
    car_set_loop(1);                                          // 设置循环次数为1次
}

// 22C赛任务7：第三题 - 三圈变道循迹
static void setup_task3(void) {
    car_path_init();                                          // 初始化路径规划
    car_add_bool(&is_outer_track, true);                      // 设置标志位：初始走外圈模式
    car_add_float(&track_default_speed, C22_BASE_SPEED);       // 设置循迹速度为基准速度（40）
    car_add_byte(BLUETOOTH_CMD_START_QUESTION3);              // 发送蓝牙指令：开始第三题
    
    // 第一圈：外圈循迹
    car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动直到检测到停车标记（第一圈完成）
    car_add_track(STOP_MARK_CLEARANCE_CM);                     // 继续循迹40cm（越过停车标记）
    
    // 第二圈：继续外圈循迹
    car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动直到检测到停车标记（第二圈完成）
		car_add_float(&track_default_speed,C22_BASE_SPEED+14);    //
    car_add_bool(&is_outer_track, false);                    // 切换标志位：改为走内圈模式
    car_add_track(150);                                  
    car_add_float(&track_default_speed,C22_BASE_SPEED); 
    // 第三圈：内圈循迹
    car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动直到检测到停车标记（第三圈完成）
    car_add_byte(BLUETOOTH_CMD_STOP);                         // 发送蓝牙指令：停止
    car_set_loop(1);                                          // 设置循环次数为1次
}

//22C赛任务8:第四题——高速行驶制停
static void setup_task4(void) {
	  car_path_init();                                          // 初始化路径规划
    car_add_bool(&is_outer_track, true);                      // 设置标志位：初始走外圈模式
	  car_add_float(&track_default_speed, C22_BASE_SPEED+15);       // 设置循迹速度为基准速度（55）
	  car_add_byte(BLUETOOTH_CMD_START_QUESTION4);              // 发送蓝牙指令：开始第四题目
	  car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动直到检测到停车标记
	  car_add_byte(BLUETOOTH_CMD_STOP);                         // 发送蓝牙指令：停止
	  car_add_delay(5000);
		car_add_byte(BLUETOOTH_CMD_RECONNECT_QUESTION4);              // 发送蓝牙指令：继续执行
	  car_add_track(20);
	  car_add_track(20);
    car_add_move_until_stop_mark(CAR_STATE_TRACK);            // 循迹移动到停车标记
	  car_add_byte(BLUETOOTH_CMD_STOP);                         //完成任务
	  car_set_loop(1);     
}

static void run_task(const char *task_name, bool *task_flag, void (*setup_func)(void)) {
    if (*task_flag == true) {
        show_message("Running Failed");
        return;
    }
    *task_flag = true;
    show_message(task_name);    
    setup_func();     // 调用设置函数
    car_start();      // 启动状态机
    enable_periodic_task(EVENT_CAR_STATE_MACHINE);
}

static void run_task01_cb(void *arg) {
    run_task("Running Task 01", &task_running_flag, setup_task1);
}

static void run_task02_cb(void *arg) {
    run_task("Running Task 02", &task_running_flag, setup_task2);
}

static void run_task03_cb(void *arg) {
    run_task("Running Task 03", &task_running_flag, setup_task3);
}

static void run_task04_cb(void *arg) {
    run_task("Running Task 04", &task_running_flag, setup_task4);
}

static void bluetooth_test_cb(void *arg) {
	show_message("Test BlueTooth");
	bluetooth_send_byte(BLUETOOTH_CMD_TEST_CONNECT);
}

static void play_music_1_cb(void *arg) {
	show_message("Play Music1");
	music_player_start(music_example_1, music_example_1_size);
}

static void play_music_2_cb(void *arg) {
	show_message("Play Music2");
	music_player_start(music_example_2, music_example_2_size);
}

static void stop_music_cb(void *arg) {
	show_message("Stop Music");
	music_player_stop();
}

static menu_variable_t car_vars[] = {
    MENU_VAR_BINARY_8BIT("Gray", &gray_byte),
    MENU_VAR_END
};

/* =============================================================================
 * 菜单创建
 * ============================================================================= */
void menu_init_and_create(void) {
    // 使用链式构建宏创建菜单结构
    MENU_BUILDER_START(main_menu, "Main Menu");
    
    // 任务执行菜单
    ADD_SUBMENU(main_menu, tasks1_menu, "Run 22c App", NULL);
		ADD_ACTION(tasks1_menu, bluetooth_test, "Bluetooth Test", bluetooth_test_cb);
    ADD_ACTION(tasks1_menu, _24h_task1, "Run 22c Task1", run_task01_cb);
    ADD_ACTION(tasks1_menu, _24h_task2, "Run 22c Task2", run_task02_cb);
    ADD_ACTION(tasks1_menu, _24h_task3, "Run 22c Task3", run_task03_cb);
	  ADD_ACTION(tasks1_menu, _24h_task4, "Run 22c Task4", run_task04_cb);


		ADD_SUBMENU(main_menu, PlayMusic, "Play Music", NULL);
		ADD_ACTION(PlayMusic, music1, "ChunRiYing", play_music_1_cb);
    ADD_ACTION(PlayMusic, music2, "TianKongZhiCheng", play_music_2_cb);
		ADD_ACTION(PlayMusic, stop_music, "StopMusic", stop_music_cb);
		
		ADD_SUBMENU(main_menu, status_menu, "System Status", NULL);
		ADD_VAR_VIEW(status_menu, car_status_view, "Car Status", car_vars);
    create_oled_menu(&main_menu);
}

#endif