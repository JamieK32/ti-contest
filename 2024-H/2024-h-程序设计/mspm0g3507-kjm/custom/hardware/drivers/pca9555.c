#include "pca9555.h"
//#include "log_config.h"
#include "log.h"


/**
 * @brief 通过 I2C 方式读取 12 路灰度传感器数据（低 12 位有效）
 * @param slave_num PCA9555 的设备地址（已左移 1 位，如 0x20<<1）
 * @return 16 位读取值（低 12 位有效）
 */
/**
 * @brief 通过 I2C 方式读取 12 路灰度传感器数据（低 12 位有效）
 * @param slave_num PCA9555 的设备地址（已左移 1 位，如 0x20<<1）
 * @return 16 位读取值（低 12 位有效）
 */
uint16_t pca9555_read_bit12(sw_i2c_t* i2c, uint8_t slave_num)
{
    uint8_t data[2]; // 存储读取的两个字节，低字节在前，高字节在后
    uint16_t bit12;

    // 检查 I2C 实例是否有效
    if (i2c == NULL) {
        log_e("Invalid I2C instance in pca9555_read_bit12\n");
        return 0;
    }

    // 使用 SoftWareI2C_Read_Len 读取两个字节的数据
    uint8_t result = SOFT_IIC_Read_Len(i2c, slave_num, INPUT_PORT_REGISTER0, 2, data);
    if (result != IIC_SUCCESS) {
        log_e("Failed to read from PCA9555, error code: %d\n", result);
        return 0;
    }

    // 组合两个字节为 16 位值，并提取低 12 位
    bit12 = ((uint16_t)data[1] << 8 | data[0]) & 0x0FFF;
    log_i("PCA9555 read result (16-bit, 12-bit masked): 0x%04X\n", bit12);

    return bit12;
}