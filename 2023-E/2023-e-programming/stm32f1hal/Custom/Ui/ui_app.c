#include "ui.h"
#include "ServoPid.h"

/* =============================================================================
 * 系统变量定义
 * ============================================================================= */
uint8_t task_counter;

// PID参数
static float speed_kp = 1.0f, speed_ki = 0.1f, speed_kd = 0.01f;
static float mileage_kp = 1.5f, mileage_ki = 0.2f, mileage_kd = 0.05f;

// 系统状态参数
static float battery_voltage = 12.0f;
static int motor_speed = 0;
static int servo_angle = 90;

// 外部变量
extern float err_x, err_y;

/* =============================================================================
 * 任务回调函数
 * ============================================================================= */
static void task_find_rect_cb(void *arg) {
	if (task_counter < 4) {
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "Find Rect %d", task_counter);
			show_message(buffer);
			printf("QUESTION0");
			task_counter++;
	} else {
			show_message("Please Reset First");
	}
}

static void task_receive_center_cb(void *arg) {
		show_message("Receive Center");
		printf("QUESTION1");
		start_servo_pid_control();
}

static void task_receive_path_cb(void *arg) {
		show_message("Receive Path");
		printf("QUESTION2");
		start_servo_pid_control();
}

static void task_reset_cb(void *arg) {
    show_message("System Reset");
    printf("RESET");
    task_counter = 0;
}


static void reset_pid_cb(void *arg) {
    speed_kp = 1.0f; speed_ki = 0.1f; speed_kd = 0.01f;
    mileage_kp = 1.5f; mileage_ki = 0.2f; mileage_kd = 0.05f;
    show_message("PID Reset!");
}

/* =============================================================================
 * 变量改变回调函数
 * ============================================================================= */
void on_speed_pid_change(void *var_ptr, VariableType type) {
    printf("Speed PID updated: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", speed_kp, speed_ki, speed_kd);
}

void on_mileage_pid_change(void *var_ptr, VariableType type) {
    printf("Mileage PID updated: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", mileage_kp, mileage_ki, mileage_kd);
}


/* =============================================================================
 * 变量数组定义
 * ============================================================================= */
// 系统状态变量（只读）
menu_variable_t system_status_vars[] = {
    MENU_VAR_READONLY("err_x", &err_x, VAR_TYPE_FLOAT),
    MENU_VAR_READONLY("err_y", &err_y, VAR_TYPE_FLOAT),
    MENU_VAR_READONLY("Battery", &battery_voltage, VAR_TYPE_FLOAT),
    MENU_VAR_READONLY("Motor RPM", &motor_speed, VAR_TYPE_INT),
    MENU_VAR_READONLY("Servo Ang", &servo_angle, VAR_TYPE_INT),
    MENU_VAR_END
};

// 速度PID参数
menu_variable_t speed_pid_vars[] = {
    MENU_VAR_PID("Speed Kp", &speed_kp),
    MENU_VAR_PID("Speed Ki", &speed_ki),
    MENU_VAR_PID("Speed Kd", &speed_kd),
    MENU_VAR_END
};

// 里程PID参数
menu_variable_t mileage_pid_vars[] = {
    MENU_VAR_PID("Mile Kp", &mileage_kp),
    MENU_VAR_PID("Mile Ki", &mileage_ki),
    MENU_VAR_PID("Mile Kd", &mileage_kd),
    MENU_VAR_END
};

// 系统设置变量
menu_variable_t system_settings_vars[] = {
    MENU_VAR_FLOAT_RANGE("Battery", &battery_voltage, 10.0f, 15.0f, 0.1f),
    MENU_VAR_INT_RANGE("Motor RPM", &motor_speed, -1000, 1000, 50),
    MENU_VAR_INT_RANGE("Servo Ang", &servo_angle, 0, 180, 5),
    MENU_VAR_END
};

/* =============================================================================
 * 菜单创建
 * ============================================================================= */
void menu_init_and_create(void) {
    // 使用链式构建宏创建菜单结构
    MENU_BUILDER_START(main_menu, "Main Menu");
    
    // 任务执行菜单
    ADD_SUBMENU(main_menu, tasks_menu, "Run Tasks", NULL);
    ADD_ACTION(tasks_menu, task1, "Find Rect", task_find_rect_cb);
    ADD_ACTION(tasks_menu, task2, "Recv Center", task_receive_center_cb);
    ADD_ACTION(tasks_menu, task3, "Recv Path", task_receive_path_cb);
    ADD_ACTION(tasks_menu, task_reset, "Reset", task_reset_cb);
    
    // 系统状态菜单
    ADD_SUBMENU(main_menu, status_menu, "System", NULL);
    ADD_VAR_VIEW(status_menu, status_view, "View Status", system_status_vars);
    
    // PID参数菜单
    ADD_SUBMENU(main_menu, pid_menu, "PID Tuning", NULL);
    ADD_VAR_MODIFY(pid_menu, speed_pid, "Speed PID", speed_pid_vars);
    ADD_VAR_MODIFY(pid_menu, mileage_pid, "Mileage PID", mileage_pid_vars);
    ADD_ACTION(pid_menu, reset_pid, "Reset PID", reset_pid_cb);
    
    // 设置变量改变回调
    set_variable_change_callback(&speed_pid, 0, on_speed_pid_change);
    set_variable_change_callback(&speed_pid, 1, on_speed_pid_change);
    set_variable_change_callback(&speed_pid, 2, on_speed_pid_change);
    
    set_variable_change_callback(&mileage_pid, 0, on_mileage_pid_change);
    set_variable_change_callback(&mileage_pid, 1, on_mileage_pid_change);
    set_variable_change_callback(&mileage_pid, 2, on_mileage_pid_change);
    
    // 初始化按钮和创建菜单
    create_oled_menu(&main_menu);
}
