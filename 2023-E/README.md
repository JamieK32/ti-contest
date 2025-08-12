# 🎯 2023年E题：运动目标控制与自动追踪系统

> 🔴 基于双激光云台的运动目标控制与追踪系统，成功完成基本要求部分

## 📋 题目概述

设计制作运动目标控制与自动追踪系统，包括：
- 🔴 **红色激光系统**: 模拟运动目标，可控制光斑在屏幕内任意移动
- 🟢 **绿色激光系统**: 自动追踪红色光斑，指示追踪效果
- 🎮 **二维云台控制**: 精确控制激光笔姿态和位置
- 📷 **视觉识别系统**: 实时检测红色激光点位置

## ✅ 完成情况

### 🎯 基本要求 (已完成)

- ✅ **复位功能**: 红色光斑从任意位置回到屏幕中心原点
- ✅ **大框循迹**: 红色光斑30秒内沿屏幕边线顺时针移动一周
- ✅ **A4靶纸循迹**: 红色光斑30秒内沿A4纸黑胶带边缘移动一周
- ✅ **指定位置循迹**: 红色光斑在指定位置完成A4靶纸循迹

### ❌ 发挥部分 (未完成)

- ❌ **自动追踪功能**: 绿色激光自动追踪红色激光点
- ❌ **高速追踪**: 复杂运动模式下的实时追踪
- ❌ **其他创新功能**: 扩展功能未实现

## 🛠️ 技术方案

### 💻 硬件平台
- **主控**: STM32F103C8T6 微控制器
- **云台**: 二维舵机云台系统 (SG90)
- **激光器**: 红色激光笔 + 绿色激光笔
- **视觉**: MaixCAM激光点识别模块
- **显示**: OLED显示屏 + 按键交互

### 🧠 软件架构
```c
// 核心组件
├── laser_draw.*        // 激光绘制控制
├── Servo.*            // 舵机驱动
├── ServoPid.*         // 舵机PID控制
├── openmv.*           // 视觉通信
├── svg_renderer.*     // SVG路径渲染
└── ui_*               // 用户界面
```

### 📷 视觉识别
```python
# MaixCAM激光检测算法
class LaserDetector:
    def __init__(self, thresholds=None, area_range=(5, 500)):
        self.red_thresholds = [
            [[50, 100, 3, 127, 0, 127]],  # 多阈值自适应
            [(45, 86, 16, 123, -8, 127)],
            [[40, 90, 10, 80, 5, 70]],
        ]
        
    def find_best_blob(self, blobs):
        # 查找最佳激光点候选
        return best_blob
```

## 🎯 核心算法

### 🎮 云台控制算法
```c
// PID舵机控制
typedef struct {
    float kp, ki, kd;           // PID参数
    float target_angle;         // 目标角度
    float current_angle;        // 当前角度
    uint16_t pwm_output;        // PWM输出
} servo_pid_t;

// 平滑运动控制
void servo_smooth_move(servo_pid_t *servo, float target) {
    // 梯形速度曲线，确保平滑运动
}
```

### 📐 路径规划算法
```c
// SVG路径渲染引擎
typedef struct {
    float x, y;                 // 路径点坐标
    uint32_t timestamp;         // 时间戳
    path_cmd_t command;         // 路径指令
} path_point_t;

// 支持的路径类型
enum {
    PATH_RESET_TO_CENTER,       // 回中心点
    PATH_SQUARE_BORDER,         // 大框边线
    PATH_A4_RECTANGLE,          // A4靶纸
    PATH_CUSTOM_POSITION,       // 自定义位置
};
```

### 🔄 实时控制系统
```c
// 主控制循环
void laser_control_task(void) {
    // 1. 读取视觉反馈
    laser_pos_t current_pos = get_laser_position();
    
    // 2. 路径规划计算
    path_point_t next_point = path_planning_update();
    
    // 3. PID控制输出
    servo_control_update(next_point.x, next_point.y);
    
    // 4. 状态更新显示
    ui_update_status();
}
```

## 📊 性能表现

| 测试项目 | 要求指标 | 实际表现 | 状态 |
|---------|---------|---------|------|
| 复位精度 | ≤2cm | ~1.5cm | ✅ |
| 大框时间 | 30s | ~28s | ✅ |
| 大框精度 | ≤2cm | ~1.8cm | ✅ |
| A4循迹时间 | 30s | ~29s | ✅ |
| A4循迹精度 | 不脱离胶带 | 轻微偏移 | ✅ |

## 📁 代码结构

```
2023-e-Code/
├── 📷 maixCam/                     # 视觉识别模块
│   ├── laser_detector.py           # 激光点检测算法
│   ├── main.py                     # 主程序
│   ├── rectangle_detector.py       # 矩形检测
│   └── rectangle_utils.py          # 几何工具函数
└── 🔧 stm32f1hal/                 # STM32主控代码
    ├── Core/                       # 系统核心
    │   ├── Inc/                    # 头文件
    │   └── Src/                    # 源文件
    ├── Custom/HardWare/            # 硬件驱动
    │   ├── Servo.*                 # 舵机控制
    │   ├── laser_draw.*            # 激光绘制
    │   ├── openmv.*                # 视觉通信
    │   └── svg_renderer.*          # SVG渲染
    ├── Custom/Ui/                  # 用户界面
    │   ├── oled_driver.*           # OLED驱动
    │   ├── ui_*                    # 界面组件
    │   └── multi_button.*          # 按键处理
    └── MDK-ARM/                    # Keil工程
```

## 🎮 使用说明

### 📋 环境要求
- STM32F103C8T6开发板
- MaixCAM视觉模块
- SG90舵机云台 x2
- 红色/绿色激光笔
- 1m距离白色屏幕
- Python 3.7+ (MaixCAM)

### ⚙️ 部署步骤

1. **硬件安装**
```bash
# 云台安装
- 红色激光: 正对屏幕1m距离
- 绿色激光: 距红色激光0.4-1m范围
- 屏幕: 0.6x0.6m白色区域，中心标记原点
```

2. **视觉模块**
```bash
cd maixCam/
python main.py  # 启动激光检测
```

3. **主控程序**
```bash
# 使用Keil打开 stm32f1hal/MDK-ARM/ 工程
# 编译下载到STM32开发板
```

4. **系统校准**
```c
// 关键参数配置
#define SERVO_X_CENTER_PWM    1500    // X轴中心位置
#define SERVO_Y_CENTER_PWM    1500    // Y轴中心位置
#define LASER_MOVE_SPEED      50      // 激光移动速度
```

## 🎯 核心亮点

- 🎯 **高精度控制**: 激光定位精度达到1.5cm，优于要求
- ⚡ **平滑运动**: 梯形速度曲线确保激光轨迹连续
- 🧠 **SVG渲染**: 支持复杂路径的矢量绘制
- 📷 **自适应视觉**: 多阈值算法适应不同光照条件
- 🎮 **用户友好**: OLED界面直观显示系统状态

## 📝 总结

本项目成功完成了2023年E题的**全部基本要求**，实现了精确的激光云台控制系统。在复位功能、大框循迹、A4靶纸循迹等方面表现出色，各项精度指标均满足题目要求。虽然发挥部分的自动追踪功能未完成，但基础控制系统稳定可靠，为后续追踪功能提供了良好的硬件和软件基础。

---

📖 **详细技术文档**: 参见 `E_运动目标控制与自动追踪系统.pdf`