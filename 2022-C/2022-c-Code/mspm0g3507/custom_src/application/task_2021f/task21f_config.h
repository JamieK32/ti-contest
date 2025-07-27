#ifndef TASK_25_H__
#define TASK_25_H__

#include "stdint.h"

typedef enum {
	CAM_CMD_GO_LEFT = 0x01,
	CAM_CMD_GO_RIGHT = 0x02,
	CAM_CMD_NONE = 0x03,
} CAM_CMD;

typedef struct maixCam_t {
	uint8_t track_data;
	uint8_t num;
	CAM_CMD cmd;
} maixCam_t;


void init_task_table(void);
void menu_init_and_create(void);

#define NO_IMU 0
#define WIT_GYRO 1
#define MPU6050_GYRO 2
#define IMU660RA_GYRO 3

#define CURRENT_IMU MPU6050_GYRO
#define USE_ANGLE_SENSOR 1

void setup_cam_protocol(void);
extern maixCam_t maix_cam;

#endif 
