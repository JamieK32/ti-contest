#include "common_defines.h"
#if (CURRENT_TASK_TYPE == TASK_TYPE_21F)
#include "car_controller.h"
#include "attitude_algorithm.h"

#define MAX_DISTANCE 						255
#define DISTANCE_THRESHOLD_CM 	1
#define ANGLE_THRESHOLD_DEG		  1
#define TRACK_DEFAULT_SPEED 		35
#define DETECTED_WHITE_COUNT		2
#define ARC_LENGTH 							120

// 定义 encoder 结构体实例
encoder_t encoder = {0};

static inline float calculate_angle_error(float target, float current);
static const uint16_t stop_mark_table_8bit[];
static inline bool is_in_table(const uint16_t *table, uint16_t table_size, uint16_t data);
static const uint8_t STOP_MARK_TABLE_SIZE;

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
#elif CURRENT_IMU == IMU660RA_GYRO
		extern Attitude_module attitude;
		return attitude.pose_module.data.yaw;
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
 * @brief 控制小车原地旋转指定角度
 * @param angle 目标角度（单位：度）
 * @return true 表示已达到目标角度，false 表示尚未达到
 */
bool spin_turn(float angle) {
    if (car.state != CAR_STATE_TURN) {
        car.state = CAR_STATE_TURN;
        car.target_angle = angle;
        car.turn_initialized = false;
        car_reset();
    }
    
    #if USE_ANGLE_SENSOR
    float current_angle = get_yaw();
    float angle_error = calculate_angle_error(car.target_angle, current_angle);
    if (fabsf(angle_error) <= ANGLE_THRESHOLD_DEG) {
        car_reset();
        car.state = CAR_STATE_STOP;
        return true;
    }
    #else
    // 基于码盘判断转向完成 - 使用左侧轮子的距离
    float target_distance = fabsf(car.target_angle) * M_PI / 180.0f * WHEEL_BASE_CM / 2.0f;
    float left_distance = 0;
    for (int i = 0; i < motor_count / 2; i++) {
        left_distance += fabsf(encoder.distance_cm[i]);
    }
    left_distance /= (motor_count / 2);
    
    if (left_distance >= target_distance) {
        car_reset();
        car.state = CAR_STATE_STOP;
				car.target_angle = 0;
        return true;
    }
    #endif
    
    return false;
}

uint8_t global_stop_mark_count = 1;

/**
 * @brief 移动直到检测到指定条件的线
 * @param move_state 小车的移动状态
 * @param l_state 目标线状态
 * @return true 表示达到目标条件，false 表示未达到
 */
bool car_move_until(CAR_STATES move_state, LINE_STATES l_state) {
		static uint8_t stop_mark_count, white_count;  // 新增：停止标记计数器
	    // 初始化移动状态
    if (car.state == CAR_STATE_STOP) {
        car.state = move_state;
        if (move_state == CAR_STATE_GO_STRAIGHT) {
            car_reset();
            car.target_mileage_cm = MAX_DISTANCE;
        }
    }
		 uint16_t sensor_data = gray_read_byte();
		
    // 根据目标状态判断
    if (l_state == UNTIL_BLACK_LINE) {
        // 检测到黑线（sensor_data != 0 表示有传感器检测到黑色）
        if (sensor_data != 0) {
            car.state = CAR_STATE_STOP;
            set_alert_count(1);
            start_alert();
            car_reset();
            white_count = 0;
            return true;
        }
    } 
    else if (l_state == UNTIL_WHITE_LINE) {
        // 检测到全白（sensor_data == 0 表示所有传感器都是白色）
        if (sensor_data == 0) {
            white_count++;
            if (white_count >= DETECTED_WHITE_COUNT && get_mileage_cm() >= ARC_LENGTH) {
                car.state = CAR_STATE_STOP;
                set_alert_count(1);
                start_alert();
                car_reset();
                white_count = 0;
                return true;
            }
        } else {
            white_count = 0;
        }
    } 
		
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

void update_straight_control(void)
{
    /*--------- 1. 里程 PID（输出基础速度） ---------*/
    float base_speed = PID_Calculate(car.target_mileage_cm,
                                     get_mileage_cm(),
                                     &mileagePid);          // cm/s

    /*--------- 2. 角度 PID（输出修正量） ---------*/
#if USE_ANGLE_SENSOR
		float yaw_err   = calculate_angle_error(car.target_angle, get_yaw());
    float correction = PID_Calculate(0.0f, 
                                     yaw_err,  
                                     &straightPid); 
#else
    float correction = 0.0f;
#endif

    /*--------- 3. 计算左右轮目标速度 ---------*/
    float left  = base_speed + correction;
    float right = base_speed - correction;

    /* motor_count 支持 2 或 4 */            /* 0,1 = 左轮；2,3 = 右轮  */
    for (int i = 0; i < motor_count; ++i)
        car.target_speed[i] = (i < motor_count / 2) ? left : right;
}




void update_track_control(void) {
		float error = gray_get_position();
    float correction = PID_Calculate(0.0f, error, &trackPid);
    for (int i = 0; i < motor_count; ++i)
        car.target_speed[i] = (i < motor_count / 2) ? car.track_speed + correction: car.track_speed - correction;
}


void update_turn_control(void) {
    #if USE_ANGLE_SENSOR
        float current_angle = get_yaw();
        float angle_error = calculate_angle_error(car.target_angle, current_angle);
        float output = PID_Calculate(0.0f, 
                                     angle_error,  
                                     &anglePid); 
        for (int i = 0; i < motor_count; ++i)
            car.target_speed[i] = (i < motor_count / 2) ? output : -output;
    #else    
        // 基于码盘的转向逻辑
        static float target_distance = 0;
        
        // 首次进入时计算目标距离
        if (!car.turn_initialized) {
            // 转向弧长 = 角度(弧度) × 轮距 / 2
            target_distance = fabsf(car.target_angle) * M_PI / 180.0f * WHEEL_BASE_CM / 2.0f;
            car.turn_initialized = true;
        }
        
        // 计算左侧轮子的平均行驶距离（取绝对值）
        float left_distance = 0;
        for (int i = 0; i < motor_count / 2; i++) {
            left_distance += fabsf(encoder.distance_cm[i]);
        }
        left_distance /= (motor_count / 2);
        
        // PID控制
        float output = PID_Calculate(target_distance, left_distance, &anglePid);
        
        // 设置左右轮速度（根据转向方向）
        float left_speed = (car.target_angle > 0) ? output : -output;   // 右转时左轮正转
        float right_speed = (car.target_angle > 0) ? -output : output;  // 右转时右轮反转
        
        for (int i = 0; i < motor_count; ++i)
            car.target_speed[i] = (i < motor_count / 2) ? left_speed : right_speed;
    #endif 
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
	PID_Reset(&mileagePid);
	PID_Reset(&straightPid);
	PID_Reset(&anglePid);
	PID_Reset(&trackPid);
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


static inline float calculate_angle_error(float target, float current) {
    float error = target - current;
    
    if (error > 180.0f) {
        error -= 360.0f;
    } else if (error < -180.0f) {
        error += 360.0f;
    }
    
    return error;
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
    
    // 连续5个传感器为1的情况
    0x1F,  // 0b00011111 (bit 0-4) 连续5个
    0x3E,  // 0b00111110 (bit 1-5) 连续5个
    0x7C,  // 0b01111100 (bit 2-6) 连续5个
    0xF8,  // 0b11111000 (bit 3-7) 连续5个
    
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
#endif 