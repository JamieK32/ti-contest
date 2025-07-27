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

// 简单的封装函数 - 只抽取重复逻辑，不改变架构

/* =============================================================================
 * 通用路径模式封装
 * ============================================================================= */

void add_right_path(float track_dist, float straight_dist) {
    car_add_track(track_dist);
    car_add_turn(-90);
    car_add_straight(straight_dist);
    car_add_turn(90);
    car_add_straight(straight_dist);
    car_add_turn(180);
}

void add_left_path(float track_dist, float straight_dist) {
    car_add_track(track_dist);
    car_add_turn(90);
    car_add_straight(straight_dist);
    car_add_turn(-90);
    car_add_straight(straight_dist);
    car_add_turn(180);
}

// 发送命令并等待响应的组合
void add_send_and_wait(void (*send_func)(void), bool (*wait_func)(void)) {
    car_add_function(send_func);
    car_add_wait_func_true(wait_func);
}

// 到停止标记并后退调整
void add_stop_and_back(float back_dist, float taget_angle) {
    car_add_move_until_stop_mark(CAR_STATE_TRACK);
    car_add_straight(back_dist);
    car_add_turn(taget_angle);
}

void task25_path4(void) { //远端病房第二次检测尚未完成
	car_add_function(send_track_cmd);
	if (maix_cam.cmd == CAM_CMD_GO_LEFT) { //远端病房第二次识别到目标在左边

	} else if (maix_cam.cmd == CAM_CMD_GO_RIGHT) {  //远端病房第二次识别到目标在右边

	}
}

void task21f_path3(void) { //远端病房第一次检测
	 car_add_function(send_track_cmd);
	 if (maix_cam.cmd == CAM_CMD_GO_LEFT) { //远端病房第一次识别到目标在左边
			car_add_function(reset_cam);
			car_add_track(35);
			car_add_turn(90);
			add_stop_and_back(-10, 90);
			car_add_delay(1500);
			add_send_and_wait(send_number_cmd, wait_cam_cmd); 
			car_add_function(task25_path4);
	 } else if (maix_cam.cmd == CAM_CMD_GO_RIGHT) { //远端病房第一次识别到目标在右边
			car_add_function(reset_cam);
			car_add_track(35);
			car_add_turn(-90);
			add_stop_and_back(-10, -90);
			car_add_delay(1500);
			add_send_and_wait(send_number_cmd, wait_cam_cmd); 
			car_add_function(task25_path4);
	 } 
}

/* =============================================================================
 * 使用封装后的代码
 * ============================================================================= */

void task21f_path2(void) { //中端病房第一次检测
    car_add_function(send_track_cmd);
    
    if (maix_cam.cmd == CAM_CMD_GO_LEFT) {
        car_add_function(reset_cam);
        add_left_path(35, 46);  // 替代重复的左转路径
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        car_add_track(100);
        car_add_turn(0);
        
    } else if(maix_cam.cmd == CAM_CMD_GO_RIGHT) {
        car_add_function(reset_cam);
        add_right_path(35, 46);  // 替代重复的右转路径
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        car_add_track(100);
        car_add_turn(0);
        
    } else if(maix_cam.cmd == CAM_CMD_NONE) {  //没有识别到去远端病房
        car_add_function(reset_cam);
        car_add_track(40);
        add_stop_and_back(-10, 0);  // 替代重复的停止后退
        car_add_delay(1500);
        add_send_and_wait(send_number_cmd, wait_cam_cmd);  // 替代发送等待
        car_add_function(task21f_path3);
    }   
}

void task21f_path1(void) { //近端病房写死
    car_add_function(send_track_cmd);
    
    if (maix_cam.num == 1) {
        car_add_function(reset_cam);
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        add_left_path(30, 46);  // 使用封装
        car_add_track(60);
        car_add_turn(0);
        
    } else if (maix_cam.num == 2) {
        car_add_function(reset_cam);
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        add_right_path(30, 46);  // 使用封装
        car_add_track(60);
        car_add_turn(0);
        
    } else {
        car_add_function(reset_cam);
        car_add_move_until_stop_mark(CAR_STATE_TRACK);
        car_add_track(30);
        add_stop_and_back(-10, 0);  // 使用封装
        car_add_delay(1500);
        add_send_and_wait(send_number_cmd, wait_cam_cmd);  // 使用封装
        car_add_function(task21f_path2);
    }
}

// 简化的setup函数
static void setup_task1(void) {
    car_path_init();
    add_send_and_wait(send_start_cmd, wait_cam_num);  // 使用封装
    car_add_function(task21f_path1);
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