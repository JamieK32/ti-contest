#include "ui.h"
#include "log_config.h"
#include "log.h"
#include "common_include.h"


/* =============================================================================
 * 任务配置和回调函数
 * ============================================================================= */
bool task_running_flag = false;

// 24h题目任务1：简单直行到黑线
static void setup_task1(void) {
    car_path_init();                                   // 初始化路径规划
    car_add_turn(0.0f);                                // 原地转向0度（校正方向）
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);   // 直行移动直到检测到黑线
    car_set_loop(1);                                   // 设置循环次数为1次
}

// 24h题目任务2：往返循迹
static void setup_task2(void) {
    car_path_init();                                   // 初始化路径规划
    car_add_turn(0.0f);                                // 原地转向0度（校正方向）
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);   // 直行移动直到检测到黑线
    car_add_move_until_white(CAR_STATE_TRACK);         // 循迹移动直到检测到白线（路径结束）
    car_add_turn(-180.0f);                             // 原地左转180度（掉头）
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);   // 直行移动直到检测到黑线
    car_add_move_until_white(CAR_STATE_TRACK);         // 循迹移动直到检测到白线（返回起点）
    car_set_loop(1);                                   // 设置循环次数为1次
}

// 24h题目任务3：角度循迹
static void setup_task3(void) {
    car_path_init();                                   // 初始化路径规划
    car_add_turn(-35.0f);                              // 原地左转35度（调整起始角度）
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);   // 直行移动直到检测到黑线
    car_add_move_until_white(CAR_STATE_TRACK);         // 循迹移动直到检测到白线
    car_add_turn(-145.0f);                             // 原地左转145度（总共转180度）
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);   // 直行移动直到检测到黑线
    car_add_move_until_white(CAR_STATE_TRACK);         // 循迹移动直到检测到白线
    car_set_loop(1);                                   // 设置循环次数为1次
}

// 24h题目任务4：角度循迹（多次循环）
static void setup_task4(void) {
    car_path_init();                                   // 初始化路径规划
    car_add_turn(-35.0f);                              // 原地左转35度（调整起始角度）
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);   // 直行移动直到检测到黑线
    car_add_move_until_white(CAR_STATE_TRACK);         // 循迹移动直到检测到白线
    car_add_turn(-145.0f);                             // 原地左转145度（总共转180度）
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);   // 直行移动直到检测到黑线
    car_add_move_until_white(CAR_STATE_TRACK);         // 循迹移动直到检测到白线
    car_set_loop(4);                                   // 设置循环次数为4次（重复执行）
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



static void set_yaw_zero(void *arg) {
		show_message("Resetting...");
		wit_imu_set_yaw_zero();
		show_message("Reset Ok");
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

#if CURRENT_IMU == MPU6050_GYRO
	extern float yaw, roll, pitch;
#endif 

static menu_variable_t gyro_vars[] = {
#if CURRENT_IMU == WIT_GYRO
    MENU_VAR_READONLY("Yaw", &jy61p.yaw, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Pitch", &jy61p.pitch, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Roll", &jy61p.roll, VAR_TYPE_FLOAT),
#elif CURRENT_IMU == MPU6050_GYRO
	    MENU_VAR_READONLY("Yaw", &yaw, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Pitch", &pitch, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Roll", &roll, VAR_TYPE_FLOAT),
#endif 
		MENU_VAR_END
};


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
    ADD_SUBMENU(main_menu, tasks1_menu, "Run 24h App", NULL);
    ADD_ACTION(tasks1_menu, _24h_task1, "Run 24h Task1", run_task01_cb);
    ADD_ACTION(tasks1_menu, _24h_task2, "Run 24h Task2", run_task02_cb);
    ADD_ACTION(tasks1_menu, _24h_task3, "Run 24h Task3", run_task03_cb);
    ADD_ACTION(tasks1_menu, _24h_task4, "Run 24h Task4", run_task04_cb);
		ADD_ACTION(tasks1_menu, _24h_task5, "Set Yaw Zero",  set_yaw_zero);
		
	
		ADD_SUBMENU(main_menu, PlayMusic, "Play Music", NULL);
		ADD_ACTION(PlayMusic, music1, "ChunRiYing", play_music_1_cb);
    ADD_ACTION(PlayMusic, music2, "TianKongZhiCheng", play_music_2_cb);
		ADD_ACTION(PlayMusic, stop_music, "StopMusic", stop_music_cb);
		
		ADD_SUBMENU(main_menu, status_menu, "System Status", NULL);
		ADD_VAR_VIEW(status_menu, gyro_status_view, "Gyro Status", gyro_vars);
		ADD_VAR_VIEW(status_menu, car_status_view, "Car Status", car_vars);
    create_oled_menu(&main_menu);
}