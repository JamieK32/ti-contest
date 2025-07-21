#ifndef CAR_CONTROLL_H__
#define CAR_CONTROLL_H__

#include "motor_user.h"
#include "encoder_user.h"
#include "wit_jyxx.h"
#include "gray_detection.h"
#include "voice_light_alert.h"
#include "car_pid.h"
#include "car_debug.h"
#include "_74hc595.h"
#include "car_config.h"

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


#endif
