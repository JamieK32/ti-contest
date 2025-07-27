#include "hal_spi.h"


// SPI 写字节函数
uint8_t spi_read_write_byte(SPI_Regs *spi_inst, uint8_t byte)
{
    // 等待 SPI 传输完成 (如果你使用的是阻塞模式的 transmitData8，这可能不需要)
    while (DL_SPI_isBusy(spi_inst));
    DL_SPI_transmitData8(spi_inst, byte);
    // 等待接收 FIFO 非空，表示数据已接收 (对于纯发送可能不需要接收数据)
    while(DL_SPI_isRXFIFOEmpty(spi_inst));
    // 返回接收到的数据 (如果不需要接收，可以返回 dummy 值)
    return DL_SPI_receiveData8(spi_inst); // 如果不需要接收，可以注释掉
    return 0; // 返回 dummy 值
}

