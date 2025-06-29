#include "mpuiic.h"  
#include "inv_mpu.h"

sw_i2c_t mpui2c = {
	.sclIOMUX = PORTA_SDA2_IOMUX,
	.sclPin = PORTA_SDA2_PIN,
	.sclPort = PORTA_PORT,
	.sdaIOMUX = PORTA_SCL2_IOMUX,
	.sdaPin = PORTA_SCL2_PIN,
	.sdaPort = PORTA_PORT,
};

void mpu6050_hardware_init(void) {
	SOFT_IIC_Init(&mpui2c);
}

uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	return SOFT_IIC_Write_Len(&mpui2c, addr, reg, len, buf);
}

uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{ 
	return SOFT_IIC_Read_Len(&mpui2c, addr, reg, len, buf);
}