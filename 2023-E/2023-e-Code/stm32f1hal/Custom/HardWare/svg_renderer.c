#include "svg_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 调试开关
#define SVG_DEBUG 1

#if SVG_DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while (0)
#endif

// 当前激光状态和位置
static bool laser_on = false;
static float current_x = 0, current_y = 0;  // 变换后的坐标（用于绘图）
static float svg_x = 0, svg_y = 0;          // 原始SVG坐标（用于相对坐标计算）
static float path_start_x = 0, path_start_y = 0;  // 路径起始点（变换后）
static float svg_path_start_x = 0, svg_path_start_y = 0;  // SVG路径起始点

/**
 * @brief 设置激光状态（避免重复开关）
 */
static void SetLaser(bool on) {
    if (laser_on != on) {
        if (on) {
            Laser_On();
        } else {
            Laser_Off();
        }
        laser_on = on;
    }
}

/**
 * @brief 跳过空白字符和逗号
 */
static const char* SkipSpaces(const char* str) {
    while (*str && (isspace(*str) || *str == ',')) {
        str++;
    }
    return str;
}

static void TransformCoordinates(float svg_x, float svg_y, float scale, 
                                int16_t offset_x, int16_t offset_y,
                                float* laser_x, float* laser_y) {
    // 1. 应用SVG缩放
    *laser_x = svg_x * scale;
    *laser_y = svg_y * scale;
    
    // 2. Y轴翻转（如果需要）
    *laser_y = -*laser_y;
    
    DEBUG_PRINT("Transform: SVG(%.1f, %.1f) * %.1f -> Relative(%.1f, %.1f)\n", 
                svg_x, svg_y, scale, *laser_x, *laser_y);
}

static void MoveToPoint(float x, float y, int16_t start_x, int16_t start_y, float scale) {
    // 保存SVG坐标
    svg_x = x;
    svg_y = y;
    
    // 坐标变换（不包含偏移）
    float rel_x, rel_y;
    TransformCoordinates(x, y, scale, 0, 0, &rel_x, &rel_y);
    
    // 计算最终的绝对坐标
    int16_t abs_x = start_x + (int16_t)rel_x;
    int16_t abs_y = start_y + (int16_t)rel_y;
    
    // 保存变换后的坐标
    current_x = abs_x;
    current_y = abs_y;
    
    SetLaser(false);
    MoveTo(abs_x, abs_y, SVG_MOVE_SPEED);
    
    DEBUG_PRINT("MoveTo: SVG(%.1f, %.1f) -> Laser(%d, %d)\n", 
                x, y, abs_x, abs_y);
}

/**
 * @brief 分段绘制直线（类似曲线的分段处理）
 * @param start_svg_x 起始点SVG X坐标
 * @param start_svg_y 起始点SVG Y坐标  
 * @param end_svg_x 结束点SVG X坐标
 * @param end_svg_y 结束点SVG Y坐标
 * @param start_x 激光起始偏移X
 * @param start_y 激光起始偏移Y
 * @param scale 缩放因子
 */
static void DrawSegmentedLine(float start_svg_x, float start_svg_y, 
                             float end_svg_x, float end_svg_y,
                             int16_t start_x, int16_t start_y, float scale) {
    
    SetLaser(true);
    
    // 计算总距离，如果距离很短就直接绘制
    float dx = end_svg_x - start_svg_x;
    float dy = end_svg_y - start_svg_y;
    float distance = sqrtf(dx*dx + dy*dy);
    
    // 如果距离小于阈值，直接绘制
    if (distance < SVG_LINE_SEGMENT_THRESHOLD) {
        // 转换为激光坐标并绘制
        float rel_x, rel_y;
        TransformCoordinates(end_svg_x, end_svg_y, scale, 0, 0, &rel_x, &rel_y);
        
        // 加上偏移
        int16_t abs_x = start_x + (int16_t)rel_x;
        int16_t abs_y = start_y + (int16_t)rel_y;
        
        DrawLineTo(abs_x, abs_y, SVG_DRAW_SPEED);
        
        DEBUG_PRINT("DrawTo (direct): SVG(%.1f, %.1f) -> Laser(%d, %d)\n", 
                    end_svg_x, end_svg_y, abs_x, abs_y);
    } else {
        // 分段绘制
        DEBUG_PRINT("DrawTo (segmented): distance=%.1f, segments=%d\n", distance, SVG_LINE_SEGMENTS);
        
        for (int i = 1; i <= SVG_LINE_SEGMENTS; i++) {
            float t = (float)i / SVG_LINE_SEGMENTS;
            
            // 线性插值计算中间点
            float svg_x_line = start_svg_x + t * dx;
            float svg_y_line = start_svg_y + t * dy;
            
            // 转换为激光坐标并绘制
            float rel_x, rel_y;
            TransformCoordinates(svg_x_line, svg_y_line, scale, 0, 0, &rel_x, &rel_y);
            
            // 加上偏移
            int16_t abs_x = start_x + (int16_t)rel_x;
            int16_t abs_y = start_y + (int16_t)rel_y;
            
            DrawLineTo(abs_x, abs_y, SVG_DRAW_SPEED);
            
            DEBUG_PRINT("Line segment %d/%d: SVG(%.1f, %.1f) -> Laser(%d, %d)\n", 
                        i, SVG_LINE_SEGMENTS, svg_x_line, svg_y_line, abs_x, abs_y);
        }
    }
    
    // 更新当前位置
    svg_x = end_svg_x;
    svg_y = end_svg_y;
    float rel_x, rel_y;
    TransformCoordinates(end_svg_x, end_svg_y, scale, 0, 0, &rel_x, &rel_y);
    current_x = start_x + rel_x;
    current_y = start_y + rel_y;
}

static void DrawToPoint(float x, float y, int16_t start_x, int16_t start_y, float scale) {
    // 使用分段直线绘制
    DrawSegmentedLine(svg_x, svg_y, x, y, start_x, start_y, scale);
}

void DrawBezierCurve(float x0, float y0, float x1, float y1, 
                     float x2, float y2, float x3, float y3,
                     int16_t start_x, int16_t start_y, float scale) {
    
    SetLaser(true);
    
    for (int i = 1; i <= SVG_CURVE_SEGMENTS; i++) {
        float t = (float)i / SVG_CURVE_SEGMENTS;
        float mt = 1.0f - t;
        
        // 贝塞尔曲线公式（在SVG坐标系中计算）
        float svg_x_curve = mt*mt*mt*x0 + 3*mt*mt*t*x1 + 3*mt*t*t*x2 + t*t*t*x3;
        float svg_y_curve = mt*mt*mt*y0 + 3*mt*mt*t*y1 + 3*mt*t*t*y2 + t*t*t*y3;
        
        // 转换为激光坐标并绘制
        float rel_x, rel_y;
        TransformCoordinates(svg_x_curve, svg_y_curve, scale, 0, 0, &rel_x, &rel_y);
        
        // 加上偏移
        int16_t abs_x = start_x + (int16_t)rel_x;
        int16_t abs_y = start_y + (int16_t)rel_y;
        
        DrawLineTo(abs_x, abs_y, SVG_DRAW_SPEED);
    }
    
    // 更新当前位置
    svg_x = x3;
    svg_y = y3;
    float rel_x, rel_y;
    TransformCoordinates(x3, y3, scale, 0, 0, &rel_x, &rel_y);
    current_x = start_x + rel_x;
    current_y = start_y + rel_y;
}

/**
 * @brief 绘制二次贝塞尔曲线（转换为三次）
 */
void DrawQuadBezier(float x0, float y0, float x1, float y1, float x2, float y2,
                    int16_t start_x, int16_t start_y, float scale) {
    // 二次转三次贝塞尔控制点
    float cp1_x = x0 + 2.0f/3.0f * (x1 - x0);
    float cp1_y = y0 + 2.0f/3.0f * (y1 - y0);
    float cp2_x = x2 + 2.0f/3.0f * (x1 - x2);
    float cp2_y = y2 + 2.0f/3.0f * (y1 - y2);
    
    DrawBezierCurve(x0, y0, cp1_x, cp1_y, cp2_x, cp2_y, x2, y2, start_x, start_y, scale);
}

static const char* ParseFloat(const char* str, float* value) {
    char* end;
    str = SkipSpaces(str);
    
    if (*str == '-' || *str == '+' || isdigit(*str) || *str == '.') {
        *value = strtof(str, &end);
        return end;
    }
    
    *value = 0;
    return str;
}

/**
 * @brief 完全重写的SVG路径解析函数
 */
void DrawSVGPath(const char* path_data, int16_t start_x, int16_t start_y, float scale) {
    const char* p = path_data;
    float last_cp_x = 0, last_cp_y = 0;  // 最后的控制点（SVG坐标）
    char current_cmd = 0;
    bool is_relative = false;
    
    // 重置状态
    current_x = current_y = 0;
    svg_x = svg_y = 0;
    path_start_x = path_start_y = 0;
    svg_path_start_x = svg_path_start_y = 0;
    
    DEBUG_PRINT("\n=== Starting SVG path parsing ===\n");
    DEBUG_PRINT("Path: '%s'\n", path_data);
    DEBUG_PRINT("Start position: (%d, %d), Scale: %.2f\n", start_x, start_y, scale);
    
    while (p && *p) {
        p = SkipSpaces(p);
        if (!*p) break;
        
        // 检查是否是新命令
        if (isalpha(*p)) {
            current_cmd = toupper(*p);
            is_relative = islower(*p);
            p++; // 跳过命令字符
            DEBUG_PRINT("\n--- Command: %c (relative: %d) ---\n", current_cmd, is_relative);
        }
        
        // 根据当前命令处理数据
        switch (current_cmd) {
            case 'M': {  // 移动到
                float x, y;
                p = ParseFloat(p, &x);
                p = ParseFloat(p, &y);
                
                if (is_relative) {
                    x += svg_x;
                    y += svg_y;
                }
                
                MoveToPoint(x, y, start_x, start_y, scale);
                
                // 保存路径起始点
                path_start_x = current_x;
                path_start_y = current_y;
                svg_path_start_x = svg_x;
                svg_path_start_y = svg_y;
                
                // M命令后面如果还有数字，自动变成L命令
                current_cmd = 'L';
                break;
            }
            
            case 'L': {  // 画线到
                float x, y;
                p = ParseFloat(p, &x);
                p = ParseFloat(p, &y);
                
                if (is_relative) {
                    x += svg_x;
                    y += svg_y;
                }
                
                DrawToPoint(x, y, start_x, start_y, scale);
                break;
            }
            
            case 'H': {  // 水平线
                float x;
                p = ParseFloat(p, &x);
                
                if (is_relative) {
                    x += svg_x;
                }
                
                DrawToPoint(x, svg_y, start_x, start_y, scale);
                break;
            }
            
            case 'V': {  // 垂直线
                float y;
                p = ParseFloat(p, &y);
                
                if (is_relative) {
                    y += svg_y;
                }
                
                DrawToPoint(svg_x, y, start_x, start_y, scale);
                break;
            }
            
            case 'C': {  // 三次贝塞尔曲线
                float cp1_x, cp1_y, cp2_x, cp2_y, end_x, end_y;
                p = ParseFloat(p, &cp1_x);
                p = ParseFloat(p, &cp1_y);
                p = ParseFloat(p, &cp2_x);
                p = ParseFloat(p, &cp2_y);
                p = ParseFloat(p, &end_x);
                p = ParseFloat(p, &end_y);
                
                if (is_relative) {
                    cp1_x += svg_x; cp1_y += svg_y;
                    cp2_x += svg_x; cp2_y += svg_y;
                    end_x += svg_x; end_y += svg_y;
                }
                
                // 保存最后的控制点（SVG坐标）
                last_cp_x = cp2_x;
                last_cp_y = cp2_y;
                
                // 注意：这里传入的是SVG坐标，DrawBezierCurve内部会处理转换
                DrawBezierCurve(svg_x, svg_y, cp1_x, cp1_y, 
                               cp2_x, cp2_y, end_x, end_y, start_x, start_y, scale);
                break;
            }
            
            case 'S': {  // 平滑三次贝塞尔曲线
                float cp2_x, cp2_y, end_x, end_y;
                p = ParseFloat(p, &cp2_x);
                p = ParseFloat(p, &cp2_y);
                p = ParseFloat(p, &end_x);
                p = ParseFloat(p, &end_y);
                
                if (is_relative) {
                    cp2_x += svg_x; cp2_y += svg_y;
                    end_x += svg_x; end_y += svg_y;
                }
                
                // 计算第一个控制点（反射前一个控制点）
                float cp1_x = 2 * svg_x - last_cp_x;
                float cp1_y = 2 * svg_y - last_cp_y;
                
                // 保存最后的控制点
                last_cp_x = cp2_x;
                last_cp_y = cp2_y;
                
                DrawBezierCurve(svg_x, svg_y, cp1_x, cp1_y, 
                               cp2_x, cp2_y, end_x, end_y, start_x, start_y, scale);
                break;
            }
            
            case 'Q': {  // 二次贝塞尔曲线
                float cp_x, cp_y, end_x, end_y;
                p = ParseFloat(p, &cp_x);
                p = ParseFloat(p, &cp_y);
                p = ParseFloat(p, &end_x);
                p = ParseFloat(p, &end_y);
                
                if (is_relative) {
                    cp_x += svg_x; cp_y += svg_y;
                    end_x += svg_x; end_y += svg_y;
                }
                
                // 保存控制点
                last_cp_x = cp_x;
                last_cp_y = cp_y;
                
                DrawQuadBezier(svg_x, svg_y, cp_x, cp_y, 
                              end_x, end_y, start_x, start_y, scale);
                break;
            }
            
            case 'T': {  // 平滑二次贝塞尔曲线
                float end_x, end_y;
                p = ParseFloat(p, &end_x);
                p = ParseFloat(p, &end_y);
                
                if (is_relative) {
                    end_x += svg_x; end_y += svg_y;
                }
                
                // 计算控制点（反射前一个控制点）
                float cp_x = 2 * svg_x - last_cp_x;
                float cp_y = 2 * svg_y - last_cp_y;
                
                // 保存控制点
                last_cp_x = cp_x;
                last_cp_y = cp_y;
                
                DrawQuadBezier(svg_x, svg_y, cp_x, cp_y, 
                              end_x, end_y, start_x, start_y, scale);
                break;
            }
            
            case 'Z': {  // 闭合路径
                DrawToPoint(svg_path_start_x, svg_path_start_y, start_x, start_y, scale);
                break;
            }
            
            default:
                DEBUG_PRINT("Unsupported command: %c, skipping...\n", current_cmd);
                // 跳过一个数字
                float dummy;
                p = ParseFloat(p, &dummy);
                break;
        }
    }
    
    SetLaser(false);
    DEBUG_PRINT("\n=== SVG path parsing completed ===\n\n");
}

/**
 * @brief 绘制简单SVG路径（仅支持：M/L/H/V/Z命令）
 */
void DrawSVGPathSimple(const char* path_data, int16_t start_x, int16_t start_y, float scale) {
    // 简化版本，只处理基本的移动和直线命令
    DrawSVGPath(path_data, start_x, start_y, scale);
}

/**
 * @brief 绘制多边形
 */
void DrawPolygon(const float* points, int point_count, 
                int16_t start_x, int16_t start_y, float scale, bool closed) {
    if (point_count < 2) return;
    
    // 移动到第一个点
    MoveToPoint(points[0], points[1], start_x, start_y, scale);
    
    // 绘制到其他点
    for (int i = 1; i < point_count; i++) {
        DrawToPoint(points[i*2], points[i*2+1], start_x, start_y, scale);
    }
    
    // 如果需要闭合，绘制回到起始点
    if (closed) {
        DrawToPoint(points[0], points[1], start_x, start_y, scale);
    }
    
    SetLaser(false);
}

/**
 * @brief 绘制简单弧线
 */
void DrawArc(float center_x, float center_y, float radius, 
            float start_angle, float end_angle) {
    // 简单的弧线实现，将弧线分段绘制
    const int segments = 20;
    float angle_step = (end_angle - start_angle) / segments;
    
    for (int i = 0; i <= segments; i++) {
        float angle = start_angle + i * angle_step;
        float x = center_x + radius * cosf(angle * M_PI / 180.0f);
        float y = center_y + radius * sinf(angle * M_PI / 180.0f);
        
        if (i == 0) {
            // 移动到起始点（这里需要传递正确的参数）
            // MoveToPoint(x, y, 0, 0, 1.0f);
        } else {
            // 绘制到下一点（这里需要传递正确的参数）
            // DrawToPoint(x, y, 0, 0, 1.0f);
        }
    }
    
    SetLaser(false);
}