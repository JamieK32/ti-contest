#include "pca9555.h"
//#include "log_config.h"
#include "log.h"


/**
 * @brief 通过 I2C 方式读取 12 路灰度传感器数据（低 12 位有效）
 * @param i2c I2C配置结构体指针
 * @param slave_addr PCA9555的7位设备地址（如0x20，不需要左移）
 * @return 16 位读取值（低 12 位有效），失败返回0xFFFF
 */
uint16_t pca9555_read_bit12(soft_iic_info_struct *i2c, uint8_t slave_addr)
{
    uint8_t data[2] = {0}; // 存储读取的两个字节
    uint16_t bit12;
    
    // 检查 I2C 实例是否有效
    if (i2c == NULL) {
        log_e("Invalid I2C instance in pca9555_read_bit12\n");
        return 0xFFFF;
    }
    
    // 临时保存原地址，设置PCA9555地址
    uint8_t original_addr = i2c->addr;
    i2c->addr = slave_addr;
    
    log_d("PCA9555 读取开始: 地址=0x%02X, 寄存器=0x%02X\n", slave_addr, INPUT_PORT_REGISTER0);
    
    // 使用软件I2C读取两个字节的数据（从INPUT_PORT_REGISTER0开始）
    soft_iic_read_8bit_registers(i2c, INPUT_PORT_REGISTER0, data, 2);
    
    // 恢复原地址
    i2c->addr = original_addr;
    
    // 组合两个字节为 16 位值
    // PCA9555: data[0] = PORT0 (P07-P00), data[1] = PORT1 (P17-P10)
    // 低12位：P11-P00
    bit12 = ((uint16_t)data[1] << 8 | data[0]) & 0x0FFF;
    
    log_d("PCA9555 读取成功: 原始数据=0x%02X%02X, 12位数据=0x%03X\n", 
          data[1], data[0], bit12);
    
    return bit12;
}