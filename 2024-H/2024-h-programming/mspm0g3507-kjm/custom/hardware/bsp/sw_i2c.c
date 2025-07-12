#include "sw_i2c.h"
#include "delay.h"
//#include "log_config.h"
#include "log.h"
#include "ti_msp_dl_config.h"


/**
 * @brief I2C 软件模拟延时函数
 * @note  延时时间根据 CPU 主频和目标 I2C 频率调整，当前设置为约 5us（基于 32MHz CPU 频率）
 */
void SOFT_IIC_DLY(uint16_t time)
{
		delay_us(time);
}

/**
 * @brief 初始化 I2C GPIO
 * @param i2c_cfg I2C 配置结构体指针
 */
 void SOFT_IIC_Init(const sw_i2c_t *i2c_cfg)
{
    if (i2c_cfg == NULL)
    {
        log_e("I2C 配置结构体为空，无法初始化\n");
        return;
    }

    // 配置 SDA 引脚为输入模式并使能上拉电阻
    DL_GPIO_initDigitalInputFeatures(i2c_cfg->sdaIOMUX, DL_GPIO_INVERSION_DISABLE, 
                                    DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, 
                                    DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(i2c_cfg->sdaIOMUX);

    // SCL 和 SDA 都设置为高电平（空闲状态）
    DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin);
    DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin);
    log_i("I2C 初始化完成，SCL 和 SDA 设置为高电平\n");
}

/**
 * @brief 设置 SDA 为输出模式
 * @param i2c_cfg I2C 配置结构体指针
 */
 void SOFT_SDA_OUT(const sw_i2c_t *i2c_cfg)
{
    DL_GPIO_initDigitalOutput(i2c_cfg->sdaIOMUX);
    DL_GPIO_enableOutput(i2c_cfg->sdaPort, i2c_cfg->sdaPin);
}

/**
 * @brief 设置 SDA 为输入模式
 * @param i2c_cfg I2C 配置结构体指针
 */
 void SOFT_SDA_IN(const sw_i2c_t *i2c_cfg)
{
    DL_GPIO_initDigitalInputFeatures(i2c_cfg->sdaIOMUX, DL_GPIO_INVERSION_DISABLE, 
                                    DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, 
                                    DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(i2c_cfg->sdaIOMUX);
}

/**
 * @brief 产生 I2C 起始信号
 * @param i2c_cfg I2C 配置结构体指针
 */
 void SOFT_IIC_Start(const sw_i2c_t *i2c_cfg)
{
    SOFT_SDA_OUT(i2c_cfg); // SDA 设为输出模式
    DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // SDA 高电平
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 高电平
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_clearPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // SDA 拉低，产生起始信号
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低，准备数据传输
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    log_i("I2C 起始信号发送完成\n");
}

/**
 * @brief 产生 I2C 停止信号
 * @param i2c_cfg I2C 配置结构体指针
 */
 void SOFT_IIC_Stop(const sw_i2c_t *i2c_cfg)
{
    SOFT_SDA_OUT(i2c_cfg); // SDA 设为输出模式
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
    DL_GPIO_clearPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // SDA 拉低
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉高
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // SDA 拉高，产生停止信号
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    log_i("I2C 停止信号发送完成\n");
}

/**
 * @brief 等待从机应答信号
 * @param i2c_cfg I2C 配置结构体指针
 * @return 0: 应答成功, 1: 应答失败
 */
 uint8_t SOFT_IIC_Wait_Ack(const sw_i2c_t *i2c_cfg)
{
    uint32_t timeout = 0;
    SOFT_SDA_IN(i2c_cfg); // SDA 设为输入模式
    DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // 释放 SDA 总线
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉高，等待从机应答
    SOFT_IIC_DLY(i2c_cfg->delay_time);

    // 检测 SDA 是否被从机拉低（表示应答）
    while ((DL_GPIO_readPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin) & i2c_cfg->sdaPin) == i2c_cfg->sdaPin)
    {
        timeout++;
        if (timeout > 300) // 超时保护，防止死循环
        {
            log_e("等待 I2C 应答超时\n");
            SOFT_IIC_Stop(i2c_cfg); // 发送停止信号
            return 1; // 应答失败
        }
        SOFT_IIC_DLY(i2c_cfg->delay_time);
    }

    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低，结束检测
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    log_i("I2C 应答信号检测成功\n");
    return 0; // 应答成功
}

/**
 * @brief 产生 ACK 应答信号
 * @param i2c_cfg I2C 配置结构体指针
 */
 void SOFT_IIC_Ack(const sw_i2c_t *i2c_cfg)
{
    SOFT_SDA_OUT(i2c_cfg); // SDA 设为输出模式
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
    DL_GPIO_clearPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // SDA 拉低，表示应答
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉高
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // 释放 SDA
    log_i("I2C ACK 应答信号发送\n");
}

/**
 * @brief 产生 NACK 非应答信号
 * @param i2c_cfg I2C 配置结构体指针
 */
 void SOFT_IIC_NAck(const sw_i2c_t *i2c_cfg)
{
    SOFT_SDA_OUT(i2c_cfg); // SDA 设为输出模式
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
    DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // SDA 拉高，表示非应答
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉高
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
    SOFT_IIC_DLY(i2c_cfg->delay_time);
    log_i("I2C NACK 非应答信号发送\n");
}

/**
 * @brief 发送一个字节数据
 * @param i2c_cfg I2C 配置结构体指针
 * @param txd 要发送的字节数据
 */
 void SOFT_IIC_Send_Byte(const sw_i2c_t *i2c_cfg, uint8_t txd)
{
    uint8_t i;
    SOFT_SDA_OUT(i2c_cfg); // SDA 设为输出模式
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低，准备数据传输

    for (i = 0; i < 8; i++)
    {
        if ((txd & 0x80) >> 7) // 先发送高位
            DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin);
        else
            DL_GPIO_clearPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin);
        txd <<= 1;
        SOFT_IIC_DLY(i2c_cfg->delay_time);
        DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉高，触发数据发送
        SOFT_IIC_DLY(i2c_cfg->delay_time);
        DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
        SOFT_IIC_DLY(i2c_cfg->delay_time);
    }
    log_i("I2C 发送字节: 0x%02X\n", txd);
}

/**
 * @brief 读取一个字节数据
 * @param i2c_cfg I2C 配置结构体指针
 * @param ack 是否发送 ACK (1: 发送 ACK, 0: 发送 NACK)
 * @return 读取到的字节数据
 */
 uint8_t SOFT_IIC_Read_Byte(const sw_i2c_t *i2c_cfg, unsigned char ack)
{
    uint8_t i, receive = 0;
    SOFT_SDA_IN(i2c_cfg); // SDA 设为输入模式
    DL_GPIO_setPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin); // 释放 SDA 总线

    for (i = 0; i < 8; i++)
    {
        DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低
        SOFT_IIC_DLY(i2c_cfg->delay_time);
        DL_GPIO_setPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉高，读取数据
        receive <<= 1;
        if ((DL_GPIO_readPins(i2c_cfg->sdaPort, i2c_cfg->sdaPin) & i2c_cfg->sdaPin) == i2c_cfg->sdaPin)
            receive++; // 读取 SDA 状态
        SOFT_IIC_DLY(i2c_cfg->delay_time);
    }
    DL_GPIO_clearPins(i2c_cfg->sclPort, i2c_cfg->sclPin); // SCL 拉低

    if (!ack)
        SOFT_IIC_NAck(i2c_cfg); // 发送 NACK
    else
        SOFT_IIC_Ack(i2c_cfg); // 发送 ACK

    log_i("I2C 读取字节: 0x%02X\n", receive);
    return receive;
}

/**
 * @brief 向指定设备写入指定寄存器的多字节数据
 * @param iic I2C 配置结构体指针
 * @param addr 设备地址
 * @param reg 寄存器地址
 * @param len 数据长度
 * @param buf 数据缓冲区
 * @return 操作结果 (IIC_SUCCESS: 成功, 其他: 错误码)
 */
uint8_t SOFT_IIC_Write_Len(const sw_i2c_t *iic, uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    if (iic == NULL || buf == NULL || len == 0)
    {
        log_e("I2C 写入参数无效\n");
        return IIC_ERROR_INVALID_PARAM;
    }

    uint8_t i;
    SOFT_IIC_Start(iic);
    SOFT_IIC_Send_Byte(iic, addr << 1); // 发送设备地址，写模式
    log_i("发送设备地址（写模式）: 0x%02X\n", addr << 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1) // 等待应答，1 表示失败
    {
        log_e("设备地址应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }

    SOFT_IIC_Send_Byte(iic, reg); // 发送寄存器地址
    log_i("发送寄存器地址: 0x%02X\n", reg);
    if (SOFT_IIC_Wait_Ack(iic) == 1) // 等待应答，1 表示失败
    {
        log_e("寄存器地址应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }

    for (i = 0; i < len; i++)
    {
        SOFT_IIC_Send_Byte(iic, buf[i]); // 发送数据
        log_i("发送数据[%d]: 0x%02X\n", i, buf[i]);
        if (SOFT_IIC_Wait_Ack(iic) == 1) // 等待应答，1 表示失败
        {
            log_e("数据发送失败，索引: %d\n", i);
            SOFT_IIC_Stop(iic);
            return IIC_ERROR_ACK_FAIL;
        }
    }

    SOFT_IIC_Stop(iic);
    log_i("I2C 写入操作完成\n");
    return IIC_SUCCESS;
}

/**
 * @brief 从指定设备读取指定寄存器的多字节数据
 * @param iic I2C 配置结构体指针
 * @param addr 设备地址
 * @param reg 寄存器地址
 * @param len 数据长度
 * @param buf 数据缓冲区
 * @return 操作结果 (IIC_SUCCESS: 成功, 其他: 错误码)
 */
uint8_t SOFT_IIC_Read_Len(const sw_i2c_t *iic, uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    if (iic == NULL || buf == NULL || len == 0)
    {
        log_e("I2C 读取参数无效\n");
        return IIC_ERROR_INVALID_PARAM;
    }

    // 第一次 START，发送设备地址（写模式）
    SOFT_IIC_Start(iic);
    SOFT_IIC_Send_Byte(iic, addr << 1); // 发送设备地址，写模式
    log_i("发送设备地址（写模式）: 0x%02X\n", addr << 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1) // 等待应答，1 表示失败
    {
        log_e("设备地址应答失败（写模式）\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }

    // 发送寄存器地址
    SOFT_IIC_Send_Byte(iic, reg); // 发送寄存器地址
    log_i("发送寄存器地址: 0x%02X\n", reg);
    if (SOFT_IIC_Wait_Ack(iic) == 1) // 等待应答，1 表示失败
    {
        log_e("寄存器地址应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }

    // 第二次 START，发送设备地址（读模式）
    SOFT_IIC_Start(iic);
    SOFT_IIC_Send_Byte(iic, (addr << 1) | 1); // 发送设备地址，读模式
    log_i("发送设备地址（读模式）: 0x%02X\n", (addr << 1) | 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1) // 等待应答，1 表示失败
    {
        log_e("设备地址应答失败（读模式）\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }

    // 读取数据
    while (len)
    {
        if (len == 1)
        {
            *buf = SOFT_IIC_Read_Byte(iic, 0); // 最后一个字节发送 NACK
        }
        else
        {
            *buf = SOFT_IIC_Read_Byte(iic, 1); // 其他字节发送 ACK
        }
        len--;
        buf++;
    }

    SOFT_IIC_Stop(iic);
    log_i("I2C 读取操作完成\n");
    return IIC_SUCCESS;
}


/**
 * @brief 向指定设备写入指定16位寄存器地址的多字节数据
 */
uint8_t SOFT_IIC_Write_Len_16bit(const sw_i2c_t *iic, uint8_t addr, uint16_t reg, uint8_t len, uint8_t *buf)
{
    if (iic == NULL || (len > 0 && buf == NULL))
    {
        log_e("I2C 16位写入参数无效\n");
        return IIC_ERROR_INVALID_PARAM;
    }
    
    uint8_t i;
    SOFT_IIC_Start(iic);
    
    // 发送设备地址（写模式）
    SOFT_IIC_Send_Byte(iic, addr << 1);
    log_i("发送设备地址（写模式）: 0x%02X\n", addr << 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("设备地址应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    // 发送16位寄存器地址（高字节在前）
    SOFT_IIC_Send_Byte(iic, (reg >> 8) & 0xFF);
    log_i("发送寄存器高字节: 0x%02X\n", (reg >> 8) & 0xFF);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("寄存器高字节应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    SOFT_IIC_Send_Byte(iic, reg & 0xFF);
    log_i("发送寄存器低字节: 0x%02X\n", reg & 0xFF);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("寄存器低字节应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    // 发送数据
    for (i = 0; i < len; i++)
    {
        SOFT_IIC_Send_Byte(iic, buf[i]);
        log_i("发送数据[%d]: 0x%02X\n", i, buf[i]);
        if (SOFT_IIC_Wait_Ack(iic) == 1)
        {
            log_e("数据发送失败，索引: %d\n", i);
            SOFT_IIC_Stop(iic);
            return IIC_ERROR_ACK_FAIL;
        }
    }
    
    SOFT_IIC_Stop(iic);
    log_i("I2C 16位写入操作完成，寄存器: 0x%04X, 长度: %d\n", reg, len);
    return IIC_SUCCESS;
}

/**
 * @brief 从指定设备读取指定16位寄存器地址的多字节数据
 */
uint8_t SOFT_IIC_Read_Len_16bit(const sw_i2c_t *iic, uint8_t addr, uint16_t reg, uint8_t len, uint8_t *buf)
{
    if (iic == NULL || buf == NULL || len == 0)
    {
        log_e("I2C 16位读取参数无效\n");
        return IIC_ERROR_INVALID_PARAM;
    }
    
    // 第一阶段：写入16位寄存器地址
    SOFT_IIC_Start(iic);
    SOFT_IIC_Send_Byte(iic, addr << 1); // 写模式
    log_i("发送设备地址（写模式）: 0x%02X\n", addr << 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("设备地址应答失败（写模式）\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    // 发送16位寄存器地址（高字节在前）
    SOFT_IIC_Send_Byte(iic, (reg >> 8) & 0xFF);
    log_i("发送寄存器高字节: 0x%02X\n", (reg >> 8) & 0xFF);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("寄存器高字节应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    SOFT_IIC_Send_Byte(iic, reg & 0xFF);
    log_i("发送寄存器低字节: 0x%02X\n", reg & 0xFF);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("寄存器低字节应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    // 第二阶段：重新开始，切换到读模式
    SOFT_IIC_Start(iic);
    SOFT_IIC_Send_Byte(iic, (addr << 1) | 1); // 读模式
    log_i("发送设备地址（读模式）: 0x%02X\n", (addr << 1) | 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("设备地址应答失败（读模式）\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    // 读取数据
    while (len)
    {
        if (len == 1)
        {
            *buf = SOFT_IIC_Read_Byte(iic, 0); // 最后一个字节发送 NACK
            log_i("读取最后字节: 0x%02X\n", *buf);
        }
        else
        {
            *buf = SOFT_IIC_Read_Byte(iic, 1); // 其他字节发送 ACK
            log_i("读取字节: 0x%02X\n", *buf);
        }
        len--;
        buf++;
    }
    
    SOFT_IIC_Stop(iic);
    log_i("I2C 16位读取操作完成，寄存器: 0x%04X\n", reg);
    return IIC_SUCCESS;
}

/**
 * @brief 只向设备写入16位寄存器地址（用于后续读取操作）
 */
uint8_t SOFT_IIC_Write_Reg_16bit(const sw_i2c_t *iic, uint8_t addr, uint16_t reg)
{
    if (iic == NULL)
    {
        log_e("I2C 寄存器写入参数无效\n");
        return IIC_ERROR_INVALID_PARAM;
    }
    
    SOFT_IIC_Start(iic);
    
    // 发送设备地址（写模式）
    SOFT_IIC_Send_Byte(iic, addr << 1);
    log_i("发送设备地址（写模式）: 0x%02X\n", addr << 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("设备地址应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    // 发送16位寄存器地址（高字节在前）
    SOFT_IIC_Send_Byte(iic, (reg >> 8) & 0xFF);
    log_i("发送寄存器高字节: 0x%02X\n", (reg >> 8) & 0xFF);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("寄存器高字节应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    SOFT_IIC_Send_Byte(iic, reg & 0xFF);
    log_i("发送寄存器低字节: 0x%02X\n", reg & 0xFF);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("寄存器低字节应答失败\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    SOFT_IIC_Stop(iic);
    log_i("I2C 寄存器地址写入完成: 0x%04X\n", reg);
    return IIC_SUCCESS;
}

/**
 * @brief 从当前寄存器位置读取数据（不发送寄存器地址）
 */
uint8_t SOFT_IIC_Read_Continue(const sw_i2c_t *iic, uint8_t addr, uint8_t len, uint8_t *buf)
{
    if (iic == NULL || buf == NULL || len == 0)
    {
        log_e("I2C 连续读取参数无效\n");
        return IIC_ERROR_INVALID_PARAM;
    }
    
    // 直接发送设备地址（读模式）
    SOFT_IIC_Start(iic);
    SOFT_IIC_Send_Byte(iic, (addr << 1) | 1); // 读模式
    log_i("发送设备地址（读模式）: 0x%02X\n", (addr << 1) | 1);
    if (SOFT_IIC_Wait_Ack(iic) == 1)
    {
        log_e("设备地址应答失败（读模式）\n");
        SOFT_IIC_Stop(iic);
        return IIC_ERROR_ACK_FAIL;
    }
    
    // 读取数据
    while (len)
    {
        if (len == 1)
        {
            *buf = SOFT_IIC_Read_Byte(iic, 0); // 最后一个字节发送 NACK
            log_i("读取最后字节: 0x%02X\n", *buf);
        }
        else
        {
            *buf = SOFT_IIC_Read_Byte(iic, 1); // 其他字节发送 ACK
            log_i("读取字节: 0x%02X\n", *buf);
        }
        len--;
        buf++;
    }
    
    SOFT_IIC_Stop(iic);
    log_i("I2C 连续读取操作完成\n");
    return IIC_SUCCESS;
}