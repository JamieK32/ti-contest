/*
 * oled_driver.h
 *
 *  Created on: 2022年7月24日
 *      Author: Unicorn_Li
 */
#ifndef __OLED_DRIVER_H__
#define __OLED_DRIVER_H__

#include "ui.h"

// 驱动模式选择
//#define OLED_DRIVER_MODE_SPI
#define OLED_DRIVER_MODE_I2C

#ifdef OLED_DRIVER_MODE_SPI

#define OLED_RST_Clr() DL_GPIO_clearPins(PORTB_PORT, PORTB_OLED_RST_PIN)
#define OLED_RST_Set() DL_GPIO_setPins(PORTB_PORT, PORTB_OLED_RST_PIN)
#define OLED_DC_Clr()  DL_GPIO_clearPins(PORTB_PORT, PORTB_OLED_DC_PIN)
#define OLED_DC_Set()  DL_GPIO_setPins(PORTB_PORT, PORTB_OLED_DC_PIN)
#define OLED_CS_Clr()  DL_GPIO_clearPins(PORTB_PORT, PORTB_OLED_CS_PIN)
#define OLED_CS_Set()  DL_GPIO_setPins(PORTB_PORT, PORTB_OLED_CS_PIN)

#endif // OLED_DRIVER_MODE_SPI

#ifdef OLED_DRIVER_MODE_I2C
// I2C 模式下使用的 GPIO 定义 (通常与 SPI 复用)
// 你的代码中复用了 SPI 的 PICO 和 SCLK 引脚作为 I2C 的 SDA 和 SCL
#define OLED_I2C_SDA_PORT   GPIOB
#define OLED_I2C_SDA_PIN    GPIO_PIN_9

#define OLED_I2C_SCL_PORT   GPIOB
#define OLED_I2C_SCL_PIN    GPIO_PIN_8


#define OLED_SDA_Clr() HAL_GPIO_WritePin(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN, GPIO_PIN_RESET)
#define OLED_SDA_Set() HAL_GPIO_WritePin(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN, GPIO_PIN_SET)

#define OLED_SCL_Clr() HAL_GPIO_WritePin(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN, GPIO_PIN_RESET)
#define OLED_SCL_Set() HAL_GPIO_WritePin(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN, GPIO_PIN_SET)


#endif // OLED_DRIVER_MODE_I2C

void u8g2_Init(void);

extern u8g2_t u8g2;

#endif // __OLED_DRIVER_H__
