#ifndef MENU_UI_H__
#define MENU_UI_H__

#include "menu_logic.h"
#include "stdio.h"

void draw_centered_text(const char* text, uint8_t draw_border);
void draw_variables_menu(menu_variables_t *menu_variables);
void draw_menu(MenuNode *current_menu);
void show_oled_opening_animation(void);

#endif
