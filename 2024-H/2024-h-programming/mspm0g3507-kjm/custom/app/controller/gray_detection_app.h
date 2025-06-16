#ifndef GRAY_DETECTION_H__
#define GRAY_DETECTION_H__

#include "pca9555.h"

void gray_detection_init(void);
void gray_read_data(uint8_t *gray_datas);

#endif 
