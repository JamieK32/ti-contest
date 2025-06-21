#include "car_controller.h"
#include "encoder_app.h"
#include "common_defines.h"
#include "log_config.h"
#include "log.h"
#include "wit_jyxx.h"
#include "gray_detection_app.h"
#include "alert.h"

// 预计算常量以提高性能
static const float CIRCLE_TO_RPM = (60.0f / (ENCODER_PERIOD_MS * 0.001f)); // 采样周期转换为 RPM 的系数
static const float RPM_TO_CMPS = (2.0f * 3.1415926f * WHEEL_RADIUS_CM / 60.0f); // RPM 转换为 cm/s 的系数
static const float TIME_INTERVAL_S = (ENCODER_PERIOD_MS * 0.001f); // 采样周期，单位：秒

#define MIN_PWM_OUTPUT -2500
#define MAX_PWM_OUTPUT 2500

#define SPEED_KP 600
#define SPEED_KI 1
#define SPEED_KD 0

#define DISTANCE_KP 0.7f           // 直线行驶比例系数
#define ANGLE_KP 0.3f              // 旋转比例系数
#define TRACK_KP 1.46f          	 // 巡线比例控制系数

#define CRUISE_SPEED 15.0f         // 直线巡航速度
#define TURN_SPEED 10.0f           // 旋转速度

#define MIN_EFFECTIVE_SPEED 6.0f   // 直线行驶最小有效速度
#define MIN_TURN_SPEED 2.0f        // 旋转最小有效速度

#define DISTANCE_THRESHOLD_CM 0.5f // 距离误差阈值
#define ANGLE_THRESHOLD_DEG 1.5f   // 角度误差阈值

#define TRACK_NUM 12							 // 巡线个数

#define MAX_TRACK_CORRECTION 17.0f  // 巡线最大修正因子
#define TRACK_BASE_SPEED 11.5f		 // 巡线基础速度

// 电机类型
MotorCount motor_count = MOTOR_TYPE_TWO_WHEEL;

// 定义 encoder 结构体实例
encoder_t encoder = {0};

car_t car = {
    .state = CAR_STATE_STOP,
};

static inline float constrain_float(float amt, float low, float high) 
{
    if (isnan(amt)) {
        return (low + high) * 0.5f;
    }
    return ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)));
}

static inline float calculate_angle_error(float target, float current) {
    float error = target - current;
    if (fabsf(error) > 180.0f) {
        if (target > 0) {
            error -= 360.0f;
        } else {
            error += 360.0f;
        }
    }
    return error;
}

float get_yaw(void) {
    return jy901s.yaw;
}

void car_task(void) {
    if (car.state == CAR_STATE_STOP) {
			clear_mileage();
			for (int i = 0; i < motor_count; i++) {

			}
			return;
		} 
    update_speed();
    update_distance();
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
        clear_mileage();
    }
    float current_mileage = get_mileage_cm();
    if (fabsf(car.target_mileage_cm - current_mileage) <= DISTANCE_THRESHOLD_CM) {
        car.target_speed[0] = 0;
        car.target_speed[1] = 0;
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
    }
    float current_angle = get_yaw();
    float angle_error = calculate_angle_error(car.target_angle, current_angle);
    if (fabsf(angle_error) <= ANGLE_THRESHOLD_DEG) {
        car.target_speed[0] = 0;
        car.target_speed[1] = 0;
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
					clear_mileage();
					car.target_mileage_cm = 0xFF; //随便给一个距离
				}
    }
    uint8_t track_data[TRACK_NUM];
    
    // 读取灰度传感器数据
    gray_read_data(track_data);
    
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
            return true;                // 达到目标条件
        } else {
            return false; // 未检测到黑色（全白），继续移动
        }
    } else if (l_state == UNTIL_WHITE_LINE) {
        // 目标是检测到全白（所有传感器值为0），且需要连续20次确认
        if (is_all_white) {
            white_detection_count++; // 检测到全白，计数器加1 
            if (white_detection_count >= 5 && get_mileage_cm() >= 100) { // 连续100次全白并且总里程达到100
                car.state = CAR_STATE_STOP; // 停止小车
                white_detection_count = 0;  // 重置计数器
								set_alert_count(1);
								start_alert();
                return true;                // 达到目标条件
            }
        } else {
            white_detection_count = 0; // 检测到非全白，重置计数器
        }
        return false; // 未达到连续20次全白，继续移动
    }
    
    return false; // 默认返回false（未匹配的目标状态）
}




// 初始化 PID 控制器（仅用于速度环）
void speed_pid_init(float Kp, float Ki, float Kd) {
//    for (int i = 0; i < motor_count; i++) {
//        car.speed_pid[i].Kp = Kp;
//        car.speed_pid[i].Ki = Ki;
//        car.speed_pid[i].Kd = Kd;
//        arm_pid_init_f32(&car.speed_pid[i], 1); // 1 表示重置 PID 状态
//    }
}

void car_init(void) {
    encoder_application_init();
    motor_init();
    speed_pid_init(SPEED_KP, SPEED_KI, SPEED_KD);
}

void update_speed(void) {
    for (int i = 0; i < motor_count; i++) {
        encoder.counts[i] = encoder_manager_read_and_reset(&robot_encoder_manager, i);
        encoder.rpms[i] = encoder.counts[i] * CIRCLE_TO_RPM / PULSE_NUM_PER_CIRCLE;
        encoder.cmps[i] = encoder.rpms[i] * RPM_TO_CMPS;
    }
}

void update_distance(void) {
    for (int i = 0; i < motor_count; i++) {
        encoder.distance_cm[i] += encoder.cmps[i] * TIME_INTERVAL_S;
    }
}

void update_speed_pid(void) {
//    float32_t error;  // 速度误差
//    float32_t outputs[MOTOR_TYPE_TWO_WHEEL];
//    for (int i = 0; i < motor_count; i++) {
//        error = car.target_speed[i] - encoder.cmps[i];
//        outputs[i] = arm_pid_f32(&car.speed_pid[i], error);
//        outputs[i] = constrain_float(outputs[i], MIN_PWM_OUTPUT, MAX_PWM_OUTPUT);
//    }
}

void update_straight_control(void) {
    // 计算里程误差并确定基础速度
    float current_mileage = get_mileage_cm();
    float error = car.target_mileage_cm - current_mileage;
    float base_speed = error * DISTANCE_KP; // 比例控制

    // 限制速度范围
    if (fabsf(base_speed) > CRUISE_SPEED) {
        base_speed = CRUISE_SPEED * (error > 0 ? 1.0f : -1.0f);
    } else if (fabsf(base_speed) < MIN_EFFECTIVE_SPEED && fabsf(base_speed) > 0.1f) {
        base_speed = MIN_EFFECTIVE_SPEED * (error > 0 ? 1.0f : -1.0f);
    }

    // 计算航向误差并进行修正
    float current_yaw = get_yaw();
    float yaw_error = calculate_angle_error(car.target_angle, current_yaw);
    float correction = yaw_error * ANGLE_KP; // 使用角度比例系数进行修正

    // 限制修正值，防止过大的速度差
    const float MAX_CORRECTION = base_speed * 0.3f; // 修正值不超过基础速度的30%
    if (fabsf(correction) > MAX_CORRECTION) {
        correction = MAX_CORRECTION * (yaw_error > 0 ? 1.0f : -1.0f);
    }

    // 设置左右轮目标速度，加入航向修正
    car.target_speed[0] = base_speed + correction; // 左轮
    car.target_speed[1] = base_speed - correction; // 右轮

    // 防止修正导致某个轮子反向转动
    if (base_speed > 0.1f) {
        car.target_speed[0] = fmaxf(0.1f, car.target_speed[0]);
        car.target_speed[1] = fmaxf(0.1f, car.target_speed[1]);
    } else if (base_speed < -0.1f) {
        car.target_speed[0] = fminf(-0.1f, car.target_speed[0]);
        car.target_speed[1] = fminf(-0.1f, car.target_speed[1]);
    }
}


/**
 * @brief 更新巡线控制逻辑，根据灰度传感器数据调整左右轮速度
 */
void update_track_control(void) {
//    float left_speed = TRACK_BASE_SPEED;
//    float right_speed = TRACK_BASE_SPEED;
//    float adjustment = 0.0f;
//    uint8_t track_data[TRACK_NUM];
//    static arm_pid_instance_f32 track_pid = {0};  // PID控制器实例
//    static bool pid_initialized = false;          // PID初始化标志
//    static float last_valid_adjustment = 0.0f;    // 上一次有效的偏差值，用于丢失目标时转向
//    static bool target_lost = false;              // 目标丢失标志

//    // 初始化PID控制器（仅在第一次调用时初始化）
//    if (!pid_initialized) {
//        track_pid.Kp = TRACK_KP;      // 比例系数
//        track_pid.Ki = 0.0f;          // 积分系数，需调试
//        track_pid.Kd = 0.0f;          // 微分系数，需调试
//        arm_pid_init_f32(&track_pid, 1);  // 1表示重置PID状态
//        pid_initialized = true;
//    }

//    // 读取灰度传感器数据
//    gray_read_data(track_data);

//    // 计算轨道偏差，假设传感器值为1表示检测到黑线
//    // 中点为 (TRACK_NUM - 1) / 2.0f，例如12路传感器的中点为5.5
//    const float center = (TRACK_NUM - 1) / 2.0f;
//    for (int i = 0; i < TRACK_NUM; i++) {
//        if (track_data[i]) {
//            adjustment += (i - center); // 加权计算偏差，负值表示偏左，正值表示偏右
//        }
//    }

//    // 判断是否丢失目标
//    if (adjustment == 0.0f) {
//        // 偏差为0，可能是丢失目标，检查是否有传感器数据为1
//        bool no_detection = true;
//        for (int i = 0; i < TRACK_NUM; i++) {
//            if (track_data[i]) {
//                no_detection = false;
//                break;
//            }
//        }
//        if (no_detection) {
//            // 确实丢失目标，设置标志并使用上一次有效偏差进行大幅度转向
//            target_lost = true;
//            adjustment = last_valid_adjustment;
//        } else {
//            // 偏差为0但有检测到黑线（可能在中心），不认为是丢失目标
//            target_lost = false;
//            last_valid_adjustment = adjustment;  // 更新上一次有效偏差
//        }
//    } else {
//        // 检测到目标，更新上一次有效偏差并清除丢失标志
//        target_lost = false;
//        last_valid_adjustment = adjustment;
//    }

//    // PID控制逻辑
//    float error = adjustment;  // 当前偏差作为误差
////    float pid_output = arm_pid_f32(&track_pid, error);  // 使用ARM Math PID控制器计算输出

//    // 根据偏差调整速度
//    if (error != 0.0f || target_lost) {
//        // 计算速度修正量，pid_output 直接作为速度差的一部分
//        // 如果目标丢失，使用更大的修正量进行大幅度转向
//        float correction_limit = target_lost ? MAX_TRACK_CORRECTION * 2.0f : MAX_TRACK_CORRECTION;
//        float speed_correction = constrain_float(pid_output, -correction_limit, correction_limit);

//        // 根据PID输出直接调整速度
//        // pid_output > 0 表示偏右，需向左转（左轮加速，右轮减速）
//        // pid_output < 0 表示偏左，需向右转（左轮减速，右轮加速）
//        left_speed = TRACK_BASE_SPEED + speed_correction;
//        right_speed = TRACK_BASE_SPEED - speed_correction;

//        // 限制速度范围，防止速度过低或反向
//        // 丢失目标时允许更大的速度差以实现大幅度转向
//        if (target_lost) {
//            left_speed = constrain_float(left_speed, TRACK_BASE_SPEED * 0.3f, TRACK_BASE_SPEED * 2.0f);
//            right_speed = constrain_float(right_speed, TRACK_BASE_SPEED * 0.3f, TRACK_BASE_SPEED * 2.0f);
//        } else {
//            left_speed = constrain_float(left_speed, TRACK_BASE_SPEED * 0.5f, TRACK_BASE_SPEED * 1.5f);
//            right_speed = constrain_float(right_speed, TRACK_BASE_SPEED * 0.5f, TRACK_BASE_SPEED * 1.5f);
//        }
//    }

//    // 设置目标速度
//    car.target_speed[0] = left_speed;  // 左轮
//    car.target_speed[1] = right_speed; // 右轮
}



void update_turn_control(void) {
    float current_angle = get_yaw();
    float angle_error = calculate_angle_error(car.target_angle, current_angle);
    float turn_speed = angle_error * ANGLE_KP; // 比例控制

    // 限制旋转速度范围
    if (fabsf(turn_speed) > TURN_SPEED) {
        turn_speed = TURN_SPEED * (angle_error > 0 ? 1.0f : -1.0f);
    } else if (fabsf(turn_speed) < MIN_TURN_SPEED && fabsf(turn_speed) > 0.1f) {
        turn_speed = MIN_TURN_SPEED * (angle_error > 0 ? 1.0f : -1.0f);
    }

    // 设置左右轮差速以实现转向
    if (angle_error > 0) {
        car.target_speed[0] = turn_speed;  // 左轮
        car.target_speed[1] = -turn_speed; // 右轮
    } else {
        car.target_speed[0] = turn_speed; // 左轮
        car.target_speed[1] = -turn_speed;  // 右轮
    }
}

void clear_mileage(void) {
    for (int i = 0; i < motor_count; i++) {
        encoder.distance_cm[i] = 0;
    }
}

float get_mileage_cm(void) {
    float output = 0;
    for (int i = 0; i < motor_count; i++) {
        output += encoder.distance_cm[i];
    }
    return output / motor_count;
}
