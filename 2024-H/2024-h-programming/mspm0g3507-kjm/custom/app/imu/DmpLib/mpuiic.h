#ifndef __MPUIIC_H
#define __MPUIIC_H

#include "sw_i2c.h"
#include "ti_msp_dl_config.h"
#include "stdio.h"

uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);//IIC连续写
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf); //IIC连续读 

void mpu6050_hardware_init(void);

#endif
