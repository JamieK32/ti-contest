#include "car_controller.h"
#include "vl53l1_read.h"

#define MAX_DISTANCE 						255
#define DISTANCE_THRESHOLD_CM 	1
#define ANGLE_THRESHOLD_DEG		  1
#define TRACK_DEFAULT_SPEED 		45
#define TOF_TARGET_DISTANCE     240

// 定义 encoder 结构体实例
encoder_t encoder = {0};

static inline float calculate_angle_error(float target, float current);
static inline bool is_in_table(const uint16_t *table, uint16_t table_size, uint16_t data);
static const uint8_t STOP_MARK_TABLE_SIZE;
static const uint16_t stop_mark_table_8bit[];

bool is_outer_track = true;
uint8_t global_stop_mark_count = 0;
uint16_t current_distance;
float track_default_speed = TRACK_DEFAULT_SPEED;

car_t car = {
    .state = CAR_STATE_STOP,
		.track_speed = TRACK_DEFAULT_SPEED,
};

float get_yaw(void) {
#if CURRENT_IMU == WIT_GYRO
    return jy61p.yaw;
#elif CURRENT_IMU == MPU6050_GYRO
		extern float yaw;
		return yaw;
#else 
		return 0;
#endif 
}

void car_task(void) {
		update_encoder();
    if (car.state == CAR_STATE_GO_STRAIGHT) {
        update_straight_control();
    } else if (car.state == CAR_STATE_TURN) {
        update_turn_control();
    } else if (car.state == CAR_STATE_TRACK) {
				update_track_control();
		} else if (car.state == CAR_STATE_STOP) {
				update_oled_debug_information();
				car_set_base_speed(0);
		}
    update_speed_pid();
}

/**
 * @brief 控制小车直线行驶指定里程
 * @param mileage 目标里程（单位：厘米）
 * @return true 表示已达到目标里程，false 表示尚未达到
 */
bool car_move_cm(float mileage, CAR_STATES move_state) {
    if (car.state != move_state) {
        car.state = move_state;
			  car_reset();
				car.target_mileage_cm = mileage;
    }
    float current_mileage = get_mileage_cm();
    if (fabsf(car.target_mileage_cm - current_mileage) <= DISTANCE_THRESHOLD_CM) {
				car_reset();
        car.state = CAR_STATE_STOP;
        return true; 
    }
    return false; 
}


/**
 * @brief 移动直到检测到指定条件的线
 * @param move_state 小车的移动状态
 * @param l_state 目标线状态
 * @return true 表示达到目标条件，false 表示未达到
 */
bool car_move_until(CAR_STATES move_state, LINE_STATES l_state) {
		static uint8_t stop_mark_count = 0;  // 新增：停止标记计数器
	    // 初始化移动状态
    if (car.state == CAR_STATE_STOP) {
        car.state = move_state;
        if (move_state == CAR_STATE_GO_STRAIGHT) {
            car_reset();
            car.target_mileage_cm = MAX_DISTANCE;
        }
    }
		 uint16_t sensor_data = gray_read_byte();
		
		if (l_state == UNTIL_STOP_MARK) {
        // 检测到停止标记
        if (is_in_table(stop_mark_table_8bit, STOP_MARK_TABLE_SIZE, sensor_data)) {
            stop_mark_count++;
            if (stop_mark_count >= global_stop_mark_count) {  // 累加到2次才返回真
                car.state = CAR_STATE_STOP;
                set_alert_count(1);
                start_alert();
                car_reset();
                stop_mark_count = 0;  // 重置计数器
                return true;
            }
        } else {
            stop_mark_count = 0;  // 如果没有检测到停止标记，重置计数器
        }
    }
		
		return false;
}

void car_init(void) {
    encoder_application_init();
    motor_init();
		car_pid_init();
		car_debug_init();
		car_reset();
}

void update_encoder(void) {
    for (int i = 0; i < motor_count; i++) {
        encoder.counts[i] = encoder_manager_read_and_reset(&robot_encoder_manager, i);
        encoder.rpms[i] = encoder.counts[i] * CIRCLE_TO_RPM / PULSE_NUM_PER_CIRCLE;
        encoder.cmps[i] = encoder.rpms[i] * RPM_TO_CMPS;
				encoder.distance_cm[i] += encoder.cmps[i] * TIME_INTERVAL_S;
    }
}

// PID速度控制更新函数
void update_speed_pid(void) {
    float outputs[motor_count];
    int pwm_outputs[motor_count];
    for (int i = 0; i < motor_count; i++) {
        outputs[i] = PID_Calculate(car.target_speed[i], 
                                  encoder.cmps[i], 
                                  &speedPid[i]);
        pwm_outputs[i] = (int)outputs[i];
    }
    motor_set_pwms(pwm_outputs);
}



    
void update_track_control(void) {
		float target_speed = track_default_speed;
		VL53L1_GetDistance(&current_distance);
    if (current_distance > 0) {
        // 有效距离测量 - 修改参数顺序，让距离大时输出为正
        float pid_output = PID_Calculate(current_distance, TOF_TARGET_DISTANCE, &followPid);
        target_speed = track_default_speed + pid_output;
    } else {
        // 距离测量失败，保持当前目标速度或缓慢回到默认速度
        target_speed = target_speed * 0.95f + track_default_speed * 0.05f;
    }
    
    // 渐进式调整实际速度，避免突变
    car.track_speed = car.track_speed * 0.8f + target_speed * 0.2f;
		float error = gray_get_position_22c_ti_contest(is_outer_track);
    float correction = PID_Calculate(0.0f, error, &trackPid);
    for (int i = 0; i < motor_count; ++i)
        car.target_speed[i] = (i < motor_count / 2) ? car.track_speed + correction: car.track_speed - correction;
}



float get_mileage_cm(void) {
    float output = 0;
    for (int i = 0; i < motor_count; i++) {
        output += encoder.distance_cm[i];
    }
    return output / motor_count;
}

void car_reset(void) {
	int pwms[motor_count];
	for (int i = 0; i < motor_count; i++) {
		car.target_speed[i] = 0;
		pwms[i] = 0;
		PID_Reset(&speedPid[i]);
		encoder.distance_cm[i] = 0;
	}
	car.target_mileage_cm = 0;
	PID_Reset(&trackPid);
	PID_Reset(&followPid);
	motor_set_pwms(pwms);
}

void car_set_track_speed(float speed) {
	car.track_speed = speed;
}

void car_set_base_speed(float speed) {
	for (int i = 0; i < motor_count; i++) {
		car.target_speed[i] = 0;
	}
}


static inline bool is_in_table(const uint16_t *table, uint16_t table_size, uint16_t data) {
    for (int i = 0; i < table_size; i++) {
        if (table[i] == data) {
						return true;
        }
    }
		return false;
}

static const uint16_t stop_mark_table_8bit[] = {
    
    // 连续6个传感器为1的情况
    0x3F,  // 0b00111111 (bit 0-5) 连续6个
    0x7E,  // 0b01111110 (bit 1-6) 连续6个
    0xFC,  // 0b11111100 (bit 2-7) 连续6个
    
    // 连续7个传感器为1的情况
    0x7F,  // 0b01111111 (bit 0-6) 连续7个
    0xFE,  // 0b11111110 (bit 1-7) 连续7个
    
    // 连续8个传感器为1的情况
    0xFF,  // 0b11111111 (bit 0-7) 连续8个
};

static const uint8_t STOP_MARK_TABLE_SIZE = (sizeof(stop_mark_table_8bit) / sizeof(stop_mark_table_8bit[0]));

/**
	保留接口
 */
void update_turn_control(void) {

}

/**
	保留接口
 */
void update_straight_control(void)
{

}

/**
	保留接口
 */
bool spin_turn(float angle) {
		return false;
}