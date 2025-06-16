#ifndef CAR_STATE_MACHINE_H__
#define CAR_STATE_MACHINE_H__

#include "stdint.h"
#include "stdbool.h"
#include "car_controller.h"

// 动作队列结构体
#define QUEUE_MAX_SIZE 10  // 队列最大容量，可根据需求调整

// 最大动作数量，限制 actions 数组的大小
#define ACTION_MAX_COUNT 10

// 动作类型枚举
typedef enum {
		ACTION_INVALID = 0, 
    ACTION_GO_STRAIGHT,  // 直行
    ACTION_SPIN_TURN,     // 原地旋转
		ACTION_TRACK,
		ACTION_MOVE_UNTIL_BLACK,
		ACTION_MOVE_UNTIL_WHITE
} action_type_t;
// 动作结构体，包含动作类型和目标值
typedef struct {
    action_type_t type;  // 动作类型
    float target_value;  // 目标值（直行：里程厘米；旋转：角度度数）
} car_action_t;

typedef struct {
    car_action_t actions[QUEUE_MAX_SIZE];  // 动作数组
    uint8_t head;                          // 队列头指针
    uint8_t tail;                          // 队列尾指针
    uint8_t size;                          // 当前队列中的动作数量
} action_queue_t;

// 动作配置结构体，包含动作序列和循环标志位
typedef struct {
    car_action_t actions[QUEUE_MAX_SIZE];  // 动作序列
    uint8_t action_count;                  // 动作数量
    bool is_loop_enabled;                  // 循环执行标志，true 表示循环，false 表示单次
		uint8_t loop_count;
} action_config_t;

void car_state_machine(void);
void car_path_init(action_config_t *config);

#endif 
