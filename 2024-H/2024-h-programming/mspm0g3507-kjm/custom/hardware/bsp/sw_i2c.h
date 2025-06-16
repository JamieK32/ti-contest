#ifndef __SW_I2C_H__
#define __SW_I2C_H__

#include <stdint.h>
#include "ti/driverlib/dl_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C 操作返回值
#define IIC_SUCCESS               0x00  // 操作成功
#define IIC_ERROR_INVALID_PARAM   0x01  // 无效参数
#define IIC_ERROR_ACK_FAIL        0x02  // 应答失败
#define IIC_ERROR_TIMEOUT         0x03  // 操作超时
#define IIC_ERROR_UNKNOWN         0xFF  // 未知错误


// 软件I2C配置结构体
typedef struct {
    GPIO_Regs* sdaPort;         // SDA端口 (如 GPIOB)
    uint32_t sdaPin;            // SDA引脚（如 DL_GPIO_PIN_3）
    uint32_t sdaIOMUX;          // SDA IOMUX配置
    GPIO_Regs* sclPort;         // SCL端口 (如 GPIOB)
    uint32_t sclPin;            // SCL引脚（如 DL_GPIO_PIN_4）
    uint32_t sclIOMUX;          // SCL IOMUX配置
} sw_i2c_t;

// 函数声明
void SOFT_IIC_Init(const sw_i2c_t *i2c_cfg);
void SOFT_SDA_OUT(const sw_i2c_t *i2c_cfg);
void SOFT_SDA_IN(const sw_i2c_t *i2c_cfg);
void SOFT_IIC_Start(const sw_i2c_t *i2c_cfg);
void SOFT_IIC_Stop(const sw_i2c_t *i2c_cfg);
uint8_t SOFT_IIC_Wait_Ack(const sw_i2c_t *i2c_cfg);
void SOFT_IIC_Ack(const sw_i2c_t *i2c_cfg);
void SOFT_IIC_NAck(const sw_i2c_t *i2c_cfg);
void SOFT_IIC_Send_Byte(const sw_i2c_t *i2c_cfg, uint8_t txd);
uint8_t SOFT_IIC_Read_Byte(const sw_i2c_t *i2c_cfg, unsigned char ack);
void SOFT_IIC_DLY(void);
uint8_t SOFT_IIC_Write_Len(const sw_i2c_t *iic, uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
uint8_t SOFT_IIC_Read_Len(const sw_i2c_t *iic, uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif // __SW_I2C_H__