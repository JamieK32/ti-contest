#include "oled_driver.h"
#include "menu_app.h"
#include "button_app.h"
#include "menu_logic.h"
#include "menu_ui.h"
#include "u8g2.h"
#include "wit_jyxx.h"
#include "common_include.h"

static action_config_t task1_action_config = {
    .actions = {
				{ACTION_SPIN_TURN, 0.0f}, 												// 旋转到0度 
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT}, // 通过直行修正的方式直到黑线停下 
    },
    .is_loop_enabled = true,
    .loop_count = 1
};

static action_config_t task2_action_config = {
    .actions = {
				{ACTION_SPIN_TURN, 0.0f}, 												// 旋转到0度 
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT}, // 通过直行修正的方式直到黑线停下 
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},       // 通过循迹修正的方式直到白线停下
				{ACTION_SPIN_TURN, -180.0f}, 											// 旋转到-180度 
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT}, // 通过直行修正的方式直到黑线停下 
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},       // 通过循迹修正的方式直到白线停下
    },
    .is_loop_enabled = true,
    .loop_count = 1
};

static action_config_t task3_action_config = {
    .actions = {
        {ACTION_SPIN_TURN, -35.0f},                       // 旋转到-35度
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT}, // 通过直行修正的方式直到黑线停下 
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},       // 通过循迹修正的方式直到白线停下
        {ACTION_SPIN_TURN, -145.0f},                      // 旋转到-145度
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT}, // 通过直行修正的方式直到黑线停下 
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},       // 通过循迹修正的方式直到白线停下
    },
    .is_loop_enabled = true,
    .loop_count = 1
};

// 默认动作配置表，用户可直接修改此表定义动作序列和循环模式
static action_config_t task4_action_config = {
    .actions = {
        {ACTION_SPIN_TURN, -35.0f},                       // 旋转到-45度
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT}, // 通过直行修正的方式直到黑线停下 
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},       // 通过循迹修正的方式直到白线停下
        {ACTION_SPIN_TURN, -145.0f},                      // 旋转到-145度
        {ACTION_MOVE_UNTIL_BLACK, CAR_STATE_GO_STRAIGHT}, // 通过直行修正的方式直到黑线停下 
        {ACTION_MOVE_UNTIL_WHITE, CAR_STATE_TRACK},       // 通过循迹修正的方式直到白线停下
    },
    .action_count = 0,  // 初始值为0，自动计算
    .is_loop_enabled = true,
    .loop_count = 4
};


static void run_task01_cb(void *arg) {
    u8g2_ClearBuffer(&u8g2);
    draw_centered_text("Running Task 01", 1);
    u8g2_SendBuffer(&u8g2);
		jy901s.reset();
		car_path_init(&task1_action_config);
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);
}

static void run_task02_cb(void *arg) {
    u8g2_ClearBuffer(&u8g2);
    draw_centered_text("Running Task 02", 1);
    u8g2_SendBuffer(&u8g2);
		jy901s.reset();
		car_path_init(&task2_action_config);
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);
}

static void run_task03_cb(void *arg) {
    u8g2_ClearBuffer(&u8g2);
    draw_centered_text("Running Task 03", 1);
    u8g2_SendBuffer(&u8g2);
		jy901s.reset();
		car_path_init(&task3_action_config);
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);
}

static void run_task04_cb(void *arg) {
    u8g2_ClearBuffer(&u8g2);
    draw_centered_text("Running Task 04", 1);
    u8g2_SendBuffer(&u8g2);
		jy901s.reset();
		car_path_init(&task4_action_config);
		enable_periodic_task(EVENT_CAR_STATE_MACHINE);
}

static void view_variables_cb(void *arg) {
		start_listening_variable_timer();
}

// 菜单节点定义和初始化示例
static MenuNode menu_root;
static MenuNode menu_run_tasks;
static MenuNode menu_view_variables;
static MenuNode set_pid_speed;
static MenuNode set_pid_mileage;
static MenuNode play_games, play_games1, play_games2, play_games3, play_games4;
static MenuNode task01;
static MenuNode task02;
static MenuNode task03;
static MenuNode task04;

// 子节点数组
static MenuNode *run_tasks_children[] = { &task01, &task02, &task03, &task04 };
static MenuNode *root_children[] = { &menu_run_tasks, &menu_view_variables, &set_pid_speed, &set_pid_mileage, &play_games, &play_games1, &play_games2, &play_games3, &play_games4 };

// 初始化所有节点
static void init_all_menu_nodes(void) {
	// 初始化菜单节点，参数格式：节点指针, 名称, 回调函数, 菜单类型, 父节点, 子节点数量, 子节点数组
	init_menu_node(&menu_root,           "Main Menu",        NULL,             MENU_TYPE_NORMAL,        NULL,            5 + 4, root_children);          // 根菜单
	init_menu_node(&menu_run_tasks,      "Run Tasks",        NULL,             MENU_TYPE_NORMAL,        &menu_root,      4, run_tasks_children);     // 一级菜单：运行任务
	init_menu_node(&menu_view_variables, "View Variables",   view_variables_cb,MENU_TYPE_VARIABLE_VIEW, &menu_root,      0, NULL);                  // 一级菜单：查看变量
	init_menu_node(&set_pid_speed,       "Set Pid Speed",    NULL,             MENU_TYPE_NORMAL,        &menu_root,      0, NULL);                  // 一级菜单：设置PID速度
	init_menu_node(&set_pid_mileage,     "Set Pid Mileage",  NULL,             MENU_TYPE_NORMAL,        &menu_root,      0, NULL);                  // 一级菜单：设置PID里程
	init_menu_node(&play_games,          "Play Game",        NULL,             MENU_TYPE_NORMAL,        &menu_root,      0, NULL);                  // 一级菜单：玩游戏
	init_menu_node(&play_games1,          "Play Game1",        NULL,             MENU_TYPE_NORMAL,        &menu_root,      0, NULL);                  // 一级菜单：玩游戏
	init_menu_node(&play_games2,          "Play Game2",        NULL,             MENU_TYPE_NORMAL,        &menu_root,      0, NULL);                  // 一级菜单：玩游戏
	init_menu_node(&play_games3,          "Play Game3",        NULL,             MENU_TYPE_NORMAL,        &menu_root,      0, NULL);                  // 一级菜单：玩游戏
	init_menu_node(&play_games4,          "Play Game4",        NULL,             MENU_TYPE_NORMAL,        &menu_root,      0, NULL);                  // 一级菜单：玩游戏
	init_menu_node(&task01,              "run Task01",       run_task01_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务1
	init_menu_node(&task02,              "run Task02",       run_task02_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务2
	init_menu_node(&task03,              "run Task03",       run_task03_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务3
	init_menu_node(&task04,              "run Task04",       run_task04_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务4

}


void menu_init_and_create(void) {
	add_variable("YAW", &jy901s.yaw);
	init_all_menu_nodes();
  user_button_init(&btn_single_click_callback, &btn_long_press_cb);    
	create_oled_menu(&menu_root);
}


static inline void btn_single_click_callback(void* btn)
{
    struct Button* button = (struct Button*) btn;
    if (button == &buttons[BUTTON_UP]) {  
			select_previous();
    } else if (button == &buttons[BUTTON_DOWN]) { 
			select_next();
    } else if (button == &buttons[BUTTON_LEFT]) {  
			enter_current(); 
    } else if (button == &buttons[BUTTON_RIGHT]) {
			return_previous();
    } else if (button == &buttons[BUTTON_MIDDLE]) {

    }
    NotifyMenuFromISR();
}

static inline void btn_long_press_cb(void *btn) {
    static TickType_t last_trigger_time = 0;
    TickType_t current_time = xTaskGetTickCount();
    if (current_time - last_trigger_time >= pdMS_TO_TICKS(LONG_PRESS_INTERVAL)) { // 每100ms触发一次
        struct Button* button = (struct Button*) btn;
        if (button == &buttons[BUTTON_UP]) {
            select_previous();
        } else if (button == &buttons[BUTTON_DOWN]) {
            select_next();
        }
        NotifyMenuFromISR();
        last_trigger_time = current_time;
    }
}

