#ifndef LASER_DRAW_H
#define LASER_DRAW_H

#include "stm32f1xx_hal.h"
#include "Emm_V5.h"
#include "Emm_V5_Receive.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// 电机地址定义
#define MOTOR_X_ADDR    1    // X轴电机地址
#define MOTOR_Y_ADDR    2    // Y轴电机地址

// 绘图参数
#define DRAW_SPEED      50  // 绘图速度 (RPM)
#define MOVE_SPEED     50  // 移动速度 (RPM)
#define ACCELERATION    0   // 加速度

// 激光控制引脚 (假设使用PA8控制激光)
#define LASER_GPIO_PORT GPIOA
#define LASER_GPIO_PIN  GPIO_PIN_8

#define AXIS_SCALE_X  1.0f      // X轴保持不变
#define AXIS_SCALE_Y  1.21f      // Y轴放大1.2倍（根据您的实际情况调整）

// 绘图状态
typedef enum {
    DRAW_IDLE = 0,      // 空闲状态
    DRAW_MOVING,        // 移动中
    DRAW_DRAWING,       // 绘图中
    DRAW_COMPLETE,      // 完成
    DRAW_ERROR          // 错误
} DrawState_t;

// 图形类型
typedef enum {
    SHAPE_NONE = 0,
    SHAPE_SQUARE,       // 正方形
    SHAPE_RECTANGLE,    // 矩形
    SHAPE_CIRCLE,       // 圆形
    SHAPE_TRIANGLE,     // 三角形
    SHAPE_LINE,         // 直线
    SHAPE_CROSS         // 十字
} ShapeType_t;

// 绘图参数结构
typedef struct {
    ShapeType_t shape;      // 图形类型
    int16_t center_x;       // 中心X坐标 (脉冲数)
    int16_t center_y;       // 中心Y坐标 (脉冲数)
    int16_t size_x;         // X方向尺寸 (脉冲数)
    int16_t size_y;         // Y方向尺寸 (脉冲数)
    uint16_t speed;         // 绘图速度 (RPM)
    bool laser_on;          // 激光开关状态
} DrawParams_t;

// 激光绘图系统结构
typedef struct {
    DrawState_t state;      // 当前状态
    DrawParams_t params;    // 绘图参数
    uint8_t step;          // 当前步骤
    uint32_t last_time;    // 上次更新时间
    bool x_ready;          // X轴是否到位
    bool y_ready;          // Y轴是否到位
} LaserDraw_t;

// 函数声明

// 初始化激光绘图系统
void LaserDraw_Init(void);

// 激光控制
void Laser_On(void);
void Laser_Off(void);

// 电机控制
void Motors_Enable(void);
void Motors_Disable(void);
void Motors_Stop(void);

// 移动到指定位置 (绝对位置)
void MoveTo(int16_t x, int16_t y, uint16_t speed);

// 画线到指定位置 (绝对位置，激光打开)
void DrawLineTo(int16_t x, int16_t y, uint16_t speed);

// 绘制基本图形
void DrawSquare(int16_t center_x, int16_t center_y, int16_t size);
void DrawRectangle(int16_t center_x, int16_t center_y, int16_t width, int16_t height);
void DrawCircle(int16_t center_x, int16_t center_y, int16_t radius);
void DrawTriangle(int16_t center_x, int16_t center_y, int16_t size);
void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void DrawCross(int16_t center_x, int16_t center_y, int16_t size);

// 开始绘制图形 (异步)
void StartDrawShape(ShapeType_t shape, int16_t center_x, int16_t center_y, 
                   int16_t size_x, int16_t size_y, uint16_t speed);

// 状态机处理 (在定时器中调用)
void LaserDraw_Process(void);

// 获取当前状态
DrawState_t GetDrawState(void);

// 检查是否绘制完成
bool IsDrawComplete(void);

// 回零操作
void GoHome(void);

// 获取当前位置
void GetCurrentPosition(int32_t* x, int32_t* y);

void DrawChineseBitmap(int16_t start_x, int16_t start_y, int16_t pixel_size);


//绘制正弦波
void DrawSineWave(int16_t start_x, int16_t start_y, int16_t length, 
                  int16_t amplitude, float frequency, float phase, int points);

//高精度三角波
void DrawTriangleWave(int16_t start_x, int16_t start_y, int16_t length, 
                     int16_t amplitude, float frequency, int points);

//绘制方波
void DrawSquareWave(int16_t start_x, int16_t start_y, int16_t length, 
                         int16_t amplitude, float frequency, float phase);

void DrawPixelDot(int16_t x, int16_t y, int16_t size);

void ResetMotorZeroPosition(void);

void DrawImage128x128(const unsigned char* image_data, int16_t start_x, int16_t start_y, 
                      int16_t pixel_size, float pixel_spacing, uint8_t skip_pixels);

#endif // LASER_DRAW_H