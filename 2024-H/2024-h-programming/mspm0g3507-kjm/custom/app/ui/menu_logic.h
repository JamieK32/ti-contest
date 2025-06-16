#ifndef MENU_LOGIC_H__
#define MENU_LOGIC_H__

#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "oled_driver.h"

#define MAX_CHILD 10                // 当前节点最多的孩子数量
#define MENU_LINE_HEIGHT 13         // 屏幕行高
#define SHOW_OPENING_ANIMATION 1 	  // 是否显示开场动画 1 / 0
#define MAX_INDEX_COUNT 4     		  // 一次可以显示的最大窗口数量
#define LONG_PRESS_INTERVAL 100  		// 按键长按的频率 越少越快
#define UNSELECTED -1

typedef void (*MenuCallback)(void *user_data);

typedef enum {
    MENU_TYPE_NORMAL,        		// 普通菜单
    MENU_TYPE_VARIABLES_VIEW, 		// 变量查看菜单
		MENU_TYPE_VARIABLES_MODIFY, 	// 修改变量菜单
} MenuType;

typedef struct menu_variables_t {
	const char *name;
	float *val_ptr;
} menu_variables_t;


/**
 * @brief 菜单节点结构体，表示一个菜单项（可能有子菜单）
 */
typedef struct MenuNode {

		// 普通菜单相关
		int current_index;
		int window_start_index;
    const char     *name;													// 菜单项显示名称
    MenuCallback    callback;                     // 此菜单项对应的回调，可以执行相应任务，渲染界面
    struct MenuNode *parent;                      // 指向父菜单节点，根节点为NULL
    struct MenuNode *children[MAX_CHILD];         // 子菜单节点指针数组
    uint8_t         child_count;                  // 当前有效子节点数
		
		// 变量显示菜单相关
    menu_variables_t* variables;         // 新增：变量数组
    uint8_t variable_count;              // 新增：变量数量
		
		// 修改变量相关
		int8_t selected_var_idx;
		
    MenuType        type;								 // 当前菜单类型
} MenuNode;

void create_oled_menu(MenuNode *root);
void select_next(void);
void select_previous(void);
void enter_current(void);
void return_previous(void);
void vOLEDTask(void *pvParameters);
void execute_callback(void);
void update_variables(void);
void vVariableUpdateTimerCallback(TimerHandle_t xTimer);
void create_listening_variable_timer(void);
void start_listening_variable_timer(void);
void stop_listening_variable_timer(void);
void NotifyMenuFromISR(void);
void init_menu_node(MenuNode *node, const char *name, MenuCallback callback, MenuType type, 
										MenuNode *parent, uint8_t child_count, MenuNode **children);
void init_variables_view_node(MenuNode *node, menu_variables_t *variables, uint8_t count);

#endif
