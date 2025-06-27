#include "stm32f10x.h"

// ���յ������ݻ�����
uint8_t rx_buffer[10];
// �������ݳ���
volatile uint16_t rx_length = 0;
// ���ձ�־
volatile uint8_t rx_flag = 0;

// ����1��ʼ������
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // ʹ��GPIOA��USART1ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // ����USART1��TX��RX����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ����USART1
    USART_InitStructure.USART_BaudRate = 115200; // ������115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // ʹ��USART1
    USART_Cmd(USART1, ENABLE);

    // �����ж�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // ����NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// ����1�жϷ�����
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // ���յ�����
        uint8_t temp = USART_ReceiveData(USART1);
        if (rx_length < sizeof(rx_buffer)) {
            rx_buffer[rx_length++] = temp;
        }
        // ����Ƿ������ɣ����Ը���Э�鶨��Ľ������жϣ�
        if (temp == 0x5B) { // ����������0x5B����
            rx_flag = 1;
        }
    }
}

// ������յ�������
void processReceivedData(void) {
    if (rx_length >= 7 && rx_flag) {
        // ������ݸ�ʽ�Ƿ���ȷ
        if (rx_buffer[0] == 0x2C && rx_buffer[6] == 0x5B) {
            // ��ȡX��Y����
            uint8_t x = rx_buffer[2];
            uint8_t y = rx_buffer[3];
            // ��ӡ���յ�������
            char log[50];
            sprintf(log, "Received X: %d, Y: %d\r\n", x, y);
            // ͨ�����ڷ�����־����ѡ��
            // sendString(log);
        }
        // ���ý��ջ�����
        rx_length = 0;
        rx_flag = 0;
    }
}


// ���������������ַ���
void sendString(const char *str) {
    while (*str) {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}