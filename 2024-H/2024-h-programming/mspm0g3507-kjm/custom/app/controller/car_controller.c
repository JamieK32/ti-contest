#include "car_controller.h"
#include "encoder_app.h"
//#include "log_config.h"
#include "log.h"
#include "wit_jyxx.h"
#include "gray_detection_app.h"
#include "alert.h"
#include "car_pid.h"
#include "car_debug.h"
#include "_74hc595.h"

#define USE_ANGLE_SENSOR 1
#define DISTANCE_THRESHOLD_CM 1
#define ANGLE_THRESHOLD_DEG 1
#define TRACK_NUM 12
#define TRACK_BASE_SPEED 40

// 定义 encoder 结构体实例
encoder_t encoder = {0};

car_t car = {
    .state = CAR_STATE_STOP,
	
};

static inline float calculate_angle_error(float target, float current) {
    float error = target - current;
    
    // 将误差限制在 -180° 到 +180° 范围内（选择最短路径）
    if (error > 180.0f) {
        error -= 360.0f;
    } else if (error < -180.0f) {
        error += 360.0f;
    }
    
    return error;
}

float get_yaw(void) {
    return jy901s.yaw;
}

void car_task(void) {
    if (car.state == CAR_STATE_STOP) {
			return;
		} 
		update_encoder();
    if (car.state == CAR_STATE_GO_STRAIGHT) {
        update_straight_control();
    } else if (car.state == CAR_STATE_TURN) {
        update_turn_control();
    } else if (car.state == CAR_STATE_TRACK) {
				update_track_control();
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
        car.target_mileage_cm = mileage;
        car_reset();
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
				car_reset();
    }
    float current_angle = get_yaw();
    float angle_error = calculate_angle_error(car.target_angle, current_angle);
    if (fabsf(angle_error) <= ANGLE_THRESHOLD_DEG) {
				car_reset();
        car.state = CAR_STATE_STOP;
        return true;
    }
    return false;
}


/**
 * @brief 移动直到检测到指定条件的线
 * @param move_state 小车的移动状态（如 CAR_STATE_GO_STRAIGHT 或 CAR_STATE_TRACK）
 * @param l_state 目标线状态（UNTIL_BLACK_LINE：移动直到检测到黑色；UNTIL_WHITE_LINE：移动直到连续20次检测到全白）
 * @return true 表示达到目标条件，false 表示未达到
 */
bool car_move_until(CAR_STATES move_state, LINE_STATES l_state) {
    if (car.state == CAR_STATE_STOP) {
        car.state = move_state;
				if (car.state == CAR_STATE_GO_STRAIGHT) {
					car_reset();
					car.target_mileage_cm = 0xFF; //随便给一个距离
				}
    }
    uint8_t track_data[TRACK_NUM];
    
    // 读取灰度传感器数据
    int temp_data = pca9555_read_bit12(&pca9555_i2c, PCA9555_ADDR);
    
		// 将 16 位数据按位压入 gray_datas 数组（只取低 12 位）
    for (int i = 0; i < 12; i++) {
        track_data[i] = (temp_data >> i) & 0x01; // 提取每一位并存入数组
    }

    // 检查当前是否全白（所有传感器值为0）
    bool is_all_white = true;
    bool has_black = false;
    for (int i = 0; i < TRACK_NUM; i++) {
        if (track_data[i] == 1) {
            is_all_white = false; // 只要有一个是黑，就不是全白
            has_black = true;     // 标记有黑色
            break;                // 可提前退出循环
        }
    }
    
    // 使用静态变量记录连续检测到全白的次数
    static uint8_t white_detection_count = 0;
    
    // 根据目标状态判断是否达到条件
    if (l_state == UNTIL_BLACK_LINE) {
        // 目标是检测到黑色（至少有一个传感器值为1）
        white_detection_count = 0; // 重置全白计数器
        if (has_black) {
            car.state = CAR_STATE_STOP; // 检测到黑色，停止小车
						set_alert_count(1);
						start_alert();
						car_reset();
            return true;                // 达到目标条件
        } else {
            return false; // 未检测到黑色（全白），继续移动
        }
    } else if (l_state == UNTIL_WHITE_LINE) {
        // 目标是检测到全白（所有传感器值为0），且需要连续5次确认
        if (is_all_white) {
            white_detection_count++; // 检测到全白，计数器加1 
            if (white_detection_count >= 3 && get_mileage_cm() >= 100) {
                car.state = CAR_STATE_STOP; // 停止小车
                white_detection_count = 0;  // 重置计数器
								set_alert_count(1);
								start_alert();
								car_reset();
                return true;                // 达到目标条件
            }
        } else {
            white_detection_count = 0; // 检测到非全白，重置计数器
        }
        return false; // 未达到连续20次全白，继续移动
    }
    
    return false; // 默认返回false（未匹配的目标状态）
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


/**
 * @brief 更新巡线控制逻辑，根据灰度传感器数据调整左右轮速度
 */
void update_track_control(void) {
	float error = gray_read_data();
	float correction = PID_Calculate(0.0f, error, &trackPid);
	for (int i = 0; i < motor_count; ++i)
		car.target_speed[i] = (i < motor_count / 2) ? TRACK_BASE_SPEED + correction: TRACK_BASE_SPEED - correction;
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