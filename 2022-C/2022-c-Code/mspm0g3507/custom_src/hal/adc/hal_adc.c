#if GANV_SENSOR


#include "hal_adc.h"
#include "systick.h"
//#include "log_config.h"
#include "log.h"

unsigned int adc_getValue(void)
{
    unsigned int gAdcResult = 0;
    uint32_t start_time = get_ms();  // 记录开始时间
    const uint32_t timeout_ms = 1;  // 超时时间20ms
    
    // 使能ADC转换
    DL_ADC12_enableConversions(ADC12_0_INST);
    
    // 软件触发ADC开始转换
    DL_ADC12_startConversion(ADC12_0_INST);
    
    // 等待转换完成，带超时保护
    while (DL_ADC12_getStatus(ADC12_0_INST) != DL_ADC12_STATUS_CONVERSION_IDLE) {
        // 检查是否超时
        if ((get_ms() - start_time) >= timeout_ms) {
            log_e("ADC conversion timeout after %dms", timeout_ms);
            
            // 超时处理：强制停止转换并清理
            DL_ADC12_stopConversion(ADC12_0_INST);
            DL_ADC12_disableConversions(ADC12_0_INST);
            
            return 0xFFFF;  // 返回错误值表示超时
        }
    }
    
    // 获取转换结果
    gAdcResult = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_ADC12_0);
    
    // 清除触发转换状态
    DL_ADC12_stopConversion(ADC12_0_INST);
    
    // 失能ADC转换
    DL_ADC12_disableConversions(ADC12_0_INST);
    
    return gAdcResult;
}

#endif 
