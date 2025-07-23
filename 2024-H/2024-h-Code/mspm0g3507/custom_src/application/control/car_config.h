#ifndef CAR_CONFIG_H__
#define CAR_CONFIG_H__

#define motor_count 4															// 轮胎数量
#define ENCODER_PERIOD_MS          20             // 20ms 采样周期
#define WHEEL_RADIUS_CM            3.3f           // 轮胎半径，单位：cm
#define PULSE_NUM_PER_CIRCLE       1470           // 轮胎一圈的编码器计数
#define WHEEL_BASE_CM 24.0f  										  // 轮距，根据实际小车调整
#define M_PI 3.14159265359f												// 定义圆周率

static const float CIRCLE_TO_RPM = (60.0f / (ENCODER_PERIOD_MS * 0.001f));
static const float RPM_TO_CMPS = (2.0f * 3.1415926f * WHEEL_RADIUS_CM / 60.0f); 
static const float TIME_INTERVAL_S = (ENCODER_PERIOD_MS * 0.001f); 


#endif 