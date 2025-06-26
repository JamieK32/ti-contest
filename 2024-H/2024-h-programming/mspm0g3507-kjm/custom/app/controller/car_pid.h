#ifndef CAR_PID_H__
#define CAR_PID_H__

#include "pid.h"
#include "car_controller.h"

void car_pid_init(void);

extern PID_Controller_t speedPid[motor_count];
extern PID_Controller_t mileagePid;
extern PID_Controller_t straightPid;
extern PID_Controller_t anglePid;
extern PID_Controller_t trackPid;

#endif 
