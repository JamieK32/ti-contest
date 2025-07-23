#include "ui.h"

struct Button buttons[BUTTON_NUM]; //4个按键

uint8_t button_ids[BUTTON_NUM] = {BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT};

uint8_t read_button_GPIO(uint8_t button_id) {
	switch(button_id)
	{
		case BUTTON_UP:
			return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 0 ? 1 : 0;
			break;
		case BUTTON_DOWN:
			return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 0 ? 1 : 0;
			break;
		case BUTTON_LEFT:
			return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == 0 ? 1 : 0;
			break;
		case BUTTON_RIGHT:
			return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == 0 ? 1 : 0;
			break;
	}
	return 0;
}

void user_button_init(BtnCallback single_click_cb, BtnCallback long_press_cb)
{
    // 批量初始化和绑定事件
    for (int i = 0; i < BUTTON_NUM; i++) {
        button_init(&buttons[i], read_button_GPIO, 0, button_ids[i]);
        button_attach(&buttons[i], SINGLE_CLICK, single_click_cb);
        // 仅为前两个按钮绑定长按事件
        if (i < 2) {
            button_attach(&buttons[i], LONG_PRESS_HOLD, long_press_cb);
        }
        button_start(&buttons[i]);
    }
}
