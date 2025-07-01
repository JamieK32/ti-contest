#include "ServoPid.h"
#include "pid.h"
#include "Servo.h"
#include "openmv.h"
#include "stdbool.h"

PID_Controller_t x_pid;
PID_Controller_t y_pid;

static bool pid_start_flag = false;

void servo_pid_init(void) {
	PID_Init(&x_pid,PID_TYPE_POSITION);
	PID_SetParams(&x_pid,0.03,0.000,0);
	PID_SetDeadzone(&x_pid, 1);
	PID_Init(&y_pid,PID_TYPE_POSITION);
	PID_SetParams(&y_pid,0.03,0.000,0);
	PID_SetDeadzone(&y_pid, 1);
}

void servo_pid_control(void) {
	if (!pid_start_flag) return;
	float x_output = PID_Calculate(0, err_x, &x_pid);
	float y_output = PID_Calculate(0, err_y, &y_pid);
	Servo_MoveIncrement(x_output, y_output);
}

void start_servo_pid_control(void) {
	pid_start_flag = true;
}

void stop_servo_pid_control(void) {
	pid_start_flag = false;
}