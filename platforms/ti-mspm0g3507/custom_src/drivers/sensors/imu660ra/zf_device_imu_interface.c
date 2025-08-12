/*********************************************************************************************************************
 * 修复后的IMU接口文件 - 正确适配你的软件I2C
 ********************************************************************************************************************/

#include "zf_device_imu_interface.h"
#include "hal_soft_i2c.h"  // 替换为你的软件I2C头文件
#include "log_config.h"
#include "log.h"
#include "delay.h"

#include <assert.h>

// 使用你的软件I2C配置结构体
soft_iic_info_struct  imu_soft_iic_obj = {
	.sclIOMUX = PORTA_SDA2_IOMUX,
	.sclPin = PORTA_SDA2_PIN,
	.sclPort = PORTA_PORT,
	.sdaIOMUX = PORTA_SCL2_IOMUX,
	.sdaPin = PORTA_SCL2_PIN,
	.sdaPort = PORTA_PORT,
	.delay_time = 1,
	.addr = 0x69,
};

// 此处定义 本文件用使用的变量与对象等 这里不允许用户修改
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Part Start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
imu_interface_obj_struct imu_interface_default_obj = {
    IMU_INTERFACE_TYPE,
    imu_delay_ms,
    imu_assert,
    imu_write_8bit_register,
    imu_write_8bit_registers,
    imu_read_8bit_register,
    imu_read_8bit_registers,
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Part   End <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 此处定义 IMU 相关的接口实现 这里不允许用户修改
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Part Start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 延时
// 参数说明     ms              毫秒数
// 返回参数     void
// 使用示例     imu_delay_ms(ms);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void imu_delay_ms(uint32_t ms) {
    delay_ms(ms);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 断言接口定义
// 参数说明     pass            断言判断条件
// 参数说明     *str            断言生效时应该输出的信息
// 参数说明     *file           文件路径信息
// 参数说明     line            报错行数
// 返回参数     uint8_t           ZF_NO_ERROR - 0 - 正常 /  !ZF_NO_ERROR - !0 - 异常
// 使用示例     imu_assert(pass, str, file, line);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8_t imu_assert(uint8_t pass, char *str, char *file, int line) {
//		log_e("%s", str);
    if (!pass) {
        for(;;);
    }
    return (pass ? 0 : 1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 写寄存器 8bit
// 参数说明     addr            IIC 器件地址 非 IIC 器件就填 0
// 参数说明     reg             寄存器地址
// 参数说明     data            数据
// 返回参数     void
// 使用示例     imu_write_8bit_register(addr, reg, data);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void imu_write_8bit_register(uint8_t addr, uint8_t reg, uint8_t data) {
#if (IMU_INTERFACE_SPI == IMU_INTERFACE_TYPE)
    IMU_CS(0);

    DL_SPI_transmitData8(IMU_SPI_INDEX, reg);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));
    DL_SPI_receiveData8(IMU_SPI_INDEX);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));

    DL_SPI_transmitData8(IMU_SPI_INDEX, data);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));
    DL_SPI_receiveData8(IMU_SPI_INDEX);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));

    IMU_CS(1);
#elif (IMU_INTERFACE_SOFT_IIC == IMU_INTERFACE_TYPE)
    soft_iic_write_8bit_register(&imu_soft_iic_obj, reg, data);
#elif (IMU_INTERFACE_IIC == IMU_INTERFACE_TYPE)
    // iic_write_8bit_register(IMU_IIC_INDEX, addr, reg, data);
    HAL_I2C_Mem_Write(&hi2c1, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 0xffff);
#endif
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 写寄存器 8bit
// 参数说明     addr            IIC 器件地址 非 IIC 器件就填 0
// 参数说明     reg             寄存器地址
// 参数说明     data            数据缓冲区
// 参数说明     len             数据长度
// 返回参数     void
// 使用示例     imu_write_8bit_registers(addr, reg, data, len);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void imu_write_8bit_registers(uint8_t addr, uint8_t reg, const uint8_t *data, uint32_t len) {
#if (IMU_INTERFACE_SPI == IMU_INTERFACE_TYPE)
    IMU_CS(0);

    DL_SPI_transmitData8(IMU_SPI_INDEX, reg);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));
    DL_SPI_receiveData8(IMU_SPI_INDEX);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));

    uint32_t send = 0;
    while (len > send) {
        DL_SPI_transmitData8(IMU_SPI_INDEX, *(data + send));
        while (DL_SPI_isBusy(IMU_SPI_INDEX));
        DL_SPI_receiveData8(IMU_SPI_INDEX);
        while (DL_SPI_isBusy(IMU_SPI_INDEX));
        send++;
    }
    IMU_CS(1);
#elif (IMU_INTERFACE_SOFT_IIC == IMU_INTERFACE_TYPE)
    soft_iic_write_8bit_registers(&imu_soft_iic_obj, reg, data, len);
#elif (IMU_INTERFACE_IIC == IMU_INTERFACE_TYPE)
    // iic_write_8bit_registers(IMU_IIC_INDEX, addr, reg, data, len);
    HAL_I2C_Mem_Write(&hi2c1, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, len, 0x3f3f3f3f);
#endif
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 读寄存器 8bit
// 参数说明     addr            IIC 器件地址 非 IIC 器件就填 0
// 参数说明     reg             寄存器地址
// 返回参数     uint8_t           数据
// 使用示例     imu_read_8bit_register(addr, reg);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8_t imu_read_8bit_register(uint8_t addr, uint8_t reg) {
#if (IMU_INTERFACE_SPI == IMU_INTERFACE_TYPE)
    uint8_t data = 0;
    IMU_CS(0);

    DL_SPI_transmitData8(IMU_SPI_INDEX, reg);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));
    DL_SPI_receiveData8(IMU_SPI_INDEX);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));

    DL_SPI_transmitData8(IMU_SPI_INDEX, 0xFF);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));
    data = DL_SPI_receiveData8(IMU_SPI_INDEX);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));

    IMU_CS(1);
    return data;
#elif (IMU_INTERFACE_SOFT_IIC == IMU_INTERFACE_TYPE)
    return soft_iic_read_8bit_register(&imu_soft_iic_obj, reg);
#elif (IMU_INTERFACE_IIC == IMU_INTERFACE_TYPE)
    // return iic_read_8bit_register(IMU_IIC_INDEX, addr, reg);
    uint8_t data = 0;
    HAL_I2C_Mem_Read(&hi2c1, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 0x3f3f3f3f);
    return data;
#endif
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 读数据 8bit
// 参数说明     addr            IIC 器件地址 非 IIC 器件就填 0
// 参数说明     reg             寄存器地址
// 参数说明     data            数据缓冲区
// 参数说明     len             数据长度
// 返回参数     void
// 使用示例     imu_read_8bit_registers(addr, reg, data, len);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void imu_read_8bit_registers(uint8_t addr, uint8_t reg, uint8_t *data, uint32_t len) {
#if (IMU_INTERFACE_SPI == IMU_INTERFACE_TYPE)
    IMU_CS(0);

    DL_SPI_transmitData8(IMU_SPI_INDEX, reg);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));
    DL_SPI_receiveData8(IMU_SPI_INDEX);
    while (DL_SPI_isBusy(IMU_SPI_INDEX));

    uint32_t read = 0;
    while (len > read) {
        DL_SPI_transmitData8(IMU_SPI_INDEX, 0xFF);
        while (DL_SPI_isBusy(IMU_SPI_INDEX));
        *(data + read) = DL_SPI_receiveData8(IMU_SPI_INDEX);
        while (DL_SPI_isBusy(IMU_SPI_INDEX));

        read++;
    }
    IMU_CS(1);
#elif (IMU_INTERFACE_SOFT_IIC == IMU_INTERFACE_TYPE)
    soft_iic_read_8bit_registers(&imu_soft_iic_obj, reg, data, len);
#elif (IMU_INTERFACE_IIC == IMU_INTERFACE_TYPE)
    // iic_read_8bit_registers(IMU_IIC_INDEX, addr, reg, data, len);
    HAL_I2C_Mem_Read(&hi2c1, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 0x3f3f3f3f);
#endif
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 接口注销初始化
// 参数说明     void
// 返回参数     uint8_t           0-成功 1-失败
// 使用示例     imu_interface_deinit();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8_t imu_interface_deinit(void) {
    uint8_t return_state = 1;
    do {
        return_state = 0;
    } while (0);
    return return_state;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IMU 接口初始化 内部调用
// 参数说明     void
// 返回参数     uint8_t           0-成功 1-失败
// 使用示例     imu_interface_init();
// 备注信息     需要在调用前配置好imu_soft_i2c_obj结构体
//-------------------------------------------------------------------------------------------------------------------
uint8_t imu_interface_init(void) {
    uint8_t return_state = 1;
    do {
        return_state = 0;
    } while (0);
    return return_state;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Part   End <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<