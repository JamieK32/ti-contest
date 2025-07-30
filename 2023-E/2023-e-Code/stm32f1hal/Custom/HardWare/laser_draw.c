#include "laser_draw.h"
#include "stdlib.h"

// 全局变量
static LaserDraw_t laser_system = {0};

// π值定义
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 在初始化时或需要重置0点时调用
void ResetMotorZeroPosition(void)
{
    // 重置X轴电机当前位置为0
    Emm_V5_Reset_CurPos_To_Zero(MOTOR_X_ADDR);
    HAL_Delay(10);
    
    // 重置Y轴电机当前位置为0  
    Emm_V5_Reset_CurPos_To_Zero(MOTOR_Y_ADDR);
    HAL_Delay(10);
}


/**
 * @brief 初始化激光绘图系统
 */
void LaserDraw_Init(void)
{
    // 初始化GPIO - 激光控制引脚
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = LASER_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LASER_GPIO_PORT, &GPIO_InitStruct);
    
    // 初始化系统状态
    laser_system.state = DRAW_IDLE;
    laser_system.step = 0;
    laser_system.x_ready = false;
    laser_system.y_ready = false;
    
    // 关闭激光
    Laser_Off();
    
    // 初始化电机接收
    InitEmmReceive();

    // 使能电机
    Motors_Enable();
    
    HAL_Delay(100); // 等待电机稳定
}

/**
 * @brief 坐标比例校正函数
 */
void ScaleCoordinates(int16_t* x, int16_t* y)
{
    if(x) *x = (int16_t)(*x * AXIS_SCALE_X);
    if(y) *y = (int16_t)(*y * AXIS_SCALE_Y);
}

/**
 * @brief 激光开启
 */
void Laser_On(void)
{
    HAL_GPIO_WritePin(LASER_GPIO_PORT, LASER_GPIO_PIN, GPIO_PIN_SET);
    laser_system.params.laser_on = true;
}

/**
 * @brief 激光关闭
 */
void Laser_Off(void)
{
    HAL_GPIO_WritePin(LASER_GPIO_PORT, LASER_GPIO_PIN, GPIO_PIN_RESET);
    laser_system.params.laser_on = false;
}

/**
 * @brief 使能电机
 */
void Motors_Enable(void)
{
    Emm_V5_En_Control(MOTOR_X_ADDR, true, false);
    HAL_Delay(10);
    Emm_V5_En_Control(MOTOR_Y_ADDR, true, false);
    HAL_Delay(10);
}

/**
 * @brief 禁用电机
 */
void Motors_Disable(void)
{
    Emm_V5_En_Control(MOTOR_X_ADDR, false, false);
    HAL_Delay(10);
    Emm_V5_En_Control(MOTOR_Y_ADDR, false, false);
}

/**
 * @brief 停止电机
 */
void Motors_Stop(void)
{
    Emm_V5_Stop_Now(MOTOR_X_ADDR, false);
    HAL_Delay(5);
    Emm_V5_Stop_Now(MOTOR_Y_ADDR, false);
}


/**
 * @brief 等待电机完成运动
 */
void WaitForMotorsComplete(bool wait_x, bool wait_y, uint32_t timeout_ms)
{
    uint32_t timeout = HAL_GetTick() + timeout_ms;
    while(HAL_GetTick() < timeout)
    {
        if(wait_x) {
            Emm_V5_Read_Sys_Params(MOTOR_X_ADDR, S_FLAG);
            ProcessEmmReceive();
        }
        if(wait_y) {
            Emm_V5_Read_Sys_Params(MOTOR_Y_ADDR, S_FLAG);
            ProcessEmmReceive();
        }
        
        bool x_done = !wait_x || IsMotorInPosition(MOTOR_X_ADDR);
        bool y_done = !wait_y || IsMotorInPosition(MOTOR_Y_ADDR);
        
        if(x_done && y_done) break;
    }
}

void Move(int16_t x, int16_t y, uint16_t speed) {
  ScaleCoordinates(&x, &y);
    
    uint32_t x_distance = (x >= 0) ? (uint32_t)x : (uint32_t)(-x);
    uint32_t y_distance = (y >= 0) ? (uint32_t)y : (uint32_t)(-y);
    
    if(x_distance == 0 && y_distance == 0) return;
    
    // 步骤1：发送X轴命令，启用同步标志
    if(x != 0) {
        uint8_t x_dir = (x >= 0) ? 1 : 0;
        Emm_V5_Pos_Control(MOTOR_X_ADDR, x_dir, speed, ACCELERATION, 
                          x_distance, true, true);  // snF=true 启用同步
        //                                   ↑     ↑
        //                              相对模式  同步标志
    }
    
    // 步骤2：发送Y轴命令，启用同步标志  
    if(y != 0) {
        uint8_t y_dir = (y >= 0) ? 0 : 1;
        Emm_V5_Pos_Control(MOTOR_Y_ADDR, y_dir, speed, ACCELERATION, 
                          y_distance, true, true);  // snF=true 启用同步
    }
    
    // 步骤3：触发同步运动 - 所有轴同时开始！
    Emm_V5_Synchronous_motion(0);  // 0为广播地址，触发所有轴同步
    
    // 步骤4：等待完成
    WaitForMotorsComplete(x != 0, y != 0, 10000);
}


void MoveTo(int16_t x, int16_t y, uint16_t speed)
{
    // 关闭激光
    Laser_Off();
		Move(x, y, speed);
}

/**
 * @brief 修正后的DrawLineTo函数
 */
void DrawLineTo(int16_t x, int16_t y, uint16_t speed)
{
		Laser_On();
		Move(x, y, speed);
}


/**
 * @brief 绘制正方形 - 真正同步版本
 */
void DrawSquare(int16_t center_x, int16_t center_y, int16_t size)
{
    int16_t half_size = size / 2;
    
    // 计算四个顶点的绝对坐标
    int16_t x1 = center_x - half_size, y1 = center_y - half_size;
    int16_t x2 = center_x + half_size, y2 = center_y - half_size;
    int16_t x3 = center_x + half_size, y3 = center_y + half_size;
    int16_t x4 = center_x - half_size, y4 = center_y + half_size;
    
    // 使用绝对坐标绘制，每个点都精确定位
    MoveTo(x1, y1, MOVE_SPEED);
    DrawLineTo(x2, y2, DRAW_SPEED);  // 底边
    DrawLineTo(x3, y3, DRAW_SPEED);  // 右边
    DrawLineTo(x4, y4, DRAW_SPEED);  // 顶边
    DrawLineTo(x1, y1, DRAW_SPEED);  // 左边，回到起点
    
    Laser_Off();
}

/**
 * @brief 绘制矩形 - 绝对坐标精确版本
 */
void DrawRectangle(int16_t center_x, int16_t center_y, int16_t width, int16_t height)
{
    int16_t half_width = width / 2;
    int16_t half_height = height / 2;
    
    // 计算四个顶点的绝对坐标
    int16_t x1 = center_x - half_width;  // 左下角
    int16_t y1 = center_y - half_height;
    int16_t x2 = center_x + half_width;  // 右下角
    int16_t y2 = center_y - half_height;
    int16_t x3 = center_x + half_width;  // 右上角
    int16_t y3 = center_y + half_height;
    int16_t x4 = center_x - half_width;  // 左上角
    int16_t y4 = center_y + half_height;
    
    // 移动到起始点 (左下角)
    MoveTo(x1, y1, MOVE_SPEED);
    
    // 使用绝对坐标画矩形的四条边，确保每个顶点精确
    DrawLineTo(x2, y2, DRAW_SPEED);    // 底边：左下 -> 右下
    DrawLineTo(x3, y3, DRAW_SPEED);    // 右边：右下 -> 右上
    DrawLineTo(x4, y4, DRAW_SPEED);    // 顶边：右上 -> 左上
    DrawLineTo(x1, y1, DRAW_SPEED);    // 左边：左上 -> 左下（回到起点）

    Laser_Off();
}

/**
 * @brief 用参数方程绘制平滑圆形 - 插值版本
 */
void DrawCircle(int16_t center_x, int16_t center_y, int16_t radius)
{
 // 根据半径确定点的密度
    int total_points = radius * 6;  // 每个单位半径6个点
    if(total_points < 50) total_points = 50;    // 最少50个点
    if(total_points > 500) total_points = 200;  // 最多500个点，避免太慢
    
    float angle_step = 2.0f * M_PI / total_points;
    
    // 逐点绘制圆形
    for(int i = 1; i < total_points; i++)
    {
        float angle = i * angle_step;
        
        // 计算点的坐标
        int16_t x = center_x + (int16_t)(radius * cosf(angle) + 0.5f);
        int16_t y = center_y + (int16_t)(radius * sinf(angle) + 0.5f);
        
        // 移动到该点位置并打点
        MoveTo(x, y, MOVE_SPEED);
        
        // 在该点打点（开启激光一小段时间）
				DrawPixelDot(x, y, 1);
    }
}


/**
 * @brief 绘制高精度三角形 - 绝对坐标修正版
 */
void DrawTriangle(int16_t center_x, int16_t center_y, int16_t size)
{
    int16_t half_size = size / 2;
    int16_t height = (int16_t)(size * 0.866f);
    
    // 三个顶点的绝对坐标
    int16_t x1 = center_x, y1 = center_y + height / 2;
    int16_t x2 = center_x - half_size, y2 = center_y - height / 2;
    int16_t x3 = center_x + half_size, y3 = center_y - height / 2;

		MoveTo(x3, y3, DRAW_SPEED);
    DrawLineTo(x3, y3, DRAW_SPEED);
    DrawLineTo(x1, y1, DRAW_SPEED);
    DrawLineTo(x2, y2, DRAW_SPEED);
		DrawLineTo(x3, y3, DRAW_SPEED);
    Laser_Off();
}
/**
 * @brief 绘制直线 - 真正同步版本
 */
void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    // 移动到起点
    MoveTo(x1, y1, MOVE_SPEED);
    
    // 画线到终点
    DrawLineTo(x2, y2, DRAW_SPEED);
    
    Laser_Off();
}

/**
 * @brief 绘制十字 - 真正同步版本
 */
void DrawCross(int16_t center_x, int16_t center_y, int16_t size)
{
    int16_t half_size = size / 2;
    
    // 画垂直线
    MoveTo(center_x, center_y - half_size, MOVE_SPEED);
    DrawLineTo(center_x, center_y + half_size, DRAW_SPEED);
    
    // 画水平线
    MoveTo(center_x - half_size, center_y, MOVE_SPEED);
    DrawLineTo(center_x + half_size, center_y, DRAW_SPEED);
    
    Laser_Off();
}

/**
 * @brief 回零操作
 */
void GoHome(void)
{
    MoveTo(0, 0, MOVE_SPEED);
}

/**
 * @brief 获取当前位置
 */
void GetCurrentPosition(int32_t* x, int32_t* y)
{
    if(x) *x = GetMotorPosition(MOTOR_X_ADDR);
    if(y) *y = GetMotorPosition(MOTOR_Y_ADDR);
}

/**
 * @brief 状态机处理 (在定时器中调用)
 */
void LaserDraw_Process(void)
{
    // 处理串口接收数据
    ProcessEmmReceive();
    
    // 更新电机状态
    laser_system.x_ready = IsMotorInPosition(MOTOR_X_ADDR);
    laser_system.y_ready = IsMotorInPosition(MOTOR_Y_ADDR);
    
    // 状态机处理
    switch(laser_system.state)
    {
        case DRAW_IDLE:
            // 空闲状态，等待命令
            break;
            
        case DRAW_MOVING:
            // 检查移动是否完成
            if(laser_system.x_ready && laser_system.y_ready)
            {
                laser_system.state = DRAW_COMPLETE;
            }
            break;
            
        case DRAW_DRAWING:
            // 检查绘图是否完成
            if(laser_system.x_ready && laser_system.y_ready)
            {
                laser_system.state = DRAW_COMPLETE;
                Laser_Off(); // 绘图完成后关闭激光
            }
            break;
            
        case DRAW_COMPLETE:
            // 绘制完成，可以接受新命令
            laser_system.state = DRAW_IDLE;
            break;
            
        case DRAW_ERROR:
            // 错误状态处理
            Laser_Off();
            Motors_Stop();
            laser_system.state = DRAW_IDLE;
            break;
    }
}

/**
 * @brief 获取当前状态
 */
DrawState_t GetDrawState(void)
{
    return laser_system.state;
}

/**
 * @brief 检查是否绘制完成
 */
bool IsDrawComplete(void)
{
    return (laser_system.state == DRAW_IDLE || laser_system.state == DRAW_COMPLETE);
}

/**
 * @brief 绘制单个像素点 - 简单快速
 */
void DrawPixelDot(int16_t x, int16_t y, int16_t size)
{
    // 方法1：画一个小点
    Laser_On();
    HAL_Delay(3);  // 短暂点亮
    Laser_Off();
}

/**
 * @brief 绘制点阵位图 - 带行列间距
 */
void DrawBitmap(const unsigned char* bitmap_data, int width, int height, 
                int16_t start_x, int16_t start_y, int16_t pixel_size, 
                float row_spacing, float col_spacing)
{
    int bytes_per_row = (width + 7) / 8;  // 每行字节数
    
    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            // 计算当前像素在数据中的位置
            int byte_index = row * bytes_per_row + (col / 8);
            int bit_index = 7 - (col % 8);  // 从高位到低位
            
            // 只处理为1的像素
            if(bitmap_data[byte_index] & (1 << bit_index))
            {
                // 计算像素的实际坐标（增加行列间距）
                int16_t pixel_x = start_x + col * (int16_t)(pixel_size * col_spacing);
                int16_t pixel_y = start_y + (height - 1 - row) * (int16_t)(pixel_size * row_spacing);
                
                // 移动到该像素位置（关闭激光）
                MoveTo(pixel_x, pixel_y, MOVE_SPEED);
                
                // 绘制这个像素（开启激光）
                DrawPixelDot(pixel_x, pixel_y, pixel_size);
            }
            // 0的像素直接跳过，不做任何操作
        }
    }
}

/**
 * @brief 简单的多汉字绘制函数 - 添加字间距参数
 */
void DrawMultipleChars(const unsigned char* chars[], int char_count, 
                      int16_t start_x, int16_t start_y, int16_t pixel_size)
{
    int16_t current_x = start_x;
    
    // 在这里调整字间距 - 简单修改这一行即可
    int16_t char_spacing = pixel_size * 30;  // 字间距 = 像素大小 × 倍数
    
    // 计算单个汉字的实际宽度
    int16_t char_width = 16 * pixel_size * 3; // 16像素 × 像素大小 × 列间距倍数
    
    for(int i = 0; i < char_count; i++)
    {
        DrawBitmap(chars[i], 16, 16, current_x, start_y, pixel_size, 3, 3);
        
        // 除了最后一个字，都要移动到下一个字的位置
        if(i < char_count - 1) {
            current_x += char_width + char_spacing; // 字宽 + 字间距
        }
    }
}

/**
 * @brief 绘制您提供的汉字位图
 */
void DrawChineseBitmap(int16_t start_x, int16_t start_y, int16_t pixel_size)
{
    static const unsigned char char1[] = 
    {
0x08,0x40,0x08,0x80,0x0B,0xFC,0x12,0x94,0x12,0x64,0x32,0x94,0x33,0xFC,0x50,0x90,
0x91,0x08,0x12,0x84,0x14,0xFA,0x11,0x08,0x12,0x90,0x10,0x60,0x11,0x98,0x16,0x06,/*"傻",0*/



    };
    
    static const unsigned char char2[] = 
    {    
0x00,0x00,0x27,0xFC,0x10,0x00,0x13,0xF8,0x02,0x08,0x03,0xF8,0xF0,0x00,0x17,0xFC,
0x14,0x44,0x17,0xFC,0x14,0x44,0x17,0xFC,0x14,0x04,0x28,0x00,0x47,0xFE,0x00,0x00,/*"逼",1*/

    };
    
    static const unsigned char char3[] = 
    {
0x01,0x00,0x11,0x00,0x11,0x00,0x1F,0xF8,0x21,0x00,0x41,0x00,0x01,0x00,0xFF,0xFE,
0x03,0x80,0x05,0x40,0x09,0x20,0x11,0x10,0x21,0x08,0xC1,0x06,0x01,0x00,0x01,0x00,/*"朱",2*/

    };

		    static const unsigned char char4[] = 
    {
0x00,0x00,0x1F,0xF0,0x10,0x10,0x10,0x10,0x1F,0xF0,0x10,0x10,0x10,0x10,0x1F,0xF0,
0x01,0x00,0x11,0x08,0x11,0x10,0x22,0x80,0x04,0x40,0x08,0x20,0x30,0x18,0xC0,0x06,/*"炅",3*/

    };
		
		   static const unsigned char char5[] = 
    {
0x00,0x28,0x40,0x24,0x20,0x20,0x27,0xFE,0x04,0x20,0x04,0x20,0xE4,0x24,0x27,0xA4,
0x24,0xA4,0x24,0xA8,0x24,0xA8,0x2C,0x90,0x36,0x92,0x29,0x2A,0x08,0x46,0x10,0x82,/*"诚",4*/
    };
    const unsigned char* my_chars[] = {char1, char2, char3, char4, char5};
    DrawMultipleChars(my_chars, 5, 0, 0, 2);
}

/**
 * @brief 绘制正弦波
 */
void DrawSineWave(int16_t start_x, int16_t start_y, int16_t length, 
                  int16_t amplitude, float frequency, float phase, int points)
{
    // 计算第一个点
    float x_step = (float)length / (points - 1);
    float first_x = start_x;
    float first_y = start_y + amplitude * sinf(phase);
   
    // 移动到起始点
    MoveTo(first_x, first_y, MOVE_SPEED);

    // 逐点绘制正弦波
    for(int i = 1; i < points; i++)
    {
        // 计算当前点的坐标
        float x = start_x + i * x_step;
        float angle = 2.0f * M_PI * frequency * i / (points - 1) + phase;
        float y = start_y + amplitude * sinf(angle);
        
        // 画线到当前点
        DrawLineTo((int16_t)x, (int16_t)y, DRAW_SPEED);
    }
    
    Laser_Off();
}


/**
 * @brief 绘制三角波 - 打点形式（更精确版本）
 */
void DrawTriangleWave(int16_t start_x, int16_t start_y, int16_t length, 
                     int16_t amplitude, float frequency, int points)
{
    float x_step = (float)length / points;
    float cycle_width = (float)length / frequency;  // 每个周期的宽度
    
    for(int i = 1; i < points; i++)
    {
        // X坐标
        int16_t x = start_x + (int16_t)(i * x_step);
        
        // 计算在当前周期中的位置 (0到1)
        float pos_in_cycle = fmodf((float)i * x_step / cycle_width, 1.0f);
        
        // 计算三角波的Y值
        int16_t y;
        if(pos_in_cycle < 0.5f) {
            // 上升：从 -amplitude 到 +amplitude
            y = start_y - amplitude + (int16_t)(4.0f * amplitude * pos_in_cycle);
        } else {
            // 下降：从 +amplitude 到 -amplitude  
            y = start_y + amplitude - (int16_t)(4.0f * amplitude * (pos_in_cycle - 0.5f));
        }
        
        // 打点
        MoveTo(x, y, MOVE_SPEED);
        DrawPixelDot(x, y, 2);
    }
}

/**
 * @brief 绘制方波 - 真正同步版本
 */
void DrawSquareWave(int16_t start_x, int16_t start_y, int16_t length, 
                         int16_t amplitude, float frequency, float phase)
{
    // 计算每个周期的宽度
    float cycle_width = (float)length / frequency;
    float half_cycle = cycle_width / 2.0f;
    
    // 计算相位偏移的像素数
    float phase_offset = phase * cycle_width;
    
    // 移动到起始点
    float start_phase = phase - floorf(phase);
    int16_t start_level = (start_phase < 0.5f) ? amplitude : -amplitude;
    MoveTo(start_x, start_y + start_level, MOVE_SPEED);
    
    // 绘制每个完整周期
    for(int cycle = 0; cycle < (int)frequency; cycle++)
    {
        float cycle_start_x = start_x + cycle * cycle_width - phase_offset;
        
        // 确保在绘制范围内
        if(cycle_start_x + cycle_width < start_x) continue;
        if(cycle_start_x > start_x + length) break;
        
        // 高电平段
        float high_start = fmaxf(cycle_start_x, (float)start_x);
        float high_end = fminf(cycle_start_x + half_cycle, (float)(start_x + length));
        if(high_start < high_end)
        {
            DrawLineTo((int16_t)high_start, start_y + amplitude, DRAW_SPEED);
            DrawLineTo((int16_t)high_end, start_y + amplitude, DRAW_SPEED);
        }
        
        // 跳跃到低电平
        if(high_end < start_x + length)
        {
            DrawLineTo((int16_t)high_end, start_y - amplitude, DRAW_SPEED);
        }
        
        // 低电平段
        float low_start = high_end;
        float low_end = fminf(cycle_start_x + cycle_width, (float)(start_x + length));
        if(low_start < low_end)
        {
            DrawLineTo((int16_t)low_end, start_y - amplitude, DRAW_SPEED);
        }
        
        // 跳跃到高电平（为下一个周期准备）
        if(low_end < start_x + length && cycle < frequency - 1)
        {
            DrawLineTo((int16_t)low_end, start_y + amplitude, DRAW_SPEED);
        }
    }
    
    Laser_Off();
}

/**
 * @brief 绘制128x128的图片 - 优化版本
 * @param image_data 图像数据，每个字节包含8个像素（1位/像素）
 * @param start_x 起始X坐标
 * @param start_y 起始Y坐标
 * @param pixel_size 每个像素的物理尺寸
 * @param pixel_spacing 像素间距倍数（1.0表示紧密排列）
 * @param skip_pixels 每隔多少个像素绘制一次（用于加速，1表示绘制所有像素）
 */
void DrawImage128x128(const unsigned char* image_data, int16_t start_x, int16_t start_y, 
                      int16_t pixel_size, float pixel_spacing, uint8_t skip_pixels)
{
    const int IMAGE_WIDTH = 128;
    const int IMAGE_HEIGHT = 128;
    const int BYTES_PER_ROW = IMAGE_WIDTH / 8;  // 128/8 = 16字节每行
    
    // 确保skip_pixels至少为1
    if(skip_pixels < 1) skip_pixels = 1;
    
    // 计算实际的像素间距
    int16_t actual_spacing = (int16_t)(pixel_size * pixel_spacing);
    
    // 用于优化的变量
    int16_t last_x = -1000;  // 记录上一个绘制点的位置
    int16_t last_y = -1000;
    uint32_t pixel_count = 0;  // 统计绘制的像素数
    
    // 逐行扫描
    for(int row = 0; row < IMAGE_HEIGHT; row += skip_pixels)
    {
        // 优化：跳行扫描以加快速度
        bool row_has_pixels = false;
        
        // 先检查这一行是否有需要绘制的像素
        for(int byte_idx = 0; byte_idx < BYTES_PER_ROW; byte_idx++)
        {
            if(image_data[row * BYTES_PER_ROW + byte_idx] != 0)
            {
                row_has_pixels = true;
                break;
            }
        }
        
        // 如果整行都是空的，跳过
        if(!row_has_pixels) continue;
        
        // 处理这一行的像素
        for(int col = 0; col < IMAGE_WIDTH; col += skip_pixels)
        {
            // 计算当前像素在数据中的位置
            int byte_index = row * BYTES_PER_ROW + (col / 8);
            int bit_index = 7 - (col % 8);
            
            // 检查像素是否需要绘制
            if(image_data[byte_index] & (1 << bit_index))
            {
                // 计算像素的实际坐标
                int16_t pixel_x = start_x + col * actual_spacing;
                int16_t pixel_y = start_y + (IMAGE_HEIGHT - 1 - row) * actual_spacing;
                
                // 优化移动：如果新位置离上一个位置很近，可以考虑直接画线
                int16_t dx = abs(pixel_x - last_x);
                int16_t dy = abs(pixel_y - last_y);
                
                // 如果距离很近（相邻像素），可以考虑连续绘制
                if(dx <= actual_spacing && dy <= actual_spacing && pixel_count > 0)
                {
                    // 直接移动并打点，不关闭激光
                    Move(pixel_x - last_x, pixel_y - last_y, DRAW_SPEED);
                    DrawPixelDot(pixel_x, pixel_y, pixel_size);
                }
                else
                {
                    // 距离较远，正常移动
                    MoveTo(pixel_x, pixel_y, MOVE_SPEED);
                    DrawPixelDot(pixel_x, pixel_y, pixel_size);
                }
                
                last_x = pixel_x;
                last_y = pixel_y;
                pixel_count++;
                
                // 每绘制一定数量的像素后，短暂延时，避免系统过载
                if(pixel_count % 100 == 0)
                {
                    HAL_Delay(1);
                }
            }
        }
    }
    
    // 确保激光关闭
    Laser_Off();
}