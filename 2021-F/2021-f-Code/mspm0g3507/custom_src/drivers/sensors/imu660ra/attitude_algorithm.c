#include "attitude_algorithm.h"
#include "math.h"
#include "delay.h"

// Inner func
void gyro_error_correct(Attitude_module* attitude_module) {
    attitude_module->attitude_data.gyro_x -= attitude_module->attitude_correct.drift_gyro_x;
    attitude_module->attitude_data.gyro_y -= attitude_module->attitude_correct.drift_gyro_y;
    attitude_module->attitude_data.gyro_z -= attitude_module->attitude_correct.drift_gyro_z;
    if (fabs(attitude_module->attitude_data.gyro_x) < attitude_module->attitude_correct.min_effective_gyro_x)
        attitude_module->attitude_data.gyro_x = 0.0f;
    if (fabs(attitude_module->attitude_data.gyro_y) < attitude_module->attitude_correct.min_effective_gyro_y)
        attitude_module->attitude_data.gyro_y = 0.0f;
    if (fabs(attitude_module->attitude_data.gyro_z) < attitude_module->attitude_correct.min_effective_gyro_z)
        attitude_module->attitude_data.gyro_z = 0.0f;
}

void get_acc(Attitude_module* attitude_module) {
#ifdef USE_IMU660RA
    imu660ra_get_acc(&(attitude_module->acc_measurement_data));
    imu660ra_get_physical_acc(&(attitude_module->acc_measurement_data), IMU660RA_ACC_RANGE_DEFAULT,
                              &(attitude_module->acc_physical_data));
    attitude_module->attitude_data.acc_x = attitude_module->acc_physical_data.x * 980.0f;
    attitude_module->attitude_data.acc_y = attitude_module->acc_physical_data.y * 980.0f;
    attitude_module->attitude_data.acc_z = attitude_module->acc_physical_data.z * 980.0f;
#elif defined(USE_IMU963RA)
    imu963ra_get_acc(&(attitude_module->acc_measurement_data));
    imu963ra_get_physical_acc(&(attitude_module->acc_measurement_data), IMU963RA_ACC_RANGE_DEFAULT,
                              &(attitude_module->acc_physical_data));
    attitude_module->attitude_data.acc_x = attitude_module->acc_physical_data.y * 980.0f;
    attitude_module->attitude_data.acc_y = -attitude_module->acc_physical_data.x * 980.0f;
    attitude_module->attitude_data.acc_z = attitude_module->acc_physical_data.z * 980.0f;
#endif
}

void get_gyro(Attitude_module* attitude_module) {
#ifdef USE_IMU660RA
    imu660ra_get_gyro(&(attitude_module->gyro_measurement_data));
    imu660ra_get_physical_gyro(&(attitude_module->gyro_measurement_data), IMU660RA_GYRO_RANGE_DEFAULT,
                               &(attitude_module->gyro_physical_data));
    attitude_module->attitude_data.gyro_x = attitude_module->gyro_physical_data.x;
    attitude_module->attitude_data.gyro_y = attitude_module->gyro_physical_data.y;
    attitude_module->attitude_data.gyro_z = attitude_module->gyro_physical_data.z;
#elif defined(USE_IMU963RA)
    imu963ra_get_gyro(&(attitude_module->gyro_measurement_data));
    imu963ra_get_physical_gyro(&(attitude_module->gyro_measurement_data), IMU963RA_GYRO_RANGE_DEFAULT,
                               &(attitude_module->gyro_physical_data));
    attitude_module->attitude_data.gyro_x = attitude_module->gyro_physical_data.y;
    attitude_module->attitude_data.gyro_y = -attitude_module->gyro_physical_data.x;
    attitude_module->attitude_data.gyro_z = attitude_module->gyro_physical_data.z;
#endif

    if (attitude_module->is_init) {
        gyro_error_correct(attitude_module);
    }
}

void calc_gyro_zero_drift(Attitude_module* attitude_module) {
    delay_ms(500);
    int cnt = 2000;
    for (int i = 0; i < cnt; i++) {
        get_gyro(attitude_module);
        attitude_module->attitude_correct.drift_gyro_x += attitude_module->attitude_data.gyro_x;
        attitude_module->attitude_correct.drift_gyro_y += attitude_module->attitude_data.gyro_y;
        attitude_module->attitude_correct.drift_gyro_z += attitude_module->attitude_data.gyro_z;
        delay_ms(2);
    }
    attitude_module->attitude_correct.drift_gyro_x /= (float)cnt;
    attitude_module->attitude_correct.drift_gyro_y /= (float)cnt;
    attitude_module->attitude_correct.drift_gyro_z /= (float)cnt;
}

// Outer Func

void init_attitude(Attitude_module* attitude_module, float sampling_period) {
#ifdef USE_IMU660RA
    imu660ra_init();
#elif defined(USE_IMU963RA)
    imu963ra_init();
#endif

    attitude_module->is_init = false;

    initPose_Module(&(attitude_module->pose_module));

    attitude_module->pose_module.flag.run = true;
    attitude_module->pose_module.flag.use_mag = false;
    attitude_module->pose_module.interface.data.a_x = &(attitude_module->attitude_data.acc_x);
    attitude_module->pose_module.interface.data.a_y = &(attitude_module->attitude_data.acc_y);
    attitude_module->pose_module.interface.data.a_z = &(attitude_module->attitude_data.acc_z);
    attitude_module->pose_module.interface.data.g_x = &(attitude_module->attitude_data.gyro_x);
    attitude_module->pose_module.interface.data.g_y = &(attitude_module->attitude_data.gyro_y);
    attitude_module->pose_module.interface.data.g_z = &(attitude_module->attitude_data.gyro_z);

    attitude_module->attitude_data.acc_x = 0;
    attitude_module->attitude_data.acc_y = 0;
    attitude_module->attitude_data.acc_z = 0;
    attitude_module->attitude_data.gyro_x = 0;
    attitude_module->attitude_data.gyro_y = 0;
    attitude_module->attitude_data.gyro_z = 0;

    attitude_module->attitude_correct.drift_gyro_x = 0.0f;
    attitude_module->attitude_correct.drift_gyro_y = 0.0f;
    attitude_module->attitude_correct.drift_gyro_z = 0.0f;
    attitude_module->attitude_correct.min_effective_gyro_x = 0.3f;
    attitude_module->attitude_correct.min_effective_gyro_y = 0.3f;
    attitude_module->attitude_correct.min_effective_gyro_z = 0.3f;

    attitude_module->sampling_period = sampling_period;

    calc_gyro_zero_drift(attitude_module);

    attitude_module->is_init = true;
}

void update_attitude(Attitude_module* attitude_module) {
    get_acc(attitude_module);
    get_gyro(attitude_module);
    calculatePose_Module(&(attitude_module->pose_module), attitude_module->sampling_period);
}
