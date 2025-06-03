#include "car_state_machine.h"

// 全局动作队列
static action_queue_t action_queue = {
    .head = 0,
    .tail = 0,
    .size = 0
};


// 当前使用的动作配置，初始化时可以被用户传入的配置覆盖
static action_config_t *current_action_config;
static uint8_t current_loop_count = 0;  // 循环次数计数器

/**
 * @brief 初始化动作队列
 */
void action_queue_init(void) {
    action_queue.head = 0;
    action_queue.tail = 0;
    action_queue.size = 0;
}

/**
 * @brief 向队列中添加一个动作
 * @param type 动作类型
 * @param target_value 目标值（里程或角度）
 * @return true 表示添加成功，false 表示队列已满
 */
bool action_queue_enqueue(action_type_t type, float target_value) {
    if (action_queue.size >= QUEUE_MAX_SIZE) {
        return false;  // 队列已满
    }
    action_queue.actions[action_queue.tail].type = type;
    action_queue.actions[action_queue.tail].target_value = target_value;
    action_queue.tail = (action_queue.tail + 1) % QUEUE_MAX_SIZE;
    action_queue.size++;
    return true;
}

/**
 * @brief 从队列中取出一个动作
 * @param action 存储取出的动作
 * @return true 表示取出成功，false 表示队列为空
 */
bool action_queue_dequeue(car_action_t *action) {
    if (action_queue.size == 0) {
        return false;  // 队列为空
    }
    *action = action_queue.actions[action_queue.head];
    action_queue.head = (action_queue.head + 1) % QUEUE_MAX_SIZE;
    action_queue.size--;
    return true;
}

/**
 * @brief 判断队列是否为空
 * @return true 表示队列为空，false 表示队列非空
 */
bool action_queue_is_empty(void) {
    return (action_queue.size == 0);
}

/**
 * @brief 自动计算动作配置中的动作数量
 * @param config 动作配置结构体指针
 * @return 计算出的动作数量
 */
uint8_t calculate_action_count(action_config_t *config) {
    uint8_t count = 0;
    // 遍历 actions 数组，直到数组结束或遇到无效动作类型
    for (uint8_t i = 0; i < ACTION_MAX_COUNT; i++) {
        // 假设 ACTION_INVALID 是一个无效动作类型，用于标记数组末尾
        // 如果你的系统中没有无效动作类型，可以根据 target_value 或其他条件判断
        if (config->actions[i].type == ACTION_INVALID || config->actions[i].type == 0) {
            break;
        }
        count++;
    }
    return count;
}

/**
 * @brief 从配置表中加载动作到队列
 * @param config 动作配置结构体指针
 * @return true 表示加载成功，false 表示队列容量不足
 */
bool action_queue_load_from_config(action_config_t *config) {
    action_queue_init();  // 清空当前队列
    // 自动计算动作数量
    config->action_count = calculate_action_count(config);
    for (uint8_t i = 0; i < config->action_count; i++) {
        if (!action_queue_enqueue(config->actions[i].type, config->actions[i].target_value)) {
            return false;  // 队列容量不足
        }
    }
    return true;
}

/**
 * @brief 初始化小车路径，从用户传入的配置表加载动作到队列
 * @param config 用户传入的动作配置结构体指针，如果为 NULL 则使用默认配置
 */
void car_path_init(action_config_t *config) {
    if (config != NULL) {
        current_action_config = config;  // 使用用户传入的配置
    }
    action_queue_load_from_config(current_action_config);  // 加载动作到队列
    current_loop_count = 0;  // 重置循环计数器
    if (current_action_config->loop_count > 0) {
        current_action_config->is_loop_enabled = true;
    }
}

/**
 * @brief 小车状态机，使用队列管理动作并支持循环或单次执行
 */
void car_state_machine(void) {
    static car_action_t current_action;  // 当前正在执行的动作
    static bool is_action_active = false;  // 是否有动作正在执行


    // 如果当前没有动作在执行，且队列为空，且循环模式启用，则重新加载配置表
    if (!is_action_active && action_queue_is_empty() && current_action_config->is_loop_enabled) {
        current_loop_count++;  // 完成一次完整序列循环，计数器加1
        if (current_loop_count >= current_action_config->loop_count) {
            current_action_config->is_loop_enabled = false;  // 达到循环次数限制，禁用循环
            car.state = CAR_STATE_STOP;  // 确保小车进入停止状态
            for (int i = 0; i < MOTOR_TYPE_TWO_WHEEL; i++) {
                car.target_speed[i] = 0;  // 目标速度清零
            }
            return;  // 退出函数，不再加载新动作
        } else {
            action_queue_load_from_config(current_action_config);  // 未达到循环次数限制，继续加载动作
        }
    }

    // 如果当前没有动作在执行，且队列不为空，则从队列中取出一个动作
    if (!is_action_active && !action_queue_is_empty()) {
        if (action_queue_dequeue(&current_action)) {
            is_action_active = true;
        }
    }

    // 如果有动作在执行，调用对应的控制函数
    if (is_action_active) {
        bool action_completed = false;
        if (current_action.type == ACTION_GO_STRAIGHT) {
            action_completed = car_move_cm(current_action.target_value, CAR_STATE_GO_STRAIGHT);
        } else if (current_action.type == ACTION_SPIN_TURN) {
            action_completed = spin_turn(current_action.target_value);
        } else if (current_action.type == ACTION_TRACK) {
            action_completed = car_move_cm(current_action.target_value, CAR_STATE_TRACK);
        } else if (current_action.type == ACTION_MOVE_UNTIL_BLACK) {
            action_completed = car_move_until((CAR_STATES)current_action.target_value, UNTIL_BLACK_LINE);
        } else if (current_action.type == ACTION_MOVE_UNTIL_WHITE) {
            action_completed = car_move_until((CAR_STATES)current_action.target_value, UNTIL_WHITE_LINE);
        }
        // 如果当前动作完成，标记为未激活，准备处理下一个动作
        if (action_completed) {
            is_action_active = false;
        }
    }
}
