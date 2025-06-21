#ifndef CAR_CONTROLL_H__
#define CAR_CONTROLL_H__

#include "motor_app.h"


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
    float distance_cm[MOTOR_TYPE_TWO_WHEEL];
    int32_t counts[MOTOR_TYPE_TWO_WHEEL]; 
    float rpms[MOTOR_TYPE_TWO_WHEEL];
    float cmps[MOTOR_TYPE_TWO_WHEEL];
} encoder_t;

typedef struct car_t {
    CAR_STATES state;
    float target_speed[MOTOR_TYPE_TWO_WHEEL];
    float target_mileage_cm;
    float target_angle;

} car_t;

extern car_t car;

void car_task(void);
void car_init(void);
void update_speed(void);
void update_distance(void);
void update_speed_pid(void);
float get_mileage_cm(void);
void clear_mileage(void);

bool car_move_cm(float mileage, CAR_STATES move_state);
bool spin_turn(float angle);
bool car_move_until(CAR_STATES move_state, LINE_STATES state);

void update_straight_control(void);
void update_turn_control(void);
void update_track_control(void);

extern encoder_t encoder;

#endif
