#include "ui.h"
#include "log.h"
#include "common_include.h"

/* =============================================================================
 * 任务配置和回调函数
 * ============================================================================= */
bool task_running_flag = false;

static void setup_task1(void) {
    car_path_init();
    car_add_turn(0.0f);
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);
    car_set_loop(1);
}

static void setup_task2(void) {
    car_path_init();
    car_add_turn(0.0f);
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);
    car_add_move_until_white(CAR_STATE_TRACK);
    car_add_turn(-180.0f);
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);
    car_add_move_until_white(CAR_STATE_TRACK);
    car_set_loop(1);
}

static void setup_task3(void) {
    car_path_init();
    car_add_turn(-35.0f);
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);
    car_add_move_until_white(CAR_STATE_TRACK);
    car_add_turn(-145.0f);
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);
    car_add_move_until_white(CAR_STATE_TRACK);
    car_set_loop(1);
}

static void setup_task4(void) {
    car_path_init();
    car_add_turn(-35.0f);
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);
    car_add_move_until_white(CAR_STATE_TRACK);
    car_add_turn(-145.0f);
    car_add_move_until_black(CAR_STATE_GO_STRAIGHT);
    car_add_move_until_white(CAR_STATE_TRACK);
    car_set_loop(4);  
}

static void setup_task5(void) {
	car_path_init();
	car_add_move_until_stop_mark(CAR_STATE_TRACK);
	car_add_straight(100);
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

void run_task01_cb(void *arg) {
    run_task("Running Task 01", &task_running_flag, setup_task1);
}

void run_task02_cb(void *arg) {
    run_task("Running Task 02", &task_running_flag, setup_task2);
}

void run_task03_cb(void *arg) {
    run_task("Running Task 03", &task_running_flag, setup_task3);
}

void run_task04_cb(void *arg) {
    run_task("Running Task 04", &task_running_flag, setup_task4);
}

void run_task05_cb(void *arg) {
		run_task("Running Task 01", &task_running_flag, setup_task5);
}

void set_yaw_zero(void *arg) {
		show_message("Resetting...");
		wit_imu_set_yaw_zero();
		show_message("Reset Ok");
}

void play_music_1_cb(void *arg) {
	show_message("Play Music1");
	music_player_start(music_example_1, music_example_1_size);
}

void play_music_2_cb(void *arg) {
	show_message("Play Music2");
	music_player_start(music_example_2, music_example_2_size);
}

void stop_music_cb(void *arg) {
	show_message("Stop Music");
	music_player_stop();
}

menu_variable_t gyro_vars[] = {
    MENU_VAR_READONLY("Yaw", &jy61p.yaw, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Pitch", &jy61p.pitch, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Roll", &jy61p.roll, VAR_TYPE_FLOAT),
		MENU_VAR_END
};


menu_variable_t car_vars[] = {
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
		
		ADD_SUBMENU(main_menu, tasks2_menu, "Run 22c App", NULL);
		ADD_ACTION(tasks2_menu, _22c_task1, "Run 22c Task1", run_task05_cb);
			
		
		ADD_SUBMENU(main_menu, PlayMusic, "Play Music", NULL);
		ADD_ACTION(PlayMusic, music1, "ChunRiYing", play_music_1_cb);
    ADD_ACTION(PlayMusic, music2, "TianKongZhiCheng", play_music_2_cb);
		ADD_ACTION(PlayMusic, stop_music, "StopMusic", stop_music_cb);
		
		ADD_SUBMENU(main_menu, status_menu, "System Status", NULL);
		ADD_VAR_VIEW(status_menu, gyro_status_view, "Gyro Status", gyro_vars);
		ADD_VAR_VIEW(status_menu, car_status_view, "Car Status", car_vars);
    create_oled_menu(&main_menu);
}