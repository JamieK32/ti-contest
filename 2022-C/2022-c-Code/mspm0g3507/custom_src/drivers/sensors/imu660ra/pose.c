#include "pose.h"
#include "pose_math.h"
#include "math.h"
#include "Fusion.h"

// 全局AHRS实例
static FusionAhrs ahrs;
static FusionOffset offset;
static bool fusion_initialized = false;

void initPose_Module(Pose_Module *pose)
{
    // 只保留必要的标志位初始化
    pose->flag.run = 1;
    pose->flag.use_mag = 0;  // 不使用磁力计
    
    // 接口初始化（保留，因为还需要用指针）
    pose->interface.data.a_x = 0;
    pose->interface.data.a_y = 0;
    pose->interface.data.a_z = 0;
    pose->interface.data.g_x = 0;
    pose->interface.data.g_y = 0;
    pose->interface.data.g_z = 0;
    
    // 四元数初始化（保留，输出需要）
    pose->process.quaternion[0] = 1;
    pose->process.quaternion[1] = 0;
    pose->process.quaternion[2] = 0;
    pose->process.quaternion[3] = 0;
    
    // 保留加速度相关初始化（输出需要）
    pose->data.acc_world.x = 0;
    pose->data.acc_world.y = 0;
    pose->data.acc_world.z = 0;
    pose->data.acc_correct.x = 0;
    pose->data.acc_correct.y = 0;
    pose->data.acc_correct.z = 0;

    // 保留欧拉角初始化（输出需要）
    pose->data.pit = 0;
    pose->data.rol = 0;
    pose->data.yaw = 0;
}

void calculatePose_Module(Pose_Module *pose, float cycle) 
{
    float length;
    
    if (pose->flag.run == 0)
        return;
				
		if (!fusion_initialized) {
				const FusionAhrsSettings settings = {
						.convention = FusionConventionNwu,      // 北-西-上坐标系（常用于导航）
						.gain = 0.5f,                           // 标准增益值，平衡陀螺仪和其他传感器
						.accelerationRejection = 10.0f,         // 加速度抑制阈值10度，适合大多数应用
						.magneticRejection = 0.0f,              // 不使用磁力计，设为0
						.recoveryTriggerPeriod =  5 * (int)(1.0f / cycle),           // 5秒超时 (5秒 × 100Hz采样率)
				};
				
				FusionAhrsInitialise(&ahrs);
				FusionAhrsSetSettings(&ahrs, &settings);
				
				// 陀螺仪零偏校正初始化
				FusionOffsetInitialise(&offset, (int)(1.0f / cycle));      // 100Hz采样率
				
				fusion_initialized = true;
		}

    
    // 准备传感器数据
    const FusionVector gyroscope = {
        .axis.x = *(pose->interface.data.g_x),
        .axis.y = *(pose->interface.data.g_y),
        .axis.z = *(pose->interface.data.g_z)
    };
    
    const FusionVector accelerometer = {
        .axis.x = *(pose->interface.data.a_x),
        .axis.y = *(pose->interface.data.a_y),
        .axis.z = *(pose->interface.data.a_z)
    };
    
    // 检查加速度计数据是否有效
    length = sqrt(accelerometer.axis.x * accelerometer.axis.x + 
                  accelerometer.axis.y * accelerometer.axis.y + 
                  accelerometer.axis.z * accelerometer.axis.z);
    
    // 陀螺仪零偏校正
    FusionVector corrected_gyroscope = FusionOffsetUpdate(&offset, gyroscope);
    
    // 更新AHRS
    if (length > 500.0f && length < 1500.0f) {
        FusionAhrsUpdateNoMagnetometer(&ahrs, corrected_gyroscope, accelerometer, cycle);
    } else {
        const FusionVector gravity = {.axis.x = 0.0f, .axis.y = 0.0f, .axis.z = 980.0f};
        FusionAhrsUpdateNoMagnetometer(&ahrs, corrected_gyroscope, gravity, cycle);
    }
    
    // 获取四元数
    const FusionQuaternion quaternion = FusionAhrsGetQuaternion(&ahrs);
    pose->process.quaternion[0] = quaternion.element.w;
    pose->process.quaternion[1] = quaternion.element.x;
    pose->process.quaternion[2] = quaternion.element.y;
    pose->process.quaternion[3] = quaternion.element.z;
    
    // 计算旋转矩阵
    float q0 = pose->process.quaternion[0];
    float q1 = pose->process.quaternion[1];
    float q2 = pose->process.quaternion[2];
    float q3 = pose->process.quaternion[3];
    
    pose->data.rotate_matrix[0][0] = q0*q0 + q1*q1 - q2*q2 - q3*q3;
    pose->data.rotate_matrix[0][1] = 2 * (q1*q2 + q0*q3);
    pose->data.rotate_matrix[0][2] = 2 * (q1*q3 - q0*q2);
    pose->data.rotate_matrix[1][0] = 2 * (q1*q2 - q0*q3);
    pose->data.rotate_matrix[1][1] = q0*q0 - q1*q1 + q2*q2 - q3*q3;
    pose->data.rotate_matrix[1][2] = 2 * (q2*q3 + q0*q1);
    pose->data.rotate_matrix[2][0] = 2 * (q1*q3 + q0*q2);
    pose->data.rotate_matrix[2][1] = 2 * (q2*q3 - q0*q1);
    pose->data.rotate_matrix[2][2] = q0*q0 - q1*q1 - q2*q2 + q3*q3;
    
    // 计算世界坐标系下的加速度值
    pose->data.acc_world.x = pose->data.rotate_matrix[0][0] * *(pose->interface.data.a_x) + 
                            pose->data.rotate_matrix[1][0] * *(pose->interface.data.a_y) + 
                            pose->data.rotate_matrix[2][0] * *(pose->interface.data.a_z);
    pose->data.acc_world.y = pose->data.rotate_matrix[0][1] * *(pose->interface.data.a_x) + 
                            pose->data.rotate_matrix[1][1] * *(pose->interface.data.a_y) + 
                            pose->data.rotate_matrix[2][1] * *(pose->interface.data.a_z);
    pose->data.acc_world.z = pose->data.rotate_matrix[0][2] * *(pose->interface.data.a_x) + 
                            pose->data.rotate_matrix[1][2] * *(pose->interface.data.a_y) + 
                            pose->data.rotate_matrix[2][2] * *(pose->interface.data.a_z);

		// 求解欧拉角 - 使用与原版本相同的公式
		pose->data.rol = arctan2(pose->data.rotate_matrix[2][2], pose->data.rotate_matrix[1][2]);
		pose->data.pit = -arcsin(pose->data.rotate_matrix[0][2]);
		pose->data.yaw = arctan2(pose->data.rotate_matrix[0][0], pose->data.rotate_matrix[0][1]);
		
    // 计算机体坐标系矫正后的加速度
    pose->data.acc_correct.x = pose->data.acc_world.x * cos_pose(pose->data.yaw) + pose->data.acc_world.y * sin_pose(pose->data.yaw);
    pose->data.acc_correct.y = -pose->data.acc_world.x * sin_pose(pose->data.yaw) + pose->data.acc_world.y * cos_pose(pose->data.yaw);
    pose->data.acc_correct.z = pose->data.acc_world.z;
}