#ifndef GRAY_DETECTION_H__
#define GRAY_DETECTION_H__

#include "pca9555.h"

void gray_detection_init(void);
float gray_read_data(void);

extern sw_i2c_t pca9555_i2c;

#endif 
