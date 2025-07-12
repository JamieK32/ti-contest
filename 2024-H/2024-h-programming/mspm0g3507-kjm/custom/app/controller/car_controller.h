#ifndef CAR_CONTROLL_H__
#define CAR_CONTROLL_H__

#include "motor_app.h"
#include "common_defines.h"

#define motor_count MOTOR_TYPE_FOUR_WHEEL

typedef enum {
    CAR_STATE_GO_STRAIGHT = 0,
    CAR_STATE_TURN,
		CAR_STATE_TRACK,
    CAR_STATE_STOP,
} CAR_STATES;

typedef enum {
		UNTIL_BLACK_LINE,
		UNTIL_WHITE_LINE,
		UNTIL_STOP_MARK,
} LINE_STATES;

typedef struct encoder_t {
    float distance_cm[motor_count];
    int32_t counts[motor_count]; 
    float rpms[motor_count];
    float cmps[motor_count];
} encoder_t;

typedef struct car_t {
    CAR_STATES state;
    float target_speed[motor_count];
    float target_mileage_cm;
    float target_angle;
		float track_speed;
		bool turn_initialized;
} car_t;


extern car_t car;
extern bool is_outer_track;
extern encoder_t encoder;
extern uint8_t global_stop_mark_count;

void car_task(void);
void car_init(void);
void update_encoder(void);
void update_speed_pid(void);
float get_mileage_cm(void);

bool car_move_cm(float mileage, CAR_STATES move_state);
bool spin_turn(float angle);
bool car_move_until(CAR_STATES move_state, LINE_STATES state);

void update_straight_control(void);
void update_turn_control(void);
void update_track_control(void);
void car_reset(void);

void car_set_track_speed(float speed);
void car_set_base_speed(float speed);
void car_zero_speed_mode(void);
void car_set_outer_track_flag(bool flag);


static const float CIRCLE_TO_RPM = (60.0f / (ENCODER_PERIOD_MS * 0.001f));
static const float RPM_TO_CMPS = (2.0f * 3.1415926f * WHEEL_RADIUS_CM / 60.0f); 
static const float TIME_INTERVAL_S = (ENCODER_PERIOD_MS * 0.001f); 

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

#define STOP_MARK_TABLE_SIZE (sizeof(stop_mark_table_8bit) / sizeof(stop_mark_table_8bit[0]))

#endif
