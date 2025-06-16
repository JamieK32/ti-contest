#ifndef BUTTON_APP_H__
#define BUTTON_APP_H__

#include "multi_button.h"

#define BUTTON_NUM 5

typedef enum {
	BUTTON_UP = 0,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_MIDDLE
} BUTTON_ID;

void user_button_init(BtnCallback single_click_cb, BtnCallback long_press_cb);

extern struct Button buttons[BUTTON_NUM];

#endif
