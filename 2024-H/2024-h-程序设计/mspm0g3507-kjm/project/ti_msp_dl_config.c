/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gMotor_PWM1Backup;
DL_TimerA_backupConfig gTIMER_delayBackup;
DL_UART_Main_backupConfig gUART_WITBackup;
DL_SPI_backupConfig gSPI_0Backup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_Motor_PWM1_init();
    SYSCFG_DL_Motor_PWM2_init();
    SYSCFG_DL_TIMER_delay_init();
    SYSCFG_DL_I2C_0_init();
    SYSCFG_DL_UART_DEBUG_init();
    SYSCFG_DL_UART_WIT_init();
    SYSCFG_DL_SPI_0_init();
    SYSCFG_DL_DMA_init();
    /* Ensure backup structures have no valid state */
	gMotor_PWM1Backup.backupRdy 	= false;
	gTIMER_delayBackup.backupRdy 	= false;
	gUART_WITBackup.backupRdy 	= false;
	gSPI_0Backup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(Motor_PWM1_INST, &gMotor_PWM1Backup);
	retStatus &= DL_TimerA_saveConfiguration(TIMER_delay_INST, &gTIMER_delayBackup);
	retStatus &= DL_UART_Main_saveConfiguration(UART_WIT_INST, &gUART_WITBackup);
	retStatus &= DL_SPI_saveConfiguration(SPI_0_INST, &gSPI_0Backup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(Motor_PWM1_INST, &gMotor_PWM1Backup, false);
	retStatus &= DL_TimerA_restoreConfiguration(TIMER_delay_INST, &gTIMER_delayBackup, false);
	retStatus &= DL_UART_Main_restoreConfiguration(UART_WIT_INST, &gUART_WITBackup);
	retStatus &= DL_SPI_restoreConfiguration(SPI_0_INST, &gSPI_0Backup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(Motor_PWM1_INST);
    DL_TimerG_reset(Motor_PWM2_INST);
    DL_TimerA_reset(TIMER_delay_INST);
    DL_I2C_reset(I2C_0_INST);
    DL_UART_Main_reset(UART_DEBUG_INST);
    DL_UART_Main_reset(UART_WIT_INST);
    DL_SPI_reset(SPI_0_INST);


    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(Motor_PWM1_INST);
    DL_TimerG_enablePower(Motor_PWM2_INST);
    DL_TimerA_enablePower(TIMER_delay_INST);
    DL_I2C_enablePower(I2C_0_INST);
    DL_UART_Main_enablePower(UART_DEBUG_INST);
    DL_UART_Main_enablePower(UART_WIT_INST);
    DL_SPI_enablePower(SPI_0_INST);

    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_Motor_PWM1_C0_IOMUX,GPIO_Motor_PWM1_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_Motor_PWM1_C0_PORT, GPIO_Motor_PWM1_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_Motor_PWM1_C1_IOMUX,GPIO_Motor_PWM1_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_Motor_PWM1_C1_PORT, GPIO_Motor_PWM1_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_Motor_PWM2_C0_IOMUX,GPIO_Motor_PWM2_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_Motor_PWM2_C0_PORT, GPIO_Motor_PWM2_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_Motor_PWM2_C1_IOMUX,GPIO_Motor_PWM2_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_Motor_PWM2_C1_PORT, GPIO_Motor_PWM2_C1_PIN);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_0_IOMUX_SDA,
        GPIO_I2C_0_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_0_IOMUX_SCL,
        GPIO_I2C_0_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_I2C_0_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_I2C_0_IOMUX_SCL);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_DEBUG_IOMUX_TX, GPIO_UART_DEBUG_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_DEBUG_IOMUX_RX, GPIO_UART_DEBUG_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_WIT_IOMUX_TX, GPIO_UART_WIT_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_WIT_IOMUX_RX, GPIO_UART_WIT_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_0_IOMUX_SCLK, GPIO_SPI_0_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_0_IOMUX_PICO, GPIO_SPI_0_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_SPI_0_IOMUX_POCI, GPIO_SPI_0_IOMUX_POCI_FUNC);

    DL_GPIO_initDigitalOutput(PORTA_BEEP_IOMUX);

    DL_GPIO_initDigitalOutput(PCA9555_SCL1_IOMUX);

    DL_GPIO_initDigitalOutput(PCA9555_SDA1_IOMUX);

    DL_GPIO_initDigitalOutput(OLED_SPI_RST_OLED_IOMUX);

    DL_GPIO_initDigitalOutput(OLED_SPI_DC_OLED_IOMUX);

    DL_GPIO_initDigitalOutput(OLED_SPI_CS_OLED_IOMUX);

    DL_GPIO_initDigitalInput(ENCODER_M1_IOMUX);

    DL_GPIO_initDigitalInput(ENCODER_M2_IOMUX);

    DL_GPIO_initDigitalInput(ENCODER_M3_IOMUX);

    DL_GPIO_initDigitalInput(ENCODER_M4_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_PIN_0_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_PIN_1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_PIN_2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_PIN_3_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_PIN_4_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(PORTB_HEATER_IOMUX);

    DL_GPIO_initDigitalOutput(PORTB_R_IOMUX);

    DL_GPIO_initDigitalOutput(PORTB_G_IOMUX);

    DL_GPIO_initDigitalOutput(PORTB_B_IOMUX);

    DL_GPIO_clearPins(GPIOA, PORTA_BEEP_PIN |
		PCA9555_SCL1_PIN |
		PCA9555_SDA1_PIN);
    DL_GPIO_enableOutput(GPIOA, PORTA_BEEP_PIN |
		PCA9555_SCL1_PIN |
		PCA9555_SDA1_PIN);
    DL_GPIO_clearPins(GPIOB, OLED_SPI_RST_OLED_PIN |
		OLED_SPI_DC_OLED_PIN |
		OLED_SPI_CS_OLED_PIN |
		PORTB_HEATER_PIN |
		PORTB_R_PIN |
		PORTB_G_PIN |
		PORTB_B_PIN);
    DL_GPIO_enableOutput(GPIOB, OLED_SPI_RST_OLED_PIN |
		OLED_SPI_DC_OLED_PIN |
		OLED_SPI_CS_OLED_PIN |
		PORTB_HEATER_PIN |
		PORTB_R_PIN |
		PORTB_G_PIN |
		PORTB_B_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOB, DL_GPIO_PIN_4_EDGE_RISE_FALL |
		DL_GPIO_PIN_5_EDGE_RISE_FALL |
		DL_GPIO_PIN_6_EDGE_RISE_FALL |
		DL_GPIO_PIN_7_EDGE_RISE_FALL);
    DL_GPIO_clearInterruptStatus(GPIOB, ENCODER_M1_PIN |
		ENCODER_M2_PIN |
		ENCODER_M3_PIN |
		ENCODER_M4_PIN);
    DL_GPIO_enableInterrupt(GPIOB, ENCODER_M1_PIN |
		ENCODER_M2_PIN |
		ENCODER_M3_PIN |
		ENCODER_M4_PIN);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_16_32_MHZ,
	.rDivClk2x              = 3,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_ENABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_DISABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK2X,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_SYSOSC,
	.qDiv                   = 9,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_2
};
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);

}


/*
 * Timer clock configuration to be sourced by  / 8 (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 10000000 Hz / (8 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gMotor_PWM1ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gMotor_PWM1Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 3000,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_Motor_PWM1_init(void) {

    DL_TimerA_setClockConfig(
        Motor_PWM1_INST, (DL_TimerA_ClockConfig *) &gMotor_PWM1ClockConfig);

    DL_TimerA_initPWMMode(
        Motor_PWM1_INST, (DL_TimerA_PWMConfig *) &gMotor_PWM1Config);

    DL_TimerA_setCaptureCompareOutCtl(Motor_PWM1_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(Motor_PWM1_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(Motor_PWM1_INST, 0, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(Motor_PWM1_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(Motor_PWM1_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(Motor_PWM1_INST, 0, DL_TIMER_CC_1_INDEX);

    DL_TimerA_enableClock(Motor_PWM1_INST);


    
    DL_TimerA_setCCPDirection(Motor_PWM1_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 8 (5000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   5000000 Hz = 5000000 Hz / (8 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gMotor_PWM2ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gMotor_PWM2Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 3000,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_Motor_PWM2_init(void) {

    DL_TimerG_setClockConfig(
        Motor_PWM2_INST, (DL_TimerG_ClockConfig *) &gMotor_PWM2ClockConfig);

    DL_TimerG_initPWMMode(
        Motor_PWM2_INST, (DL_TimerG_PWMConfig *) &gMotor_PWM2Config);

    DL_TimerG_setCaptureCompareOutCtl(Motor_PWM2_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(Motor_PWM2_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(Motor_PWM2_INST, 0, DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareOutCtl(Motor_PWM2_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_1_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(Motor_PWM2_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(Motor_PWM2_INST, 0, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(Motor_PWM2_INST);


    
    DL_TimerG_setCCPDirection(Motor_PWM2_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   101010.101010101 Hz = 10000000 Hz / (8 * (98 + 1))
 */
static const DL_TimerA_ClockConfig gTIMER_delayClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale    = 98U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_delay_INST_LOAD_VALUE = (0.01ms * 101010.101010101 Hz) - 1
 */
static const DL_TimerA_TimerConfig gTIMER_delayTimerConfig = {
    .period     = TIMER_delay_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_ONE_SHOT,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_delay_init(void) {

    DL_TimerA_setClockConfig(TIMER_delay_INST,
        (DL_TimerA_ClockConfig *) &gTIMER_delayClockConfig);

    DL_TimerA_initTimerMode(TIMER_delay_INST,
        (DL_TimerA_TimerConfig *) &gTIMER_delayTimerConfig);
    DL_TimerA_enableInterrupt(TIMER_delay_INST , DL_TIMERA_INTERRUPT_ZERO_EVENT);
    DL_TimerA_enableClock(TIMER_delay_INST);





}


static const DL_I2C_ClockConfig gI2C_0ClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_I2C_0_init(void) {

    DL_I2C_setClockConfig(I2C_0_INST,
        (DL_I2C_ClockConfig *) &gI2C_0ClockConfig);
    DL_I2C_setAnalogGlitchFilterPulseWidth(I2C_0_INST,
        DL_I2C_ANALOG_GLITCH_FILTER_WIDTH_50NS);
    DL_I2C_enableAnalogGlitchFilter(I2C_0_INST);

    /* Configure Controller Mode */
    DL_I2C_resetControllerTransfer(I2C_0_INST);
    /* Set frequency to 800000 Hz*/
    DL_I2C_setTimerPeriod(I2C_0_INST, 4);
    DL_I2C_setControllerTXFIFOThreshold(I2C_0_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(I2C_0_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(I2C_0_INST);


    /* Enable module */
    DL_I2C_enableController(I2C_0_INST);


}


static const DL_UART_Main_ClockConfig gUART_DEBUGClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_DEBUGConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_DEBUG_init(void)
{
    DL_UART_Main_setClockConfig(UART_DEBUG_INST, (DL_UART_Main_ClockConfig *) &gUART_DEBUGClockConfig);

    DL_UART_Main_init(UART_DEBUG_INST, (DL_UART_Main_Config *) &gUART_DEBUGConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_DEBUG_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_DEBUG_INST, UART_DEBUG_IBRD_40_MHZ_115200_BAUD, UART_DEBUG_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_DEBUG_INST,
                                 DL_UART_MAIN_INTERRUPT_DMA_DONE_TX |
                                 DL_UART_MAIN_INTERRUPT_EOT_DONE |
                                 DL_UART_MAIN_INTERRUPT_RX);

    /* Configure DMA Transmit Event */
    DL_UART_Main_enableDMATransmitEvent(UART_DEBUG_INST);
    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(UART_DEBUG_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_DEBUG_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(UART_DEBUG_INST, DL_UART_TX_FIFO_LEVEL_ONE_ENTRY);

    DL_UART_Main_enable(UART_DEBUG_INST);
}

static const DL_UART_Main_ClockConfig gUART_WITClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_WITConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_WIT_init(void)
{
    DL_UART_Main_setClockConfig(UART_WIT_INST, (DL_UART_Main_ClockConfig *) &gUART_WITClockConfig);

    DL_UART_Main_init(UART_WIT_INST, (DL_UART_Main_Config *) &gUART_WITConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 9600
     *  Actual baud rate: 9600.1
     */
    DL_UART_Main_setOversampling(UART_WIT_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_WIT_INST, UART_WIT_IBRD_80_MHZ_9600_BAUD, UART_WIT_FBRD_80_MHZ_9600_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_WIT_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(UART_WIT_INST_INT_IRQN, 2);


    DL_UART_Main_enable(UART_WIT_INST);
}

static const DL_SPI_Config gSPI_0_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gSPI_0_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SPI_0_init(void) {
    DL_SPI_setClockConfig(SPI_0_INST, (DL_SPI_ClockConfig *) &gSPI_0_clockConfig);

    DL_SPI_init(SPI_0_INST, (DL_SPI_Config *) &gSPI_0_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     1000000 = (80000000)/((1 + 39) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(SPI_0_INST, 39);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(SPI_0_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(SPI_0_INST);
}

static const DL_DMA_Config gDMA_CH0Config = {
    .transferMode   = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_UNCHANGED,
    .srcIncrement   = DL_DMA_ADDR_INCREMENT,
    .destWidth      = DL_DMA_WIDTH_BYTE,
    .srcWidth       = DL_DMA_WIDTH_BYTE,
    .trigger        = UART_DEBUG_INST_DMA_TRIGGER,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_CH0_init(void)
{
    DL_DMA_initChannel(DMA, DMA_CH0_CHAN_ID , (DL_DMA_Config *) &gDMA_CH0Config);
}
SYSCONFIG_WEAK void SYSCFG_DL_DMA_init(void){
    SYSCFG_DL_DMA_CH0_init();
}


