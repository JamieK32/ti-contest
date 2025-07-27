#ifndef CAR_STATE_MACHINE_H__
#define CAR_STATE_MACHINE_H__
#include "stdint.h"
#include "stdbool.h"
#include "car_controller.h"

// 动作类型定义
typedef enum {
    ACTION_NONE = 0,
    ACTION_GO_STRAIGHT,
    ACTION_SPIN_TURN,
    ACTION_TRACK,
    ACTION_MOVE_UNTIL_BLACK,
    ACTION_MOVE_UNTIL_WHITE,
    ACTION_MOVE_UNTIL_STOP_MARK,
    ACTION_DELAY,
    ACTION_FUNCTION,       // 函数调用动作
    ACTION_SET_BOOL,       // 新增：布尔值设置动作
    ACTION_SET_FLOAT,       // 新增：浮点值设置动作
		ACTION_WAIT_FUNC_TRUE,
		ACTION_SEND_BYTE
} action_type_t;

// 动作参数联合体
typedef union {
    struct { float distance; } move;                  // 移动参数
    struct { float angle; } turn;                     // 转弯参数
    struct { int state; } until;                      // 条件等待参数
    struct { uint32_t ms; } delay;                    // 延时参数
    struct { void (*func)(void); } function;          // 函数指针参数
    struct { bool *flag; bool value; } set_bool;      // 布尔值设置参数
    struct { float *var; float value; } set_float;    // 浮点值设置参数
    struct { uint8_t byte; } send_byte;               // 新增：发送字节参数
		struct { bool (*func)(void); } wait_func_true; // <== 新增
} action_params_t;

// 动作结构体
typedef struct {
    action_type_t type;
    action_params_t params;
} car_action_t;

// 状态机结构
#define MAX_ACTIONS 50
static struct {
    car_action_t actions[MAX_ACTIONS];
    uint8_t count;
    uint8_t current;
    uint8_t loop_count;
    uint8_t current_loop;
    bool is_running;
    bool first_call;
    uint32_t start_time;
} sm = {0};

// 初始化路径（清空之前的所有动作）
void car_path_init(void);

// 添加动作
void car_add_straight(float distance);
void car_add_turn(float angle);
void car_add_track(float distance);
void car_add_move_until_black(int state);
void car_add_move_until_white(int state);
void car_add_move_until_stop_mark(int state);
void car_add_delay(uint32_t ms);
void car_add_function(void (*func)(void));  // 添加函数调用
void car_add_bool(bool *flag, bool val);    // 添加布尔值设置
void car_add_float(float *var, float val);  // 添加浮点值设置
void car_add_byte(uint8_t byte);            // 新增：添加蓝牙字节发送
void car_add_wait_func_true(bool (*func)(void));

// 设置循环
void car_set_loop(uint8_t loop_count);  // 0 = 无限循环

// 启动和停止
void car_start(void);
void car_stop(void);
bool car_is_running(void);

// 状态机更新（在主循环调用）
void car_state_machine(void);

// 动态控制
void car_clear_actions(void);  // 清空所有动作

#endif // CAR_STATE_MACHINE_H__