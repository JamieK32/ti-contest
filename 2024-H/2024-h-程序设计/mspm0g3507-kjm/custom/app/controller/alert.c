#include "alert.h"
#include "freertos.h"
#include "task.h"

static Color current_color = COLOR_GREEN;
static uint8_t alert_count = 0;
static uint8_t alert_enable = 0;
static uint16_t alert_time = 300;
static TickType_t last_tick_time = 0;
static uint8_t current_state = 0;
static uint8_t completed_cycles = 0;

void play_alert_blocking(uint8_t count, Color c) {
	for (int i = 0; i < count; i++) {
		led_set_color(c);
		beep_on();
		delay_ms(200);
		beep_off();
		led_set_color(COLOR_OFF);
		delay_ms(200);
	}
}

void set_alert_color(Color c) {
	current_color = c;
}

void set_alert_count(uint8_t count) {
	alert_count = count;
}

void set_alert_interval_time(uint16_t time) {
	alert_time = time;
}

void start_alert(void) {
	// 只有在警报未启用且设置了次数时才启动
	if (!alert_enable && alert_count > 0) {
        alert_enable = 1;
        completed_cycles = 0; // 重置已完成的循环次数
        current_state = 0; // 初始状态为关
        last_tick_time = xTaskGetTickCount(); // 初始化计时器，以便立即开始第一个间隔
    }
}

void stop_alert(void) {
    alert_enable = 0;
    completed_cycles = 0; // 清零已完成循环次数
    // 确保最后是关闭状态
    led_set_color(COLOR_OFF);
    beep_off();
    current_state = 0; // 重置状态
}

void alert_ticks(void) {
	if (!alert_enable) return;

	TickType_t current_tick_time = xTaskGetTickCount();
	// 检查时间间隔是否到达
	if (current_tick_time - last_tick_time >= pdMS_TO_TICKS(alert_time)) {
		// 时间间隔到了，切换状态
		last_tick_time = current_tick_time; // 更新上次时间
		
		if (current_state == 0) { // 当前是关状态，切换到开
			led_set_color(current_color);
			beep_on();
			current_state = 1;
		} else { // 当前是开状态，切换到关
			led_set_color(COLOR_OFF);
			beep_off();
			current_state = 0;
			// 只有当从开到关切换时，才计算完成了一个循环
			completed_cycles++;
            
			// 检查是否完成所有预期的循环次数
			if (completed_cycles >= alert_count) {
				alert_enable = 0; // 停止提示
				completed_cycles = 0; // 重置计数
			}
		}
	}
}
