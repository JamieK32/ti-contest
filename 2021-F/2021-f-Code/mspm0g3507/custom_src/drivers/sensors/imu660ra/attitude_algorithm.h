#ifndef _ATTITUDE_ALGORITHM_H_
#define _ATTITUDE_ALGORITHM_H_

#include <stdbool.h>

#include "pose.h"

// choose one
#define USE_IMU660RA
// #define USE_IMU963RA

#ifdef USE_IMU660RA
#include "zf_device_imu660ra.h"
#elif defined(USE_IMU963RA)
#include "imu660ra/zf_device_imu963ra.h"
#endif

// 原始数据
typedef struct Attitude_data {
    float gyro_x, gyro_y, gyro_z;  // 度/秒
    float acc_x, acc_y, acc_z;     // 厘米/二次方秒
} Attitude_data;

// 修正参数
typedef struct Attitude_correct {
    float drift_gyro_x, drift_gyro_y, drift_gyro_z;                          // 零点漂移
    float min_effective_gyro_x, min_effective_gyro_y, min_effective_gyro_z;  // 数据的最小有效值,低于时置零
    // float drift_acc_x,drift_acc_y,drift_acc_z;
    // float min_effective_acc_x,min_effective_acc_y,min_effective_acc_z;
} Attitude_correct;

typedef struct Attitude_module {
    Pose_Module pose_module;
    Attitude_data attitude_data;
    Attitude_correct attitude_correct;
    float sampling_period;
    bool is_init;
#ifdef USE_IMU660RA
    imu660ra_measurement_data_struct gyro_measurement_data, acc_measurement_data;
    imu660ra_physical_data_struct gyro_physical_data, acc_physical_data;
#elif defined(USE_IMU963RA)
    imu963ra_measurement_data_struct gyro_measurement_data, acc_measurement_data;
    imu963ra_physical_data_struct gyro_physical_data, acc_physical_data;
#endif
} Attitude_module;

void init_attitude(Attitude_module* attitude_module, float sampling_period);
void update_attitude(Attitude_module* attitude_module);

#endif
