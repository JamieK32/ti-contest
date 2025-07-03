#include "ui.h"


/* =============================================================================
 * 动感开机动画 - 酷炫但不跳动
 * ============================================================================= */
void show_oled_opening_animation(void) {
    float angle = 0.0f;
    uint32_t startTime = UI_GET_TICK();
    do {
        u8g2_ClearBuffer(&u8g2);
        uint32_t elapsedTime = UI_GET_TICK() - startTime;
        
        // 1. --- 主标题打字机效果 + 光标闪烁 ---
        u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr); // 常见字体，如果没有可改为 u8g2_font_10x20_tr
        const char* text = "Simple Ui";
        int textWidth = u8g2_GetStrWidth(&u8g2, text);
        int textX = 64 - textWidth / 2;
        int textY = 25;
        
        // 打字机效果：逐字符显示
        int chars_typed = (elapsedTime / 120); // 每120ms一个字符
        if (chars_typed > 9) chars_typed = 9;
        
        // 显示已打出的字符
        for (int i = 0, x = textX; i < chars_typed && text[i] != '\0'; i++) {
            char c[2] = {text[i], '\0'};
            int charWidth = u8g2_GetStrWidth(&u8g2, c);
            u8g2_DrawStr(&u8g2, x, textY, c);
            x += charWidth;
        }
        
        // 光标闪烁效果
        if (chars_typed < 9 && (elapsedTime % 400) < 200) {
            int cursor_x = textX;
            for (int i = 0; i < chars_typed && text[i] != '\0'; i++) {
                char c[2] = {text[i], '\0'};
                cursor_x += u8g2_GetStrWidth(&u8g2, c);
            }
            u8g2_DrawVLine(&u8g2, cursor_x, textY - 12, 15);
        }
        
        // 2. --- 扫描线效果 ---
        if (elapsedTime > 800) {
            int scan_y = ((elapsedTime - 800) / 20) % 64; // 扫描线从上到下
            // 主扫描线
            u8g2_DrawHLine(&u8g2, 0, scan_y, 128);
            // 拖尾效果
            if (scan_y > 1) u8g2_DrawHLine(&u8g2, 0, scan_y - 1, 128);
            if (scan_y > 2 && (elapsedTime % 4) < 2) u8g2_DrawHLine(&u8g2, 0, scan_y - 2, 128);
        }
        
        // 3. --- 副标题矩阵雨效果 ---
        if (elapsedTime > 1000) {
            u8g2_SetFont(&u8g2, u8g2_font_6x10_tr); // 常见字体，如果没有可改为 u8g2_font_5x7_tr
            const char* subtext = "W e l c o m e";
            int subWidth = u8g2_GetStrWidth(&u8g2, subtext);
            int subX = 64 - subWidth / 2;
            int subY = 45;
            
            // 字符从上方"掉落"到位置
            for (int i = 0; subtext[i] != '\0'; i++) {
                if (subtext[i] == ' ') continue;
                
                int drop_start = 1000 + i * 80; // 每个字符延迟80ms开始掉落
                if (elapsedTime > drop_start) {
                    int fall_time = elapsedTime - drop_start;
                    int char_y = -10 + (fall_time / 15); // 掉落速度
                    if (char_y > subY) char_y = subY; // 到达目标位置
                    
                    char c[2] = {subtext[i], '\0'};
                    int char_x = subX + i * u8g2_GetStrWidth(&u8g2, "W");
                    u8g2_DrawStr(&u8g2, char_x, char_y, c);
                    
                    // 掉落轨迹
                    if (char_y < subY) {
                        for (int trail = 1; trail <= 3; trail++) {
                            int trail_y = char_y - trail * 4;
                            if (trail_y > 0 && (elapsedTime % (trail * 2)) < trail) {
                                u8g2_DrawPixel(&u8g2, char_x + 2, trail_y);
                            }
                        }
                    }
                }
            }
        }
        
        // 4. --- 动态边框脉冲 ---
        float pulse = sinf(angle * 2.0f) * 0.5f + 0.5f; // 0-1脉冲
        int border_thickness = (int)(pulse * 3) + 1;
        
        for (int i = 0; i < border_thickness; i++) {
            // 只画四个角的边框
            int corner_len = 15 + (int)(pulse * 10);
            
            // 左上
            u8g2_DrawHLine(&u8g2, i, i, corner_len);
            u8g2_DrawVLine(&u8g2, i, i, corner_len);
            
            // 右上  
            u8g2_DrawHLine(&u8g2, 127 - corner_len, i, corner_len);
            u8g2_DrawVLine(&u8g2, 127 - i, i, corner_len);
            
            // 左下
            u8g2_DrawHLine(&u8g2, i, 63 - i, corner_len);
            u8g2_DrawVLine(&u8g2, i, 63 - corner_len, corner_len);
            
            // 右下
            u8g2_DrawHLine(&u8g2, 127 - corner_len, 63 - i, corner_len);
            u8g2_DrawVLine(&u8g2, 127 - i, 63 - corner_len, corner_len);
        }
        
        // 5. --- 粒子特效 ---
        for (int i = 0; i < 15; i++) {
            int particle_x = (int)(64 + 40 * cosf(angle + i * 0.4f));
            int particle_y = (int)(32 + 20 * sinf(angle * 1.5f + i * 0.3f));
            
            if (particle_x >= 0 && particle_x < 128 && particle_y >= 0 && particle_y < 64) {
                // 根据时间和位置决定粒子大小
                int size = ((elapsedTime + i * 100) % 1000) < 500 ? 1 : 0;
                if (size > 0) {
                    u8g2_DrawPixel(&u8g2, particle_x, particle_y);
                    // 有些粒子更大
                    if (i % 3 == 0) {
                        u8g2_DrawPixel(&u8g2, particle_x + 1, particle_y);
                        u8g2_DrawPixel(&u8g2, particle_x, particle_y + 1);
                    }
                }
            }
        }
        
        // 6. --- 进度条能量波 ---
        if (elapsedTime > 500) {
            int progress = ((elapsedTime - 500) * 100) / 1500;
            if (progress > 100) progress = 100;
            
            // 外框
            u8g2_DrawFrame(&u8g2, 10, 55, 108, 6);
            
            // 能量条主体
            int fill_width = (progress * 106) / 100;
            if (fill_width > 0) {
                u8g2_DrawBox(&u8g2, 11, 56, fill_width, 4);
            }
            
            // 能量波效果
            for (int x = 0; x < fill_width; x += 4) {
                int wave_height = (int)(2 * sinf(angle * 3.0f + x * 0.2f));
                if ((x + (int)angle) % 8 < 4) {
                    u8g2_DrawPixel(&u8g2, 11 + x, 56 + 2 + wave_height);
                }
            }
            
            // 前端闪电效果
            if (progress < 100 && fill_width > 5) {
                for (int i = 0; i < 3; i++) {
                    if ((elapsedTime % 200) < 100) {
                        u8g2_DrawPixel(&u8g2, 11 + fill_width + i, 56 + i % 4);
                    }
                }
            }
        }
        
        u8g2_SendBuffer(&u8g2);
        angle += 0.25f;
				
        uint32_t frameTime = UI_GET_TICK();
        while((UI_GET_TICK() - frameTime) < 40) {
            // 等待到下一帧时间
        }
        
    } while((UI_GET_TICK() - startTime) < 2500);
    
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}
