#include "wit_jyxx.h"
#include "ti_msp_dl_config.h"
#include "uart_driver.h"
#include "delay.h"

static uint8_t cmd_unlock[] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
static uint8_t cmd_calibration_z[] = {0xFF, 0xAA, 0x01, 0x04, 0x00};
static uint8_t cmd_save[] = {0xFF, 0xAA, 0x00, 0x00, 0x00};

WitImu_TypeDef jy901s = {
		.init = wit_jyxx_init,
    .reset = Serial_Jy61p_Zero_Yaw,
    .rxState = WAIT_HEADER1,
    .dataIndex = 0,
};

static void wit_jyxx_init(void) {
	NVIC_EnableIRQ(UART_1_INST_INT_IRQN);
}

// 发送置偏航角置零命令（只有6轴需要发送九轴陀螺仪是绝对z轴）
static void Serial_Jy61p_Zero_Yaw(void) {
		usart_send_bytes(UART_1_INST, cmd_unlock, sizeof(cmd_unlock));
    delay_ms(200);
    usart_send_bytes(UART_1_INST, cmd_calibration_z, sizeof(cmd_calibration_z));
    delay_ms(3000);
    usart_send_bytes(UART_1_INST, cmd_save, sizeof(cmd_save));
}


void UART_1_INST_IRQHandler(void) {
		DL_UART_IIDX idx = DL_UART_getPendingInterrupt(UART_1_INST);
		uint8_t uartData = DL_UART_Main_receiveData(UART_1_INST);
    switch (jy901s.rxState) {
        case WAIT_HEADER1:
            if (uartData == 0x55) {
                jy901s.rxState = WAIT_HEADER2;
            }
            break;
        case WAIT_HEADER2:
            if (uartData == 0x53) {
                jy901s.rxState = RECEIVE_EULER_ANGLE;
                jy901s.dataIndex = 0;
            } else {
                jy901s.rxState = WAIT_HEADER1;
            }
            break;
        case RECEIVE_EULER_ANGLE:
            ((uint8_t*)&jy901s.euler_angle)[jy901s.dataIndex++] = uartData;
            if (jy901s.dataIndex == sizeof(jy901s.euler_angle)) {
                uint8_t calculatedSum = 0x55 + 0x53 + jy901s.euler_angle.rollH + jy901s.euler_angle.rollL +
                                        jy901s.euler_angle.pitchH + jy901s.euler_angle.pitchL +
                                        jy901s.euler_angle.yawH + jy901s.euler_angle.yawL +
                                        jy901s.euler_angle.vH + jy901s.euler_angle.vL;

                if (calculatedSum == jy901s.euler_angle.sum) {
                    jy901s.roll = ((float)(((uint16_t)jy901s.euler_angle.rollH << 8) | jy901s.euler_angle.rollL) / 32768 * 180);
                    if (jy901s.roll > 180) jy901s.roll -= 360;

                    jy901s.pitch = ((float)(((uint16_t)jy901s.euler_angle.pitchH << 8) | jy901s.euler_angle.pitchL) / 32768 * 180);
                    if (jy901s.pitch > 180) jy901s.pitch -= 360;

                    jy901s.yaw = ((float)(((uint16_t)jy901s.euler_angle.yawH << 8) | jy901s.euler_angle.yawL) / 32768 * 180);
                    if (jy901s.yaw > 180) jy901s.yaw -= 360;
                }
                jy901s.rxState = WAIT_HEADER1;
            }
            break;
    }
		DL_UART_clearInterruptStatus(UART_1_INST, idx);
}

