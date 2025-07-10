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

#define MAX_DISTANCE 255
#define USE_ANGLE_SENSOR 1
#define DISTANCE_THRESHOLD_CM 1
#define ANGLE_THRESHOLD_DEG 1
#define TRACK_DEFAULT_SPEED 40

// 定义 encoder 结构体实例
encoder_t encoder = {0};

car_t car = {
    .state = CAR_STATE_STOP,
		.track_speed = TRACK_DEFAULT_SPEED,
};

float get_yaw(void) {
    return jy61p.yaw;
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
				update_debug_information();
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
        car.target_mileage_cm = mileage;
        car_reset();
				gray_get_position();
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
 * @param move_state 小车的移动状态
 * @param l_state 目标线状态
 * @return true 表示达到目标条件，false 表示未达到
 */
bool car_move_until(CAR_STATES move_state, LINE_STATES l_state) {
    static uint8_t white_count = 0;
    
    // 初始化移动状态
    if (car.state == CAR_STATE_STOP) {
        car.state = move_state;
        if (move_state == CAR_STATE_GO_STRAIGHT) {
            car_reset();
            car.target_mileage_cm = MAX_DISTANCE;
        }
    }
    
    // 读取传感器数据
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
            if (white_count >= 5 && get_mileage_cm() >= 120) {
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
		else if (l_state == UNTIL_STOP_MARK) {
        if (is_in_table(stop_mark_table, STOP_MARK_TABLE_SIZE, sensor_data)) {
            car.state = CAR_STATE_STOP;
            set_alert_count(1);
            start_alert();
            car_reset();
            return true;
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


/**
 * @brief 更新巡线控制逻辑，根据灰度传感器数据调整左右轮速度
 */
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