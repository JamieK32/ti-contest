#include "car_pid.h"


PID_Controller_t speedPid[motor_count];  // 支持最多4个电机
PID_Controller_t mileagePid;
PID_Controller_t straightPid;
PID_Controller_t anglePid;
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

/* ------------ 里程 PID ------------ */
void mileage_pid_init(void) {
	PID_Init(&mileagePid, PID_TYPE_POSITION);    
	PID_SetParams(&mileagePid, 4.0, 0.1, 0.0); 
	PID_SetOutputLimit(&mileagePid, 41.0, -41.0); 
	PID_SetIntegralLimit(&mileagePid, 41.0, -41.0);
	PID_SetIntegralSeparation(&mileagePid, 5);
}

/* ------------ 直行 PID ------------ */
void straight_pid_init(void) {
	PID_Init(&straightPid, PID_TYPE_POSITION);    
	PID_SetParams(&straightPid, 1.5, 0.0, 0.2);  
	PID_SetOutputLimit(&straightPid, 10.0, -10.0); 
}

/* ------------ 角度 PID ------------ */
void angle_pid_init(void) {
	PID_Init(&anglePid, PID_TYPE_POSITION);    
	PID_SetParams(&anglePid, 5, 0.0, 0.0);  
	PID_SetOutputLimit(&anglePid, 40.0, -40.0); 
}

/* ------------ 循迹 PID ------------ */
void track_pid_init(void) {
    PID_Init(&trackPid, PID_TYPE_POSITION);
    PID_SetParams(&trackPid, 10, 0, 0);
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
		mileage_pid_init();
		straight_pid_init();
		angle_pid_init();
		track_pid_init();
		follow_pid_init();
}
