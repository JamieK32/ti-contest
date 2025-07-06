#ifndef BSP_SPI_H__
#define BSP_SPI_H__

#include "ti_msp_dl_config.h"

uint8_t spi_read_write_byte(SPI_Regs *spi_inst, uint8_t byte);

#endif 
