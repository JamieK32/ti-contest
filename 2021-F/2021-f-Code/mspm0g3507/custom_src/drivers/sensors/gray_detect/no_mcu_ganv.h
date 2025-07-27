#ifndef NO_MCU_GANV_GRAYSCALE_SENSOR_CONFIG_H_
#define NO_MCU_GANV_GRAYSCALE_SENSOR_CONFIG_H_
#include <string.h>
#include "ti_msp_dl_config.h"
#include "hal_adc.h"
/**************************** 传感器版本配置 ****************************/
#define Class		    0  // 经典版传感器
#define Younth      1  // 青春版传感器

/**************************** ADC分辨率配置 ****************************/
#define _14Bits 0     // 14位ADC模式
#define _12Bits 1     // 12位ADC模式
#define _10Bits 2     // 10位ADC模式
#define _8Bits  3     // 8位ADC模式

/**************************** 用户可配置区域 ***************************/
// 传感器版本选择（二选一）
#define Sensor_Edition Class		  // 使用基础版传感器
// #define Sensor_Edition Younth  // 使用青春版传感器


/************************* 根据单片机自行选择 **************************/
// 输出结果方向，与预期方向不同选1
#define Direction 1
// ADC分辨率选择（四选一）
// #define Sensor_ADCbits _14Bits
#define Sensor_ADCbits _12Bits
// #define Sensor_ADCbits _10Bits
// #define Sensor_ADCbits _8Bits

// 定时器功能开关（需要时取消注释）
// #define Use_Timer 1


/*************************** 硬件抽象层配置 ****************************/
// GPIO地址切换宏定义
#define Switch_Address_0(i) ((i)?(DL_GPIO_setPins(PORTA_PORT, PORTA_GW_ADDR0_PIN)) : (DL_GPIO_clearPins(PORTA_PORT, PORTA_GW_ADDR0_PIN)))// 地址位0控制

#define Switch_Address_1(i) ((i)?(DL_GPIO_setPins(PORTA_PORT, PORTA_GW_ADDR1_PIN)) : (DL_GPIO_clearPins(PORTA_PORT, PORTA_GW_ADDR1_PIN)))// 地址位1控制

#define Switch_Address_2(i) ((i)?(DL_GPIO_setPins(PORTA_PORT, PORTA_GW_ADDR2_PIN)) : (DL_GPIO_clearPins(PORTA_PORT, PORTA_GW_ADDR2_PIN)))// 地址位2控制

// ADC值获取接口宏定义 需要自己根据单片机完成对应位数的ADC采样函数
#define Get_adc_of_user() adc_getValue()  // 用户自定义ADC读取函数
/**********************************************************************/

/*************************** 传感器数据结构 ***************************/
typedef struct {
    uint16_t Analog_value[8];    // 原始模拟量值
    uint16_t Normal_value[8];   // 归一化后的值
    uint16_t Calibrated_white[8]; // 白校准基准值
    uint16_t Calibrated_black[8]; // 黑校准基准值
    uint16_t Gray_white[8];      // 白平衡灰度值
    uint16_t Gray_black[8];      // 黑平衡灰度值
    double Normal_factor[8];          // 归一化系数
    double bits;                      // ADC分辨率对应位数
    uint8_t Digtal;              // 数字输出状态
    uint8_t Time_out;            // 超时标志
    uint8_t Tick;                // 时基计数器
    uint8_t ok;                  // 传感器就绪标志
} No_MCU_Sensor;

#ifdef __cplusplus
extern "C" {
#endif

/*************************** 函数声明区域 *****************************/
// 初始化函数
void No_MCU_Ganv_Sensor_Init_Frist(No_MCU_Sensor* sensor); // 首次初始化
void No_MCU_Ganv_Sensor_Init(No_MCU_Sensor* sensor,uint16_t* Calibrated_white, uint16_t* Calibrated_black);// 带校准参数的初始化
#ifndef Use_Timer
// 任务处理函数
void No_Mcu_Ganv_Sensor_Task_Without_tick(No_MCU_Sensor* sensor); // 无时基版本
//区别：后者使用定时器提供，需要一个1ms基准的定时器调用Task_tick(&sensor)函数，再将函数No_Mcu_Ganv_Sensor_Task_With_tick(&sensor)放入while1
//前者只需要在while(1)里，delay 1ms，或者定时器1ms调用
//前者优点：简单，方便，后者优点：能释放CPU，不需要通过delay完成这个任务，避免阻塞其他任务，同时避免了中断里处理事件，常驻任务优先级降低
#else
void No_Mcu_Ganv_Sensor_Task_With_tick(No_MCU_Sensor* sensor);  // 有时基版本
void Task_tick(No_MCU_Sensor* sensor);                          // 时基更新函数
#endif

// 用户接口函数
uint8_t Get_Digtal_For_User(No_MCU_Sensor* sensor);          									// 获取数字量
uint8_t Get_Normalize_For_User(No_MCU_Sensor* sensor,uint16_t* result); // 获取归一化值
uint8_t Get_Anolog_Value(No_MCU_Sensor* sensor,uint16_t* result);       // 获取模拟值

#ifdef __cplusplus
}
#endif

#endif /* NO_MCU_GANV_GRAYSCALE_SENSOR_CONFIG_H_ */