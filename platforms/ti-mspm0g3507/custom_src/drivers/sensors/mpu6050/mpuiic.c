#include "mpuiic.h"  
#include "inv_mpu.h"

soft_iic_info_struct  mpui2c = {
	.sclIOMUX = PORTA_SDA2_IOMUX,
	.sclPin = PORTA_SDA2_PIN,
	.sclPort = PORTA_PORT,
	.sdaIOMUX = PORTA_SCL2_IOMUX,
	.sdaPin = PORTA_SCL2_PIN,
	.sdaPort = PORTA_PORT,
	.delay_time = 1,
	.addr = 0x68,
};

void mpu6050_hardware_init(void) {
	soft_iic_init(&mpui2c);
}

uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	soft_iic_write_8bit_registers(&mpui2c, reg, buf, len);
	return 0;
}

uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{ 
	soft_iic_read_8bit_registers(&mpui2c, reg, buf, len);
	return 0;
}