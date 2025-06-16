/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000



/* Defines for Motor_PWM1 */
#define Motor_PWM1_INST                                                    TIMA0
#define Motor_PWM1_INST_IRQHandler                              TIMA0_IRQHandler
#define Motor_PWM1_INST_INT_IRQN                                (TIMA0_INT_IRQn)
#define Motor_PWM1_INST_CLK_FREQ                                        10000000
/* GPIO defines for channel 0 */
#define GPIO_Motor_PWM1_C0_PORT                                            GPIOB
#define GPIO_Motor_PWM1_C0_PIN                                    DL_GPIO_PIN_14
#define GPIO_Motor_PWM1_C0_IOMUX                                 (IOMUX_PINCM31)
#define GPIO_Motor_PWM1_C0_IOMUX_FUNC                IOMUX_PINCM31_PF_TIMA0_CCP0
#define GPIO_Motor_PWM1_C0_IDX                               DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_Motor_PWM1_C1_PORT                                            GPIOA
#define GPIO_Motor_PWM1_C1_PIN                                     DL_GPIO_PIN_3
#define GPIO_Motor_PWM1_C1_IOMUX                                  (IOMUX_PINCM8)
#define GPIO_Motor_PWM1_C1_IOMUX_FUNC                 IOMUX_PINCM8_PF_TIMA0_CCP1
#define GPIO_Motor_PWM1_C1_IDX                               DL_TIMER_CC_1_INDEX

/* Defines for Motor_PWM2 */
#define Motor_PWM2_INST                                                    TIMG8
#define Motor_PWM2_INST_IRQHandler                              TIMG8_IRQHandler
#define Motor_PWM2_INST_INT_IRQN                                (TIMG8_INT_IRQn)
#define Motor_PWM2_INST_CLK_FREQ                                         5000000
/* GPIO defines for channel 0 */
#define GPIO_Motor_PWM2_C0_PORT                                            GPIOA
#define GPIO_Motor_PWM2_C0_PIN                                     DL_GPIO_PIN_7
#define GPIO_Motor_PWM2_C0_IOMUX                                 (IOMUX_PINCM14)
#define GPIO_Motor_PWM2_C0_IOMUX_FUNC                IOMUX_PINCM14_PF_TIMG8_CCP0
#define GPIO_Motor_PWM2_C0_IDX                               DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_Motor_PWM2_C1_PORT                                            GPIOA
#define GPIO_Motor_PWM2_C1_PIN                                     DL_GPIO_PIN_4
#define GPIO_Motor_PWM2_C1_IOMUX                                  (IOMUX_PINCM9)
#define GPIO_Motor_PWM2_C1_IOMUX_FUNC                 IOMUX_PINCM9_PF_TIMG8_CCP1
#define GPIO_Motor_PWM2_C1_IDX                               DL_TIMER_CC_1_INDEX



/* Defines for TIMER_delay */
#define TIMER_delay_INST                                                 (TIMA1)
#define TIMER_delay_INST_IRQHandler                             TIMA1_IRQHandler
#define TIMER_delay_INST_INT_IRQN                               (TIMA1_INT_IRQn)
#define TIMER_delay_INST_LOAD_VALUE                                         (0U)




/* Defines for I2C_0 */
#define I2C_0_INST                                                          I2C1
#define I2C_0_INST_IRQHandler                                    I2C1_IRQHandler
#define I2C_0_INST_INT_IRQN                                        I2C1_INT_IRQn
#define I2C_0_BUS_SPEED_HZ                                                800000
#define GPIO_I2C_0_SDA_PORT                                                GPIOA
#define GPIO_I2C_0_SDA_PIN                                        DL_GPIO_PIN_30
#define GPIO_I2C_0_IOMUX_SDA                                      (IOMUX_PINCM5)
#define GPIO_I2C_0_IOMUX_SDA_FUNC                       IOMUX_PINCM5_PF_I2C1_SDA
#define GPIO_I2C_0_SCL_PORT                                                GPIOA
#define GPIO_I2C_0_SCL_PIN                                        DL_GPIO_PIN_29
#define GPIO_I2C_0_IOMUX_SCL                                      (IOMUX_PINCM4)
#define GPIO_I2C_0_IOMUX_SCL_FUNC                       IOMUX_PINCM4_PF_I2C1_SCL


/* Defines for UART_DEBUG */
#define UART_DEBUG_INST                                                    UART0
#define UART_DEBUG_INST_IRQHandler                              UART0_IRQHandler
#define UART_DEBUG_INST_INT_IRQN                                  UART0_INT_IRQn
#define GPIO_UART_DEBUG_RX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_TX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_RX_PIN                                    DL_GPIO_PIN_11
#define GPIO_UART_DEBUG_TX_PIN                                    DL_GPIO_PIN_10
#define GPIO_UART_DEBUG_IOMUX_RX                                 (IOMUX_PINCM22)
#define GPIO_UART_DEBUG_IOMUX_TX                                 (IOMUX_PINCM21)
#define GPIO_UART_DEBUG_IOMUX_RX_FUNC                  IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_DEBUG_IOMUX_TX_FUNC                  IOMUX_PINCM21_PF_UART0_TX
#define UART_DEBUG_BAUD_RATE                                            (115200)
#define UART_DEBUG_IBRD_40_MHZ_115200_BAUD                                  (21)
#define UART_DEBUG_FBRD_40_MHZ_115200_BAUD                                  (45)
/* Defines for UART_WIT */
#define UART_WIT_INST                                                      UART3
#define UART_WIT_INST_IRQHandler                                UART3_IRQHandler
#define UART_WIT_INST_INT_IRQN                                    UART3_INT_IRQn
#define GPIO_UART_WIT_RX_PORT                                              GPIOB
#define GPIO_UART_WIT_TX_PORT                                              GPIOB
#define GPIO_UART_WIT_RX_PIN                                       DL_GPIO_PIN_3
#define GPIO_UART_WIT_TX_PIN                                       DL_GPIO_PIN_2
#define GPIO_UART_WIT_IOMUX_RX                                   (IOMUX_PINCM16)
#define GPIO_UART_WIT_IOMUX_TX                                   (IOMUX_PINCM15)
#define GPIO_UART_WIT_IOMUX_RX_FUNC                    IOMUX_PINCM16_PF_UART3_RX
#define GPIO_UART_WIT_IOMUX_TX_FUNC                    IOMUX_PINCM15_PF_UART3_TX
#define UART_WIT_BAUD_RATE                                                (9600)
#define UART_WIT_IBRD_80_MHZ_9600_BAUD                                     (520)
#define UART_WIT_FBRD_80_MHZ_9600_BAUD                                      (53)




/* Defines for SPI_0 */
#define SPI_0_INST                                                         SPI1
#define SPI_0_INST_IRQHandler                                   SPI1_IRQHandler
#define SPI_0_INST_INT_IRQN                                       SPI1_INT_IRQn
#define GPIO_SPI_0_PICO_PORT                                              GPIOB
#define GPIO_SPI_0_PICO_PIN                                      DL_GPIO_PIN_15
#define GPIO_SPI_0_IOMUX_PICO                                   (IOMUX_PINCM32)
#define GPIO_SPI_0_IOMUX_PICO_FUNC                   IOMUX_PINCM32_PF_SPI1_PICO
#define GPIO_SPI_0_POCI_PORT                                              GPIOA
#define GPIO_SPI_0_POCI_PIN                                      DL_GPIO_PIN_16
#define GPIO_SPI_0_IOMUX_POCI                                   (IOMUX_PINCM38)
#define GPIO_SPI_0_IOMUX_POCI_FUNC                   IOMUX_PINCM38_PF_SPI1_POCI
/* GPIO configuration for SPI_0 */
#define GPIO_SPI_0_SCLK_PORT                                              GPIOA
#define GPIO_SPI_0_SCLK_PIN                                      DL_GPIO_PIN_17
#define GPIO_SPI_0_IOMUX_SCLK                                   (IOMUX_PINCM39)
#define GPIO_SPI_0_IOMUX_SCLK_FUNC                   IOMUX_PINCM39_PF_SPI1_SCLK



/* Defines for DMA_CH0 */
#define DMA_CH0_CHAN_ID                                                      (0)
#define UART_DEBUG_INST_DMA_TRIGGER                          (DMA_UART0_TX_TRIG)



/* Port definition for Pin Group PORTA */
#define PORTA_PORT                                                       (GPIOA)

/* Defines for BEEP: GPIOA.27 with pinCMx 60 on package pin 31 */
#define PORTA_BEEP_PIN                                          (DL_GPIO_PIN_27)
#define PORTA_BEEP_IOMUX                                         (IOMUX_PINCM60)
/* Port definition for Pin Group PCA9555 */
#define PCA9555_PORT                                                     (GPIOA)

/* Defines for SCL1: GPIOA.8 with pinCMx 19 on package pin 54 */
#define PCA9555_SCL1_PIN                                         (DL_GPIO_PIN_8)
#define PCA9555_SCL1_IOMUX                                       (IOMUX_PINCM19)
/* Defines for SDA1: GPIOA.9 with pinCMx 20 on package pin 55 */
#define PCA9555_SDA1_PIN                                         (DL_GPIO_PIN_9)
#define PCA9555_SDA1_IOMUX                                       (IOMUX_PINCM20)
/* Port definition for Pin Group OLED_SPI */
#define OLED_SPI_PORT                                                    (GPIOB)

/* Defines for RST_OLED: GPIOB.16 with pinCMx 33 on package pin 4 */
#define OLED_SPI_RST_OLED_PIN                                   (DL_GPIO_PIN_16)
#define OLED_SPI_RST_OLED_IOMUX                                  (IOMUX_PINCM33)
/* Defines for DC_OLED: GPIOB.17 with pinCMx 43 on package pin 14 */
#define OLED_SPI_DC_OLED_PIN                                    (DL_GPIO_PIN_17)
#define OLED_SPI_DC_OLED_IOMUX                                   (IOMUX_PINCM43)
/* Defines for CS_OLED: GPIOB.20 with pinCMx 48 on package pin 19 */
#define OLED_SPI_CS_OLED_PIN                                    (DL_GPIO_PIN_20)
#define OLED_SPI_CS_OLED_IOMUX                                   (IOMUX_PINCM48)
/* Port definition for Pin Group ENCODER */
#define ENCODER_PORT                                                     (GPIOB)

/* Defines for M1: GPIOB.4 with pinCMx 17 on package pin 52 */
// pins affected by this interrupt request:["M1","M2","M3","M4"]
#define ENCODER_INT_IRQN                                        (GPIOB_INT_IRQn)
#define ENCODER_INT_IIDX                        (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define ENCODER_M1_IIDX                                      (DL_GPIO_IIDX_DIO4)
#define ENCODER_M1_PIN                                           (DL_GPIO_PIN_4)
#define ENCODER_M1_IOMUX                                         (IOMUX_PINCM17)
/* Defines for M2: GPIOB.5 with pinCMx 18 on package pin 53 */
#define ENCODER_M2_IIDX                                      (DL_GPIO_IIDX_DIO5)
#define ENCODER_M2_PIN                                           (DL_GPIO_PIN_5)
#define ENCODER_M2_IOMUX                                         (IOMUX_PINCM18)
/* Defines for M3: GPIOB.6 with pinCMx 23 on package pin 58 */
#define ENCODER_M3_IIDX                                      (DL_GPIO_IIDX_DIO6)
#define ENCODER_M3_PIN                                           (DL_GPIO_PIN_6)
#define ENCODER_M3_IOMUX                                         (IOMUX_PINCM23)
/* Defines for M4: GPIOB.7 with pinCMx 24 on package pin 59 */
#define ENCODER_M4_IIDX                                      (DL_GPIO_IIDX_DIO7)
#define ENCODER_M4_PIN                                           (DL_GPIO_PIN_7)
#define ENCODER_M4_IOMUX                                         (IOMUX_PINCM24)
/* Port definition for Pin Group GPIO_KEY */
#define GPIO_KEY_PORT                                                    (GPIOB)

/* Defines for PIN_0: GPIOB.12 with pinCMx 29 on package pin 64 */
#define GPIO_KEY_PIN_0_PIN                                      (DL_GPIO_PIN_12)
#define GPIO_KEY_PIN_0_IOMUX                                     (IOMUX_PINCM29)
/* Defines for PIN_1: GPIOB.8 with pinCMx 25 on package pin 60 */
#define GPIO_KEY_PIN_1_PIN                                       (DL_GPIO_PIN_8)
#define GPIO_KEY_PIN_1_IOMUX                                     (IOMUX_PINCM25)
/* Defines for PIN_2: GPIOB.9 with pinCMx 26 on package pin 61 */
#define GPIO_KEY_PIN_2_PIN                                       (DL_GPIO_PIN_9)
#define GPIO_KEY_PIN_2_IOMUX                                     (IOMUX_PINCM26)
/* Defines for PIN_3: GPIOB.10 with pinCMx 27 on package pin 62 */
#define GPIO_KEY_PIN_3_PIN                                      (DL_GPIO_PIN_10)
#define GPIO_KEY_PIN_3_IOMUX                                     (IOMUX_PINCM27)
/* Defines for PIN_4: GPIOB.11 with pinCMx 28 on package pin 63 */
#define GPIO_KEY_PIN_4_PIN                                      (DL_GPIO_PIN_11)
#define GPIO_KEY_PIN_4_IOMUX                                     (IOMUX_PINCM28)
/* Port definition for Pin Group PORTB */
#define PORTB_PORT                                                       (GPIOB)

/* Defines for HEATER: GPIOB.0 with pinCMx 12 on package pin 47 */
#define PORTB_HEATER_PIN                                         (DL_GPIO_PIN_0)
#define PORTB_HEATER_IOMUX                                       (IOMUX_PINCM12)
/* Defines for R: GPIOB.26 with pinCMx 57 on package pin 28 */
#define PORTB_R_PIN                                             (DL_GPIO_PIN_26)
#define PORTB_R_IOMUX                                            (IOMUX_PINCM57)
/* Defines for G: GPIOB.27 with pinCMx 58 on package pin 29 */
#define PORTB_G_PIN                                             (DL_GPIO_PIN_27)
#define PORTB_G_IOMUX                                            (IOMUX_PINCM58)
/* Defines for B: GPIOB.22 with pinCMx 50 on package pin 21 */
#define PORTB_B_PIN                                             (DL_GPIO_PIN_22)
#define PORTB_B_IOMUX                                            (IOMUX_PINCM50)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_Motor_PWM1_init(void);
void SYSCFG_DL_Motor_PWM2_init(void);
void SYSCFG_DL_TIMER_delay_init(void);
void SYSCFG_DL_I2C_0_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_UART_WIT_init(void);
void SYSCFG_DL_SPI_0_init(void);
void SYSCFG_DL_DMA_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
