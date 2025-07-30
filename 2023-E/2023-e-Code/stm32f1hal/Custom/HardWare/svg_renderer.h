#ifndef SVG_RENDERER_H
#define SVG_RENDERER_H

#include <stdint.h>
#include <stdbool.h>

// 包含激光绘图系统的头文件
#include "laser_draw.h"

// SVG绘制参数配置
#define SVG_CURVE_SEGMENTS  20      // 曲线分段数
#define SVG_LINE_SEGMENTS   10      // 直线分段数
#define SVG_DRAW_SPEED      25      // 绘制速度
#define SVG_MOVE_SPEED      25      // 移动速度
#define SVG_LINE_SEGMENT_THRESHOLD 5.0f  // 直线分段距离阈值

/**
 * @brief 绘制三次贝塞尔曲线
 * @param x0,y0 起始点坐标
 * @param x1,y1 第一个控制点坐标
 * @param x2,y2 第二个控制点坐标
 * @param x3,y3 结束点坐标
 * @param start_x,start_y 绘制起始偏移
 * @param scale 缩放比例
 */
void DrawBezierCurve(float x0, float y0, float x1, float y1, 
                    float x2, float y2, float x3, float y3,
                    int16_t start_x, int16_t start_y, float scale);

/**
 * @brief 绘制二次贝塞尔曲线（转换为三次）
 * @param x0,y0 起始点坐标
 * @param x1,y1 控制点坐标
 * @param x2,y2 结束点坐标
 * @param start_x,start_y 绘制起始偏移
 * @param scale 缩放比例
 */
void DrawQuadBezier(float x0, float y0, float x1, float y1, float x2, float y2,
                   int16_t start_x, int16_t start_y, float scale);

/**
 * @brief 绘制完整SVG路径（支持主要命令：M/L/H/V/C/S/Q/T/A/Z）
 * @param path_data SVG路径数据字符串
 * @param start_x,start_y 绘制起始位置
 * @param scale 缩放比例
 */
void DrawSVGPath(const char* path_data, int16_t start_x, int16_t start_y, float scale);

/**
 * @brief 绘制简单SVG路径（仅支持：M/L/H/V/Z命令）
 * @param path_data SVG路径数据字符串
 * @param start_x,start_y 绘制起始位置
 * @param scale 缩放比例
 */
void DrawSVGPathSimple(const char* path_data, int16_t start_x, int16_t start_y, float scale);

/**
 * @brief 绘制多边形
 * @param points 点坐标数组 [x1,y1,x2,y2,...]
 * @param point_count 点的数量
 * @param start_x,start_y 绘制起始位置
 * @param scale 缩放比例
 * @param closed 是否闭合多边形
 */
void DrawPolygon(const float* points, int point_count, 
                int16_t start_x, int16_t start_y, float scale, bool closed);

// 兼容旧版本的弧线绘制函数（如果需要的话）
/**
 * @brief 绘制简单弧线
 * @param center_x,center_y 圆心坐标
 * @param radius 半径
 * @param start_angle,end_angle 起始和结束角度（度数）
 */
void DrawArc(float center_x, float center_y, float radius, 
            float start_angle, float end_angle);

#endif // SVG_RENDERER_H