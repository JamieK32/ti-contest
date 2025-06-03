#include "beep.h"
#include "ti_msp_dl_config.h"

void beep_init(void) {
	beep_off();
}

void beep_control(bool state) {
	state ? beep_on() : beep_off();
}

void beep_on(void) {
	DL_GPIO_setPins(PORTA_PORT, PORTA_BEEP_PIN);
}

void beep_off(void) {
	DL_GPIO_clearPins(PORTA_PORT, PORTA_BEEP_PIN);
}