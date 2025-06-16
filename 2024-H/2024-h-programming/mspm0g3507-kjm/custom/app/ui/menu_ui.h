#ifndef MENU_UI_H__
#define MENU_UI_H__

#include "menu_logic.h"
#include "stdio.h"

void show_oled_opening_animation(void);
void draw_centered_text(const char* text, uint8_t draw_border);
void draw_menu(MenuNode *current_menu);
void draw_frame();
void draw_title_bar(const char* title);
void draw_variables_content(MenuNode *current_menu);
void draw_normal_menu_content(MenuNode *current_menu);
void draw_empty_message(const char* message);
void draw_scrollbar(MenuNode *current_menu);
void draw_status_bar(MenuNode *current_menu);

#endif
