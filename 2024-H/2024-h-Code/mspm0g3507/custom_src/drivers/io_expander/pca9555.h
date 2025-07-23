#ifndef __PCA9555_H__
#define __PCA9555_H__

#include <stdint.h>
#include "ti_msp_dl_config.h"
#include "hal_soft_i2c.h"

#include "delay.h"

/* Status definitions */
#define SUCCESS 0
#define ERROR   1

/* PCA9555 device address */
#define PCA9555_ADDR  0x20
#define SLAVE_ADDR0   0x40

/* I2C read/write command flags */
#define HOST_WRITE_COMMAND 0x00
#define HOST_READ_COMMAND  0x01

/* PCA9555 register addresses */
#define INPUT_PORT_REGISTER0              0x00  /* Input Port Register 0, controls IO00-IO07 */
#define INPUT_PORT_REGISTER1              0x01  /* Input Port Register 1, controls IO10-IO17 */
#define OUTPUT_PORT_REGISTER0             0x02  /* Output Port Register 0, controls IO00-IO07 */
#define OUTPUT_PORT_REGISTER1             0x03  /* Output Port Register 1, controls IO10-IO17 */
#define POLARITY_INVERSION_PORT_REGISTER0 0x04  /* Polarity Inversion Port Register 0, controls IO00-IO07 */
#define POLARITY_INVERSION_PORT_REGISTER1 0x05  /* Polarity Inversion Port Register 1, controls IO10-IO17 */
#define CONFIG_PORT_REGISTER0             0x06  /* Configuration Port Register 0, controls IO00-IO07 */
#define CONFIG_PORT_REGISTER1             0x07  /* Configuration Port Register 1, controls IO10-IO17 */

/* GPIO port definitions */
#define GPIO_PORT0 0
#define GPIO_PORT1 1

/* GPIO pin definitions */
#define GPIO_0 0x01
#define GPIO_1 0x02
#define GPIO_2 0x04
#define GPIO_3 0x08
#define GPIO_4 0x10
#define GPIO_5 0x20
#define GPIO_6 0x40
#define GPIO_7 0x80

/* Function prototypes */
uint16_t pca9555_read_bit12(sw_i2c_t* i2c, uint8_t slave_num);

#endif /* __PCA9555_H__ */