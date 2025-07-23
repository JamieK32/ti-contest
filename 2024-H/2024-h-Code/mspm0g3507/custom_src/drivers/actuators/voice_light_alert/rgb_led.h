#ifndef RGB_LED_H__
#define RGB_LED_H__

#include "stdint.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

#define COLOR_RED    (Color){1, 0, 0}
#define COLOR_GREEN  (Color){0, 1, 0}
#define COLOR_BLUE   (Color){0, 0, 1}
#define COLOR_CYAN   (Color){0, 1, 1}
#define COLOR_YELLOW (Color){1, 1, 0}
#define COLOR_PURPLE (Color){1, 0, 1}
#define COLOR_WHITE  (Color){1, 1, 1}
#define COLOR_OFF    (Color){0, 0, 0}


void rgb_led_init(void);
void led_off(void);
void led_set_rgb(uint8_t R, uint8_t G, uint8_t B);
void led_set_color(Color color);

#endif
