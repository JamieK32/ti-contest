#include "ui.h"

void show_oled_opening_animation(void)
{
    float angle = 0.0f;
    uint32_t startTime = HAL_GetTick(); // 获取起始时间
    do {
        u8g2_ClearBuffer(&u8g2);

        // 计算已过时间(ms)
        uint32_t elapsedTime = HAL_GetTick() - startTime;

        // 1. --- MSPM0 波浪主标题 + “高亮字母动画” ---
        u8g2_SetFont(&u8g2, u8g2_font_inb16_mr);
        const char* text = "MSPM0";
        int textWidth = u8g2_GetStrWidth(&u8g2, text);

        // 每一帧随机一个字符用白底黑字高亮
        int highlight_i = ((int)(angle * 1.5f)) % 5;
        for (int i = 0, x = 64 - textWidth / 2; text[i] != '\0'; i++)
        {
            char c[2] = {text[i], '\0'};
            int charWidth = u8g2_GetStrWidth(&u8g2, c);
            int y = 30 + 8 * sinf(angle + i * 0.7f);
            if(i == highlight_i) {
                // 椭圆高亮底框
                u8g2_SetDrawColor(&u8g2, 1);
                u8g2_DrawBox(&u8g2, x - 2, y - 15, charWidth + 4, 19);
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
        for(int i = 0, x = 64 - subw / 2; subtext[i] != '\0'; i++) {
            char c[2] = {subtext[i], '\0'};
            int cw = u8g2_GetStrWidth(&u8g2, c);
            int y = 50 + 2 * sinf(angle + i * 0.5f);  // 副标题也微微波动
            // 字符显示点阵按alpha填补，制造淡入
            if((i * 13 + y + (int)angle) % 10 < (int)(10 * alpha)) // 随机采样点数呈现淡入
                u8g2_DrawStr(&u8g2, x, y, c);
            x += cw;
        }

        // 3. --- 顶部&底部波浪，交错+加星点 ---
        for (int x = 0; x < 128; x++)
        {
            int y1 = 60 + 3 * sinf(angle + x * 0.09f);
            u8g2_DrawPixel(&u8g2, x, y1);

            // 底部点星星
            if (((x * 15 + (int)angle * 8) % 113) == 0 && (elapsedTime % 600) < 400)
                u8g2_DrawPixel(&u8g2, x, y1 - 2);

            int y2 = 7 + 3 * sinf(angle + x * 0.1f + 3.14159f);
            u8g2_DrawPixel(&u8g2, x, y2);

            // 顶部点星星
            if (((x * 9 + (int)angle * 11) % 127) == 0 && (elapsedTime % 700) < 300)
                u8g2_DrawPixel(&u8g2, x, y2 + 2);
        }

        // 4. --- 进度条波浪填充 ---
        int bar_len = (elapsedTime * 118) / 3000;
        if (bar_len > 118) bar_len = 118;
        u8g2_DrawFrame(&u8g2, 5, 62, 118, 2);
        // 用小波浪做进度条填充
        for (int i = 0; i < bar_len; i++) {
            int y = 62 + (int)(sinf(angle + i * 0.25f) * 1.1f);
            u8g2_DrawPixel(&u8g2, 5 + i, y);
            u8g2_DrawPixel(&u8g2, 5 + i, 63); // 下沿也补齐
        }
        u8g2_SendBuffer(&u8g2);
        angle += 0.2f;
        HAL_Delay(50); // 20FPS 延迟
    } while((HAL_GetTick() - startTime) < 2000); // 持续 2 秒
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}



/**
 * @brief 在 OLED 上居中显示一串文本，并可选地绘制边框
 * @param text 要显示的文本
 */
void draw_centered_text(const char* text) {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    
    // 计算文字居中的位置
    uint8_t text_width = u8g2_GetStrWidth(&u8g2, text);
    uint8_t display_width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t display_height = u8g2_GetDisplayHeight(&u8g2);
    uint8_t x = (display_width - text_width) / 2;
    uint8_t y = display_height / 2;
    
    // 美化的边框参数
    uint8_t padding = 10;
    uint8_t frame_x = x - padding;
    uint8_t frame_y = y - 14;
    uint8_t frame_w = text_width + 2 * padding;
    uint8_t frame_h = 20;
    
    // 绘制阴影
    u8g2_DrawRFrame(&u8g2, frame_x + 1, frame_y + 1, frame_w, frame_h, 3);
    
    // 绘制主边框（圆角 + 背景）
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawRBox(&u8g2, frame_x, frame_y, frame_w, frame_h, 4);
    u8g2_SetDrawColor(&u8g2, 0);  // 反色文字
    
    // 绘制文本
    u8g2_DrawStr(&u8g2, x, y, text);
    
    u8g2_SetDrawColor(&u8g2, 1);  // 恢复正常颜色
    u8g2_SendBuffer(&u8g2);
}


// 美化版本的u8g2菜单显示代码（简洁无图标版）
// 保持美化效果的同时节省空间，删除图标让界面更简洁

// 优化的主菜单绘制函数
void draw_menu(MenuNode *current_menu) {    
    u8g2_ClearBuffer(&u8g2);
    
    // 如果当前的菜单有回调那么渲染的逻辑交给回调执行 
    if (current_menu->callback != NULL && current_menu->type == MENU_TYPE_NORMAL) {
        u8g2_SendBuffer(&u8g2);
        return;
    }
    
    // 绘制美化的边框
    draw_enhanced_frame();
    
    // 绘制美化的标题栏
    draw_enhanced_title_bar(current_menu->name, current_menu->type);
    
    // 根据菜单类型选择渲染方式
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW || current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        draw_enhanced_variables_content(current_menu);
    } else {
        draw_enhanced_normal_menu_content(current_menu);
    }
    
    // 绘制美化的滚动指示器
    draw_enhanced_scrollbar(current_menu);
    
    // 绘制美化的状态栏
    draw_enhanced_status_bar(current_menu);
    
    u8g2_SendBuffer(&u8g2);
}

// 绘制美化的边框（紧凑版）
void draw_enhanced_frame() {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    // 外边框（圆角效果）
    u8g2_DrawRFrame(&u8g2, 0, 0, width, height, 2);
    
    // 标题栏分隔线（单线，更紧凑）
    u8g2_DrawHLine(&u8g2, 1, 9, width - 2);
    
    // 底部状态栏分隔线（单线，更紧凑）
    u8g2_DrawHLine(&u8g2, 1, height - 7, width - 2);
    
    // 添加装饰性角落点
    u8g2_DrawPixel(&u8g2, 2, 2);
    u8g2_DrawPixel(&u8g2, width - 3, 2);
    u8g2_DrawPixel(&u8g2, 2, height - 3);
    u8g2_DrawPixel(&u8g2, width - 3, height - 3);
}

// 绘制美化的标题栏（简洁版）
void draw_enhanced_title_bar(const char* title, int menu_type) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    
    // 标题背景填充
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawBox(&u8g2, 1, 1, width - 2, 7);
    u8g2_SetDrawColor(&u8g2, 0);  // 反色文字
    
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);  // 使用更小的字体
    
    // 计算标题位置（居中）
    uint8_t title_width = u8g2_GetStrWidth(&u8g2, title);
    uint8_t title_x = (width - title_width) / 2;
    
    // 绘制标题
    u8g2_DrawStr(&u8g2, title_x, 7, title);
    
    u8g2_SetDrawColor(&u8g2, 1);  // 恢复正常颜色
}

void draw_enhanced_variables_content(MenuNode *current_menu) {
    if (current_menu->variable_count == 0) {
        draw_enhanced_empty_message("No Variables");
        return;
    }
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    
    for (int i = 0; i < MAX_INDEX_COUNT && (i + current_menu->window_start_index) < current_menu->variable_count; ++i) {
        int y = 17 + i * 11;  // 从更高位置开始，减少行间距
        int variable_index = i + current_menu->window_start_index;
        
        if (current_menu->variables && current_menu->variables[variable_index].name != NULL) {
            // 检查是否为选中的变量
            bool is_selected = (current_menu->selected_var_idx != UNSELECTED && 
                              variable_index == current_menu->selected_var_idx);
            bool is_current = (variable_index == current_menu->current_index);
            
            // 绘制选中背景（紧凑的圆角矩形）
            if (is_selected) {
                u8g2_SetDrawColor(&u8g2, 1);
                u8g2_DrawRBox(&u8g2, 2, y - 8, u8g2_GetDisplayWidth(&u8g2) - 4, 9, 1);
                u8g2_SetDrawColor(&u8g2, 0);  // 反色文字
            } else if (is_current) {
                // 当前项使用边框高亮
                u8g2_DrawRFrame(&u8g2, 2, y - 8, u8g2_GetDisplayWidth(&u8g2) - 4, 9, 1);
            }
            
            // 状态指示器（简洁符号）
            if (is_current) {
                u8g2_DrawStr(&u8g2, 4, y, is_selected ? "*" : ">");
            }
            
            // 变量名
            u8g2_DrawStr(&u8g2, 12, y, current_menu->variables[variable_index].name);
            
            // 变量值（右对齐）
            char val_str[16];
            snprintf(val_str, sizeof(val_str), "%.2f", *current_menu->variables[variable_index].val_ptr);
            uint8_t val_width = u8g2_GetStrWidth(&u8g2, val_str);
            u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - val_width - 4, y, val_str);
            
            // 恢复默认绘制颜色
            if (is_selected) {
                u8g2_SetDrawColor(&u8g2, 1);
            }
        }
    }
}

// 美化的普通菜单内容显示（简洁版）
void draw_enhanced_normal_menu_content(MenuNode *current_menu) {
    if (current_menu->child_count == 0) {
        draw_enhanced_empty_message("Empty Menu");
        return;
    }
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    
    for (int i = 0; i < MAX_INDEX_COUNT && (i + current_menu->window_start_index) < current_menu->child_count; ++i) {
        int y = 17 + i * 11;  // 从更高位置开始，减少行间距
        int menu_index = i + current_menu->window_start_index;
        bool is_current = (menu_index == current_menu->current_index);
        
        // 当前选中项的背景高亮
        if (is_current) {
            u8g2_SetDrawColor(&u8g2, 1);
            u8g2_DrawRBox(&u8g2, 2, y - 8, u8g2_GetDisplayWidth(&u8g2) - 4, 9, 1);
            u8g2_SetDrawColor(&u8g2, 0);  // 反色文字
        }
        
        // 选中指示器
        if (is_current) {
            u8g2_DrawStr(&u8g2, 4, y, ">");
        }
        
        // 菜单项名称
        u8g2_DrawStr(&u8g2, 12, y, current_menu->children[menu_index]->name);
        
        // 子菜单指示器
        if (current_menu->children[menu_index]->child_count > 0 || 
            current_menu->children[menu_index]->callback != NULL ||
            current_menu->children[menu_index]->variable_count > 0) {
            
            u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - 8, y, ">");
        }
        
        // 恢复正常颜色
        if (is_current) {
            u8g2_SetDrawColor(&u8g2, 1);
        }
    }
}

// 绘制美化的空内容提示（简洁版）
void draw_enhanced_empty_message(const char* message) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    
    uint8_t msg_width = u8g2_GetStrWidth(&u8g2, message);
    uint8_t msg_x = (width - msg_width) / 2;
    uint8_t msg_y = (height + 9) / 2;  // 考虑新的标题栏高度
    
    u8g2_DrawStr(&u8g2, msg_x, msg_y, message);
}

// 美化的滚动指示器（简洁版）
void draw_enhanced_scrollbar(MenuNode *current_menu) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    int total_items = (current_menu->type == MENU_TYPE_VARIABLES_VIEW || 
                      current_menu->type == MENU_TYPE_VARIABLES_MODIFY) ? 
                      current_menu->variable_count : current_menu->child_count;
    
    if (total_items > MAX_INDEX_COUNT) {
        // 滚动条背景
        u8g2_DrawVLine(&u8g2, width - 2, 11, height - 18);
        
        // 计算滚动条位置和大小
        int scroll_area_height = height - 18;
        int thumb_height = (MAX_INDEX_COUNT * scroll_area_height) / total_items;
        if (thumb_height < 2) thumb_height = 2;  // 最小高度
        
        int thumb_pos = (current_menu->window_start_index * (scroll_area_height - thumb_height)) / 
                       (total_items - MAX_INDEX_COUNT);
        
        // 滚动条滑块（填充）
        u8g2_DrawBox(&u8g2, width - 2, 11 + thumb_pos, 1, thumb_height);
        
        // 简洁的箭头指示器
        if (current_menu->window_start_index > 0) {
            // 向上箭头
            u8g2_DrawPixel(&u8g2, width - 4, 12);
            u8g2_DrawPixel(&u8g2, width - 3, 11);
            u8g2_DrawPixel(&u8g2, width - 5, 11);
        }
        
        if ((current_menu->window_start_index + MAX_INDEX_COUNT) < total_items) {
            // 向下箭头
            u8g2_DrawPixel(&u8g2, width - 4, height - 9);
            u8g2_DrawPixel(&u8g2, width - 3, height - 8);
            u8g2_DrawPixel(&u8g2, width - 5, height - 8);
        }
    }
}

// 美化的状态栏（简洁版）
void draw_enhanced_status_bar(MenuNode *current_menu) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    // 状态栏背景
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawBox(&u8g2, 1, height - 6, width - 2, 5);
    u8g2_SetDrawColor(&u8g2, 0);  // 反色文字
    
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);
    
    int total_items = (current_menu->type == MENU_TYPE_VARIABLES_VIEW || 
                      current_menu->type == MENU_TYPE_VARIABLES_MODIFY) ? 
                      current_menu->variable_count : current_menu->child_count;
    
    // 右侧：位置信息
    if (total_items > 0) {
        char pos_text[12];
        snprintf(pos_text, sizeof(pos_text), "%d/%d", 
                current_menu->current_index + 1, total_items);
        
        uint8_t text_width = u8g2_GetStrWidth(&u8g2, pos_text);
        u8g2_DrawStr(&u8g2, width - text_width - 3, height - 2, pos_text);
    }
    
    // 左侧：菜单类型指示
    const char* type_text;
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW) {
        type_text = "VAR";
    } else if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        type_text = "EDIT";
    } else if (current_menu->parent != NULL) {
        type_text = "SUB";
    } else {
        type_text = "ROOT";
    }
    
    u8g2_DrawStr(&u8g2, 3, height - 2, type_text);
    
    u8g2_SetDrawColor(&u8g2, 1);  // 恢复正常颜色
}
