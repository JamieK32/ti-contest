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
} car_t;

static const float CIRCLE_TO_RPM = (60.0f / (ENCODER_PERIOD_MS * 0.001f));
static const float RPM_TO_CMPS = (2.0f * 3.1415926f * WHEEL_RADIUS_CM / 60.0f); 
static const float TIME_INTERVAL_S = (ENCODER_PERIOD_MS * 0.001f); 

extern car_t car;

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

extern encoder_t encoder;

#endif
