#ifndef __JYXX_H
#define __JYXX_H

#include "ti_msp_dl_config.h"
#include "stdio.h"

#define WAIT_HEADER1 0
#define WAIT_HEADER2 1
#define RECEIVE_EULER_ANGLE 2


typedef struct {
    uint8_t rollL;
    uint8_t rollH;
    uint8_t pitchL;
    uint8_t pitchH;
    uint8_t yawL;
    uint8_t yawH;
    uint8_t vL;
    uint8_t vH;
    uint8_t sum;
} eulerAngle;

typedef struct {
    eulerAngle euler_angle;
    uint16_t dataIndex;
    uint8_t rxState;
    float roll;
    float pitch;
    float yaw;
} WitImu_TypeDef;

void wit_imu_init(void);
void wit_imu_process(void);
void wit_imu_uart_irq_handler(DL_UART_IIDX idx);
void wit_imu_set_yaw_zero(void);
void wit_imu_get_euler_angle(float *yaw, float *roll, float *pitch);

extern WitImu_TypeDef jy61p;

#endif
