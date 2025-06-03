#ifndef MOTOR_CONLLER_H_
#define MOTOR_CONLLER_H_

#include "motor_hardware.h"

void motor_init(void);
void motor_set_pwm(MotorID id, int pwm);
void motor_stop(void);

#endif
