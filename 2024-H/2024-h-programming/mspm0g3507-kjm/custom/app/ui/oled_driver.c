/*
 * oled.c
 *
 *  Created on: 2022年7月24日
 *      Author: Unicorn_Li
 */
#include "oled_driver.h"

u8g2_t u8g2;

#ifdef OLED_DRIVER_MODE_SPI

// SPI 写字节函数
uint8_t spi_write_byte(uint8_t byte)
{
    // 等待 SPI 传输完成 (如果你使用的是阻塞模式的 transmitData8，这可能不需要)
    while (DL_SPI_isBusy(SPI_0_INST));
    DL_SPI_transmitData8(SPI_0_INST, byte);
    // 等待接收 FIFO 非空，表示数据已接收 (对于纯发送可能不需要接收数据)
    while(DL_SPI_isRXFIFOEmpty(SPI_0_INST));
    // 返回接收到的数据 (如果不需要接收，可以返回 dummy 值)
    return DL_SPI_receiveData8(SPI_0_INST); // 如果不需要接收，可以注释掉
    return 0; // 返回 dummy 值
}

// SPI 模式的 OLED 初始化
void oled_spi_hardware_init(void)
{
    // 初始化 SPI 外设 (假设已经在其他地方完成)

    // 复位 OLED
    OLED_RST_Clr();
    // 使用 FreeRTOS 延时，避免阻塞整个系统
    delay_ms(10);
    OLED_RST_Set();
		delay_ms(10);

    // 拉高 CS (根据硬件连接和时序要求调整)
    OLED_CS_Set();
}

// U8g2 SPI 字节发送回调函数
uint8_t u8x8_byte_3wire_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    uint8_t *data_ptr = (uint8_t *)arg_ptr;

    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        // 通过 SPI 发送 arg_int 个字节数据
        for (int i = 0; i < arg_int; i++)
        {
            spi_write_byte(*(data_ptr + i));
        }
        break;

    case U8X8_MSG_BYTE_SET_DC:
        // 设置 DC 引脚 (数据/命令)
        if (arg_int)
            OLED_DC_Set(); // 数据模式
        else
            OLED_DC_Clr(); // 命令模式
        break;

    case U8X8_MSG_BYTE_INIT:
        // 初始化 SPI 硬件
        oled_spi_hardware_init();
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        // 拉低 CS，开始传输
        OLED_CS_Clr();
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        // 拉高 CS，结束传输
        OLED_CS_Set();
        break;

    default:
        return 0; // 未知消息
    }
    return 1; // 消息处理成功
}

// U8g2 SPI GPIO 和延时回调函数
// 如果所有 GPIO 控制都在 u8x8_byte_3wire_hw_spi 中处理，这个函数可能只需要处理延时
uint8_t u8g2_gpio_and_delay_mspm0(U8X8_UNUSED u8x8_t *u8x8,
                                  U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,
                                  U8X8_UNUSED void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_DELAY_MILLI:
            // 毫秒延时，使用 FreeRTOS 的 vTaskDelay
            delay_ms(arg_int);
            break;
        case U8X8_MSG_DELAY_I2C:
            // 微秒延时，使用你的 delay_us 函数
            // 注意：在 FreeRTOS 中使用短时阻塞延时需要谨慎
            delay_us(arg_int <= 2 ? 5 : 1);
            break;
        // 其他延时消息...

        // 如果 SPI 模式有独立的 GPIO 控制消息，可以在这里处理
        // 例如复位引脚
        case U8X8_MSG_GPIO_RESET:
             if (arg_int == 0)
                OLED_RST_Clr();
             else
                OLED_RST_Set();
            break;

        default:
            return 0; // 未知消息
    }
    return 1; // 消息处理成功
}


#endif // OLED_DRIVER_MODE_SPI

#ifdef OLED_DRIVER_MODE_I2C

// I2C 模式的 OLED 初始化 (GPIO 配置)
void oled_i2c_hardware_init(void)
{
		DL_GPIO_initDigitalOutputFeatures(
				OLED_I2C_SCL_IOMUX,          // 引脚 IOMUX 定义，复用 SPI 的 SCLK
				DL_GPIO_INVERSION_DISABLE,      // 不反转
				DL_GPIO_RESISTOR_NONE,          // （I2C 通常需要外部上拉电阻）
				DL_GPIO_DRIVE_STRENGTH_HIGH,     // 驱动强度设为低（可根据硬件需求调整）
				DL_GPIO_HIZ_ENABLE              // 启用高阻态，符合 I2C 开漏模式
		);

		// 配置 SDA 引脚 (复用 SPI 的 PICO 引脚)
		DL_GPIO_initDigitalOutputFeatures(
				OLED_I2C_SDA_IOMUX,          // 引脚 IOMUX 定义，复用 SPI 的 PICO
				DL_GPIO_INVERSION_DISABLE,      // 不反转
				DL_GPIO_RESISTOR_NONE,          // （I2C 通常需要外部上拉电阻）
				DL_GPIO_DRIVE_STRENGTH_HIGH,     // 驱动强度设为低（可根据硬件需求调整）
				DL_GPIO_HIZ_ENABLE              // 启用高阻态，符合 I2C 开漏模式
		);

    // 设置初始电平为高，以便模拟 I2C 空闲状态
    DL_GPIO_setPins(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN);
    DL_GPIO_setPins(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN);

    // 启用输出功能
    DL_GPIO_enableOutput(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN);
    DL_GPIO_enableOutput(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN);
		
		    // 复位 OLED
    OLED_I2C_RST_Clr();
    // 使用 FreeRTOS 延时，避免阻塞整个系统
    delay_ms(10);
    OLED_I2C_RST_Set();
    delay_ms(10);
}

// U8g2 I2C GPIO 和延时回调函数
uint8_t u8x8_gpio_and_delay_mspm0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    // 初始化GPIO
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        // 初始化 I2C 相关的 GPIO
        oled_i2c_hardware_init();
        break;

    // 毫秒延时
    case U8X8_MSG_DELAY_MILLI:
				delay_ms(arg_int);
        break;

    // I2C 总线延时 (通常用于时钟拉伸或数据稳定延时)
    case U8X8_MSG_DELAY_I2C:
        // 使用你的微秒延时函数
        //delay_us(arg_int <= 2 ? 5 : 1); // 确保这里的延时足够长以满足 I2C 时序要求
        break;

    // I2C时钟信号控制
    case U8X8_MSG_GPIO_I2C_CLOCK:
        if (arg_int == 0)
            OLED_SCL_Clr();
        else
            OLED_SCL_Set();
        break;

    // I2C数据信号控制
    case U8X8_MSG_GPIO_I2C_DATA:
        if (arg_int == 0)
            OLED_SDA_Clr();
        else
            OLED_SDA_Set();
        break;
		}
    return 1; // 消息处理成功
}

#endif // OLED_DRIVER_MODE_I2C

// U8g2 初始化函数
void u8g2_Init(void)
{
    // 使用条件编译选择 SPI 或 I2C 驱动
#ifdef OLED_DRIVER_MODE_SPI
    // 初始化 U8g2，使用 SPI 驱动
    u8g2_Setup_ssd1306_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_3wire_hw_spi, u8g2_gpio_and_delay_mspm0);
#endif

#ifdef OLED_DRIVER_MODE_I2C
    // 初始化 U8g2，使用 I2C 驱动
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay_mspm0);
#endif

    // 初始化显示屏（发送初始化命令）
    u8g2_InitDisplay(&u8g2);

    // 打开显示屏
    u8g2_SetPowerSave(&u8g2, 0);

    // 清空显示缓冲区
    u8g2_ClearBuffer(&u8g2);
}
