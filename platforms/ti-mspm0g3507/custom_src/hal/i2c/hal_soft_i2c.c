/*********************************************************************************************************************
 * MM32F527X-E9P Opensource Library 即（MM32F527X-E9P 开源库）是一个基于官方 SDK 接口的第三方开源库
 * Copyright (c) 2022 SEEKFREE 逐飞科技
 *
 * 本文件是 MM32F527X-E9P 开源库的一部分
 *
 * MM32F527X-E9P 开源库 是免费软件
 * 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
 * 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
 *
 * 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
 * 甚至没有隐含的适销性或适合特定用途的保证
 * 更多细节请参见 GPL
 *
 * 您应该在收到本开源库的同时收到一份 GPL 的副本
 * 如果没有，请参阅<https://www.gnu.org/licenses/>
 *
 * 额外注明：
 * 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
 * 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
 * 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
 * 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
 *
 * 文件名称          zf_driver_soft_iic
 * 公司名称          成都逐飞科技有限公司
 * 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
 * 开发环境          MDK 5.37
 * 适用平台          MM32F527X_E9P
 * 店铺链接          https://seekfree.taobao.com/
 *
 * 修改记录
 * 日期              作者                备注
 * 2022-08-10        Teternal            first version
 ********************************************************************************************************************/

#include "hal_soft_i2c.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define soft_iic_delay(x) for (volatile uint32_t i = x; i--;)
#define gpio_high(port, pin) DL_GPIO_setPins(port, pin)
#define gpio_low(port, pin) DL_GPIO_clearPins(port, pin)
#define gpio_set_input(iomux, port, pin)                    \
    do {                                                    \
        DL_GPIO_initDigitalInputFeatures(iomux,             \
                                        DL_GPIO_INVERSION_DISABLE, \
                                        DL_GPIO_RESISTOR_PULL_UP,  \
                                        DL_GPIO_HYSTERESIS_DISABLE, \
                                        DL_GPIO_WAKEUP_DISABLE);    \
        DL_GPIO_enableHiZ(iomux);                           \
    } while (0)

#define gpio_set_output(iomux, port, pin)                   \
    do {                                                    \
        DL_GPIO_initDigitalOutput(iomux);                   \
        DL_GPIO_enableOutput(port, pin);                    \
    } while (0)


#define gpio_get_level(port, pin) ((DL_GPIO_readPins(port, pin) & pin) == pin)



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC START 信号（修复版）
//-------------------------------------------------------------------------------------------------------------------
static void soft_iic_start(soft_iic_info_struct *soft_iic_obj) {
    // 确保SDA为输出模式
    gpio_set_output(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
    
    gpio_high(soft_iic_obj->sclPort, soft_iic_obj->sclPin);  // SCL 高电平
    gpio_high(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);  // SDA 高电平

    soft_iic_delay(soft_iic_obj->delay_time);
    gpio_low(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);   // SDA 先拉低
    soft_iic_delay(soft_iic_obj->delay_time);
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 再拉低
    soft_iic_delay(soft_iic_obj->delay_time);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC STOP 信号（修复版）
//-------------------------------------------------------------------------------------------------------------------
static void soft_iic_stop(soft_iic_info_struct *soft_iic_obj) {
    // 确保SDA为输出模式
    gpio_set_output(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
    
    gpio_low(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);   // SDA 低电平
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 低电平

    soft_iic_delay(soft_iic_obj->delay_time);
    gpio_high(soft_iic_obj->sclPort, soft_iic_obj->sclPin);  // SCL 先拉高
    soft_iic_delay(soft_iic_obj->delay_time);
    gpio_high(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);  // SDA 再拉高
    soft_iic_delay(soft_iic_obj->delay_time);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 发送 ACK/NACK 信号（修复版）
//-------------------------------------------------------------------------------------------------------------------
static void soft_iic_send_ack(soft_iic_info_struct *soft_iic_obj, uint8_t ack) {
    // 确保SDA为输出模式
    gpio_set_output(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
    
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 低电平

    if (ack) {
        gpio_high(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);  // SDA 拉高 (NACK)
    } else {
        gpio_low(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);   // SDA 拉低 (ACK)
    }

    soft_iic_delay(soft_iic_obj->delay_time);
    gpio_high(soft_iic_obj->sclPort, soft_iic_obj->sclPin);  // SCL 拉高
    soft_iic_delay(soft_iic_obj->delay_time);
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 拉低
    gpio_high(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);  // SDA 拉高
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 获取 ACK/NACK 信号（修复版）
//-------------------------------------------------------------------------------------------------------------------
static uint8_t soft_iic_wait_ack(soft_iic_info_struct *soft_iic_obj) {
    uint8_t temp = 0;
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 低电平
    gpio_high(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);  // SDA 高电平 释放 SDA

    // SDA 设置为输入模式
    gpio_set_input(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
    soft_iic_delay(soft_iic_obj->delay_time);

    gpio_high(soft_iic_obj->sclPort, soft_iic_obj->sclPin);  // SCL 高电平
    soft_iic_delay(soft_iic_obj->delay_time);

    if (gpio_get_level(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin)) {
        temp = 1;  // NACK
    }
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 低电平

    // SDA 设置为输出模式
    gpio_set_output(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
    soft_iic_delay(soft_iic_obj->delay_time);

    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 发送 8bit 数据（修复版）
//-------------------------------------------------------------------------------------------------------------------
static uint8_t soft_iic_send_data(soft_iic_info_struct *soft_iic_obj, const uint8_t data) {
    uint8_t temp = 0x80;

    // 确保SDA为输出模式
    gpio_set_output(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);

    while (temp) {
        // 设置数据位
        if (data & temp) {
            gpio_high(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
        } else {
            gpio_low(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
        }
        temp >>= 1;

        soft_iic_delay(soft_iic_obj->delay_time / 2);
        gpio_high(soft_iic_obj->sclPort, soft_iic_obj->sclPin);  // SCL 拉高
        soft_iic_delay(soft_iic_obj->delay_time);
        gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 拉低
        soft_iic_delay(soft_iic_obj->delay_time / 2);
    }
    
    // 等待ACK，返回值：0=ACK收到，1=NACK或无应答
    return soft_iic_wait_ack(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 读取 8bit 数据（修复版）
//-------------------------------------------------------------------------------------------------------------------
static uint8_t soft_iic_read_data(soft_iic_info_struct *soft_iic_obj, uint8_t ack) {
    uint8_t data = 0x00;
    uint8_t temp = 8;
    
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 低电平
    soft_iic_delay(soft_iic_obj->delay_time);
    gpio_high(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);  // SDA 高电平 释放 SDA
    
    // SDA 设置为输入模式
    gpio_set_input(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);

    while (temp--) {
        gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 拉低
        soft_iic_delay(soft_iic_obj->delay_time);
        gpio_high(soft_iic_obj->sclPort, soft_iic_obj->sclPin);  // SCL 拉高
        soft_iic_delay(soft_iic_obj->delay_time);
        data = ((data << 1) | gpio_get_level(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin));
    }
    
    gpio_low(soft_iic_obj->sclPort, soft_iic_obj->sclPin);   // SCL 低电平
    
    // SDA 设置为输出模式
    gpio_set_output(soft_iic_obj->sdaIOMUX, soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);

    soft_iic_delay(soft_iic_obj->delay_time);
    soft_iic_send_ack(soft_iic_obj, ack);
    return data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口写 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     data            要写入的数据
// 返回参数     void
// 使用示例     soft_iic_write_8bit_register(soft_iic_obj, 0x01);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit(soft_iic_info_struct *soft_iic_obj, const uint8_t data) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, data);
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口写 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     *data           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void
// 使用示例     soft_iic_write_8bit_array(soft_iic_obj, data, 6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit_array(soft_iic_info_struct *soft_iic_obj, const uint8_t *data, uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while (len--) {
        soft_iic_send_data(soft_iic_obj, *data++);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口器写 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     data            要写入的数据
// 返回参数     void
// 使用示例     soft_iic_write_16bit(soft_iic_obj, 0x0101);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit(soft_iic_info_struct *soft_iic_obj, const uint16_t data) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8_t)((data & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8_t)(data & 0x00FF));
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口写 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     *data           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void
// 使用示例     soft_iic_write_16bit_array(soft_iic_obj, data, 6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit_array(soft_iic_info_struct *soft_iic_obj, const uint16_t *data, uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while (len--) {
        soft_iic_send_data(soft_iic_obj, (uint8_t)((*data & 0xFF00) >> 8));
        soft_iic_send_data(soft_iic_obj, (uint8_t)(*data++ & 0x00FF));
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     data            要写入的数据
// 返回参数     void
// 使用示例     soft_iic_write_8bit_register(soft_iic_obj, 0x01, 0x01);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, const uint8_t data) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_send_data(soft_iic_obj, data);
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *data           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void
// 使用示例     soft_iic_write_8bit_registers(soft_iic_obj, 0x01, data, 6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit_registers(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, const uint8_t *data,
                                   uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    while (len--) {
        soft_iic_send_data(soft_iic_obj, *data++);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     data            要写入的数据
// 返回参数     void
// 使用示例     soft_iic_write_16bit_register(soft_iic_obj, 0x0101, 0x0101);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit_register(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name,
                                   const uint16_t data) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8_t)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8_t)(register_name & 0x00FF));
    soft_iic_send_data(soft_iic_obj, (uint8_t)((data & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8_t)(data & 0x00FF));
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *data           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void
// 使用示例     soft_iic_write_16bit_registers(soft_iic_obj, 0x0101, data, 6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit_registers(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name,
                                    const uint16_t *data, uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8_t)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8_t)(register_name & 0x00FF));
    while (len--) {
        soft_iic_send_data(soft_iic_obj, (uint8_t)((*data & 0xFF00) >> 8));
        soft_iic_send_data(soft_iic_obj, (uint8_t)(*data++ & 0x00FF));
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口读取 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     soft_iic_read_8bit(soft_iic_obj);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8_t soft_iic_read_8bit(soft_iic_info_struct *soft_iic_obj) {
    uint8_t temp = 0;
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 1);
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *data           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void
// 使用示例     soft_iic_read_8bit_array(soft_iic_obj, data, 8);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_8bit_array(soft_iic_info_struct *soft_iic_obj, uint8_t *data, uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while (len--) {
        *data++ = soft_iic_read_data(soft_iic_obj, len == 0);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口读取 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint16          返回读取的 16bit 数据
// 使用示例     soft_iic_read_16bit(soft_iic_obj);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16_t soft_iic_read_16bit(soft_iic_info_struct *soft_iic_obj) {
    uint16_t temp = 0;
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 0);
    temp = ((temp << 8) | soft_iic_read_data(soft_iic_obj, 1));
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口读取 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     *data           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void
// 使用示例     soft_iic_read_16bit_array(soft_iic_obj, data, 8);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_16bit_array(soft_iic_info_struct *soft_iic_obj, uint16_t *data, uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while (len--) {
        *data = soft_iic_read_data(soft_iic_obj, 0);
        *data = ((*data << 8) | soft_iic_read_data(soft_iic_obj, 0 == len));
        data++;
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     soft_iic_read_8bit_register(soft_iic_obj, 0x01);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8_t soft_iic_read_8bit_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name) {
    uint8_t temp = 0;
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 1);
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *data           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void
// 使用示例     soft_iic_read_8bit_registers(soft_iic_obj, 0x01, data, 8);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_8bit_registers(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, uint8_t *data,
                                  uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while (len--) {
        *data++ = soft_iic_read_data(soft_iic_obj, len == 0);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint16          返回读取的 16bit 数据
// 使用示例     soft_iic_read_16bit_register(soft_iic_obj, 0x0101);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16_t soft_iic_read_16bit_register(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name) {
    uint16_t temp = 0;
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8_t)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8_t)(register_name & 0x00FF));
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 0);
    temp = ((temp << 8) | soft_iic_read_data(soft_iic_obj, 1));
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *data           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void
// 使用示例     soft_iic_read_16bit_registers(soft_iic_obj, 0x0101, data, 8);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_16bit_registers(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name, uint16_t *data,
                                   uint32_t len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8_t)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8_t)(register_name & 0x00FF));
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while (len--) {
        *data = soft_iic_read_data(soft_iic_obj, 0);
        *data = ((*data << 8) | soft_iic_read_data(soft_iic_obj, 0 == len));
        data++;
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口传输 8bit 数组 先写后读取
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     *write_data     发送数据存放缓冲区
// 参数说明     write_len       发送缓冲区长度
// 参数说明     *read_data      读取数据存放缓冲区
// 参数说明     read_len        读取缓冲区长度
// 返回参数     void
// 使用示例     iic_transfer_8bit_array(IIC_1, addr, data, 64, data, 64);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_transfer_8bit_array(soft_iic_info_struct *soft_iic_obj, const uint8_t *write_data, uint32_t write_len,
                                  uint8_t *read_data, uint32_t read_len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while (write_len--) {
        soft_iic_send_data(soft_iic_obj, *write_data++);
    }
    if (read_len) {
        soft_iic_start(soft_iic_obj);
        soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
        while (read_len--) {
            *read_data++ = soft_iic_read_data(soft_iic_obj, 0 == read_len);
        }
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口传输 16bit 数组 先写后读取
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     *write_data     发送数据存放缓冲区
// 参数说明     write_len       发送缓冲区长度
// 参数说明     *read_data      读取数据存放缓冲区
// 参数说明     read_len        读取缓冲区长度
// 返回参数     void
// 使用示例     iic_transfer_16bit_array(IIC_1, addr, data, 64, data, 64);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_transfer_16bit_array(soft_iic_info_struct *soft_iic_obj, const uint16_t *write_data, uint32_t write_len,
                                   uint16_t *read_data, uint32_t read_len) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while (write_len--) {
        soft_iic_send_data(soft_iic_obj, (uint8_t)((*write_data & 0xFF00) >> 8));
        soft_iic_send_data(soft_iic_obj, (uint8_t)(*write_data++ & 0x00FF));
    }
    if (read_len) {
        soft_iic_start(soft_iic_obj);
        soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
        while (read_len--) {
            *read_data = soft_iic_read_data(soft_iic_obj, 0);
            *read_data = ((*read_data << 8) | soft_iic_read_data(soft_iic_obj, 0 == read_len));
            read_data++;
        }
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口 SCCB 模式向传感器寄存器写 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     data            要写入的数据
// 返回参数     void
// 使用示例     soft_iic_sccb_write_register(soft_iic_obj, 0x01, 0x01);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_sccb_write_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, uint8_t data) {
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_send_data(soft_iic_obj, data);
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口 SCCB 模式从传感器寄存器读取 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 zf_driver_soft_iic.h 里的格式看看
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     soft_iic_sccb_read_register(soft_iic_obj, 0x01);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8_t soft_iic_sccb_read_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name) {
    uint8_t temp = 0;
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_stop(soft_iic_obj);

    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 1);
    soft_iic_stop(soft_iic_obj);
    return temp;
}
// ====== 需要在软件I2C库中添加的辅助函数 ======

/**
 * @brief 向16位寄存器地址写入数据
 * @param soft_iic_obj I2C对象
 * @param reg_addr 16位寄存器地址
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return 0表示成功，其他表示失败
 */
uint8_t soft_iic_write_16bit_register_with_data(soft_iic_info_struct *soft_iic_obj, uint16_t reg_addr, const uint8_t *data, uint32_t len)
{
    if (soft_iic_obj == NULL || (len > 0 && data == NULL)) {
        return 1; // 参数错误
    }
    
    soft_iic_start(soft_iic_obj);
    
    // 发送设备地址（写模式）
    if (soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1)) {
        soft_iic_stop(soft_iic_obj);
        return 2; // 设备地址无应答
    }
    
    // 发送16位寄存器地址（高字节在前）
    if (soft_iic_send_data(soft_iic_obj, (reg_addr >> 8) & 0xFF)) {
        soft_iic_stop(soft_iic_obj);
        return 3; // 寄存器高字节无应答
    }
    
    if (soft_iic_send_data(soft_iic_obj, reg_addr & 0xFF)) {
        soft_iic_stop(soft_iic_obj);
        return 4; // 寄存器低字节无应答
    }
    
    // 发送数据
    for (uint32_t i = 0; i < len; i++) {
        if (soft_iic_send_data(soft_iic_obj, data[i])) {
            soft_iic_stop(soft_iic_obj);
            return 5; // 数据无应答
        }
    }
    
    soft_iic_stop(soft_iic_obj);
    return 0; // 成功
}

/**
 * @brief 只写入16位寄存器地址（用于后续读取）
 * @param soft_iic_obj I2C对象
 * @param reg_addr 16位寄存器地址
 * @return 0表示成功，其他表示失败
 */
uint8_t soft_iic_write_16bit_register_addr_only(soft_iic_info_struct *soft_iic_obj, uint16_t reg_addr)
{
    if (soft_iic_obj == NULL) {
        return 1; // 参数错误
    }
    
    soft_iic_start(soft_iic_obj);
    
    // 发送设备地址（写模式）
    if (soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1)) {
        soft_iic_stop(soft_iic_obj);
        return 2; // 设备地址无应答
    }
    
    // 发送16位寄存器地址（高字节在前）
    if (soft_iic_send_data(soft_iic_obj, (reg_addr >> 8) & 0xFF)) {
        soft_iic_stop(soft_iic_obj);
        return 3; // 寄存器高字节无应答
    }
    
    if (soft_iic_send_data(soft_iic_obj, reg_addr & 0xFF)) {
        soft_iic_stop(soft_iic_obj);
        return 4; // 寄存器低字节无应答
    }
    
    soft_iic_stop(soft_iic_obj);
    return 0; // 成功
}

/**
 * @brief 连续读取数据（不发送寄存器地址）
 * @param soft_iic_obj I2C对象
 * @param len 数据长度
 * @param data 数据缓冲区
 * @return 0表示成功，其他表示失败
 */
uint8_t soft_iic_read_continue(soft_iic_info_struct *soft_iic_obj, uint32_t len, uint8_t *data)
{
    if (soft_iic_obj == NULL || data == NULL || len == 0) {
        return 1; // 参数错误
    }
    
    soft_iic_start(soft_iic_obj);
    
    // 发送设备地址（读模式）
    if (soft_iic_send_data(soft_iic_obj, (soft_iic_obj->addr << 1) | 0x01)) {
        soft_iic_stop(soft_iic_obj);
        return 2; // 设备地址无应答
    }
    
    // 读取数据
    for (uint32_t i = 0; i < len; i++) {
        data[i] = soft_iic_read_data(soft_iic_obj, (i == len - 1) ? 1 : 0); // 最后一个字节发送NACK
    }
    
    soft_iic_stop(soft_iic_obj);
    return 0; // 成功
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口初始化 默认 MASTER 模式 不提供 SLAVE 模式
// 参数说明     *soft_iic_obj   软件 IIC 指定信息存放结构体的指针
// 参数说明     addr            软件 IIC 地址 这里需要注意 标准七位地址 最高位忽略 写入时请务必确认无误
// 参数说明     delay           软件 IIC 延时 就是时钟高电平时间 越短 IIC 速率越高
// 参数说明     scl_pin         软件 IIC 时钟引脚 参照 zf_driver_gpio.h 内 gpio_pin_enum 枚举体定义
// 参数说明     sda_pin         软件 IIC 数据引脚 参照 zf_driver_gpio.h 内 gpio_pin_enum 枚举体定义
// 返回参数     void
// 使用示例     soft_iic_init(&soft_iic_obj, addr, 100, B6, B7);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_init(soft_iic_info_struct *soft_iic_obj) {
   // 初始化SCL引脚为输出模式
    DL_GPIO_initDigitalOutput(soft_iic_obj->sclIOMUX);
    DL_GPIO_enableOutput(soft_iic_obj->sclPort, soft_iic_obj->sclPin);
    DL_GPIO_setPins(soft_iic_obj->sclPort, soft_iic_obj->sclPin);  // SCL初始为高电平
    
    // 初始化SDA引脚为输出模式
    DL_GPIO_initDigitalOutput(soft_iic_obj->sdaIOMUX);
    DL_GPIO_enableOutput(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);
    DL_GPIO_setPins(soft_iic_obj->sdaPort, soft_iic_obj->sdaPin);  // SDA初始为高电平
    
}