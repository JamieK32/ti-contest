#include "menu_logic.h"
#include "menu_ui.h"
#include "common_defines.h"
#include "periodic_event_task.h"
#include "log_config.h"
#include "log.h"


static TaskHandle_t xOLEDTaskHandle;
static MenuNode *current_menu;
static menu_variables_t menu_variables[MAX_INDEX_COUNT];

void create_oled_menu(MenuNode *root) {
    current_menu = root;
    xTaskCreate(vOLEDTask, "OLED_MENU", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &xOLEDTaskHandle);
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


void vOLEDTask(void *pvParameters)
{
		u8g2_Init();
#if SHOW_OPENING_ANIMATION
		show_oled_opening_animation();
#endif
		draw_menu(current_menu);
    for ( ; ; ) {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // 等待通知、收到自动清0
      draw_menu(current_menu);
			execute_callback();
    }
}

void execute_callback(void) {
	if (current_menu->callback != NULL) current_menu->callback(NULL);
}


// 启动定时器
void start_listening_variable_timer(void) {
	enable_periodic_task(EVENT_MENU_VAR_UPDATE);
}

// 停止定时器
void stop_listening_variable_timer(void) {
	disable_periodic_task(EVENT_MENU_VAR_UPDATE);
}

extern float test_1;

/**
 * @brief 在中断服务程序中通知指定任务
 * @param taskHandle 要通知的任务句柄
 * @note 此函数只能在 ISR 中调用
 */
void NotifyMenuFromISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // 向指定任务发送通知
    vTaskNotifyGiveFromISR(xOLEDTaskHandle, &xHigherPriorityTaskWoken);
    
    // 根据优先级情况决定是否需要任务切换
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		test_1 += 1;
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