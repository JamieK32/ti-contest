#ifndef UI_H__
#define UI_H__

#include "ti_msp_dl_config.h"

#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>
#include "math.h"

#include "u8g2.h"
#include "ui_button.h"
#include "oled_driver.h"
#include "systick.h"

/* =============================================================================
 * 配置相关宏
 * ============================================================================= */
#ifndef UI_GET_TICK
#define UI_GET_TICK() system_time_get_ms()  // 替换为你的时间函数
#endif

#ifndef VIEW_VAR_TIME_INTERVAL  
#define VIEW_VAR_TIME_INTERVAL 1000   // 变量刷新间隔
#endif

#ifndef SHOW_OPENING_ANIMATION
#define SHOW_OPENING_ANIMATION 1     // 是否显示开机动画
#endif


/* =============================================================================
 * 基础配置
 * ============================================================================= */
#define MAX_CHILD 5                    // 最大子菜单数量
#define MAX_INDEX_COUNT 4               // 一次显示的最大项目数
#define UNSELECTED -1
#define LONG_PRESS_INTERVAL 100         // 长按频率

/* =============================================================================
 * 变量类型和修改模式
 * ============================================================================= */
typedef enum {
    VAR_TYPE_FLOAT,
    VAR_TYPE_INT,
    VAR_TYPE_UINT,
    VAR_TYPE_BOOL,
    VAR_TYPE_READONLY
} VariableType;

typedef enum {
    MODIFY_MODE_PERCENTAGE,             // 百分比修改（默认8%）
    MODIFY_MODE_FIXED_STEP,             // 固定步长
    MODIFY_MODE_ADAPTIVE_STEP,          // 自适应步长
    MODIFY_MODE_EXPONENTIAL             // 指数修改
} ModifyMode;

/* =============================================================================
 * 菜单类型
 * ============================================================================= */
typedef enum {
    MENU_TYPE_NORMAL,                   // 普通菜单
    MENU_TYPE_VARIABLES_VIEW,           // 变量查看
    MENU_TYPE_VARIABLES_MODIFY,         // 变量修改
    MENU_TYPE_ACTION                    // 动作菜单
} MenuType;

/* =============================================================================
 * 回调函数类型
 * ============================================================================= */
typedef void (*MenuCallback)(void *user_data);
typedef bool (*MenuCondition)(void);
typedef void (*VariableChangeCallback)(void *var_ptr, VariableType type);

/* =============================================================================
 * 变量结构体
 * ============================================================================= */
typedef struct {
    const char *name;                   // 变量名
    void *val_ptr;                      // 变量指针
    VariableType type;                  // 变量类型
    ModifyMode modify_mode;             // 修改模式
    
    // 范围限制
    union {
        struct { float min, max; } f_range;
        struct { int min, max; } i_range;
        struct { unsigned int min, max; } u_range;
    };
    
    // 步长设置
    union {
        float f_step;
        int i_step;
        unsigned int u_step;
    };
    
    float percentage;                   // 百分比修改比例
    VariableChangeCallback on_change;   // 变化回调
    const char *unit;                   // 单位
    const char *format;                 // 显示格式
} menu_variable_t;

/* =============================================================================
 * 菜单节点结构体
 * ============================================================================= */
typedef struct MenuNode {
    // 基本属性
    const char *name;
    MenuType type;
    MenuCallback callback;
    MenuCondition show_condition;
    
    // 层次结构
    struct MenuNode *parent;
    struct MenuNode *children[MAX_CHILD];
    uint8_t child_count;
    
    // 导航状态
    int current_index;
    int window_start_index;
    
    // 变量相关
    menu_variable_t *variables;
    uint8_t variable_count;
    int8_t selected_var_idx;
    
    // 扩展属性
    uint8_t flags;
    void *user_data;
} MenuNode;

/* =============================================================================
 * 消息显示系统
 * ============================================================================= */
typedef struct {
    bool is_showing;
    char message_text[64];
} MessageState;


/* =============================================================================
 * 菜单标志位
 * ============================================================================= */
#define MENU_FLAG_HIDDEN        0x01
#define MENU_FLAG_DISABLED      0x02
#define MENU_FLAG_AUTO_RETURN   0x04

/* =============================================================================
 * 便利宏 - 变量定义
 * ============================================================================= */
#define MENU_VAR_FLOAT(name, ptr) \
    {name, ptr, VAR_TYPE_FLOAT, MODIFY_MODE_PERCENTAGE, .f_range = {-999999, 999999}, .f_step = 0.1, .percentage = 0.08, NULL, NULL, "%.2f"}

#define MENU_VAR_INT(name, ptr) \
    {name, ptr, VAR_TYPE_INT, MODIFY_MODE_FIXED_STEP, .i_range = {-999999, 999999}, .i_step = 1, .percentage = 0.08, NULL, NULL, "%d"}

#define MENU_VAR_BOOL(name, ptr) \
    {name, ptr, VAR_TYPE_BOOL, MODIFY_MODE_FIXED_STEP, .i_range = {0, 1}, .i_step = 1, .percentage = 0, NULL, NULL, "%s"}

#define MENU_VAR_READONLY(name, ptr, type) \
    {name, ptr, VAR_TYPE_READONLY, MODIFY_MODE_FIXED_STEP, .f_range = {0, 0}, .f_step = 0, .percentage = 0, NULL, NULL, NULL}

#define MENU_VAR_FLOAT_RANGE(name, ptr, min_val, max_val, step_val) \
    {name, ptr, VAR_TYPE_FLOAT, MODIFY_MODE_FIXED_STEP, .f_range = {min_val, max_val}, .f_step = step_val, .percentage = 0.08, NULL, NULL, "%.2f"}

#define MENU_VAR_INT_RANGE(name, ptr, min_val, max_val, step_val) \
    {name, ptr, VAR_TYPE_INT, MODIFY_MODE_FIXED_STEP, .i_range = {min_val, max_val}, .i_step = step_val, .percentage = 0.08, NULL, NULL, "%d"}

#define MENU_VAR_PID(name, ptr) \
    {name, ptr, VAR_TYPE_FLOAT, MODIFY_MODE_EXPONENTIAL, .f_range = {0, 1000}, .f_step = 0.01, .percentage = 0.1, NULL, NULL, "%.3f"}

#define MENU_VAR_END {NULL, NULL, VAR_TYPE_FLOAT, MODIFY_MODE_FIXED_STEP, .f_range = {0, 0}, .f_step = 0, .percentage = 0, NULL, NULL, NULL}

/* =============================================================================
 * 便利宏 - 菜单构建
 * ============================================================================= */
#define MENU_BUILDER_START(root_node, root_name) \
    static MenuNode root_node = {0}; \
    init_menu_node(&root_node, root_name, NULL, MENU_TYPE_NORMAL, NULL, 0, NULL);

#define ADD_SUBMENU(parent, child, child_name, callback_func) \
    static MenuNode child = {0}; \
    init_menu_node(&child, child_name, callback_func, MENU_TYPE_NORMAL, &parent, 0, NULL); \
    add_child_menu(&parent, &child);

#define ADD_ACTION(parent, child, child_name, callback_func) \
    static MenuNode child = {0}; \
    init_menu_node(&child, child_name, callback_func, MENU_TYPE_ACTION, &parent, 0, NULL); \
    child.flags = MENU_FLAG_AUTO_RETURN; \
    add_child_menu(&parent, &child);

#define ADD_VAR_VIEW(parent, child, child_name, var_array) \
    static MenuNode child = {0}; \
    init_menu_node(&child, child_name, NULL, MENU_TYPE_VARIABLES_VIEW, &parent, 0, NULL); \
    child.variables = var_array; \
    child.variable_count = sizeof(var_array)/sizeof(var_array[0])-1; \
    add_child_menu(&parent, &child);

#define ADD_VAR_MODIFY(parent, child, child_name, var_array) \
    static MenuNode child = {0}; \
    init_menu_node(&child, child_name, NULL, MENU_TYPE_VARIABLES_MODIFY, &parent, 0, NULL); \
    child.variables = var_array; \
    child.variable_count = sizeof(var_array)/sizeof(var_array[0])-1; \
    add_child_menu(&parent, &child);

/* =============================================================================
 * 核心函数声明
 * ============================================================================= */
// 基础菜单操作
void create_oled_menu(MenuNode *root);
void select_next(void);
void select_previous(void);
void enter_current(void);
void return_previous(void);
void oled_menu_tick(void);
void notify_menu_update(void);

// 消息显示系统
void show_message(const char* text);            // 显示消息（手动退出）
void draw_centered_text(const char* text);      // 兼容函数
bool is_message_showing(void);

// 按键处理
void menu_button_up(void);
void menu_button_down(void);
void menu_button_enter(void);
void menu_button_back(void);

// 菜单初始化
void init_menu_node(MenuNode *node, const char *name, MenuCallback callback, 
                   MenuType type, MenuNode *parent, uint8_t child_count, MenuNode **children);
void add_child_menu(MenuNode *parent, MenuNode *child);
void set_menu_condition(MenuNode *node, MenuCondition condition);
void set_variable_change_callback(MenuNode *node, int var_idx, VariableChangeCallback callback);

// 绘制函数
void draw_menu(MenuNode *current_menu);
void draw_enhanced_frame(void);
void draw_enhanced_title_bar(const char* title, int menu_type);
void draw_enhanced_variables_content(MenuNode *current_menu);
void draw_enhanced_normal_menu_content(MenuNode *current_menu);
void draw_enhanced_empty_message(const char* message);
void draw_enhanced_scrollbar(MenuNode *current_menu);
void draw_enhanced_status_bar(MenuNode *current_menu);

// 开机动画
void show_oled_opening_animation(void);

void menu_init_and_create(void);


// 需要外部提供的u8g2相关
extern u8g2_t u8g2;

#endif // UI_H__
