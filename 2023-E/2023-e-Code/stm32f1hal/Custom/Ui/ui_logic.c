#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* =============================================================================
 * 内部状态变量
 * ============================================================================= */
static volatile bool menu_update_pending = false;
static MenuNode *current_menu = NULL;
static uint32_t view_var_last_time = 0;
static uint32_t ui_current_time = 0;
static MessageState msg_state = {false, ""};

/* =============================================================================
 * 内部函数声明
 * ============================================================================= */
static void modify_variable_value(menu_variable_t *var, bool increase);
static float calculate_adaptive_step(menu_variable_t *var);
static void trigger_variable_callback(menu_variable_t *var);
static void draw_message_immediate(void);
static bool handle_message_input(void);

/* =============================================================================
 * 消息显示函数
 * ============================================================================= */
void show_message(const char* text) {
    msg_state.is_showing = true;
    
    strncpy(msg_state.message_text, text, sizeof(msg_state.message_text) - 1);
    msg_state.message_text[sizeof(msg_state.message_text) - 1] = '\0';
    
    draw_message_immediate();
}

// 兼容性函数
void draw_centered_text(const char* text) { 
    show_message(text); 
}

static void draw_message_immediate(void) {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    
    uint8_t text_width = u8g2_GetStrWidth(&u8g2, msg_state.message_text);
    uint8_t display_width = u8g2_GetDisplayWidth(&u8g2);
    uint8_t display_height = u8g2_GetDisplayHeight(&u8g2);
    uint8_t x = (display_width - text_width) / 2;
    uint8_t y = display_height / 2;
    
    uint8_t padding = 8;
    uint8_t frame_x = x - padding;
    uint8_t frame_y = y - 12;
    uint8_t frame_w = text_width + 2 * padding;
    uint8_t frame_h = 16;
    
    u8g2_DrawRFrame(&u8g2, frame_x, frame_y, frame_w, frame_h, 3);
    u8g2_DrawStr(&u8g2, x, y, msg_state.message_text);
    u8g2_SendBuffer(&u8g2);
}

static bool handle_message_input(void) {
    if (!msg_state.is_showing) return false;
    
    msg_state.is_showing = false;
    notify_menu_update();
    return true;
}

bool is_message_showing(void) {
    return msg_state.is_showing;
}

/* =============================================================================
 * 按钮处理函数
 * ============================================================================= */
static inline void btn_single_click_callback(void* btn) {
    struct Button* button = (struct Button*) btn;
    
    if (button == &buttons[BUTTON_UP]) {  
        menu_button_up();
    } else if (button == &buttons[BUTTON_DOWN]) { 
        menu_button_down();
    } else if (button == &buttons[BUTTON_LEFT]) {  
        menu_button_enter(); 
    } else if (button == &buttons[BUTTON_RIGHT]) {
        menu_button_back();
    }
}

static inline void btn_long_press_cb(void *btn) {
    static uint32_t last_trigger_time = 0;
    uint32_t current_time = UI_GET_TICK();
    
    if (current_time - last_trigger_time >= LONG_PRESS_INTERVAL) {
        struct Button* button = (struct Button*) btn;
        
        if (button == &buttons[BUTTON_UP]) {
            menu_button_up();
        } else if (button == &buttons[BUTTON_DOWN]) {
            menu_button_down();
        } else if (button == &buttons[BUTTON_LEFT]) {
            // 长按左键的特殊功能可以在这里添加
            menu_button_enter();
        } else if (button == &buttons[BUTTON_RIGHT]) {
            // 长按右键的特殊功能可以在这里添加
            menu_button_back();
        }
        
        last_trigger_time = current_time;
    }
}


/* =============================================================================
 * 核心菜单操作
 * ============================================================================= */
void create_oled_menu(MenuNode *root) {
    if (root == NULL) return;
		u8g2_Init();
		
		current_menu = root;
		
    user_button_init(&btn_single_click_callback, &btn_long_press_cb);    
		
#if SHOW_OPENING_ANIMATION
    show_oled_opening_animation();
#endif
    draw_menu(current_menu);
}

void select_next(void) {
    if (current_menu == NULL) return;
    
    // 变量修改模式
    if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY && 
        current_menu->selected_var_idx != UNSELECTED) {
        
        menu_variable_t *var = &current_menu->variables[current_menu->selected_var_idx];
        modify_variable_value(var, true);
        trigger_variable_callback(var);
        return;
    }
    
    // 计算最大索引
    uint8_t max_count = 0;
    if (current_menu->type == MENU_TYPE_NORMAL) {
        for (uint8_t i = 0; i < current_menu->child_count; i++) {
            MenuNode *child = current_menu->children[i];
            if (child != NULL && 
                !(child->flags & MENU_FLAG_HIDDEN) &&
                (child->show_condition == NULL || child->show_condition())) {
                max_count++;
            }
        }
    } else {
        max_count = current_menu->variable_count;
    }
    
    if (max_count == 0) return;
    
    if (current_menu->current_index < max_count - 1) {
        current_menu->current_index++;
        
        if (current_menu->current_index >= current_menu->window_start_index + MAX_INDEX_COUNT) {
            current_menu->window_start_index = current_menu->current_index - MAX_INDEX_COUNT + 1;
        }
    }
}

void select_previous(void) {
    if (current_menu == NULL) return;
    
    // 变量修改模式
    if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY && 
        current_menu->selected_var_idx != UNSELECTED) {
        
        menu_variable_t *var = &current_menu->variables[current_menu->selected_var_idx];
        modify_variable_value(var, false);
        trigger_variable_callback(var);
        return;
    }
    
    if (current_menu->current_index > 0) {
        current_menu->current_index--;
        
        if (current_menu->current_index < current_menu->window_start_index) {
            current_menu->window_start_index = current_menu->current_index;
        }
    }
}

void enter_current(void) {
    if (current_menu == NULL) return;
    
    if (current_menu->type == MENU_TYPE_NORMAL) {
        if (current_menu->child_count == 0) return;
        
        // 查找可见子菜单
        uint8_t visible_index = 0;
        MenuNode *target_child = NULL;
        
        for (uint8_t i = 0; i < current_menu->child_count; i++) {
            MenuNode *child = current_menu->children[i];
            if (child != NULL && 
                !(child->flags & MENU_FLAG_HIDDEN) &&
                !(child->flags & MENU_FLAG_DISABLED) &&
                (child->show_condition == NULL || child->show_condition())) {
                
                if (visible_index == current_menu->current_index) {
                    target_child = child;
                    break;
                }
                visible_index++;
            }
        }
        
        if (target_child != NULL) {
            // 动作菜单处理
            if (target_child->type == MENU_TYPE_ACTION) {
                if (target_child->callback != NULL) {
                    target_child->callback(target_child->user_data);
                }
                if (target_child->flags & MENU_FLAG_AUTO_RETURN) {
                    return;
                }
            }
            
            current_menu = target_child;
            current_menu->current_index = 0;
            current_menu->window_start_index = 0;
        }
        
    } else if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        // 进入变量修改模式
        if (current_menu->current_index < current_menu->variable_count) {
            current_menu->selected_var_idx = current_menu->current_index;
        }
    }
}

void return_previous(void) {
    if (current_menu == NULL) return;
    
    // 退出变量修改模式
    if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY && 
        current_menu->selected_var_idx != UNSELECTED) {
        current_menu->selected_var_idx = UNSELECTED;
        return;
    }
    
    // 返回父菜单
    if (current_menu->parent != NULL) {
        current_menu = current_menu->parent;
    }
}

/* =============================================================================
 * 按键处理
 * ============================================================================= */
void menu_button_up(void) {
    if (handle_message_input()) return;
    select_previous();
    notify_menu_update();
}

void menu_button_down(void) {
    if (handle_message_input()) return;
    select_next();
    notify_menu_update();
}

void menu_button_enter(void) {
    if (handle_message_input()) return;
    enter_current();
    notify_menu_update();
}

void menu_button_back(void) {
    if (handle_message_input()) return;
    return_previous();
    notify_menu_update();
}

/* =============================================================================
 * 变量操作
 * ============================================================================= */
static void modify_variable_value(menu_variable_t *var, bool increase) {
    if (var == NULL || var->val_ptr == NULL) return;
    if (var->type == VAR_TYPE_READONLY) return;
    
    float step = 0;
    
    // 根据修改模式计算步长
    switch (var->modify_mode) {
        case MODIFY_MODE_PERCENTAGE: {
            if (var->type == VAR_TYPE_FLOAT) {
                float current_val = *(float*)var->val_ptr;
                step = fabsf(current_val) * var->percentage;
                step = fmaxf(step, 0.001f);
            }
            break;
        }
        
        case MODIFY_MODE_FIXED_STEP:
            if (var->type == VAR_TYPE_FLOAT) step = var->f_step;
            else if (var->type == VAR_TYPE_INT) step = var->i_step;
            else if (var->type == VAR_TYPE_UINT) step = var->u_step;
            break;
            
        case MODIFY_MODE_ADAPTIVE_STEP:
            step = calculate_adaptive_step(var);
            break;
            
        case MODIFY_MODE_EXPONENTIAL: {
            if (var->type == VAR_TYPE_FLOAT) {
                float current_val = *(float*)var->val_ptr;
                if (fabsf(current_val) < 0.001f) {
                    step = 0.001f;
                } else {
                    step = fabsf(current_val) * 0.1f;
                }
            }
            break;
        }
    }
    
    if (!increase) step = -step;
    
    // 根据变量类型修改值
    switch (var->type) {
        case VAR_TYPE_FLOAT: {
            float *val = (float*)var->val_ptr;
            *val += step;
            if (*val < var->f_range.min) *val = var->f_range.min;
            if (*val > var->f_range.max) *val = var->f_range.max;
            break;
        }
        
        case VAR_TYPE_INT: {
            int *val = (int*)var->val_ptr;
            *val += (int)step;
            if (*val < var->i_range.min) *val = var->i_range.min;
            if (*val > var->i_range.max) *val = var->i_range.max;
            break;
        }
        
        case VAR_TYPE_UINT: {
            unsigned int *val = (unsigned int*)var->val_ptr;
            int new_val = (int)*val + (int)step;
            if (new_val < (int)var->u_range.min) new_val = var->u_range.min;
            if (new_val > (int)var->u_range.max) new_val = var->u_range.max;
            *val = (unsigned int)new_val;
            break;
        }
        
        case VAR_TYPE_BOOL: {
            bool *val = (bool*)var->val_ptr;
            *val = !(*val);
            break;
        }
        
        default:
            break;
    }
}

static float calculate_adaptive_step(menu_variable_t *var) {
    if (var->type != VAR_TYPE_FLOAT) return 1.0f;
    
    float current_val = *(float*)var->val_ptr;
    float range = var->f_range.max - var->f_range.min;
    
    if (range > 0) {
        return range * 0.01f;
    }
    
    return fmaxf(fabsf(current_val) * 0.05f, 0.001f);
}

static void trigger_variable_callback(menu_variable_t *var) {
    if (var != NULL && var->on_change != NULL) {
        var->on_change(var->val_ptr, var->type);
    }
}

/* =============================================================================
 * 菜单初始化
 * ============================================================================= */
void init_menu_node(MenuNode *node, const char *name, MenuCallback callback, 
                   MenuType type, MenuNode *parent, uint8_t child_count, MenuNode **children) {
    if (node == NULL) return;
    
    memset(node, 0, sizeof(MenuNode));
    
    node->name = name;
    node->type = type;
    node->callback = callback;
    node->parent = parent;
    
    node->current_index = 0;
    node->window_start_index = 0;
    node->selected_var_idx = UNSELECTED;
    
    node->child_count = (child_count > MAX_CHILD) ? MAX_CHILD : child_count;
    if (children != NULL && child_count > 0) {
        for (uint8_t i = 0; i < node->child_count; i++) {
            node->children[i] = children[i];
            if (children[i] != NULL) {
                children[i]->parent = node;
            }
        }
    }
}

void add_child_menu(MenuNode *parent, MenuNode *child) {
    if (parent == NULL || child == NULL) return;
    if (parent->child_count >= MAX_CHILD) return;
    
    parent->children[parent->child_count] = child;
    child->parent = parent;
    parent->child_count++;
}

void set_menu_condition(MenuNode *node, MenuCondition condition) {
    if (node != NULL) {
        node->show_condition = condition;
    }
}

void set_variable_change_callback(MenuNode *node, int var_idx, VariableChangeCallback callback) {
    if (node == NULL || node->variables == NULL) return;
    if (var_idx < 0 || var_idx >= node->variable_count) return;
    
    node->variables[var_idx].on_change = callback;
}

/* =============================================================================
 * 主循环更新
 * ============================================================================= */
void oled_menu_tick(void) {
    ui_current_time = UI_GET_TICK();

    // 如果正在显示消息，不处理菜单逻辑
    if (msg_state.is_showing) {
        return;
    }

    // 检查菜单更新
    if (menu_update_pending) {
        menu_update_pending = false;
        draw_menu(current_menu);
        
        if (current_menu != NULL && current_menu->callback != NULL) {
            current_menu->callback(current_menu->user_data);
        }
    }
    
    // 变量查看菜单自动刷新
    if (current_menu != NULL && 
        current_menu->type == MENU_TYPE_VARIABLES_VIEW && 
        ui_current_time - view_var_last_time >= VIEW_VAR_TIME_INTERVAL) {
        view_var_last_time = ui_current_time;
        menu_update_pending = true;
    }
}

void notify_menu_update(void) {
    menu_update_pending = true;
}
