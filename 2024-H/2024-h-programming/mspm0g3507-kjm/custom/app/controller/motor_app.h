#ifndef MOTOR_CONLLER_H_
#define MOTOR_CONLLER_H_

#include "motor_hardware.h"

void motor_init(void);
void motor_set_pwms(int *pwms);
void motor_stop(void);

#endif
