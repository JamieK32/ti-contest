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

#ifndef _zf_driver_soft_iic_h_
#define _zf_driver_soft_iic_h_

#include "ti_msp_dl_config.h"

typedef struct {
    uint32_t sclIOMUX;          // SCL引脚IOMUX配置
    uint32_t sclPin;            // SCL引脚编号
    GPIO_Regs *sclPort;         // SCL端口
    uint32_t sdaIOMUX;          // SDA引脚IOMUX配置
    uint32_t sdaPin;            // SDA引脚编号
    GPIO_Regs *sdaPort;         // SDA端口
    uint32_t delay_time;        // I2C时钟延时
    uint8_t addr;               // 设备地址
} soft_iic_info_struct;


void soft_iic_write_8bit(soft_iic_info_struct *soft_iic_obj, const uint8_t data);
void soft_iic_write_8bit_array(soft_iic_info_struct *soft_iic_obj, const uint8_t *data, uint32_t len);

void soft_iic_write_16bit(soft_iic_info_struct *soft_iic_obj, const uint16_t data);
void soft_iic_write_16bit_array(soft_iic_info_struct *soft_iic_obj, const uint16_t *data, uint32_t len);

void soft_iic_write_8bit_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, const uint8_t data);
void soft_iic_write_8bit_registers(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, const uint8_t *data,
                                   uint32_t len);

void soft_iic_write_16bit_register(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name,
                                   const uint16_t data);
void soft_iic_write_16bit_registers(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name,
                                    const uint16_t *data, uint32_t len);

uint8_t soft_iic_read_8bit(soft_iic_info_struct *soft_iic_obj);
void soft_iic_read_8bit_array(soft_iic_info_struct *soft_iic_obj, uint8_t *data, uint32_t len);

uint16_t soft_iic_read_16bit(soft_iic_info_struct *soft_iic_obj);
void soft_iic_read_16bit_array(soft_iic_info_struct *soft_iic_obj, uint16_t *data, uint32_t len);

uint8_t soft_iic_read_8bit_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name);
void soft_iic_read_8bit_registers(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, uint8_t *data,
                                  uint32_t len);

uint16_t soft_iic_read_16bit_register(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name);
void soft_iic_read_16bit_registers(soft_iic_info_struct *soft_iic_obj, const uint16_t register_name, uint16_t *data,
                                   uint32_t len);

void soft_iic_transfer_8bit_array(soft_iic_info_struct *soft_iic_obj, const uint8_t *write_data, uint32_t write_len,
                                  uint8_t *read_data, uint32_t read_len);
void soft_iic_transfer_16bit_array(soft_iic_info_struct *soft_iic_obj, const uint16_t *write_data, uint32_t write_len,
                                   uint16_t *read_data, uint32_t read_len);

void soft_iic_sccb_write_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name, uint8_t data);
uint8_t soft_iic_sccb_read_register(soft_iic_info_struct *soft_iic_obj, const uint8_t register_name);
uint8_t soft_iic_write_16bit_register_addr_only(soft_iic_info_struct *soft_iic_obj, uint16_t reg_addr);
uint8_t soft_iic_write_16bit_register_with_data(soft_iic_info_struct *soft_iic_obj, uint16_t reg_addr, const uint8_t *data, uint32_t len);
uint8_t soft_iic_read_continue(soft_iic_info_struct *soft_iic_obj, uint32_t len, uint8_t *data);

void soft_iic_init(soft_iic_info_struct *soft_iic_obj);


#endif
