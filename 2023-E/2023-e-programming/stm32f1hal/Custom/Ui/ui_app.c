#include "ui.h"
#include "ServoPid.h"
#include "pid.h"

/* =============================================================================
 * 系统变量定义
 * ============================================================================= */
uint8_t task_counter;

// 外部变量
extern float err_x, err_y;
extern PID_Controller_t x_pid;
extern PID_Controller_t y_pid;

/* =============================================================================
 * 任务回调函数
 * ============================================================================= */
static void task_manual_marking_cb(void *arg) {
    if (task_counter < 4) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Manual Marking %d", task_counter);
        show_message(buffer);
        printf("MANUAL_MARKING");
        task_counter++;
    } else {
        show_message("Please Reset First");
    }
}

static void task_reset_manual_cb(void *arg) {
    show_message("Reset Manual Mark");
    printf("RESET_MANUAL_MARK");
    task_counter = 0;
}

static void task_track_center_cb(void *arg) {
    show_message("Track Center");
    printf("TRACK_POINT_CENTER");
		PID_SetParams(&x_pid,0.01,0.0,0.1);
		PID_SetDeadzone(&x_pid, 2);
		PID_SetOutputLimit(&x_pid, 1, -1);
		PID_SetParams(&y_pid,0.01,0.0,0.1);
		PID_SetDeadzone(&y_pid, 2);
		PID_SetOutputLimit(&y_pid, 1, -1);
    start_servo_pid_control();
}

static void task_track_path_cb(void *arg) {
    show_message("Track Path Point");
    printf("TRACK_PATH_POINT");
		PID_SetParams(&x_pid,0.01,0.0,0.1);
		PID_SetDeadzone(&x_pid, 5);
		PID_SetOutputLimit(&x_pid, 1, -1);

		PID_SetParams(&y_pid,0.01,0.0,0.1);
		PID_SetDeadzone(&y_pid, 5);
		PID_SetOutputLimit(&y_pid, 1, -1);
    start_servo_pid_control();
}

static void task_detect_rectangle(void *arg) {
	show_message("Detect Rectangle");
	printf("DETECT_RECTANGLE");
	PID_SetParams(&x_pid,0.01,0.0,0.07);
	PID_SetDeadzone(&x_pid, 3);
	PID_SetOutputLimit(&x_pid, 1.4, -1.4);
	PID_SetParams(&y_pid,0.006,0.0,0.07);
	PID_SetDeadzone(&y_pid, 3);
	PID_SetOutputLimit(&y_pid, 0.4, -0.4);
  start_servo_pid_control();
}

static void task_stop_cb(void *arg) {
    show_message("Stop Control");
    stop_servo_pid_control();
}

static void task_start_cb(void *arg) {
    show_message("Start Control");
    start_servo_pid_control();
}

/* =============================================================================
 * 变量数组定义
 * ============================================================================= */
// 系统状态变量（只读）
menu_variable_t system_status_vars[] = {
    MENU_VAR_READONLY("err_x", &err_x, VAR_TYPE_FLOAT),
    MENU_VAR_READONLY("err_y", &err_y, VAR_TYPE_FLOAT),
    MENU_VAR_END
};

/* =============================================================================
 * 菜单创建
 * ============================================================================= */
void menu_init_and_create(void) {
    // 使用链式构建宏创建菜单结构
    MENU_BUILDER_START(main_menu, "Main Menu");
    
    // 任务执行菜单
    ADD_SUBMENU(main_menu, tasks_menu, "Laser Control", NULL);
    ADD_ACTION(tasks_menu, task1, "Manual Marking", task_manual_marking_cb);
    ADD_ACTION(tasks_menu, task2, "Reset Manual", task_reset_manual_cb);
    ADD_ACTION(tasks_menu, task3, "Track Center", task_track_center_cb);
    ADD_ACTION(tasks_menu, task4, "Track Path", task_track_path_cb);
		ADD_ACTION(tasks_menu, task5, "Detect Rectangle", task_detect_rectangle);
    ADD_ACTION(tasks_menu, task6, "Stop Control", task_stop_cb);
		ADD_ACTION(tasks_menu, task7, "Start Control", task_start_cb);
		
    // 系统状态菜单
    ADD_SUBMENU(main_menu, status_menu, "System Status", NULL);
    ADD_VAR_VIEW(status_menu, status_view, "View Errors", system_status_vars);
    
    // 初始化按钮和创建菜单
    create_oled_menu(&main_menu);
}
