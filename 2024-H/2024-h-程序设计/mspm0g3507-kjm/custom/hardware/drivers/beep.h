#ifndef BEEP_H__
#define BEEP_H__

#include "stdbool.h"

void beep_init(void);
void beep_control(bool state);
void beep_on(void);
void beep_off(void);


#endif
