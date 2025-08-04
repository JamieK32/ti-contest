# 🚗 TI竞赛智能车系统 - 四年完整方案集

> 🏆 基于TI MSPM0G3507微控制器的智能车竞赛系统，涵盖2021-2024年全国大学生电子设计竞赛完整解决方案

## 📋 项目概述

## 📚 Table of Contents

- [📋 项目概述](#-项目概述)
- [🛠 硬件支持](#-硬件支持)
- [📁 项目结构](#-项目结构)
- [🚀 快速开始](#-快速开始)
- [💡 核心算法](#-核心算法)
- [🎮 功能模块](#-功能模块)
- [🏆 竞赛成果](#-竞赛成果)
- [🤝 贡献指南](#-贡献指南)
- [📄 许可证](#-许可证)
- [🙏 致谢](#-致谢)
- [📞 联系方式](#-联系方式)

本项目是基于 **TI MSPM0G3507** 微控制器开发的智能小车系统，涵盖了**2021-2024年全国大学生电子设计竞赛**的完整解决方案。项目采用统一的软件架构和硬件平台，通过模块化设计实现多年度题目适配，展现了系统性设计在电子竞赛中的优势。

### ✨ 核心特性

- 🏆 **全题目覆盖**: 支持2021-2024年全国电赛智能车相关题目
- 🎯 **统一架构**: 基于状态机的通用任务调度系统
- 👁️ **多视觉方案**: 集成MaixCAM数字识别、激光检测、目标跟踪
- 🧭 **精确导航**: 多传感器融合定位与路径规划
- 🔧 **模块化设计**: 易于扩展和维护的分层架构
- 📱 **人机交互**: OLED显示屏 + 按键菜单系统
- 🎵 **多媒体支持**: 音频播放和LED指示系统

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

```
ti-contest/
├── 🚗 2021-F/                       # 智能送药小车
│   ├── 📄 README.md
│   ├── 📄 F_智能送药小车.pdf
│   ├── 💻 2021-f-Code/
│   │   ├── 🎯 maixcam-digit-recognition/  # 数字识别
│   │   └── 🔧 mspm0g3507/                # 主控代码
│   ├── 📋 2021-f-DesignReport/
│   ├── ⚡ 2021-f-HardwareDesign/
│   └── 📚 2021-f-Resources/
├── 🤝 2022-C/                       # 双车跟随系统
│   ├── 📄 README.md
│   ├── 📄 C_运动目标控制与跟踪.pdf
│   ├── 💻 2022-c-Code/
│   │   ├── 🎯 maixcam-tracking/          # 目标跟踪
│   │   └── 🔧 mspm0g3507/                # 主控代码
│   └── 📋 设计报告/
├── 🎯 2023-E/                       # 激光笔追踪系统
│   ├── 📄 README.md
│   ├── 📄 E_运动目标控制与自动追踪系统.pdf
│   ├── 💻 2023-e-Code/
│   │   ├── 🔴 maixCam/                   # 激光检测
│   │   └── 🔧 stm32f1hal/                # STM32方案
│   └── 📋 设计报告/
├── 🧭 2024-H/                       # 循迹惯导系统
│   ├── 📄 README.md
│   ├── 📄 H_移动机器人.pdf
│   ├── 💻 2024-h-Code/
│   │   ├── 🎯 navigation-fusion/         # 惯导融合
│   │   └── 🔧 mspm0g3507/                # 主控代码
│   └── 📋 设计报告/
└── 📄 README.md                     # 项目说明
```

### 🏗️ 统一软件架构

```
应用层/
├── 📋 task_2021f/           # 2021年F题: 智能送药小车
├── 📋 task_2022c/           # 2022年C题: 双车跟随系统  
├── 📋 task_2023e/           # 2023年E题: 激光笔追踪系统
├── 📋 task_2024h/           # 2024年H题: 循迹惯导系统
├── 🎛️ control/              # 通用控制算法
│   ├── car_state_machine/   # 状态机核心
│   ├── car_controller/      # 运动控制
│   └── car_pid/            # PID算法
├── 🔧 drivers/             # 硬件驱动层
│   ├── sensors/            # 传感器驱动
│   ├── actuators/          # 执行器驱动
│   └── communication/      # 通信模块
├── 🧬 middleware/          # 中间件层
│   ├── ui/                 # 用户界面
│   ├── fusion/             # 传感器融合
│   └── protocol/           # 通信协议
└── 🔩 hal/                 # 硬件抽象层
```

## 🚀 快速开始

### 📋 环境要求
- Keil MDK-ARM 5.x 或更高版本
- TI Code Composer Studio (可选)
- Python 3.7+ (视觉模块)
- 硬件调试器 (J-Link/ST-Link)

### 🔧 编译步骤

1. **克隆仓库**
```bash
git clone https://github.com/jamiek32/ti-contest
cd ti-contest-2021-f/2021-f-Code/mspm0g3507
```

### 🔧 配置任务类型
```c
// 在 common_defines.h 中选择题目
#define CURRENT_TASK_TYPE TASK_TYPE_21F  // 2021年F题: 智能送药小车
// #define CURRENT_TASK_TYPE TASK_TYPE_22C  // 2022年C题: 双车跟随系统
// #define CURRENT_TASK_TYPE TASK_TYPE_23E  // 2023年E题: 激光追踪系统
// #define CURRENT_TASK_TYPE TASK_TYPE_24H  // 2024年H题: 循迹惯导系统
```

3. **硬件配置**
```c
// 在对应的 config.h 文件中配置硬件
#define CURRENT_MOTOR TB6612_MOTOR        // 电机类型
#define CURRENT_IMU MPU6050_GYRO          // 陀螺仪类型
#define CURRENT_DISPLAY OLED_DISPLAY      // 显示屏类型
```

4. **编译下载**
- 打开 `project/Keil/EmbedBolt316.uvprojx`
- 编译工程 (F7)
- 下载到目标板 (F8)

### 🎯 视觉模块部署
```bash
cd maixcam-digit-recognition
python main.py  # 启动数字识别程序
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

## 🎮 功能模块

### 📱 菜单系统
- **任务执行**: 一键启动竞赛任务
- **参数调节**: 实时调整PID参数
- **传感器监控**: 显示传感器实时数据
- **系统状态**: 查看系统运行状态

### 🎵 多媒体功能
- **音乐播放**: 支持背景音乐播放
- **状态提示**: 蜂鸣器声音反馈
- **LED指示**: RGB状态指示灯

### 🔧 调试功能
- **串口调试**: 实时数据输出
- **蓝牙遥控**: 无线参数调节
- **单元测试**: 各模块独立测试

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