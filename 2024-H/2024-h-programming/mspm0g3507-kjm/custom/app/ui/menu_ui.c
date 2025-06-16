#include "menu_ui.h"
#include "menu_logic.h"
#include "oled_driver.h"

void show_oled_opening_animation(void)
{
    float angle = 0.0f;
    uint32_t startTime = xTaskGetTickCount();
    do {
        u8g2_ClearBuffer(&u8g2);

        // 计算已过ms
        uint32_t elapsedTime = (xTaskGetTickCount() - startTime) * portTICK_PERIOD_MS;

        // 1. --- MSPM0 波浪主标题 + “高亮字母动画” ---
        u8g2_SetFont(&u8g2, u8g2_font_inb16_mr);
        const char* text = "MSPM0";
        int textWidth = u8g2_GetStrWidth(&u8g2, text);

        // 每一帧随机一个字符用白底黑字高亮
        int highlight_i = ((int)(angle * 1.5f)) % 5;
        for (int i = 0, x = 64 - textWidth/2; text[i] != '\0'; i++)
        {
            char c[2] = {text[i], '\0'};
            int charWidth = u8g2_GetStrWidth(&u8g2, c);
            int y = 30 + 8 * sinf(angle + i * 0.7f);
            if(i == highlight_i) {
                // 椭圆高亮底框
                u8g2_SetDrawColor(&u8g2, 1);
                u8g2_DrawBox(&u8g2, x-2, y-15, charWidth+4, 19);
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawStr(&u8g2, x, y, c);
                u8g2_SetDrawColor(&u8g2, 1);
            } else {
                u8g2_DrawStr(&u8g2, x, y, c);
            }
            x += charWidth;
        }

        // 2. --- 副标题 WELCOME，淡入 ---
        uint32_t fadein = elapsedTime < 1500 ? elapsedTime : 1500;
        float alpha = (float)fadein / 1500.f;
        u8g2_SetFont(&u8g2, u8g2_font_profont12_tr);
        const char* subtext = "Welcome";
        int subw = u8g2_GetStrWidth(&u8g2, subtext);
        // 点阵淡入，随机点分布
        for(int i=0,x=64-subw/2;subtext[i]!='\0';i++) {
            char c[2]={subtext[i],'\0'};
            int cw = u8g2_GetStrWidth(&u8g2,c);
            int y = 50 + 2*sinf(angle + i*0.5f);  //副标题也微微波动
            // 字符显示点阵按alpha填补，制造淡入
            if((i*13+y+(int)angle)%10 < (int)(10*alpha)) // 随机采样点数呈现淡入
                u8g2_DrawStr(&u8g2,x,y,c);
            x+=cw;
        }

        // 3. --- 顶部&底部波浪，交错+加星点 ---
        for (int x = 0; x < 128; x++)
        {
            int y1 = 60 + 3 * sinf(angle + x * 0.09f);
            u8g2_DrawPixel(&u8g2, x, y1);

            // 底部点星星
            if (((x*15 + (int)angle*8) % 113) == 0 && (elapsedTime%600)<400)
                u8g2_DrawPixel(&u8g2, x, y1-2);

            int y2 = 7 + 3 * sinf(angle + x * 0.1f + 3.14159f);
            u8g2_DrawPixel(&u8g2, x, y2);

            // 顶部点星星
            if (((x*9+(int)angle*11)%127)==0 && (elapsedTime%700)<300)
                u8g2_DrawPixel(&u8g2, x, y2+2);
        }

        // 4. --- 进度条波浪填充 ---
        int bar_len = (elapsedTime * 118) / 3000;
        if (bar_len > 118) bar_len = 118;
        u8g2_DrawFrame(&u8g2, 5, 62, 118, 2);
        // 用小波浪做进度条填充
        for(int i=0;i<bar_len;i++) {
            int y = 62 + (int)(sinf(angle+i*0.25f)*1.1f);
            u8g2_DrawPixel(&u8g2, 5+i, y);
            u8g2_DrawPixel(&u8g2, 5+i, 63); //下沿也补齐
        }
        u8g2_SendBuffer(&u8g2);
        angle += 0.2f;
        vTaskDelay(pdMS_TO_TICKS(50)); // 20FPS
    } while((xTaskGetTickCount() - startTime) * portTICK_PERIOD_MS < 2000);
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}


/**
 * @brief 在 OLED 上居中显示一串文本，并可选地绘制边框
 * @param text 要显示的文本
 * @param draw_border 是否绘制边框，1 为绘制，0 为不绘制
 */
void draw_centered_text(const char* text, uint8_t draw_border) {
    // 计算文字居中的位置
    uint8_t text_width = u8g2_GetStrWidth(&u8g2, text);
    uint8_t x = (u8g2_GetDisplayWidth(&u8g2) - text_width) / 2;
    uint8_t y = u8g2_GetDisplayHeight(&u8g2) / 2;

    // 绘制文本
    u8g2_DrawStr(&u8g2, x, y, text);

    // 如果需要，绘制边框
    if (draw_border) {
        u8g2_DrawFrame(&u8g2, x - 10, y - 12, text_width + 20, 20);
    }
}


void draw_variables_menu(menu_variables_t *menu_variables) {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);

    // 绘制标题
    const char* title = "Variables";
    uint8_t title_width = u8g2_GetStrWidth(&u8g2, title);
    uint8_t title_x = (u8g2_GetDisplayWidth(&u8g2) - title_width) / 2;
    u8g2_DrawStr(&u8g2, title_x, 8, title);

    // 绘制变量列表
    for (uint8_t i = 0; i < MAX_INDEX_COUNT; i++) {
        if (menu_variables[i].name != NULL) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%s: %.2f", menu_variables[i].name, *menu_variables[i].val_ptr);
            u8g2_DrawStr(&u8g2, 10, 20 + i * MENU_LINE_HEIGHT, buffer);
        }
    }

    u8g2_SendBuffer(&u8g2);
}

void draw_menu(MenuNode *current_menu) {	
    u8g2_ClearBuffer(&u8g2);
    // 设置字体为英文小字体
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);

    // 检查当前界面类型
		
		if (current_menu->callback != NULL) return; //如果当前的菜单有回调那么渲染的逻辑交给回调执行 
		
    if (current_menu->child_count == 0) {
        // 无子菜单且无回调，显示静态提示界面
        draw_centered_text("Nothing Here!", 1); // 1 表示绘制边框
    } else {
        // 绘制当前菜单标题（位于屏幕顶部，居中显示）
        uint8_t title_width = u8g2_GetStrWidth(&u8g2, current_menu->name);
        uint8_t title_x = (u8g2_GetDisplayWidth(&u8g2) - title_width) / 2; // 计算居中位置
        u8g2_DrawStr(&u8g2, title_x, 8, current_menu->name); // 显示当前菜单名称作为标题

        // 如果有子菜单，绘制菜单项
        if (current_menu->child_count > 0) {
            for (int i = 0; i < MAX_INDEX_COUNT && (i + current_menu->window_start_index) < current_menu->child_count; ++i) {
                int y = 20 + i * MENU_LINE_HEIGHT; // 起始位置下移，为标题留空间
                
                if ((i + current_menu->window_start_index) == current_menu->current_index) {
                    // 绘制选中项的边框
                    int y1 = y - 9;
                    int width = 128;
                    int height = MENU_LINE_HEIGHT - 2;
                    
                    // 绘制实心边框
                    u8g2_DrawFrame(&u8g2, 0, y1, width, height); // 使用 DrawFrame 代替四条线
                    
                    // 绘制指示箭头
                    u8g2_DrawStr(&u8g2, 2, y, ">");
                    u8g2_DrawStr(&u8g2, 10, y, current_menu->children[i + current_menu->window_start_index]->name);
                } else {
                    // 绘制非选中项
                    u8g2_DrawStr(&u8g2, 10, y, current_menu->children[i + current_menu->window_start_index]->name);
                }
            }
            
            // 绘制滚动指示器（如果需要）
            if (current_menu->window_start_index > 0) {
                // 上滚动指示器（放大且位置稍低）
                u8g2_DrawTriangle(&u8g2, 124, 14, 120, 20, 128, 20); // 调整位置，避免与标题冲突
            }
            
            if (current_menu->window_start_index + MAX_INDEX_COUNT < current_menu->child_count) {
                // 下滚动指示器（放大且确保在屏幕内）
                u8g2_DrawTriangle(&u8g2, 124, 60, 120, 54, 128, 54);
            }
        }
    }
    
    u8g2_SendBuffer(&u8g2);
}
