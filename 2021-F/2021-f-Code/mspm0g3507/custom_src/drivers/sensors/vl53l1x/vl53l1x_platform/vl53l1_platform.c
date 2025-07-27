
/* 
* This file is part of VL53L1 Platform 
* 
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved 
* 
* License terms: BSD 3-clause "New" or "Revised" License. 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this 
* list of conditions and the following disclaimer. 
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution. 
* 
* 3. Neither the name of the copyright holder nor the names of its contributors 
* may be used to endorse or promote products derived from this software 
* without specific prior written permission. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
* 
*/


#include "vl53l1_platform.h"
#include "vl53l1_platform_log.h"
#include "vl53l1_api.h"
#include "delay.h"
#include "systick.h"
#include <string.h>
#include <time.h>
#include <math.h>
#include "log.h"

#define I2C_TIME_OUT_BASE   10
#define I2C_TIME_OUT_BYTE   1

#ifdef VL53L1_LOG_ENABLE
#define trace_print(level, ...) VL53L1_trace_print_module_function(VL53L1_TRACE_MODULE_PLATFORM, level, VL53L1_TRACE_FUNCTION_NONE, ##__VA_ARGS__)
#define trace_i2c(...) VL53L1_trace_print_module_function(VL53L1_TRACE_MODULE_NONE, VL53L1_TRACE_LEVEL_NONE, VL53L1_TRACE_FUNCTION_I2C, ##__VA_ARGS__)
#endif


//extern I2C_HandleTypeDef hi2c1;
//#define VL53L0X_pI2cHandle    (&hi2c1)

/* when not customized by application define dummy one */
#ifndef VL53L1_GetI2cBus
/** This macro can be overloaded by user to enforce i2c sharing in RTOS context
 */
#   define VL53L1_GetI2cBus(...) (void)0
#endif

#ifndef VL53L1_PutI2cBus
/** This macro can be overloaded by user to enforce i2c sharing in RTOS context
 */
#   define VL53L1_PutI2cBus(...) (void)0
#endif

// ====== 全局变量和宏定义 ======
uint8_t _I2CBuffer[256];
soft_iic_info_struct *g_vl53l1_i2c = NULL;  // 修改为实际的I2C结构体指针
#define VL53L1_I2C_ADDR 0x29  // VL53L1默认I2C地址（7位地址）

/**
 * @brief VL53L1 I2C初始化函数
 * @param i2c_config 软件I2C配置指针
 */
void VL53L1_I2C_Init(soft_iic_info_struct *i2c_config)
{
    if (i2c_config == NULL) {
        log_e("VL53L1 I2C配置为空\n");
        return;
    }
    
    g_vl53l1_i2c = i2c_config;
    
    // 设置VL53L1的I2C地址
    g_vl53l1_i2c->addr = VL53L1_I2C_ADDR;
    
    log_i("VL53L1 I2C初始化完成，地址: 0x%02X\n", VL53L1_I2C_ADDR);
}

// ====== 修改后的I2C读写函数 ======

/**
 * @brief VL53L1 I2C写入函数
 * @param Dev VL53L1设备句柄（未使用，保持兼容性）
 * @param pdata 数据缓冲区（前2字节为16位寄存器地址，后续为数据）
 * @param count 总字节数
 * @return 0表示成功，-1表示失败
 */
int _I2CWrite(VL53L1_DEV Dev, uint8_t *pdata, uint32_t count) 
{
    uint8_t status;
    
    // 检查I2C是否已初始化
    if (g_vl53l1_i2c == NULL) {
        log_e("VL53L1 I2C未初始化\n");
        return -1;
    }
    
    if (pdata == NULL || count < 2) {
        log_e("VL53L1 写入参数无效\n");
        return -1;
    }
    
    // 提取16位寄存器地址（大端序）
    uint16_t reg_addr = (pdata[0] << 8) | pdata[1];
    
    log_d("VL53L1 写入: 寄存器=0x%04X, 数据长度=%d\n", reg_addr, count-2);
    
    if (count == 2) {
        // 只写寄存器地址的情况（用于后续读取）
        status = soft_iic_write_16bit_register_addr_only(g_vl53l1_i2c, reg_addr);
    } else {
        // 写入寄存器地址和数据
        status = soft_iic_write_16bit_register_with_data(g_vl53l1_i2c, reg_addr, &pdata[2], count-2);
    }
    
    // 转换返回值：软件I2C返回0表示成功，VL53L1期望0表示成功
    int result = (status == 0) ? 0 : -1;
    
    if (result != 0) {
        log_e("VL53L1 写入失败: 寄存器=0x%04X, 错误码=%d\n", reg_addr, status);
    }
    
    return result;
}

/**
 * @brief VL53L1 I2C读取函数
 * @param Dev VL53L1设备句柄（未使用，保持兼容性）
 * @param pdata 数据缓冲区
 * @param count 要读取的字节数
 * @return 0表示成功，-1表示失败
 */
int _I2CRead(VL53L1_DEV Dev, uint8_t *pdata, uint32_t count) 
{
    uint8_t status;
    
    // 检查I2C是否已初始化
    if (g_vl53l1_i2c == NULL) {
        log_e("VL53L1 I2C未初始化\n");
        return -1;
    }
    
    if (pdata == NULL || count == 0) {
        log_e("VL53L1 读取参数无效\n");
        return -1;
    }
    
    log_d("VL53L1 读取: 数据长度=%d\n", count);
    
    // 使用连续读取函数（假设寄存器地址已经通过_I2CWrite设置）
    status = soft_iic_read_continue(g_vl53l1_i2c, count, pdata);
    
    // 转换返回值
    int result = (status == 0) ? 0 : -1;
    
    if (result != 0) {
        log_e("VL53L1 读取失败: 错误码=%d\n", status);
    }
    
    return result;
}

VL53L1_Error VL53L1_WriteMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    int status_int;
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    if (count > sizeof(_I2CBuffer) - 1) {
        return VL53L1_ERROR_INVALID_PARAMS;
    }
    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    memcpy(&_I2CBuffer[2], pdata, count);
    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, count + 2);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
    }
    VL53L1_PutI2cBus();
    return Status;
}

// the ranging_sensor_comms.dll will take care of the page selection
VL53L1_Error VL53L1_ReadMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, 2);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
        goto done;
    }
    status_int = _I2CRead(Dev, pdata, count);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
    }
done:
    VL53L1_PutI2cBus();
    return Status;
}

VL53L1_Error VL53L1_WrByte(VL53L1_DEV Dev, uint16_t index, uint8_t data) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = data;

    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, 3);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
    }
    VL53L1_PutI2cBus();
    return Status;
}

VL53L1_Error VL53L1_WrWord(VL53L1_DEV Dev, uint16_t index, uint16_t data) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = data >> 8;
    _I2CBuffer[3] = data & 0x00FF;

    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, 4);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
    }
    VL53L1_PutI2cBus();
    return Status;
}

VL53L1_Error VL53L1_WrDWord(VL53L1_DEV Dev, uint16_t index, uint32_t data) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    int32_t status_int;
    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = (data >> 24) & 0xFF;
    _I2CBuffer[3] = (data >> 16) & 0xFF;
    _I2CBuffer[4] = (data >> 8)  & 0xFF;
    _I2CBuffer[5] = (data >> 0 ) & 0xFF;
    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, 6);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
    }
    VL53L1_PutI2cBus();
    return Status;
}

VL53L1_Error VL53L1_UpdateByte(VL53L1_DEV Dev, uint16_t index, uint8_t AndData, uint8_t OrData) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    uint8_t data;

    Status = VL53L1_RdByte(Dev, index, &data);
    if (Status) {
        goto done;
    }
    data = (data & AndData) | OrData;
    Status = VL53L1_WrByte(Dev, index, data);
done:
    return Status;
}

VL53L1_Error VL53L1_RdByte(VL53L1_DEV Dev, uint16_t index, uint8_t *data) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    int32_t status_int;

	_I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;
    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, 2);
    if( status_int ){
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
        goto done;
    }
    status_int = _I2CRead(Dev, data, 1);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
    }
done:
    VL53L1_PutI2cBus();
    return Status;
}

VL53L1_Error VL53L1_RdWord(VL53L1_DEV Dev, uint16_t index, uint16_t *data) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;
    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, 2);

    if( status_int ){
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
        goto done;
    }
    status_int = _I2CRead(Dev, _I2CBuffer, 2);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
        goto done;
    }

    *data = ((uint16_t)_I2CBuffer[0]<<8) + (uint16_t)_I2CBuffer[1];
done:
    VL53L1_PutI2cBus();
    return Status;
}

VL53L1_Error VL53L1_RdDWord(VL53L1_DEV Dev, uint16_t index, uint32_t *data) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;
    VL53L1_GetI2cBus();
    status_int = _I2CWrite(Dev, _I2CBuffer, 2);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
        goto done;
    }
    status_int = _I2CRead(Dev, _I2CBuffer, 4);
    if (status_int != 0) {
        Status = VL53L1_ERROR_CONTROL_INTERFACE;
        goto done;
    }

    *data = ((uint32_t)_I2CBuffer[0]<<24) + ((uint32_t)_I2CBuffer[1]<<16) + ((uint32_t)_I2CBuffer[2]<<8) + (uint32_t)_I2CBuffer[3];

done:
    VL53L1_PutI2cBus();
    return Status;
}

VL53L1_Error VL53L1_GetTickCount(
	uint32_t *ptick_count_ms)
{

    /* Returns current tick count in [ms] */

	VL53L1_Error status  = VL53L1_ERROR_NONE;

	//*ptick_count_ms = timeGetTime();
	*ptick_count_ms = 0;

#ifdef VL53L1_LOG_ENABLE
	trace_print(
		VL53L1_TRACE_LEVEL_DEBUG,
		"VL53L1_GetTickCount() = %5u ms;\n",
	*ptick_count_ms);
#endif

	return status;
}











#define trace_print(level, ...) \
	_LOG_TRACE_PRINT(VL53L1_TRACE_MODULE_PLATFORM, \
	level, VL53L1_TRACE_FUNCTION_NONE, ##__VA_ARGS__)

#define trace_i2c(...) \
	_LOG_TRACE_PRINT(VL53L1_TRACE_MODULE_NONE, \
	VL53L1_TRACE_LEVEL_NONE, VL53L1_TRACE_FUNCTION_I2C, ##__VA_ARGS__)














VL53L1_Error VL53L1_GetTimerFrequency(int32_t *ptimer_freq_hz)
{
	*ptimer_freq_hz = 0;
	
	trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GetTimerFrequency: Freq : %dHz\n", *ptimer_freq_hz);
	return VL53L1_ERROR_NONE;
}


VL53L1_Error VL53L1_WaitMs(VL53L1_Dev_t *pdev, int32_t wait_ms){
	(void)pdev;
	delay_ms(wait_ms);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WaitUs(VL53L1_Dev_t *pdev, int32_t wait_us){
	(void)pdev;
		delay_us(wait_us);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WaitValueMaskEx(
	VL53L1_Dev_t *pdev,
	uint32_t      timeout_ms,
	uint16_t      index,
	uint8_t       value,
	uint8_t       mask,
	uint32_t      poll_delay_ms)
{

	/*
	 * Platform implementation of WaitValueMaskEx V2WReg script command
	 *
	 * WaitValueMaskEx(
	 *          duration_ms,
	 *          index,
	 *          value,
	 *          mask,
	 *          poll_delay_ms);
	 */

	VL53L1_Error status         = VL53L1_ERROR_NONE;
	uint32_t     start_time_ms = 0;
	uint32_t     current_time_ms = 0;
	uint32_t     polling_time_ms = 0;
	uint8_t      byte_value      = 0;
	uint8_t      found           = 0;
#ifdef VL53L1_LOG_ENABLE
	uint8_t      trace_functions = VL53L1_TRACE_FUNCTION_NONE;
#endif

	char   register_name[VL53L1_MAX_STRING_LENGTH];

    /* look up register name */
#ifdef PAL_EXTENDED
	VL53L1_get_register_name(
			index,
			register_name);
#else
	VL53L1_COPYSTRING(register_name, "");
#endif

	/* Output to I2C logger for FMT/DFT  */

    /*trace_i2c("WaitValueMaskEx(%5d, 0x%04X, 0x%02X, 0x%02X, %5d);\n",
    		     timeout_ms, index, value, mask, poll_delay_ms); */
    trace_i2c("WaitValueMaskEx(%5d, %s, 0x%02X, 0x%02X, %5d);\n",
    		     timeout_ms, register_name, value, mask, poll_delay_ms);

	/* calculate time limit in absolute time */

	 VL53L1_GetTickCount(&start_time_ms);

	/* remember current trace functions and temporarily disable
	 * function logging
	 */

#ifdef VL53L1_LOG_ENABLE
	trace_functions = VL53L1_get_trace_functions();
	VL53L1_set_trace_functions(VL53L1_TRACE_FUNCTION_NONE);
#endif

	/* wait until value is found, timeout reached on error occurred */

	while ((status == VL53L1_ERROR_NONE) &&
		   (polling_time_ms < timeout_ms) &&
		   (found == 0)) {

		if (status == VL53L1_ERROR_NONE)
			status = VL53L1_RdByte(
							pdev,
							index,
							&byte_value);

		if ((byte_value & mask) == value)
			found = 1;

		if (status == VL53L1_ERROR_NONE  &&
			found == 0 &&
			poll_delay_ms > 0)
			status = VL53L1_WaitMs(
					pdev,
					poll_delay_ms);

		/* Update polling time (Compare difference rather than absolute to
		negate 32bit wrap around issue) */
		VL53L1_GetTickCount(&current_time_ms);
		polling_time_ms = current_time_ms - start_time_ms;

	}

#ifdef VL53L1_LOG_ENABLE
	/* Restore function logging */
	VL53L1_set_trace_functions(trace_functions);
#endif

	if (found == 0 && status == VL53L1_ERROR_NONE)
		status = VL53L1_ERROR_TIME_OUT;

	return status;
}


