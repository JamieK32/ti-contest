#ifndef MENU_UI_H__
#define MENU_UI_H__

#include "ui.h"
#include "stdio.h"

void draw_menu(MenuNode *current_menu);                          
void draw_enhanced_frame(void);                                  
void draw_enhanced_title_bar(const char *title, int menu_type);  
void draw_enhanced_variables_content(MenuNode *current_menu);    
void draw_enhanced_normal_menu_content(MenuNode *current_menu);   
void draw_enhanced_scrollbar(MenuNode *current_menu);            
void draw_enhanced_status_bar(MenuNode *current_menu);           
void draw_enhanced_empty_message(const char* message);

#endif
