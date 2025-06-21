# IQ PID控制器使用手册

## 目录
1. [概述](#概述)
2. [快速开始](#快速开始)
3. [基本配置](#基本配置)
4. [高级功能](#高级功能)
5. [使用示例](#使用示例)
6. [参数调优指南](#参数调优指南)
7. [常见问题](#常见问题)
8. [API参考](#api参考)

## 概述

这是一个基于TI IQmath库的高性能PID控制器，支持定点数运算，适用于DSP和嵌入式系统。

### 主要特性
- 支持位置式和增量式PID算法
- 集成积分分离、积分限幅、输出限幅功能
- 提供多种积分抗饱和策略
- 支持输入死区处理
- 兼容float和IQ定点数格式
- 参数可实时调整

## 快速开始

### 1. 包含头文件
```c
#include "iq_pid.h"
```

### 2. 创建PID控制器
```c
PID_Controller_t pid_controller;
```

### 3. 初始化控制器
```c
// 初始化为位置式PID
PID_Init(&pid_controller, PID_TYPE_POSITION);

// 或初始化为增量式PID
PID_Init(&pid_controller, PID_TYPE_INCREMENT);
```

### 4. 设置PID参数
```c
pid_controller.Kp = _IQ(1.0);    // 比例系数
pid_controller.Ki = _IQ(0.1);    // 积分系数  
pid_controller.Kd = _IQ(0.01);   // 微分系数
```

### 5. 执行PID计算
```c
_iq target = _IQ(100.0);      // 目标值
_iq feedback = _IQ(85.0);     // 反馈值
_iq output = PID_Calculate(target, feedback, &pid_controller);
```

## 基本配置

### PID参数设置

#### 使用IQ格式
```c
pid_controller.Kp = _IQ(1.5);     // 比例系数
pid_controller.Ki = _IQ(0.2);     // 积分系数
pid_controller.Kd = _IQ(0.05);    // 微分系数
```

#### 使用float格式（需要转换）
```c
pid_controller.Kp = FLOAT_TO_IQ(1.5);
pid_controller.Ki = FLOAT_TO_IQ(0.2);
pid_controller.Kd = FLOAT_TO_IQ(0.05);
```

### 输出限幅设置
```c
pid_controller.output_max = _IQ(1000.0);   // 最大输出
pid_controller.output_min = _IQ(-1000.0);  // 最小输出
pid_controller.enable_output_limit = 1;     // 启用输出限幅
```

### 积分限幅设置
```c
pid_controller.integral_max = _IQ(500.0);   // 积分最大值
pid_controller.integral_min = _IQ(-500.0);  // 积分最小值
pid_controller.enable_integral_limit = 1;   // 启用积分限幅
```

## 高级功能

### 1. 积分分离
当误差较大时，暂停积分作用，防止积分项过大。

```c
// 启用积分分离
pid_controller.enable_integral_separation = 1;
pid_controller.integral_separation_threshold = _IQ(50.0);  // 分离阈值
```

### 2. 死区处理
忽略小的输入误差，减少系统抖动。

```c
// 启用死区
pid_controller.enable_deadzone = 1;
pid_controller.deadzone = _IQ(2.0);  // 死区大小
```

### 3. 积分抗饱和
防止积分饱和，提高控制性能。

#### 方法一：积分夹紧法（推荐）
```c
PID_SetAntiWindupMethod(&pid_controller, ANTI_WINDUP_CLAMPING, _IQ(30.0), _IQ(0.1));
```

#### 方法二：反向计算法
```c
PID_SetAntiWindupMethod(&pid_controller, ANTI_WINDUP_BACK_CALC, _IQ(30.0), _IQ(0.2));
```

#### 方法三：条件积分法
```c
PID_SetAntiWindupMethod(&pid_controller, ANTI_WINDUP_CONDITIONAL, _IQ(40.0), _IQ(0.1));
```

### 4. 输出偏移
为输出添加固定偏移量。

```c
pid_controller.output_offset = _IQ(50.0);  // 输出偏移50
```

## 使用示例

### 示例1：电机速度控制（位置式PID）
```c
#include "iq_pid.h"

PID_Controller_t speed_pid;

void motor_speed_control_init(void) {
    // 初始化位置式PID
    PID_Init(&speed_pid, PID_TYPE_POSITION);
    
    // 设置PID参数
    speed_pid.Kp = _IQ(0.8);
    speed_pid.Ki = _IQ(0.15);
    speed_pid.Kd = _IQ(0.02);
    
    // 设置输出限幅
    speed_pid.output_max = _IQ(1000.0);
    speed_pid.output_min = _IQ(-1000.0);
    
    // 启用积分分离
    speed_pid.enable_integral_separation = 1;
    speed_pid.integral_separation_threshold = _IQ(100.0);
    
    // 设置抗饱和
    PID_SetAntiWindupMethod(&speed_pid, ANTI_WINDUP_CLAMPING, _IQ(50.0), _IQ(0.1));
}

_iq motor_speed_control(_iq target_speed, _iq actual_speed) {
    return PID_Calculate(target_speed, actual_speed, &speed_pid);
}
```

### 示例2：温度控制（增量式PID）
```c
#include "iq_pid.h"

PID_Controller_t temp_pid;

void temperature_control_init(void) {
    // 初始化增量式PID
    PID_Init(&temp_pid, PID_TYPE_INCREMENT);
    
    // 设置PID参数
    temp_pid.Kp = _IQ(2.0);
    temp_pid.Ki = _IQ(0.05);
    temp_pid.Kd = _IQ(0.1);
    
    // 设置死区
    temp_pid.enable_deadzone = 1;
    temp_pid.deadzone = _IQ(0.5);  // 0.5度死区
    
    // 设置输出限幅
    temp_pid.output_max = _IQ(100.0);   // 最大加热功率
    temp_pid.output_min = _IQ(0.0);     // 最小加热功率
    
    // 设置积分限幅
    temp_pid.integral_max = _IQ(200.0);
    temp_pid.integral_min = _IQ(-200.0);
}

_iq temperature_control(float target_temp, float actual_temp) {
    // 使用float接口
    PID_SetTargetFloat(&temp_pid, target_temp);
    PID_SetFeedbackFloat(&temp_pid, actual_temp);
    
    return PID_Calculate(temp_pid.target, temp_pid.feedback, &temp_pid);
}
```

### 示例3：位置伺服控制
```c
#include "iq_pid.h"

PID_Controller_t position_pid;

void servo_position_init(void) {
    PID_Init(&position_pid, PID_TYPE_POSITION);
    
    // 精密位置控制参数
    position_pid.Kp = _IQ(5.0);
    position_pid.Ki = _IQ(0.8);
    position_pid.Kd = _IQ(0.15);
    
    // 严格的输出限制
    position_pid.output_max = _IQ(500.0);
    position_pid.output_min = _IQ(-500.0);
    
    // 小死区以减少抖动
    position_pid.enable_deadzone = 1;
    position_pid.deadzone = _IQ(0.1);
    
    // 使用反向计算抗饱和
    PID_SetAntiWindupMethod(&position_pid, ANTI_WINDUP_BACK_CALC, _IQ(20.0), _IQ(0.3));
}

_iq servo_position_control(_iq target_pos, _iq actual_pos) {
    return PID_Calculate(target_pos, actual_pos, &position_pid);
}
```

## 参数调优指南

### 1. PID参数调优步骤

#### 第一步：确定Kp
```c
// 从小开始逐步增大Kp，直到系统开始振荡
pid_controller.Kp = _IQ(0.1);  // 起始值
pid_controller.Ki = _IQ(0.0);  // 暂时关闭
pid_controller.Kd = _IQ(0.0);  // 暂时关闭
```

#### 第二步：加入Ki
```c
// Kp确定后，逐步加入Ki
pid_controller.Ki = _IQ(0.01);  // 从小值开始
```

#### 第三步：加入Kd
```c
// 最后加入Kd以改善动态响应
pid_controller.Kd = _IQ(0.001);  // 通常比Ki小
```

### 2. 参数调优建议

| 控制对象 | Kp范围 | Ki范围 | Kd范围 | 备注 |
|---------|--------|--------|--------|------|
| 电机速度 | 0.5-3.0 | 0.1-0.5 | 0.01-0.1 | 响应要求高 |
| 温度控制 | 1.0-5.0 | 0.01-0.2 | 0.05-0.2 | 惯性大，响应慢 |
| 位置控制 | 2.0-10.0 | 0.5-2.0 | 0.1-0.5 | 精度要求高 |
| 压力控制 | 0.8-4.0 | 0.05-0.3 | 0.02-0.15 | 波动敏感 |

### 3. 限幅参数设置

```c
// 积分限幅：通常设为输出限幅的50%-80%
pid_controller.integral_max = _IQ(输出最大值 * 0.7);

// 输出限幅：根据执行器能力设定
pid_controller.output_max = _IQ(执行器最大输出);

// 死区：设为测量精度的2-5倍
pid_controller.deadzone = _IQ(测量精度 * 3);
```

## 常见问题

### Q1: 什么时候使用位置式PID，什么时候使用增量式PID？

**位置式PID适用于：**
- 需要精确控制到目标值的场合
- 执行器能接受绝对位置指令
- 系统稳定性要求高

**增量式PID适用于：**
- 执行器只能接受增量指令
- 系统存在积分饱和问题
- 需要手动/自动切换的场合

### Q2: 如何选择积分抗饱和方法？

- **积分夹紧法**：最常用，适合大多数场合
- **反向计算法**：适合对响应速度要求高的系统
- **条件积分法**：适合噪声较大的系统

### Q3: 系统振荡怎么办？

```c
// 减小Kp
pid_controller.Kp = _IQmpy(pid_controller.Kp, _IQ(0.8));

// 增加Kd（如果没有噪声问题）
pid_controller.Kd = _IQmpy(pid_controller.Kd, _IQ(1.2));

// 启用死区
pid_controller.enable_deadzone = 1;
pid_controller.deadzone = _IQ(适当值);
```

### Q4: 稳态误差太大怎么办？

```c
// 增大Ki
pid_controller.Ki = _IQmpy(pid_controller.Ki, _IQ(1.5));

// 检查积分限幅是否过小
pid_controller.integral_max = _IQ(更大的值);

// 确保积分分离阈值合适
pid_controller.integral_separation_threshold = _IQ(合适的值);
```

### Q5: 如何处理启动冲击？

```c
// 启动时重置PID
PID_Reset(&pid_controller);

// 使用输出偏移
pid_controller.output_offset = _IQ(预设值);

// 启用积分分离
pid_controller.enable_integral_separation = 1;
pid_controller.integral_separation_threshold = _IQ(较小值);
```

## API参考

### 核心函数

#### PID_Init
```c
void PID_Init(PID_Controller_t *pid, PID_Type_e type);
```
- **功能**：初始化PID控制器
- **参数**：
  - `pid`: PID控制器指针
  - `type`: PID类型（位置式/增量式）

#### PID_Calculate
```c
_iq PID_Calculate(_iq target, _iq feedback, PID_Controller_t *pid);
```
- **功能**：执行PID计算
- **参数**：
  - `target`: 目标值
  - `feedback`: 反馈值
  - `pid`: PID控制器指针
- **返回值**：PID输出

#### PID_Reset
```c
void PID_Reset(PID_Controller_t *pid);
```
- **功能**：重置PID控制器状态
- **参数**：`pid`: PID控制器指针

### 辅助函数

#### Float转换函数
```c
float PID_GetOutputFloat(PID_Controller_t *pid);          // 获取float格式输出
float PID_GetErrorFloat(PID_Controller_t *pid);           // 获取float格式误差
void PID_SetTargetFloat(PID_Controller_t *pid, float target);    // 设置float格式目标
void PID_SetFeedbackFloat(PID_Controller_t *pid, float feedback); // 设置float格式反馈
```

#### 抗饱和函数
```c
void PID_SetAntiWindupMethod(PID_Controller_t *pid, AntiWindup_Method_e method, 
                            _iq threshold, _iq gain);
```
- **功能**：设置积分抗饱和方法
- **参数**：
  - `method`: 抗饱和方法
  - `threshold`: 抗饱和阈值
  - `gain`: 抗饱和增益

### 数据类型转换宏
```c
#define IQ_TO_FLOAT(iq_val)    _IQtoF(iq_val)     // IQ转float
#define FLOAT_TO_IQ(float_val) _IQ(float_val)     // float转IQ
```

### 控制器状态查询
```c
// 直接访问结构体成员
float current_error = IQ_TO_FLOAT(pid_controller.error);
float current_output = IQ_TO_FLOAT(pid_controller.output);
float integral_value = IQ_TO_FLOAT(pid_controller.integral);
```

---

## 版本信息
- **版本**：v2.0
- **更新日期**：2025-06-21
- **兼容性**：TI IQmath库，C99标准
- **作者**：PID控制器开发团队

如有问题请参考源码注释或联系技术支持。