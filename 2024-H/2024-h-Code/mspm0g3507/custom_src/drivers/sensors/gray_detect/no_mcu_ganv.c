#include "no_mcu_ganv.h"

/* 函数功能：采集8个通道的模拟值并进行均值滤波
   参数说明：result - 存储8个通道处理结果的数组 */
void Get_Analog_value(uint16_t *result)
{
    uint8_t i,j;
    unsigned int Anolag=0;
    
    // 遍历8个传感器通道（3位地址线组合）
    for(i=0;i<8;i++)
    {
        // 通过地址线组合切换传感器通道（注意取反逻辑）
        Switch_Address_0(!(i&0x01));  // 地址线0，对应bit0
        Switch_Address_1(!(i&0x02));  // 地址线1，对应bit1
        Switch_Address_2(!(i&0x04));  // 地址线2，对应bit2

        // 每个通道采集8次ADC值进行均值滤波
        for(j=0;j<8;j++)
        {
            Anolag += Get_adc_of_user();  // 累加ADC采样值
        }
				if(!Direction)result[i]=Anolag/8;  // 计算平均值
        else result[7-i]=Anolag/8;  // 计算平均值
        Anolag=0;  // 重置累加器
    }
}

/* 函数功能：将模拟值转换为数字信号（二值化处理）
   参数说明：
   adc_value - 原始ADC值数组
   Gray_white - 白色阈值数组
   Gray_black - 黑色阈值数组
   Digital - 输出的数字信号（按位表示） */
void convertAnalogToDigital(uint16_t *adc_value,uint16_t *Gray_white,uint16_t *Gray_black,uint8_t *Digital)
{
    for (int i = 0; i < 8; i++) {
        if (adc_value[i] > Gray_white[i]) {
            *Digital |= (1 << i);   // 超过白阈值置1（白色）
        } else if (adc_value[i] < Gray_black[i]) {
            *Digital &= ~(1 << i);  // 低于黑阈值置0（黑色）
        }
        // 中间灰度值保持原有状态
    }
}

/* 函数功能：归一化ADC值到指定范围
   参数说明：
   adc_value - 原始ADC值数组
   Normal_factor - 归一化系数数组
   Calibrated_black - 校准黑值数组
   result - 存储归一化结果的数组
   bits - ADC最大量程值（如255/1024等） */
void normalizeAnalogValues(uint16_t *adc_value,double *Normal_factor,uint16_t *Calibrated_black,uint16_t *result,double bits)
{
    for (int i = 0; i < 8; i++) {
        uint16_t n ;
        // 计算归一化值（减去黑电平后缩放）
        if(adc_value[i]<Calibrated_black[i]) n=0;  // 低于黑电平归零
        else n = (adc_value[i] - Calibrated_black[i]) * Normal_factor[i];

        // 限幅处理
        if (n > bits) {
            n = bits;
        }
        result[i]=n;
    }
}

/* 函数功能：传感器结构体初始化（首次初始化）
   参数说明：sensor - 传感器结构体指针 */
void No_MCU_Ganv_Sensor_Init_Frist(No_MCU_Sensor*sensor)
{
    // 清零所有校准数据和状态
    memset(sensor->Calibrated_black,0,16);
    memset(sensor->Calibrated_white,0,16);
    memset(sensor->Normal_value,0,16);
    memset(sensor->Analog_value,0,16);
    
    // 初始化归一化系数
    for(int i = 0; i < 8; i++)
    {
        sensor->Normal_factor[i]=0.0;
    }
    
    // 初始化状态变量
    sensor->Digtal=0;
    sensor->Time_out=0;
    sensor->Tick=0;
    sensor->ok=0;  // 标记未完成校准
}

/* 函数功能：传感器完整初始化（带校准参数）
   参数说明：
   sensor - 传感器结构体指针
   Calibrated_white - 校准白值数组
   Calibrated_black - 校准黑值数组 */
void No_MCU_Ganv_Sensor_Init(No_MCU_Sensor*sensor,uint16_t *Calibrated_white,uint16_t *Calibrated_black)
{
    No_MCU_Ganv_Sensor_Init_Frist(sensor);
    
    // 根据配置设置ADC量程
    if(Sensor_ADCbits==_8Bits)sensor->bits=255.0;
    else if(Sensor_ADCbits==_10Bits)sensor->bits=1024.0;
    else if(Sensor_ADCbits==_12Bits)sensor->bits=4096.0;
    else if(Sensor_ADCbits==_14Bits)sensor->bits=16384.0;

    // 设置采样超时时间（基础版/青春版）
    if(Sensor_Edition==Class)sensor->Time_out=1;
    else sensor->Time_out=10;

    double Normal_Diff[8];
    uint16_t temp;
    
    for (int i = 0; i < 8; i++)
    {
        // 确保白值 > 黑值（必要时交换）
        if(Calibrated_black[i]>=Calibrated_white[i])
        {
            temp=Calibrated_white[i];
            Calibrated_white[i]=Calibrated_black[i];
            Calibrated_black[i]=temp;
        }

        // 计算灰度阈值（1:2和2:1分界点）
        sensor->Gray_white[i]=(Calibrated_white[i]*2+Calibrated_black[i])/3;
        sensor->Gray_black[i]=(Calibrated_white[i]+Calibrated_black[i]*2)/3;

        // 保存校准数据
        sensor->Calibrated_black[i]=Calibrated_black[i];
        sensor->Calibrated_white[i]=Calibrated_white[i];

        // 处理无效校准数据（全黑/全白/相等情况）
        if ((Calibrated_white[i] == 0 && Calibrated_black[i] == 0)||
            (Calibrated_white[i]==Calibrated_black[i]))
        {
            sensor->Normal_factor[i] = 0.0;  // 无效通道
            continue;
        }
        
        // 计算归一化系数
        Normal_Diff[i] = (double)Calibrated_white[i] - (double)Calibrated_black[i];
        sensor->Normal_factor[i] = sensor->bits / Normal_Diff[i];
    }
    sensor->ok=1;  // 标记初始化完成
}

/* 函数功能：传感器主任务（无定时器版本）*/
void No_Mcu_Ganv_Sensor_Task_Without_tick(No_MCU_Sensor*sensor)
{
    Get_Analog_value(sensor->Analog_value);  // 采集数据
    convertAnalogToDigital(sensor->Analog_value, sensor->Gray_white,sensor->Gray_black,&sensor->Digtal);// 二值化处理
    normalizeAnalogValues(sensor->Analog_value,  sensor->Normal_factor,sensor->Calibrated_black,sensor->Normal_value,sensor->bits);// 归一化处理
}

/* 函数功能：传感器主任务（带定时器版本）*/
void No_Mcu_Ganv_Sensor_Task_With_tick(No_MCU_Sensor*sensor)
{
    if(sensor->Tick>=sensor->Time_out)  // 检查是否到达采样周期
    {
        // 执行数据采集和处理
        Get_Analog_value(sensor->Analog_value);
        convertAnalogToDigital(sensor->Analog_value,sensor->Gray_white,sensor->Gray_black,&sensor->Digtal);
        normalizeAnalogValues(sensor->Analog_value,sensor->Normal_factor,sensor->Calibrated_black,sensor->Normal_value,sensor->bits);  
        sensor->Tick=0;  // 重置定时器
    }
}

/* 函数功能：定时器tick递增 */
void Task_tick(No_MCU_Sensor*sensor)
{
    sensor->Tick++;
}

/* 函数功能：获取数字信号状态 */
uint8_t Get_Digtal_For_User(No_MCU_Sensor*sensor)
{
    return sensor->Digtal;  // 返回8位数字状态（每位对应一个传感器）
}

/* 函数功能：获取归一化后的数据
   返回值：1-成功 0-未初始化 */
uint8_t Get_Normalize_For_User(No_MCU_Sensor*sensor,uint16_t* result)
{
    if(!sensor->ok)return 0;
    else 
    {
        memcpy(result,sensor->Normal_value,16);  // 拷贝归一化数据
        return 1;     
    }
}

/* 函数功能：获取原始校准数据
   返回值：1-成功 0-未初始化 */
uint8_t Get_Anolog_Value(No_MCU_Sensor*sensor,uint16_t *result)
{   

    Get_Analog_value(sensor->Analog_value);  // 重新采集数据
    memcpy(result,sensor->Analog_value,16);
    if(!sensor->ok)return 0;
    else return 1;
}