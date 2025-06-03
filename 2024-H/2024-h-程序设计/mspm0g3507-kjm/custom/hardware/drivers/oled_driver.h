/*
 * oled_driver.h
 *
 *  Created on: 2022年7月24日
 *      Author: Unicorn_Li
 */
#ifndef __OLED_DRIVER_H__
#define __OLED_DRIVER_H__

#include "stdint.h"
#include "delay.h"
#include "u8g2.h"
#include "ti_msp_dl_config.h"


// 驱动模式选择
#define OLED_DRIVER_MODE_SPI
//#define OLED_DRIVER_MODE_I2C

#ifdef OLED_DRIVER_MODE_SPI

#define OLED_RST_Clr() DL_GPIO_clearPins(OLED_SPI_PORT, OLED_SPI_RST_OLED_PIN)
#define OLED_RST_Set() DL_GPIO_setPins(OLED_SPI_PORT, OLED_SPI_RST_OLED_PIN)
#define OLED_DC_Clr()  DL_GPIO_clearPins(OLED_SPI_PORT, OLED_SPI_DC_OLED_PIN)
#define OLED_DC_Set()  DL_GPIO_setPins(OLED_SPI_PORT, OLED_SPI_DC_OLED_PIN)
#define OLED_CS_Clr()  DL_GPIO_clearPins(OLED_SPI_PORT, OLED_SPI_CS_OLED_PIN)
#define OLED_CS_Set()  DL_GPIO_setPins(OLED_SPI_PORT, OLED_SPI_CS_OLED_PIN)

#endif // OLED_DRIVER_MODE_SPI

#ifdef OLED_DRIVER_MODE_I2C
// I2C 模式下使用的 GPIO 定义 (通常与 SPI 复用)
// 你的代码中复用了 SPI 的 PICO 和 SCLK 引脚作为 I2C 的 SDA 和 SCL
#define OLED_I2C_SDA_PORT   GPIO_SPI_0_PICO_PORT
#define OLED_I2C_SDA_PIN    GPIO_SPI_0_PICO_PIN

#define OLED_I2C_SCL_PORT   GPIO_SPI_0_SCLK_PORT
#define OLED_I2C_SCL_PIN    GPIO_SPI_0_SCLK_PIN

#define OLED_I2C_SCL_IOMUX 	GPIO_SPI_0_IOMUX_SCLK
#define OLED_I2C_SDA_IOMUX  GPIO_SPI_0_IOMUX_PICO

#define OLED_I2C_RST_PORT OLED_SPI_PORT 
#define OLED_I2C_RST_PIN  OLED_SPI_RST_OLED_PIN

#define OLED_SDA_Clr() DL_GPIO_clearPins(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN)
#define OLED_SDA_Set() DL_GPIO_setPins(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN)

#define OLED_SCL_Clr() DL_GPIO_clearPins(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN)
#define OLED_SCL_Set() DL_GPIO_setPins(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN)

#define OLED_I2C_RST_Clr() DL_GPIO_clearPins(OLED_I2C_RST_PORT, OLED_I2C_RST_PIN)
#define OLED_I2C_RST_Set() DL_GPIO_setPins(OLED_I2C_RST_PORT, OLED_I2C_RST_PIN)

#endif // OLED_DRIVER_MODE_I2C

void u8g2_Init(void);

extern u8g2_t u8g2;

#endif // __OLED_DRIVER_H__
