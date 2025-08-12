# 🚗 TI竞赛智能车系统 - 统一平台架构

> 🏆 基于TI MSPM0G3507微控制器的智能车竞赛统一平台，采用现代化软件架构，涵盖2021-2024年全国大学生电子设计竞赛完整解决方案

## 📋 项目概述

本项目是基于 **TI MSPM0G3507** 微控制器开发的**统一智能车竞赛平台**，通过**模块化分层架构**和**标准化接口设计**，实现了对**2021-2024年全国大学生电子设计竞赛**智能车题目的完整支持。项目经过大幅度重构，建立了统一的开发平台和代码规范，具备出色的可维护性和扩展性。

## 📚 Table of Contents

- [📋 项目概述](#-项目概述)
- [🏗️ 架构特色](#-架构特色)
- [🛠 硬件支持](#-硬件支持)
- [📁 项目结构](#-项目结构)
- [🚀 快速开始](#-快速开始)
- [💡 核心算法](#-核心算法)
- [🎮 功能模块](#-功能模块)
- [🏆 竞赛成果](#-竞赛成果)
- [🧪 测试支持](#-测试支持)
- [🤝 贡献指南](#-贡献指南)
- [📄 许可证](#-许可证)
- [🙏 致谢](#-致谢)
- [📞 联系方式](#-联系方式)

## 🏗️ 架构特色

### ✨ 核心优势

- 🏗️ **统一平台架构**: 重构后的分层设计，支持多项目复用
- 🔧 **标准化接口**: HAL抽象层统一硬件访问接口
- 📦 **模块化组件**: 高度解耦的功能模块，易于测试和维护
- 🎯 **面向任务设计**: 基于状态机的任务调度系统
- 🧪 **完整测试支持**: 内置单元测试框架
- 📚 **丰富中间件**: 融合算法、通信协议、UI组件一应俱全
- ⚡ **高性能驱动**: 优化的传感器和执行器驱动程序

### 🏗️ 平台架构

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层 (Application)                     │
├─────────────────────────────────────────────────────────────┤
│  task_2021f │ task_2022c │ task_2023e │ task_2024h │ 控制模块 │
├─────────────────────────────────────────────────────────────┤
│                     中间件层 (Middleware)                     │
├─────────────────────────────────────────────────────────────┤
│   融合算法   │   通信协议   │    UI组件    │   状态机调度   │
├─────────────────────────────────────────────────────────────┤
│                      驱动层 (Drivers)                        │
├─────────────────────────────────────────────────────────────┤
│  传感器驱动  │  执行器驱动  │  通信驱动   │   显示驱动    │
├─────────────────────────────────────────────────────────────┤
│                   硬件抽象层 (HAL)                           │
├─────────────────────────────────────────────────────────────┤
│    UART     │     I2C     │     SPI     │     ADC      │
├─────────────────────────────────────────────────────────────┤
│                  TI MSPM0G3507 硬件平台                      │
└─────────────────────────────────────────────────────────────┘
```

## 🛠 硬件支持

### 🚀 核心控制器
- **主控芯片**: TI MSPM0G3507 (ARM Cortex-M0+)
- **开发环境**: Keil MDK-ARM
- **调试接口**: SWD

### ⚡ 电机驱动系统
- **L298N驱动器**: 双H桥电机驱动，支持正反转和调速
- **TB6612驱动器**: 高效率MOSFET驱动，低功耗设计
- **编码器支持**: 精确的速度和位置反馈
- **PID控制**: 闭环速度和位置控制算法

### 🎯 传感器阵列

#### 📐 姿态感知
- **MPU6050**: 6轴IMU，DMP硬件融合算法
- **IMU660RA**: 高精度陀螺仪加速度计
- **WIT陀螺仪 (JY61P)**: 串口输出姿态角度

#### 📏 距离测量
- **VL53L1X**: ToF激光测距传感器 (4m范围)
- **超声波传感器**: 备用距离检测方案

#### 🔍 视觉传感
- **灰度传感器阵列**: 循迹和路径检测
- **MaixCAM模块**: 数字识别和图像处理
- **自适应阈值**: GANV自动阈值调节算法

### 📡 通信模块
- **蓝牙模块**: 无线调试和遥控
- **UART**: 与视觉模块通信
- **I2C总线**: 传感器数据采集
- **SPI接口**: 高速数据传输
- **软件I2C**: 灵活的传感器扩展

### 🖥️ 显示交互
- **OLED显示屏**: 128x64像素，实时状态显示
- **U8G2图形库**: 丰富的显示效果
- **多级菜单系统**: 参数调节和功能选择
- **按键输入**: 多按键检测，支持长按短按

### 🔌 IO扩展
- **PCA9555**: I2C GPIO扩展芯片
- **74HC595**: SPI移位寄存器扩展
- **RGB LED**: 状态指示和视觉反馈
- **蜂鸣器**: 音频提示和音乐播放

## 📁 项目结构

### 🏗️ 新版统一平台架构

```
ti-contest/
├── 📦 platforms/                           # 统一开发平台
│   └── ti-mspm0g3507/                      # TI MSPM0G3507平台
│       ├── 💻 custom_src/                  # 平台源代码
│       │   ├── 🎯 application/             # 应用层
│       │   │   └── control/                # 控制算法模块
│       │   │       ├── car_config.h        # 小车配置参数
│       │   │       ├── car_controller.h    # 运动控制器
│       │   │       ├── car_state_machine.* # 状态机核心
│       │   │       └── pid.*               # PID算法实现
│       │   ├── 🔧 drivers/                 # 驱动层
│       │   │   ├── sensors/                # 传感器驱动
│       │   │   │   ├── encoder/            # 编码器驱动
│       │   │   │   ├── gray_detect/        # 灰度检测
│       │   │   │   ├── imu660ra/           # 高精度IMU
│       │   │   │   ├── mpu6050/            # MPU6050 DMP
│       │   │   │   ├── vl53l1x/            # ToF激光测距
│       │   │   │   └── wit_gyro/           # WIT陀螺仪
│       │   │   ├── actuators/              # 执行器驱动
│       │   │   │   ├── motor/              # 电机驱动(L298N/TB6612)
│       │   │   │   └── voice_light_alert/  # 声光报警
│       │   │   ├── communication/          # 通信驱动
│       │   │   │   ├── bluetooth.*         # 蓝牙通信
│       │   │   │   └── maix_cam.*          # MaixCAM通信
│       │   │   ├── display/                # 显示驱动
│       │   │   │   └── oled/               # OLED显示屏
│       │   │   └── io_expander/            # IO扩展
│       │   │       ├── _74hc595.*          # 移位寄存器
│       │   │       └── pca9555.*           # I2C GPIO扩展
│       │   ├── 🧬 middleware/              # 中间件层
│       │   │   ├── communication/          # 通信协议
│       │   │   │   ├── lwpkt/              # 轻量级数据包协议
│       │   │   │   ├── lwrb/               # 环形缓冲区
│       │   │   │   └── protocol/           # 自定义协议
│       │   │   ├── fusion/                 # 传感器融合算法
│       │   │   │   ├── Fusion.*            # 姿态融合库
│       │   │   │   ├── FusionAhrs.*        # AHRS算法
│       │   │   │   └── FusionCompass.*     # 电子罗盘
│       │   │   └── ui/                     # 用户界面
│       │   │       ├── button/             # 按键处理
│       │   │       └── graphics/           # 图形界面
│       │   ├── 🔩 hal/                     # 硬件抽象层
│       │   │   ├── adc/                    # ADC抽象
│       │   │   ├── i2c/                    # I2C抽象(含软件I2C)
│       │   │   ├── spi/                    # SPI抽象
│       │   │   └── uart/                   # UART抽象
│       │   ├── 🛠️ core/                    # 核心系统
│       │   │   ├── config/                 # 系统配置
│       │   │   └── system/                 # 中断&调度
│       │   └── 🔧 utils/                   # 工具函数
│       │       ├── delay.*                 # 延时函数
│       │       └── log.*                   # 日志系统
│       ├── 📊 tests/                       # 单元测试
│       │   └── unit_tests/                 # 各模块单元测试
│       ├── 🗂️ source/                      # 第三方库&TI SDK
│       │   ├── third_party/                # 第三方库
│       │   │   ├── CMSIS/                  # ARM CMSIS
│       │   │   └── u8g2/                   # U8G2图形库
│       │   └── ti/                         # TI官方库
│       │       ├── devices/                # 设备头文件
│       │       └── driverlib/              # TI驱动库
│       └── 🔨 project/                     # 工程文件
│           └── startup/                    # 启动文件
├── 🏆 2021-F/                             # 2021年F题: 智能送药小车
│   ├── 📄 README.md                       # 题目说明
│   ├── 📄 F_智能送药小车.pdf               # 题目要求
│   └── 💻 2021-f-Code/                    # 题目专用代码
│       ├── 🎯 maixCam/                    # 数字识别视觉
│       └── 🔧 mspm0g3507/                 # 主控代码
├── 🤝 2022-C/                             # 2022年C题: 双车跟随系统
│   ├── 📄 README.md
│   ├── 📄 C_小车跟随行驶系统.pdf
│   ├── 💻 2022-c-Code/
│   └── 📋 2022-c-DesignReport/            # 设计报告
├── 🎯 2023-E/                             # 2023年E题: 激光笔追踪系统
│   ├── 📄 README.md
│   ├── 📄 E_运动目标控制与自动追踪系统.pdf
│   └── 💻 2023-e-Code/
│       ├── 🔴 maixCam/                    # 激光检测视觉
│       └── 🔧 stm32f1hal/                 # STM32方案(历史)
├── 🧭 2024-H/                             # 2024年H题: 循迹惯导系统
│   ├── 📄 README.md
│   ├── 📄 H题_自动行驶小车.pdf
│   ├── 💻 2024-h-Code/
│   └── 📋 2024-h-DesignReport/
├── ⚡ hardware-design/                    # 硬件设计文件
│   ├── stm32-f103c8t6-23e/               # 2023年E题定制板
│   └── ti-mspm0g3507/                     # MSPM0G3507扩展板
└── 📚 resources/                          # 技术资料
    └── MSPM0G3507/                        # 芯片资料
```

### 🔄 重构亮点

- 🏗️ **统一平台**: 所有年度题目共享同一套硬件抽象层和驱动程序
- 📦 **模块解耦**: 传感器、执行器、通信模块完全独立，可任意组合
- 🧪 **测试完备**: 每个驱动模块都有对应的单元测试
- 🔧 **配置灵活**: 通过配置文件轻松切换硬件组合
- 📚 **中间件丰富**: 内置传感器融合、通信协议、UI组件等

## 🚀 快速开始

### 📋 环境要求
- Keil MDK-ARM 5.37 或更高版本
- TI Code Composer Studio (可选)
- Python 3.7+ (视觉模块开发)
- 硬件调试器 (J-Link/XDS110)

### 🔧 平台部署

1. **克隆仓库**
```bash
git clone https://github.com/jamiek32/ti-contest
cd ti-contest/platforms/ti-mspm0g3507
```

2. **配置开发环境**
```bash
# 选择对应年度的题目配置
cp task_configs/task_2024h_config.h custom_src/core/config/task_config.h
```

3. **编译工程**
- 打开 `project/Keil/` 下的工程文件
- 选择编译配置 (Debug/Release)
- 编译工程 (F7)

4. **下载调试**
- 连接调试器到目标板
- 下载程序 (F8)
- 启动调试 (Ctrl+F5)

### 🎯 任务配置

#### 2024年H题配置示例
```c
// custom_src/core/config/task_config.h
#define CURRENT_TASK             TASK_2024H_NAVIGATION
#define ENABLE_IMU_FUSION        1
#define ENABLE_GRAY_TRACKING     1
#define ENABLE_DISTANCE_SENSOR   1
#define MOTOR_TYPE               TB6612_MOTOR
#define IMU_TYPE                 IMU660RA_SENSOR
```

#### 硬件配置
```c
// custom_src/application/control/car_config.h
#define motor_count              4
#define WHEEL_RADIUS_CM          3.3f
#define WHEEL_BASE_CM            24.0f
#define PULSE_NUM_PER_CIRCLE     1470
```

## 💡 核心算法

### 🎛️ 状态机架构
```c
// 动作类型定义
typedef enum {
    ACTION_GO_STRAIGHT,      // 直行
    ACTION_SPIN_TURN,        // 原地转弯
    ACTION_TRACK,            // 循迹
    ACTION_MOVE_UNTIL_BLACK, // 移动到黑线
    ACTION_DELAY,            // 延时
    ACTION_FUNCTION,         // 函数调用
    ACTION_WAIT_FUNC_TRUE,   // 等待条件
} car_action_type_t;

// 使用示例
void setup_task(void) {
    car_path_init();
    car_add_straight(100);    // 直行100cm
    car_add_turn(90);         // 右转90度
    car_add_track(50);        // 循迹50cm
    car_start();              // 开始执行
}
```

### 🎯 PID控制算法
```c
// PID参数结构
typedef struct {
    float kp, ki, kd;        // PID参数
    float integral;          // 积分项
    float last_error;        // 上次误差
    float output_limit;      // 输出限制
} pid_t;

// PID计算
float pid_compute(pid_t *pid, float setpoint, float measured);
```

### 📐 传感器融合
```c
// 姿态融合算法
void attitude_update(float ax, float ay, float az,
                    float gx, float gy, float gz);
float get_yaw_angle(void);   // 获取偏航角
```

## 🧪 测试支持

### 🔬 单元测试框架

项目内置完整的单元测试支持，每个驱动模块都有对应的测试用例：

```
platforms/ti-mspm0g3507/tests/unit_tests/
├── 📊 tests.h                    # 测试框架头文件
├── 📶 bluetooth_test.c           # 蓝牙通信测试
├── 🌫️ gray_detection_test.c      # 灰度检测测试
├── 📷 maix_test.c                # MaixCAM通信测试
├── ⚙️ motor_test.c               # 电机驱动测试
├── 🎯 mpu6050_dmp_test.c         # MPU6050 DMP测试
├── 🔍 no_mcu_ganv_test.c         # GANV算法测试
├── 📏 vl53l1_test.c              # ToF测距传感器测试
└── 🧭 wit_jyxx_test.c            # WIT陀螺仪测试
```

### 🧪 测试运行

```c
// 在main函数中调用测试
#ifdef UNIT_TEST_MODE
    run_all_tests();
#else
    // 正常程序运行
    app_main();
#endif
```

### 📊 测试覆盖

- ✅ **硬件驱动测试**: 传感器、执行器、通信模块
- ✅ **算法验证测试**: PID控制、传感器融合、路径规划
- ✅ **通信协议测试**: 串口、蓝牙、MaixCAM通信
- ✅ **系统集成测试**: 多模块协同工作验证

## 🏆 竞赛成果

### 📊 支持题目
- **2021年全国赛F题**: 智能送药小车 ⭐
- **2022年全国赛C题**: 双车跟随系统 ⭐
- **2023年全国赛E题**: 激光笔追踪系统 ⭐
- **2024年全国赛H题**: 移动机器人 ⭐

### 🎯 竞赛题目覆盖

| 年份 | 题目 | 技术方案 | 核心特色 |
|------|------|----------|----------|
| **2021年F题** | 智能送药小车 | 数字识别 + 路径规划 | MaixCAM数字识别、多路径决策 |
| **2022年C题** | 双车跟随系统 | 目标跟踪 + 协同控制 | 多车通信、跟随算法 |
| **2023年E题** | 激光笔追踪系统 | 激光检测 + 伺服控制 | 红色激光点识别、云台追踪 |
| **2024年H题** | 循迹惯导系统 | 传感器融合 + 精确导航 | IMU融合、高精度定位 |

## 🤝 贡献指南

我们欢迎任何形式的贡献！

### 🐛 问题反馈
- 在 [Issues](https://github.com/JamieK32/ti-contest/issues) 中报告bug
- 提供详细的复现步骤和环境信息

### 💡 功能请求
- 在 [Discussions](https://github.com/JamieK32/ti-contest/discussions) 中讨论新功能
- 描述使用场景和期望效果

### 🔧 代码贡献
1. Fork 本仓库
2. 创建功能分支: `git checkout -b feature/新功能`
3. 提交更改: `git commit -m '添加新功能'`
4. 推送分支: `git push origin feature/新功能`
5. 提交 Pull Request

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源协议。

## 🙏 致谢

- [TI德州仪器](https://www.ti.com.cn/) - 提供优秀的微控制器平台
- [全国大学生电子设计竞赛](http://nuedc.sjtu.edu.cn/) - 推动电子技术发展
- 所有为本项目贡献代码的开发者们

## 📞 联系方式

- **项目维护者**: [JamieK]
- **项目维护者qq**: 2428956158

---

⭐ 如果这个项目对您有帮助，请给我们一个Star！

📢 **涵盖2021-2024年TI杯电赛智能车题目完整解决方案，助力电赛冲奖！**