#include "car_state_machine.h"
#include "systick.h"
#include <string.h>
#include "bluetooth.h"

// 外部标志
extern bool task_running_flag;


// 时间函数（需要根据平台实现）
static uint32_t get_time_ms(void) {
    return get_ms();
}

/* =============================================================================
 * API实现
 * ============================================================================= */

void car_path_init(void) {
    memset(&sm, 0, sizeof(sm));
    task_running_flag = false;
}

void car_add_straight(float distance) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_GO_STRAIGHT;
        sm.actions[sm.count].params.move.distance = distance;
        sm.count++;
    }
}

void car_add_turn(float angle) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_SPIN_TURN;
        sm.actions[sm.count].params.turn.angle = angle;
        sm.count++;
    }
}

void car_add_track(float distance) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_TRACK;
        sm.actions[sm.count].params.move.distance = distance;
        sm.count++;
    }
}

void car_add_move_until_black(int state) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_MOVE_UNTIL_BLACK;
        sm.actions[sm.count].params.until.state = state;
        sm.count++;
    }
}

void car_add_move_until_white(int state) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_MOVE_UNTIL_WHITE;
        sm.actions[sm.count].params.until.state = state;
        sm.count++;
    }
}

void car_add_move_until_stop_mark(int state) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_MOVE_UNTIL_STOP_MARK;
        sm.actions[sm.count].params.until.state = state;
        sm.count++;
    }
}

void car_add_delay(uint32_t ms) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_DELAY;
        sm.actions[sm.count].params.delay.ms = ms;
        sm.count++;
    }
}

// 函数调用功能
void car_add_function(void (*func)(void)) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_FUNCTION;
        sm.actions[sm.count].params.function.func = func;
        sm.count++;
    }
}

// 新增：布尔值设置功能
void car_add_bool(bool *flag, bool val) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_SET_BOOL;
        sm.actions[sm.count].params.set_bool.flag = flag;
        sm.actions[sm.count].params.set_bool.value = val;
        sm.count++;
    }
}

// 新增：浮点值设置功能
void car_add_float(float *var, float val) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_SET_FLOAT;
        sm.actions[sm.count].params.set_float.var = var;
        sm.actions[sm.count].params.set_float.value = val;
        sm.count++;
    }
}

// 新增：字节发送功能
void car_add_byte(uint8_t byte) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_SEND_BYTE;
        sm.actions[sm.count].params.send_byte.byte = byte;
        sm.count++;
    }
}

void car_set_loop(uint8_t loop_count) {
    sm.loop_count = loop_count;
}

void car_start(void) {
    if (sm.count > 0) {
        sm.is_running = true;
        sm.current = 0;
        sm.current_loop = 0;
        sm.first_call = true;
        task_running_flag = true;
    }
}

void car_stop(void) {
    sm.is_running = false;
    car.state = CAR_STATE_STOP;
    car_reset();
    task_running_flag = false;
}

bool car_is_running(void) {
    return sm.is_running;
}

void car_clear_actions(void) {
    sm.count = 0;
    sm.current = 0;
}

/* =============================================================================
 * 状态机核心
 * ============================================================================= */

void car_state_machine(void) {
    if (!sm.is_running) {
        return;
    }
    
    // 检查是否完成所有动作
    if (sm.current >= sm.count) {
        // 检查循环
        if (sm.loop_count == 0 || ++sm.current_loop < sm.loop_count) {
            sm.current = 0;
            sm.first_call = true;
        } else {
            car_stop();
            return;
        }
    }
    
    // 获取当前动作
    car_action_t* action = &sm.actions[sm.current];
    bool completed = false;
    
    // 记录开始时间
    if (sm.first_call) {
        sm.start_time = get_time_ms();
    }
    
    // 执行动作
    switch (action->type) {
        case ACTION_GO_STRAIGHT:
            completed = car_move_cm(action->params.move.distance, CAR_STATE_GO_STRAIGHT);
            break;
            
        case ACTION_SPIN_TURN:
            completed = spin_turn(action->params.turn.angle);
            break;
            
        case ACTION_TRACK:
            completed = car_move_cm(action->params.move.distance, CAR_STATE_TRACK);
            break;
            
        case ACTION_MOVE_UNTIL_BLACK:
            completed = car_move_until((CAR_STATES)action->params.until.state, UNTIL_BLACK_LINE);
            break;
            
        case ACTION_MOVE_UNTIL_WHITE:
            completed = car_move_until((CAR_STATES)action->params.until.state, UNTIL_WHITE_LINE);
            break;
            
        case ACTION_DELAY:
            completed = (get_time_ms() - sm.start_time) >= action->params.delay.ms;
            break;
         
        case ACTION_MOVE_UNTIL_STOP_MARK:
            completed = car_move_until((CAR_STATES)action->params.until.state, UNTIL_STOP_MARK);
            break;
            
        case ACTION_FUNCTION:
            if (sm.first_call) {
                // 立即执行函数
                if (action->params.function.func != NULL) {
                    action->params.function.func();
                }
                completed = true; // 立即完成，不等待
            }
            break;
           
        case ACTION_SET_BOOL:
            if (sm.first_call) {
                // 立即设置布尔值
                if (action->params.set_bool.flag != NULL) {
                    *(action->params.set_bool.flag) = action->params.set_bool.value;
                }
                completed = true; // 立即完成，不等待
            }
            break;
     
				case ACTION_SET_FLOAT:
            if (sm.first_call) {
                // 立即设置布尔值
                if (action->params.set_float.var != NULL) {
                    *(action->params.set_float.var) = action->params.set_float.value;
                }
                completed = true; // 立即完成，不等待
            }
            break;
						
        case ACTION_SEND_BYTE:
            if (sm.first_call) {
                // 立即发送蓝牙字节
                bluetooth_send_byte(action->params.send_byte.byte);
                completed = true; // 立即完成，不等待
            }
            break;
            
        default:
            completed = true;
            break;
    }
    
    // 动作完成，切换到下一个
    if (completed) {
        sm.current++;
        sm.first_call = true;
    } else {
        sm.first_call = false;
    }
}