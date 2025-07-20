#ifndef _DELAY_H_
#define _DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ti_msp_dl_config.h"

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void delay_s(uint32_t s);

#ifdef __cplusplus
}
#endif

#endif /* _DELAY_H_ */