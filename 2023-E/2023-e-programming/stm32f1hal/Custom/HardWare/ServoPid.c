#include "ServoPid.h"
#include "pid.h"
#include "Servo.h"
#include "openmv.h"

PID_Controller_t x_controller;
PID_Controller_t y_controller;

int target_x = 0;
int target_y = 0;

void servo_pid_init(void) {
	PID_Init(&x_controller, PID_TYPE_POSITION);
	PID_SetParams(&x_controller, 0, 0, 0);
	PID_Init(&y_controller, PID_TYPE_POSITION);
	PID_SetParams(&y_controller, 0, 0, 0);
}

void servo_pid_control(void) {
	float x_output = PID_Calculate(target_x, x_value, &x_controller);
	float y_output = PID_Calculate(target_y, y_value, &y_controller);
	Servo_MoveIncrement(x_output, y_output);
}

