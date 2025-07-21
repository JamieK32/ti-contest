#include "common_defines.h"
#if (CURRENT_TASK_TYPE == TASK_TYPE_22C)
#include "car_pid.h"

PID_Controller_t speedPid[motor_count];  // 支持最多4个电机
PID_Controller_t trackPid;
PID_Controller_t followPid;

/* ------------ 速度 PID ------------ */
void speed_pid_init(void) {
    for (int i = 0; i < motor_count; i++) {
        PID_Init(&speedPid[i], PID_TYPE_POSITION);    
        PID_SetParams(&speedPid[i], 50.0, 5.0, 3.0);    
        PID_SetOutputLimit(&speedPid[i], 3000.0, -3000.0); 
        PID_SetIntegralLimit(&speedPid[i], 3000.0, -3000.0); 

    }
}

/* ------------ 循迹 PID ------------ */
void track_pid_init(void) {
    PID_Init(&trackPid, PID_TYPE_POSITION);
    PID_SetParams(&trackPid, 14, 0, 0);
    PID_SetOutputLimit(&trackPid, 30, -30);
}


/* ------------ 跟踪 PID ------------ */
void follow_pid_init(void) {
    PID_Init(&followPid, PID_TYPE_POSITION);
    PID_SetParams(&followPid, 0.1, 0, 0);
    PID_SetOutputLimit(&followPid, 30, -30);
}

void car_pid_init(void) {
		speed_pid_init();
		track_pid_init();
		follow_pid_init();
}
#endif

