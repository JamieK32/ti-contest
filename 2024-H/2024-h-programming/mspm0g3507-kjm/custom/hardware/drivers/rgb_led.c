#include "rgb_led.h"
#include "ti_msp_dl_config.h"


void rgb_led_init(void)
{
	led_off();
}


void led_off(void) {
	led_set_color(COLOR_OFF);
}

void led_set_rgb(uint8_t R, uint8_t G, uint8_t B) {
	R == 0 ? DL_GPIO_clearPins(PORTB_PORT, PORTB_R_PIN) : DL_GPIO_setPins(PORTB_PORT, PORTB_R_PIN);
	G == 0 ? DL_GPIO_clearPins(PORTB_PORT, PORTB_G_PIN) : DL_GPIO_setPins(PORTB_PORT, PORTB_G_PIN);
	B == 0 ? DL_GPIO_clearPins(PORTB_PORT, PORTB_B_PIN) : DL_GPIO_setPins(PORTB_PORT, PORTB_B_PIN);
}

void led_set_color(Color color) {
    led_set_rgb(color.r, color.g, color.b);
}