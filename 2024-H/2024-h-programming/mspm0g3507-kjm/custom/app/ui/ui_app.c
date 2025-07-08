#include "ui.h"
#include "log.h"
#include "common_include.h"

/* =============================================================================
 * 任务配置和回调函数
 * ============================================================================= */
bool task_running_flag = false;

static action_config_t task1_action_config = {
    .actions = {
        {ACTION_SPIN_TURN, 0.0f}, 
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT},
    },
    .is_loop_enabled = true,
    .loop_count = 1
};

static action_config_t task2_action_config = {
    .actions = {
        {ACTION_SPIN_TURN, 0.0f}, 
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT},
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},
        {ACTION_SPIN_TURN, -180.0f}, 
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT},
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},
    },
    .is_loop_enabled = true,
    .loop_count = 1
};

static action_config_t task3_action_config = {
    .actions = {
        {ACTION_SPIN_TURN, -35.0f},
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT},
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},
        {ACTION_SPIN_TURN, -145.0f},
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT},
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},
    },
    .is_loop_enabled = true,
    .loop_count = 1
};

static action_config_t task4_action_config = {
    .actions = {
        {ACTION_SPIN_TURN, -35.0f},
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT},
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},
        {ACTION_SPIN_TURN, -145.0f},
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT},
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},
    },
    .action_count = 0,  // 自动计算
    .is_loop_enabled = true,
    .loop_count = 4
};

static void run_task(const char *task_name, bool *task_flag, action_config_t *task_config) {
    if (*task_flag == true) {
        show_message("Running Failed");
        return;
    }
    *task_flag = true;
    show_message(task_name);
    jy901s.reset();
    car_path_init(task_config);
    enable_periodic_task(EVENT_CAR_STATE_MACHINE);
}

void run_task01_cb(void *arg) {
    run_task("Running Task 01", &task_running_flag, &task1_action_config);
}

void run_task02_cb(void *arg) {
    run_task("Running Task 02", &task_running_flag, &task2_action_config);
}

void run_task03_cb(void *arg) {
    run_task("Running Task 03", &task_running_flag, &task3_action_config);
}

void run_task04_cb(void *arg) {
    run_task("Running Task 04", &task_running_flag, &task4_action_config);
}

void set_yaw_zero(void *arg) {
		show_message("Resetting...");
		jy901s.reset();
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
    MENU_VAR_READONLY("Yaw", &jy901s.yaw, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Pitch", &jy901s.pitch, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Roll", &jy901s.roll, VAR_TYPE_FLOAT),
		MENU_VAR_END
};

/* =============================================================================
 * 菜单创建
 * ============================================================================= */
void menu_init_and_create(void) {
    // 使用链式构建宏创建菜单结构
    MENU_BUILDER_START(main_menu, "Main Menu");
    
    // 任务执行菜单
    ADD_SUBMENU(main_menu, tasks_menu, "Run 24h App", NULL);
    ADD_ACTION(tasks_menu, task1, "Run 24h Task1", run_task01_cb);
    ADD_ACTION(tasks_menu, task2, "Run 24h Task2", run_task02_cb);
    ADD_ACTION(tasks_menu, task3, "Run 24h Task3", run_task03_cb);
    ADD_ACTION(tasks_menu, task4, "Run 24h Task4", run_task04_cb);
		ADD_ACTION(tasks_menu, task5, "Set Yaw Zero",  set_yaw_zero);
		
		ADD_SUBMENU(main_menu, PlayMusic, "Play Music", NULL);
		ADD_ACTION(PlayMusic, music1, "ChunRiYing", play_music_1_cb);
    ADD_ACTION(PlayMusic, music2, "TianKongZhiCheng", play_music_2_cb);
		ADD_ACTION(PlayMusic, stop_music, "StopMusic", stop_music_cb);
		
		ADD_SUBMENU(main_menu, status_menu, "System Status", NULL);
		ADD_VAR_VIEW(status_menu, status_view, "Gyro Status", gyro_vars);
    create_oled_menu(&main_menu);
}