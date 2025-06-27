#include "ui.h"



static void run_task01_cb(void *arg) {
	draw_centered_text("Running Failed");
}

static void run_task02_cb(void *arg) {
	draw_centered_text("Running Failed");
}

static void run_task03_cb(void *arg) {
	draw_centered_text("Running Failed");
}

static void run_task04_cb(void *arg) {
	draw_centered_text("Running Failed");
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
static MenuNode task01;
static MenuNode task02;
static MenuNode task03;
static MenuNode task04;

// 子节点数组
static MenuNode *run_tasks_children[] = { &task01, &task02, &task03, &task04 };
static MenuNode *root_children[] = { &menu_run_tasks, &menu_view_variables, &set_pid_speed, &set_pid_mileage};

float test_1, test_2;
extern float x_value;
extern float y_value;

// 创建变量菜单
menu_variables_t my_variables_1[] = {
    {"x_value", &x_value},
    {"y_value", &y_value},
    {"SpeedKd", &test_1},
		{"Voltage1", &test_1},
		{"Voltage2", &test_1},
    {NULL, NULL}  // 结束标记
};

// 创建变量菜单 （可修改）
menu_variables_t my_variables_2[] = {
    {"SpeedKp", &test_2},
    {"SpeedKi", &test_2},
    {"SpeedKd", &test_2},
		{"Voltage1", &test_2},
		{"Voltage2", &test_2},
    {NULL, NULL}  // 结束标记
};

// 初始化所有节点
static void init_all_menu_nodes(void) {
	// 初始化菜单节点，参数格式：节点指针, 名称, 回调函数, 菜单类型, 父节点, 子节点数量, 子节点数组
	init_menu_node(&menu_root,           "Main Menu",        NULL,             MENU_TYPE_NORMAL,        NULL,            4, root_children);          // 根菜单
	init_menu_node(&menu_run_tasks,      "Run Tasks",        NULL,             MENU_TYPE_NORMAL,        &menu_root,      4, run_tasks_children);     // 一级菜单：运行任务
	
	init_variables_view_node(&menu_view_variables, my_variables_1, 5);
	init_menu_node(&menu_view_variables, "View Variables",   view_variables_cb,MENU_TYPE_VARIABLES_VIEW, &menu_root,      0, NULL);                  // 一级菜单：查看变量
	
	init_variables_view_node(&set_pid_speed, my_variables_2, 5);	
	init_menu_node(&set_pid_speed,       "Set Pid Speed",    NULL,             MENU_TYPE_VARIABLES_MODIFY,        &menu_root,      0, NULL);                  // 一级菜单：设置PID速度

	init_variables_view_node(&set_pid_mileage, my_variables_2, 5);
	init_menu_node(&set_pid_mileage,     "Set Pid Mileage",  NULL,             MENU_TYPE_VARIABLES_MODIFY,        &menu_root,      0, NULL);                  // 一级菜单：设置PID里程
	
	init_menu_node(&task01,              "run Task01",       run_task01_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务1
	init_menu_node(&task02,              "run Task02",       run_task02_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务2
	init_menu_node(&task03,              "run Task03",       run_task03_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务3
	init_menu_node(&task04,              "run Task04",       run_task04_cb,    MENU_TYPE_NORMAL,        &menu_run_tasks, 0, NULL);                // 二级菜单：任务4
}


void menu_init_and_create(void) {
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
    }
    notify_menu_update();
}

// 按钮长按回调函数的实现
static inline void btn_long_press_cb(void *btn) {
    static uint32_t last_trigger_time = 0;  // 上次触发的时间（毫秒）
    uint32_t current_time = HAL_GetTick(); // 获取当前的系统时间（单位：毫秒）
    // 进行时间间隔判断，确保每次的响应时间间隔大于 LONG_PRESS_INTERVAL
    if (current_time - last_trigger_time >= LONG_PRESS_INTERVAL) {
        struct Button* button = (struct Button*) btn;
        // 判断按钮类型，执行相应操作
        if (button == &buttons[BUTTON_UP]) {
            select_previous();  // 上一步操作
        } else if (button == &buttons[BUTTON_DOWN]) {
            select_next();      // 下一步操作
        }
        // 通知菜单进行刷新
        notify_menu_update();
        // 更新上次触发时间
        last_trigger_time = current_time;
    }
}
