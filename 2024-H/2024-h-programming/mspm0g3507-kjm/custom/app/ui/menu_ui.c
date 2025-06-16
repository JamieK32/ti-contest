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


// 优化字体的主菜单绘制函数
void draw_menu(MenuNode *current_menu) {    
    u8g2_ClearBuffer(&u8g2);
    
    // 如果当前的菜单有回调那么渲染的逻辑交给回调执行 
    if (current_menu->callback != NULL && current_menu->type == MENU_TYPE_NORMAL) {
        u8g2_SendBuffer(&u8g2);
        return;
    }
    
    // 绘制简化框架
    draw_frame();
    
    // 绘制标题栏
    draw_title_bar(current_menu->name);
    
    // 根据菜单类型选择渲染方式
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW || current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        draw_variables_content(current_menu);
		} else {
        draw_normal_menu_content(current_menu);
    }
    
    // 绘制滚动指示器
    draw_scrollbar(current_menu);
    
    // 绘制状态栏
    draw_status_bar(current_menu);
    
    u8g2_SendBuffer(&u8g2);
}

// 绘制简化框架
void draw_frame() {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    // 标题栏分隔线
    u8g2_DrawHLine(&u8g2, 0, 9, width);   // 调整标题栏高度
    
    // 底部状态栏分隔线
    u8g2_DrawHLine(&u8g2, 0, height - 7, width);  // 调整状态栏高度
}

// 绘制标题栏（使用小字体）
void draw_title_bar(const char* title) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);  // 使用5x8字体
    
    // 计算标题位置（居中）
    uint8_t title_width = u8g2_GetStrWidth(&u8g2, title);
    uint8_t title_x = (width - title_width) / 2;
    
    // 绘制标题
    u8g2_DrawStr(&u8g2, title_x, 7, title);  // 调整Y位置
}

void draw_variables_content(MenuNode *current_menu) {
    if (current_menu->variable_count == 0) {
        draw_empty_message("No Variables");
        return;
    }
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);  // 使用5x8字体
    
    for (int i = 0; i < MAX_INDEX_COUNT && (i + current_menu->window_start_index) < current_menu->variable_count; ++i) {
        int y = 18 + i * 12;  // 调整行高为12像素，增加间距
        int variable_index = i + current_menu->window_start_index;
        
        if (current_menu->variables && current_menu->variables[variable_index].name != NULL) {
            // 检查是否为选中的变量
            bool is_selected = (current_menu->selected_var_idx != UNSELECTED && 
                              variable_index == current_menu->selected_var_idx);
            
            // 如果是选中状态，绘制选中背景或边框
            if (is_selected) {
                // 方案1：绘制反色背景
                int bg_width = u8g2_GetDisplayWidth(&u8g2) - 2;
                u8g2_SetDrawColor(&u8g2, 1);
                u8g2_DrawBox(&u8g2, 1, y - 9, bg_width, 10);  // 绘制填充矩形
                u8g2_SetDrawColor(&u8g2, 0);  // 设置为反色绘制文字
                
                // 方案2：或者绘制选中边框（注释掉上面的代码，使用下面的代码）
                // u8g2_DrawFrame(&u8g2, 0, y - 9, u8g2_GetDisplayWidth(&u8g2), 10);
            } else {
                u8g2_SetDrawColor(&u8g2, 1);  // 正常颜色绘制
            }
            
            // 当前光标指示器
            if (variable_index == current_menu->current_index) {
                u8g2_DrawStr(&u8g2, 1, y, ">");
            }
            
            // 变量名
            u8g2_DrawStr(&u8g2, 8, y, current_menu->variables[variable_index].name);
            
            // 变量值（右对齐显示）
            char val_str[12];
            snprintf(val_str, sizeof(val_str), "%.2f", *current_menu->variables[variable_index].val_ptr);
            uint8_t val_width = u8g2_GetStrWidth(&u8g2, val_str);
            u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - val_width - 3, y, val_str);
            
            // 恢复默认绘制颜色
            if (is_selected) {
                u8g2_SetDrawColor(&u8g2, 1);
            }
        }
    }
}

// 优化的普通菜单内容显示
void draw_normal_menu_content(MenuNode *current_menu) {
    if (current_menu->child_count == 0) {
        draw_empty_message("Empty Menu");
        return;
    }
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);  // 使用5x8字体
    
    for (int i = 0; i < MAX_INDEX_COUNT && (i + current_menu->window_start_index) < current_menu->child_count; ++i) {
        int y = 18 + i * 12;  // 调整行高为12像素，增加间距
        int menu_index = i + current_menu->window_start_index;
        
        // 选中指示器
        if (menu_index == current_menu->current_index) {
            u8g2_DrawStr(&u8g2, 1, y, ">");
        }
        
        // 菜单项名称
        u8g2_DrawStr(&u8g2, 8, y, current_menu->children[menu_index]->name);
        
        // 如果有子菜单，显示箭头指示
        if (current_menu->children[menu_index]->child_count > 0 || 
            current_menu->children[menu_index]->callback != NULL ||
						current_menu->children[menu_index]->variable_count > 0) {
            u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - 6, y, "<-");
        }
    }
}

// 绘制空内容提示
void draw_empty_message(const char* message) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    uint8_t msg_width = u8g2_GetStrWidth(&u8g2, message);
    uint8_t msg_x = (width - msg_width) / 2;
    uint8_t msg_y = (height + 9) / 2;  // 考虑新的标题栏高度和间距
    
    u8g2_DrawStr(&u8g2, msg_x, msg_y, message);
}

// 简化的滚动指示器
void draw_scrollbar(MenuNode *current_menu) {
		uint8_t width = u8g2_GetDisplayWidth(&u8g2);
		uint8_t height = u8g2_GetDisplayHeight(&u8g2);
		
		int total_items = (current_menu->type == MENU_TYPE_VARIABLES_VIEW || current_menu->type == MENU_TYPE_VARIABLES_MODIFY) ? 
											current_menu->variable_count : current_menu->child_count;
		
		if (total_items > MAX_INDEX_COUNT) {
				// 上方有更多内容 - 显示向上箭头
				if (current_menu->window_start_index > 0) {
						u8g2_DrawPixel(&u8g2, width - 2, 12);
						u8g2_DrawPixel(&u8g2, width - 3, 13);
						u8g2_DrawPixel(&u8g2, width - 1, 13);
				}
				
				// 下方有更多内容 - 显示向下箭头
				if ((current_menu->window_start_index + MAX_INDEX_COUNT) < total_items) {
						u8g2_DrawPixel(&u8g2, width - 2, height - 10);
						u8g2_DrawPixel(&u8g2, width - 3, height - 11);
						u8g2_DrawPixel(&u8g2, width - 1, height - 11);
				}
		}
}

// 简化的状态栏
void draw_status_bar(MenuNode *current_menu) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);  // 状态栏使用最小字体
    
    int total_items = (current_menu->type == MENU_TYPE_VARIABLES_VIEW  || current_menu->type == MENU_TYPE_VARIABLES_MODIFY) ? 
                      current_menu->variable_count : current_menu->child_count;
    
    // 显示位置信息
    if (total_items > 0) {
        char pos_text[10];
        snprintf(pos_text, sizeof(pos_text), "%d/%d", 
                current_menu->current_index + 1, total_items);
        
        uint8_t text_width = u8g2_GetStrWidth(&u8g2, pos_text);
        u8g2_DrawStr(&u8g2, width - text_width - 2, height - 2, pos_text);
    }
    
    // 左侧显示菜单类型指示
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW) {
        u8g2_DrawStr(&u8g2, 2, height - 2, "VAR");
    } else if (current_menu->parent != NULL) {
        u8g2_DrawStr(&u8g2, 2, height - 2, "SUB");
    } else {
        u8g2_DrawStr(&u8g2, 2, height - 2, "ROOT");
    }
}