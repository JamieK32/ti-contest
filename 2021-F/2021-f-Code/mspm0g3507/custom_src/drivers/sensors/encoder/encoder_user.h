#ifndef ENCODER_APP_H__
#define ENCODER_APP_H__

#include "encoder.h"

typedef struct {
    uint32_t pin_mask;
    uint32_t iidx;
} encoder_pin_config_t;

extern encoder_manager_t robot_encoder_manager;

void encoder_application_init(void);
void encoder_group1_irq_handler(void);

#endif
