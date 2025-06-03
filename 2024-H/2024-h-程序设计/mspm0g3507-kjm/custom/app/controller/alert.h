#ifndef ALERT_H__
#define ALERT_H__

#include "rgb_led.h"
#include "beep.h"
#include "delay.h"

void play_alert_blocking(uint8_t count, Color c);
void set_alert_color(Color c);
void set_alert_count(uint8_t count);
void set_alert_interval_time(uint16_t time);
void start_alert(void);
void stop_alert(void);
void alert_ticks(void);

#endif
