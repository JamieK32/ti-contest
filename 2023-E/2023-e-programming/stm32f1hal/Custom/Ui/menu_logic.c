#include "ui.h"

// 替换 FreeRTOS 相关的变量
static volatile bool menu_update_pending = false;
static MenuNode *current_menu;

void create_oled_menu(MenuNode *root) {
    current_menu = root;
    
    // 初始化OLED显示
    u8g2_Init();
#if SHOW_OPENING_ANIMATION
    show_oled_opening_animation();
#endif
    draw_menu(current_menu);
}

void select_next(void)
{
    if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY && current_menu->selected_var_idx != UNSELECTED) {
        float num = *current_menu->variables[current_menu->selected_var_idx].val_ptr;
        float increment = num * 0.08;
        increment = fmin(100, fmax(0.1, increment));
        *current_menu->variables[current_menu->selected_var_idx].val_ptr += increment; 
        return;
    }
    uint8_t max_count = (current_menu->type == MENU_TYPE_NORMAL) ? 
                        current_menu->child_count : current_menu->variable_count;
    
    if (current_menu->current_index < max_count - 1) {
        current_menu->current_index++;
        if (current_menu->current_index >= current_menu->window_start_index + MAX_INDEX_COUNT)
            current_menu->window_start_index = current_menu->current_index - MAX_INDEX_COUNT + 1;
    }
}

void select_previous(void)
{
    if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY && current_menu->selected_var_idx != UNSELECTED) {
        float num = *current_menu->variables[current_menu->selected_var_idx].val_ptr;
        float increment = num * 0.08;
        increment = fmin(100, fmax(0.1, increment));
        *current_menu->variables[current_menu->selected_var_idx].val_ptr -= increment; 
        return;
    }
    if (current_menu->current_index > 0) current_menu->current_index--;
    if (current_menu->current_index < current_menu->window_start_index) current_menu->window_start_index = current_menu->current_index;
}

void enter_current(void) {
    if (current_menu->type == MENU_TYPE_NORMAL) {
        if (current_menu->child_count == 0) return;
        current_menu = current_menu->children[current_menu->current_index];
    } else if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        current_menu->selected_var_idx = current_menu->current_index;
    }
}

void return_previous(void) {
    if (current_menu->type == MENU_TYPE_NORMAL || current_menu->selected_var_idx == UNSELECTED) {
        if (current_menu->parent == NULL) return;
        current_menu = current_menu->parent;
    } else if (current_menu->type == MENU_TYPE_VARIABLES_MODIFY) {
        current_menu->selected_var_idx = UNSELECTED;
    }
}

// 替换原来的 vOLEDTask，改为tick函数
void oled_menu_tick(void)
{
    // 检查是否有待更新的显示
    if (menu_update_pending) {
        menu_update_pending = false;
        draw_menu(current_menu);
        execute_callback();
				if (current_menu->type != MENU_TYPE_VARIABLES_VIEW) {
					stop_listening_variable_timer();
				}
    }
}

void start_listening_variable_timer(void) {
	enable_task("VIEW_VAR", true);
}

void stop_listening_variable_timer(void) {
	enable_task("VIEW_VAR", false);
}

void execute_callback(void) {
    if (current_menu->callback != NULL) current_menu->callback(NULL);
}


void notify_menu_update(void)
{
    menu_update_pending = true;
}

// 实现初始化函数
void init_menu_node(MenuNode *node, const char *name, MenuCallback callback, MenuType type, MenuNode *parent, uint8_t child_count, MenuNode **children) {
    node->current_index = 0;
    node->window_start_index = 0;
    node->selected_var_idx = UNSELECTED;
    node->name = name;
    node->callback = callback;
    node->type = type;
    node->parent = parent;
    node->child_count = child_count;
    memset(node->children, 0, sizeof(node->children));
    if (children != NULL && child_count > 0) {
        for (uint8_t i = 0; i < child_count && i < MAX_CHILD; i++) {
            node->children[i] = children[i];
        }
    }
}

void init_variables_view_node(MenuNode *node, menu_variables_t *variables, uint8_t count) {
    node->variables = variables;
    node->variable_count = count;
}
