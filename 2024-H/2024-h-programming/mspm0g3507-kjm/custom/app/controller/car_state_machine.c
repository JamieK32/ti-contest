#include "car_state_machine.h"
#include "systick.h"
#include <string.h>

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
        sm.actions[sm.count].params.move.speed = 0; // 默认速度
        sm.count++;
    }
}

void car_add_straight_speed(float distance, float speed) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_GO_STRAIGHT;
        sm.actions[sm.count].params.move.distance = distance;
        sm.actions[sm.count].params.move.speed = speed;
        sm.count++;
    }
}

void car_add_turn(float angle) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_SPIN_TURN;
        sm.actions[sm.count].params.turn.angle = angle;
        sm.actions[sm.count].params.turn.speed = 0; // 默认速度
        sm.count++;
    }
}

void car_add_turn_speed(float angle, float speed) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_SPIN_TURN;
        sm.actions[sm.count].params.turn.angle = angle;
        sm.actions[sm.count].params.turn.speed = speed;
        sm.count++;
    }
}

void car_add_track(float distance) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_TRACK;
        sm.actions[sm.count].params.move.distance = distance;
        sm.actions[sm.count].params.move.speed = 0;
        sm.count++;
    }
}

void car_add_track_speed(float distance, float speed) {
    if (sm.count < MAX_ACTIONS) {
        sm.actions[sm.count].type = ACTION_TRACK;
        sm.actions[sm.count].params.move.distance = distance;
        sm.actions[sm.count].params.move.speed = speed;
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
            // 如果有速度参数且是第一次调用，设置速度
            if (sm.first_call && action->params.move.speed > 0) {
                // set_motor_speed(action->params.move.speed);
            }
            completed = car_move_cm(action->params.move.distance, CAR_STATE_GO_STRAIGHT);
            break;
            
        case ACTION_SPIN_TURN:
            if (sm.first_call && action->params.turn.speed > 0) {
                // set_turn_speed(action->params.turn.speed);
            }
            completed = spin_turn(action->params.turn.angle);
            break;
            
        case ACTION_TRACK:
            if (sm.first_call && action->params.move.speed > 0) {
                // set_motor_speed(action->params.move.speed);
            }
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