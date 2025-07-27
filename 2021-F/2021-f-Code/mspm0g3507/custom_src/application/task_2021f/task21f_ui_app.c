#include "common_defines.h"
#if (CURRENT_TASK_TYPE == TASK_TYPE_21F)
#include "task21f_config.h"
#include "ui.h"
#include "log_config.h"
#include "log.h"
#include "common_include.h"


/* =============================================================================
 * 任务配置和回调函数
 * ============================================================================= */
bool task_running_flag = false;

void reset_cam(void) {
	maix_cam.num = 0;
	maix_cam.cmd = 0;
}

bool wait_cam_num(void) {
		if (maix_cam.num != 0) {
			return true;
		} else {
			return false;
		}
}

bool wait_cam_cmd(void) {
		if (maix_cam.cmd != 0) {
			return true;
		} else {
			return false;
		}
}
void send_start_cmd(void) {
		camera_send_data((uint8_t *)"START", 5);
}

void send_number_cmd(void) {
		camera_send_data((uint8_t *)"NUMBER", 6);
}

void send_track_cmd(void) {
		camera_send_data((uint8_t *)"TRACK", 5);
}

/* =============================================================================
 * 优雅的路径模式封装函数
 * ============================================================================= */

/* 基础移动模式 */
void navigate_detour_right(float track_distance, float straight_distance) {
    car_add_track(track_distance);
    car_add_turn(-90);
    car_add_straight(straight_distance);
    car_add_turn(90);
    car_add_straight(straight_distance);
    car_add_turn(180);
}

void navigate_detour_left(float track_distance, float straight_distance) {
    car_add_track(track_distance);
    car_add_turn(90);
    car_add_straight(straight_distance);
    car_add_turn(-90);
    car_add_straight(straight_distance);
    car_add_turn(180);
}

/* 终点路径模式 */
void navigate_to_final_position_via_right(void) {
    car_add_turn(-90);
    car_add_track(80);
    car_add_turn(-180);
    car_add_track(80);
    car_add_track(80);
    car_add_track(80);
    car_add_turn(0);
}

void navigate_to_final_position_via_left(void) {
    car_add_turn(90);
    car_add_track(80);
    car_add_turn(-180);
    car_add_track(80);
    car_add_track(80);
    car_add_track(80);
    car_add_turn(0);
}

/* 停车与调整模式 */
void stop_at_marker_and_adjust(float reverse_distance, float target_angle) {
    car_add_move_until_stop_mark(CAR_STATE_TRACK);
    car_add_straight(reverse_distance);
    car_add_turn(target_angle);
}

/* 命令发送与等待模式 */
void send_command_and_await_response(void (*command_sender)(void), bool (*response_checker)(void)) {
    car_add_function(command_sender);
    car_add_wait_func_true(response_checker);
}

/* 检测与路径规划模式 */
void execute_detection_and_planning(void (*command_sender)(void), bool (*response_checker)(void), uint32_t delay_ms) {
    car_add_delay(delay_ms);
    send_command_and_await_response(command_sender, response_checker);
}

/* 病房访问路径模式 */
void visit_ward_with_left_approach(float approach_distance, float room_depth) {
    car_add_move_until_stop_mark(CAR_STATE_TRACK);
    navigate_detour_left(approach_distance, room_depth);
    car_add_track(60);
    car_add_turn(0);
}

void visit_ward_with_right_approach(float approach_distance, float room_depth) {
    car_add_move_until_stop_mark(CAR_STATE_TRACK);
    navigate_detour_right(approach_distance, room_depth);
    car_add_track(60);
    car_add_turn(0);
}

/* 通用检测位置准备 */
void prepare_for_detection_at_position(float track_distance, float reverse_distance) {
    car_add_track(track_distance);
    stop_at_marker_and_adjust(reverse_distance, 0);
}

/* 远程病房第二次检测的完整路径 */
void complete_remote_ward_path_right_final(void) {
    car_add_straight(40);
    car_add_turn(180);
    car_add_straight(46);
    car_add_turn(0);
    car_add_straight(46);
    navigate_to_final_position_via_right();
}

void complete_remote_ward_path_left_final(void) {
    car_add_straight(40);
    car_add_turn(0);
    car_add_straight(46);
    car_add_turn(180);
    car_add_straight(46);
    navigate_to_final_position_via_right();
}

void complete_remote_ward_path_left_alt_final(void) {
    car_add_straight(40);
    car_add_turn(0);
    car_add_straight(46);
    car_add_turn(180);
    car_add_straight(36);
    navigate_to_final_position_via_left();
}

void complete_remote_ward_path_right_alt_final(void) {
    car_add_straight(40);
    car_add_turn(180);
    car_add_straight(46);
    car_add_turn(0);
    car_add_straight(46);
    navigate_to_final_position_via_left();
}

/* =============================================================================
 * 重构后的主要任务函数
 * ============================================================================= */

void execute_remote_ward_second_detection_type_a(void) {
    car_add_function(send_track_cmd);
		if (maix_cam.cmd == CAM_CMD_GO_LEFT) { //远端病房第二次识别在左边（第一次为左的情况）
        car_add_function(reset_cam);
        complete_remote_ward_path_right_final();
    } else if (maix_cam.cmd == CAM_CMD_GO_RIGHT) { //远端病房第二次识别在右边 （第一次为左的情况）
        car_add_function(reset_cam);
        complete_remote_ward_path_left_final();
    }
}

void execute_remote_ward_second_detection_type_b(void) {
    car_add_function(send_track_cmd);
    if (maix_cam.cmd == CAM_CMD_GO_LEFT) { //远端病房第二次识别在左边（第一次为右的情况）
        car_add_function(reset_cam);
        complete_remote_ward_path_left_alt_final();
    } else if (maix_cam.cmd == CAM_CMD_GO_RIGHT) {  //远端病房第二次识别在右边 （第一次为右的情况）
        car_add_function(reset_cam);
        complete_remote_ward_path_right_alt_final();
    }
}

void execute_remote_ward_first_detection(void) {
    car_add_function(send_track_cmd);
    if (maix_cam.cmd == CAM_CMD_GO_LEFT) { //远端病房第一次识别在左边
        car_add_function(reset_cam);
        car_add_straight(43);
        car_add_turn(90);
        stop_at_marker_and_adjust(-10, 90);
        execute_detection_and_planning(send_number_cmd, wait_cam_cmd, 1500);
        car_add_function(execute_remote_ward_second_detection_type_a);
    } else if (maix_cam.cmd == CAM_CMD_GO_RIGHT) { //远端病房第一次识别在右边
        car_add_function(reset_cam);
        car_add_straight(43);
        car_add_turn(-90);
        stop_at_marker_and_adjust(-10, -90);
        execute_detection_and_planning(send_number_cmd, wait_cam_cmd, 1500);
        car_add_function(execute_remote_ward_second_detection_type_b);
    }
}

void execute_middle_ward_detection(void) {
    car_add_function(send_track_cmd);
    
    if (maix_cam.cmd == CAM_CMD_GO_LEFT) { //去中端病房左边识别成功并且回家
        car_add_function(reset_cam);
        navigate_detour_left(35, 46);
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        car_add_track(100);
        car_add_turn(0);
    } else if (maix_cam.cmd == CAM_CMD_GO_RIGHT) {  //去中端病房右边识别成功并且回家
        car_add_function(reset_cam);
        navigate_detour_right(35, 46);
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        car_add_track(100);
        car_add_turn(0);
    } else if (maix_cam.cmd == CAM_CMD_NONE) {  //去远端病房
        car_add_function(reset_cam);
        prepare_for_detection_at_position(40, -10);
        execute_detection_and_planning(send_number_cmd, wait_cam_cmd, 1500);
        car_add_function(execute_remote_ward_first_detection);
    }
}

void execute_near_ward_detection(void) {
    car_add_function(send_track_cmd);
    
    if (maix_cam.num == 1) { //1号 近端病房写死
        car_add_function(reset_cam);
        visit_ward_with_left_approach(30, 46);
    } else if (maix_cam.num == 2) { //2号 近端病房写死
        car_add_function(reset_cam);
        visit_ward_with_right_approach(30, 46);
    } else {	//去中端病房
        car_add_function(reset_cam);
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        prepare_for_detection_at_position(30, -10);
        execute_detection_and_planning(send_number_cmd, wait_cam_cmd, 1500);
        car_add_function(execute_middle_ward_detection);
    }
}

static void initialize_task_sequence(void) {
    car_path_init();
    send_command_and_await_response(send_start_cmd, wait_cam_num);
    car_add_function(execute_near_ward_detection);
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
    run_task("Running Task 01", &task_running_flag, initialize_task_sequence);
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
#elif (CURRENT_IMU == IMU660RA_GYRO)
extern Attitude_module attitude;
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
#elif (CURRENT_IMU == IMU660RA_GYRO)

		MENU_VAR_READONLY("Yaw", &attitude.pose_module.data.yaw, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Pitch", &attitude.pose_module.data.pit, VAR_TYPE_FLOAT),
		MENU_VAR_READONLY("Roll", &attitude.pose_module.data.rol, VAR_TYPE_FLOAT),
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
    ADD_SUBMENU(main_menu, tasks1_menu, "Run 21f App", NULL);
    ADD_ACTION(tasks1_menu, _25_task1, "Run 21f Task1", run_task01_cb);
	
		ADD_SUBMENU(main_menu, PlayMusic, "Play Music", NULL);
		ADD_ACTION(PlayMusic, music1, "ChunRiYing", play_music_1_cb);
    ADD_ACTION(PlayMusic, music2, "TianKongZhiCheng", play_music_2_cb);
		ADD_ACTION(PlayMusic, stop_music, "StopMusic", stop_music_cb);
		
		ADD_SUBMENU(main_menu, status_menu, "System Status", NULL);
		ADD_VAR_VIEW(status_menu, gyro_status_view, "Gyro Status", gyro_vars);
		ADD_VAR_VIEW(status_menu, car_status_view, "Car Status", car_vars);
    create_oled_menu(&main_menu);
}

#endif