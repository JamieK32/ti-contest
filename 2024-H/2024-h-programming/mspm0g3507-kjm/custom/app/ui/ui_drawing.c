#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* =============================================================================
 * 内部辅助函数
 * ============================================================================= */
static bool format_variable_value(menu_variable_t *var, char *buffer, size_t buffer_size);
static const char* get_variable_type_indicator(VariableType type);
static bool is_menu_item_visible(MenuNode *menu, int index);
static int count_visible_menu_items(MenuNode *menu);

/* =============================================================================
 * 变量值格式化
 * ============================================================================= */
static bool format_variable_value(menu_variable_t *var, char *buffer, size_t buffer_size) {
    if (var == NULL || buffer == NULL || var->val_ptr == NULL) {
        snprintf(buffer, buffer_size, "NULL");
        return false;
    }
    
    switch (var->type) {
        case VAR_TYPE_FLOAT: {
            float val = *(float*)var->val_ptr;
            if (var->format != NULL) {
                snprintf(buffer, buffer_size, var->format, val);
            } else {
                snprintf(buffer, buffer_size, "%.2f", val);
            }
            break;
        }
        
        case VAR_TYPE_INT: {
            int val = *(int*)var->val_ptr;
            if (var->format != NULL) {
                snprintf(buffer, buffer_size, var->format, val);
            } else {
                snprintf(buffer, buffer_size, "%d", val);
            }
            break;
        }
        
        case VAR_TYPE_UINT: {
            unsigned int val = *(unsigned int*)var->val_ptr;
            if (var->format != NULL) {
                snprintf(buffer, buffer_size, var->format, val);
            } else {
                snprintf(buffer, buffer_size, "%u", val);
            }
            break;
        }
        
        case VAR_TYPE_BOOL: {
            bool val = *(bool*)var->val_ptr;
            snprintf(buffer, buffer_size, "%s", val ? "ON" : "OFF");
            break;
        }
        
        case VAR_TYPE_READONLY:
        default: {
            float val = *(float*)var->val_ptr;
            if (var->format != NULL) {
                snprintf(buffer, buffer_size, var->format, val);
            } else {
                snprintf(buffer, buffer_size, "%.2f", val);
            }
            break;
        }
    }
    
    // 添加单位
    if (var->unit != NULL && strlen(var->unit) > 0) {
        size_t current_len = strlen(buffer);
        size_t remaining = buffer_size - current_len;
        if (remaining > strlen(var->unit)) {
            strncat(buffer, var->unit, remaining - 1);
        }
    }
    
    return true;
}

static const char* get_variable_type_indicator(VariableType type) {
    switch (type) {
        case VAR_TYPE_FLOAT: return "F";
        case VAR_TYPE_INT: return "I";
        case VAR_TYPE_UINT: return "U";
        case VAR_TYPE_BOOL: return "B";
        case VAR_TYPE_READONLY: return "R";
        default: return "?";
    }
}

/* =============================================================================
 * 菜单可见性检查
 * ============================================================================= */
static bool is_menu_item_visible(MenuNode *menu, int index) {
    if (menu == NULL || index >= menu->child_count) return false;
    
    MenuNode *child = menu->children[index];
    if (child == NULL) return false;
    if (child->flags & MENU_FLAG_HIDDEN) return false;
    if (child->show_condition && !child->show_condition()) return false;
    
    return true;
}

static int count_visible_menu_items(MenuNode *menu) {
    if (menu == NULL) return 0;
    
    int count = 0;
    for (int i = 0; i < menu->child_count; i++) {
        if (is_menu_item_visible(menu, i)) {
            count++;
        }
    }
    return count;
}

/* =============================================================================
 * 主菜单绘制
 * ============================================================================= */
void draw_menu(MenuNode *current_menu) {    
    if (current_menu == NULL) return;
    
    u8g2_ClearBuffer(&u8g2);
    
    // 如果有回调且是普通菜单，交给回调执行
    if (current_menu->callback != NULL && current_menu->type == MENU_TYPE_NORMAL) {
        u8g2_SendBuffer(&u8g2);
        return;
    }
    
    draw_enhanced_frame();
    draw_enhanced_title_bar(current_menu->name, current_menu->type);
    
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW || 
        current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        draw_enhanced_variables_content(current_menu);
    } else {
        draw_enhanced_normal_menu_content(current_menu);
    }
    
    draw_enhanced_scrollbar(current_menu);
    draw_enhanced_status_bar(current_menu);
    
    u8g2_SendBuffer(&u8g2);
}

/* =============================================================================
 * 边框绘制
 * ============================================================================= */
void draw_enhanced_frame() {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    // 外边框
    u8g2_DrawRFrame(&u8g2, 0, 0, width, height, 2);
    
    // 标题栏分隔线
    u8g2_DrawHLine(&u8g2, 1, 9, width - 2);
    
    // 底部状态栏分隔线
    u8g2_DrawHLine(&u8g2, 1, height - 7, width - 2);
    
    // 装饰性角落点
    u8g2_DrawPixel(&u8g2, 2, 2);
    u8g2_DrawPixel(&u8g2, width - 3, 2);
    u8g2_DrawPixel(&u8g2, 2, height - 3);
    u8g2_DrawPixel(&u8g2, width - 3, height - 3);
}

void draw_enhanced_title_bar(const char* title, int menu_type) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    
    // 标题背景填充
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawBox(&u8g2, 1, 1, width - 2, 7);
    u8g2_SetDrawColor(&u8g2, 0);  // 反色文字
    
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);
    
    // 左侧：菜单类型指示器
    const char* type_indicator = "";
    switch (menu_type) {
        case MENU_TYPE_VARIABLES_VIEW: type_indicator = "[V]"; break;
        case MENU_TYPE_VARIABLES_MODIFY: type_indicator = "[E]"; break;
        case MENU_TYPE_ACTION: type_indicator = "[A]"; break;
        default: type_indicator = "[M]"; break;
    }
    u8g2_DrawStr(&u8g2, 2, 7, type_indicator);
    
    // 标题居中显示
    if (title != NULL) {
        uint8_t title_width = u8g2_GetStrWidth(&u8g2, title);
        uint8_t indicator_width = u8g2_GetStrWidth(&u8g2, type_indicator);
        uint8_t available_width = width - indicator_width - 6;
        
        char display_title[32];
        if (title_width > available_width) {
            strncpy(display_title, title, sizeof(display_title) - 4);
            display_title[sizeof(display_title) - 4] = '\0';
            strcat(display_title, "...");
        } else {
            strncpy(display_title, title, sizeof(display_title) - 1);
            display_title[sizeof(display_title) - 1] = '\0';
        }
        
        uint8_t title_x = indicator_width + 4 + (available_width - u8g2_GetStrWidth(&u8g2, display_title)) / 2;
        u8g2_DrawStr(&u8g2, title_x, 7, display_title);
    }
    
    u8g2_SetDrawColor(&u8g2, 1);
}

/* =============================================================================
 * 变量菜单内容绘制
 * ============================================================================= */
void draw_enhanced_variables_content(MenuNode *current_menu) {
    if (current_menu->variable_count == 0) {
        draw_enhanced_empty_message("No Variables");
        return;
    }
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    
    for (int i = 0; i < MAX_INDEX_COUNT && (i + current_menu->window_start_index) < current_menu->variable_count; ++i) {
        int y = 17 + i * 11;
        int variable_index = i + current_menu->window_start_index;
        
        if (current_menu->variables && current_menu->variables[variable_index].name != NULL) {
            menu_variable_t *var = &current_menu->variables[variable_index];
            
            bool is_selected = (current_menu->selected_var_idx != UNSELECTED && 
                              variable_index == current_menu->selected_var_idx);
            bool is_current = (variable_index == current_menu->current_index);
            bool is_readonly = (var->type == VAR_TYPE_READONLY);
            
            // 绘制选中背景
            if (is_selected && !is_readonly) {
                u8g2_SetDrawColor(&u8g2, 1);
                u8g2_DrawRBox(&u8g2, 2, y - 8, u8g2_GetDisplayWidth(&u8g2) - 4, 9, 1);
                u8g2_SetDrawColor(&u8g2, 0);
            } else if (is_current) {
                if (is_readonly) {
                    // 只读变量使用虚线边框
                    for (int x = 2; x < u8g2_GetDisplayWidth(&u8g2) - 2; x += 2) {
                        u8g2_DrawPixel(&u8g2, x, y - 8);
                        u8g2_DrawPixel(&u8g2, x, y);
                    }
                    for (int yy = y - 8; yy <= y; yy += 2) {
                        u8g2_DrawPixel(&u8g2, 2, yy);
                        u8g2_DrawPixel(&u8g2, u8g2_GetDisplayWidth(&u8g2) - 3, yy);
                    }
                } else {
                    u8g2_DrawRFrame(&u8g2, 2, y - 8, u8g2_GetDisplayWidth(&u8g2) - 4, 9, 1);
                }
            }
            
            // 状态指示器
            if (is_current) {
                if (is_selected && !is_readonly) {
                    u8g2_DrawStr(&u8g2, 4, y, "*");  // 编辑模式
                } else if (is_readonly) {
                    u8g2_DrawStr(&u8g2, 4, y, "o");  // 只读
                } else {
                    u8g2_DrawStr(&u8g2, 4, y, ">");  // 可编辑
                }
            }
            
            // 变量名
            u8g2_DrawStr(&u8g2, 12, y, var->name);
            
            // 变量类型指示器
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);
            const char* type_indicator = get_variable_type_indicator(var->type);
            u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - 10, y - 3, type_indicator);
            u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
            
            // 变量值
            char val_str[32];
            if (format_variable_value(var, val_str, sizeof(val_str))) {
                uint8_t val_width = u8g2_GetStrWidth(&u8g2, val_str);
                u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - val_width - 14, y, val_str);
                
                if (is_selected && !is_readonly) {
                    u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - val_width - 22, y, "<");
                    u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - 12, y, ">");
                }
            }
            
            if (is_selected && !is_readonly) {
                u8g2_SetDrawColor(&u8g2, 1);
            }
        }
    }
}

/* =============================================================================
 * 普通菜单内容绘制
 * ============================================================================= */
void draw_enhanced_normal_menu_content(MenuNode *current_menu) {
    int visible_count = count_visible_menu_items(current_menu);
    
    if (visible_count == 0) {
        draw_enhanced_empty_message("Empty Menu");
        return;
    }
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    
    int visible_start = current_menu->window_start_index;
    int displayed = 0;
    
    for (int i = 0; i < current_menu->child_count && displayed < MAX_INDEX_COUNT; i++) {
        if (!is_menu_item_visible(current_menu, i)) continue;
        
        if (visible_start > 0) {
            visible_start--;
            continue;
        }
        
        int y = 17 + displayed * 11;
        bool is_current = (i == current_menu->current_index);
        MenuNode *child = current_menu->children[i];
        
        // 当前选中项背景
        if (is_current) {
            if (child->flags & MENU_FLAG_DISABLED) {
                for (int x = 2; x < u8g2_GetDisplayWidth(&u8g2) - 2; x += 2) {
                    u8g2_DrawPixel(&u8g2, x, y - 8);
                    u8g2_DrawPixel(&u8g2, x, y);
                }
            } else {
                u8g2_SetDrawColor(&u8g2, 1);
                u8g2_DrawRBox(&u8g2, 2, y - 8, u8g2_GetDisplayWidth(&u8g2) - 4, 9, 1);
                u8g2_SetDrawColor(&u8g2, 0);
            }
        }
        
        // 选中指示器
        if (is_current) {
            if (child->flags & MENU_FLAG_DISABLED) {
                u8g2_DrawStr(&u8g2, 4, y, "x");
            } else if (child->type == MENU_TYPE_ACTION) {
                u8g2_DrawStr(&u8g2, 4, y, "!");
            } else {
                u8g2_DrawStr(&u8g2, 4, y, ">");
            }
        }
        
        // 菜单项名称
        if (child->name != NULL) {
            char display_name[24];
            strncpy(display_name, child->name, sizeof(display_name) - 1);
            display_name[sizeof(display_name) - 1] = '\0';
            u8g2_DrawStr(&u8g2, 12, y, display_name);
        }
        
        // 右侧指示器
        const char* right_indicator = "";
        if (child->flags & MENU_FLAG_AUTO_RETURN) {
            right_indicator = "#";
        } else if (child->child_count > 0) {
            right_indicator = ">";
        } else if (child->callback != NULL) {
            right_indicator = ".";
        } else if (child->variable_count > 0) {
            if (child->type == MENU_TYPE_VARIABLES_MODIFY) {
                right_indicator = "E";
            } else {
                right_indicator = "V";
            }
        }
        
        if (strlen(right_indicator) > 0) {
            u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - 8, y, right_indicator);
        }
        
        if (is_current && !(child->flags & MENU_FLAG_DISABLED)) {
            u8g2_SetDrawColor(&u8g2, 1);
        }
        
        displayed++;
    }
}

/* =============================================================================
 * 空内容提示
 * ============================================================================= */
void draw_enhanced_empty_message(const char* message) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    
    uint8_t msg_width = u8g2_GetStrWidth(&u8g2, message);
    uint8_t msg_x = (width - msg_width) / 2;
    uint8_t msg_y = (height + 9) / 2;
    
    // 虚线边框
    for (int x = msg_x - 5; x < msg_x + msg_width + 5; x += 2) {
        u8g2_DrawPixel(&u8g2, x, msg_y - 10);
        u8g2_DrawPixel(&u8g2, x, msg_y + 3);
    }
    for (int y = msg_y - 10; y < msg_y + 3; y += 2) {
        u8g2_DrawPixel(&u8g2, msg_x - 5, y);
        u8g2_DrawPixel(&u8g2, msg_x + msg_width + 5, y);
    }
    
    u8g2_DrawStr(&u8g2, msg_x, msg_y, message);
}

/* =============================================================================
 * 滚动条绘制
 * ============================================================================= */
void draw_enhanced_scrollbar(MenuNode *current_menu) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    int total_items;
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW || 
        current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        total_items = current_menu->variable_count;
    } else {
        total_items = count_visible_menu_items(current_menu);
    }
    
    if (total_items > MAX_INDEX_COUNT) {
        // 滚动条背景
        u8g2_DrawVLine(&u8g2, width - 2, 11, height - 18);
        
        // 计算滚动条位置和大小
        int scroll_area_height = height - 18;
        int thumb_height = (MAX_INDEX_COUNT * scroll_area_height) / total_items;
        if (thumb_height < 3) thumb_height = 3;
        
        int thumb_pos = 0;
        if (total_items > MAX_INDEX_COUNT) {
            thumb_pos = (current_menu->window_start_index * (scroll_area_height - thumb_height)) / 
                       (total_items - MAX_INDEX_COUNT);
        }
        
        // 滚动条滑块
        u8g2_DrawBox(&u8g2, width - 3, 11 + thumb_pos, 2, thumb_height);
        
        // 滚动指示箭头
        if (current_menu->window_start_index > 0) {
            u8g2_DrawPixel(&u8g2, width - 6, 12);
            u8g2_DrawLine(&u8g2, width - 7, 13, width - 5, 13);
            u8g2_DrawLine(&u8g2, width - 8, 14, width - 4, 14);
        }
        
        if ((current_menu->window_start_index + MAX_INDEX_COUNT) < total_items) {
            u8g2_DrawLine(&u8g2, width - 8, height - 11, width - 4, height - 11);
            u8g2_DrawLine(&u8g2, width - 7, height - 10, width - 5, height - 10);
            u8g2_DrawPixel(&u8g2, width - 6, height - 9);
        }
    }
}

/* =============================================================================
 * 状态栏绘制
 * ============================================================================= */
void draw_enhanced_status_bar(MenuNode *current_menu) {
    uint8_t width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t height = u8g2_GetDisplayHeight(&u8g2);
    
    // 状态栏背景
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawBox(&u8g2, 1, height - 6, width - 2, 5);
    u8g2_SetDrawColor(&u8g2, 0);
    
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);
    
    int total_items;
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW || 
        current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        total_items = current_menu->variable_count;
    } else {
        total_items = count_visible_menu_items(current_menu);
    }
    
    // 右侧：位置信息
    if (total_items > 0) {
        char pos_text[16];
        snprintf(pos_text, sizeof(pos_text), "%d/%d", 
                current_menu->current_index + 1, total_items);
        
        uint8_t text_width = u8g2_GetStrWidth(&u8g2, pos_text);
        u8g2_DrawStr(&u8g2, width - text_width - 3, height - 2, pos_text);
    }
    
    // 左侧：菜单类型状态
    const char* status_text = "";
    if (current_menu->type == MENU_TYPE_VARIABLES_VIEW) {
        status_text = "VIEW";
    } else if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        if (current_menu->selected_var_idx != UNSELECTED) {
            status_text = "EDIT*";
        } else {
            status_text = "EDIT";
        }
    } else if (current_menu->type == MENU_TYPE_ACTION) {
        status_text = "ACT";
    } else if (current_menu->parent != NULL) {
        status_text = "SUB";
    } else {
        status_text = "ROOT";
    }
    
    u8g2_DrawStr(&u8g2, 3, height - 2, status_text);
    
    // 中间：特殊状态指示
    if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY && 
        current_menu->selected_var_idx != UNSELECTED &&
        current_menu->variables != NULL) {
        
        menu_variable_t *var = &current_menu->variables[current_menu->selected_var_idx];
        char mode_text[8];
        
        switch (var->modify_mode) {
            case MODIFY_MODE_PERCENTAGE: strcpy(mode_text, "%"); break;
            case MODIFY_MODE_FIXED_STEP: strcpy(mode_text, "±"); break;
            case MODIFY_MODE_ADAPTIVE_STEP: strcpy(mode_text, "~"); break;
            case MODIFY_MODE_EXPONENTIAL: strcpy(mode_text, "×"); break;
            default: strcpy(mode_text, "?"); break;
        }
        
        uint8_t mode_width = u8g2_GetStrWidth(&u8g2, mode_text);
        u8g2_DrawStr(&u8g2, (width - mode_width) / 2, height - 2, mode_text);
    }
    
    u8g2_SetDrawColor(&u8g2, 1);
}

