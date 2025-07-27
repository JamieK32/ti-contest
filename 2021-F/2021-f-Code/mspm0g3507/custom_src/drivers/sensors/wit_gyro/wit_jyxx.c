#include "wit_jyxx.h"

#include "hal_uart.h"
#include "delay.h"
#include "lwrb.h"

// ====================  配置定义  ====================

#define WIT_IMU_UART          UART_1_INST
#define WIT_IMU_UART_IRQ      UART_1_INST_INT_IRQN

#define WIT_UART_RX_BUFFER_SIZE       256


// 环形缓冲区相关定义
static uint8_t uart_rx_buffer[WIT_UART_RX_BUFFER_SIZE];
static lwrb_t uart_rx_rb;

static uint8_t cmd_unlock[] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
static uint8_t cmd_calibration_z[] = {0xFF, 0xAA, 0x01, 0x04, 0x00};
static uint8_t cmd_save[] = {0xFF, 0xAA, 0x00, 0x00, 0x00};

WitImu_TypeDef jy61p = {
    .rxState = WAIT_HEADER1,
    .dataIndex = 0,
};

void wit_imu_init(void) {
    // 初始化环形缓冲区
    lwrb_init(&uart_rx_rb, uart_rx_buffer, sizeof(uart_rx_buffer));
    
    // 启用UART中断
    NVIC_EnableIRQ(WIT_IMU_UART_IRQ);
}

void wit_imu_get_euler_angle(float *yaw, float *roll, float *pitch) {
	*yaw = jy61p.yaw;
	*roll = jy61p.roll;
	*pitch = jy61p.pitch;
}

// 发送置偏航角置零命令（只有6轴需要发送九轴陀螺仪是绝对z轴）
void wit_imu_set_yaw_zero(void) {
    usart_send_bytes(WIT_IMU_UART, cmd_unlock, sizeof(cmd_unlock));
    delay_ms(3000);
    usart_send_bytes(WIT_IMU_UART, cmd_calibration_z, sizeof(cmd_calibration_z));
    delay_ms(200);
    usart_send_bytes(WIT_IMU_UART, cmd_save, sizeof(cmd_save));
}

// 处理接收到的数据
static void process_received_data(void) {
    uint8_t data;
    
    // 从环形缓冲区读取数据并处理
    while (lwrb_read(&uart_rx_rb, &data, 1) == 1) {
        switch (jy61p.rxState) {
            case WAIT_HEADER1:
                if (data == 0x55) {
                    jy61p.rxState = WAIT_HEADER2;
                }
                break;
                
            case WAIT_HEADER2:
                if (data == 0x53) {
                    jy61p.rxState = RECEIVE_EULER_ANGLE;
                    jy61p.dataIndex = 0;
                } else {
                    jy61p.rxState = WAIT_HEADER1;
                }
                break;
                
            case RECEIVE_EULER_ANGLE:
                ((uint8_t*)&jy61p.euler_angle)[jy61p.dataIndex++] = data;
                if (jy61p.dataIndex == sizeof(jy61p.euler_angle)) {
                    uint8_t calculatedSum = 0x55 + 0x53 + jy61p.euler_angle.rollH + jy61p.euler_angle.rollL +
                                            jy61p.euler_angle.pitchH + jy61p.euler_angle.pitchL +
                                            jy61p.euler_angle.yawH + jy61p.euler_angle.yawL +
                                            jy61p.euler_angle.vH + jy61p.euler_angle.vL;
                    if (calculatedSum == jy61p.euler_angle.sum) {
                        jy61p.roll = ((float)(((uint16_t)jy61p.euler_angle.rollH << 8) | jy61p.euler_angle.rollL) / 32768 * 180);
                        if (jy61p.roll > 180) jy61p.roll -= 360;
                        jy61p.pitch = ((float)(((uint16_t)jy61p.euler_angle.pitchH << 8) | jy61p.euler_angle.pitchL) / 32768 * 180);
                        if (jy61p.pitch > 180) jy61p.pitch -= 360;
                        jy61p.yaw = ((float)(((uint16_t)jy61p.euler_angle.yawH << 8) | jy61p.euler_angle.yawL) / 32768 * 180);
                        if (jy61p.yaw > 180) jy61p.yaw -= 360;
                    }
                    jy61p.rxState = WAIT_HEADER1;
                }
                break;
        }
    }
}

void wit_imu_uart_irq_handler(DL_UART_IIDX idx) {
		if (idx == DL_UART_IIDX_RX) {
        uint8_t uartData = DL_UART_Main_receiveData(WIT_IMU_UART);
        lwrb_write(&uart_rx_rb, &uartData, 1);
    }
}

// 在主循环中调用此函数来处理数据
void wit_imu_process(void) {
    process_received_data();
}