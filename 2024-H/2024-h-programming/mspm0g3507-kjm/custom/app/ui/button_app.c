#include "button_app.h"
#include "multi_button.h"
#include "ti_msp_dl_config.h"

struct Button buttons[BUTTON_NUM]; //4个按键

uint8_t button_ids[BUTTON_NUM] = {BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT};

static inline uint8_t read_button_GPIO(uint8_t button_id) {
	switch(button_id)
	{
		case BUTTON_UP:
			return DL_GPIO_readPins(PORTB_PORT, PORTB_KEY1_PIN) == 0 ? 0 : 1;
			break;
		case BUTTON_DOWN:
			return DL_GPIO_readPins(PORTB_PORT, PORTB_KEY2_PIN) == 0 ? 0 : 1;
			break;
		case BUTTON_LEFT:
			return DL_GPIO_readPins(PORTB_PORT, PORTB_KEY3_PIN) == 0 ? 0 : 1;
			break;
		case BUTTON_RIGHT:
			return DL_GPIO_readPins(PORTB_PORT, PORTB_KEY4_PIN) == 0 ? 0 : 1;
			break;
		default:
			return 0;
			break;
	}
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
