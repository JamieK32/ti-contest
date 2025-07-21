#include "vl53l1_read.h"

// I2C配置
static sw_i2c_t vl53l1_i2c_config = {
    .sdaIOMUX = PORTA_SCL2_IOMUX,
    .sdaPin = PORTA_SCL2_PIN,
    .sdaPort = PORTA_PORT,
    .sclIOMUX = PORTA_SDA2_IOMUX,
    .sclPin = PORTA_SDA2_PIN,
    .sclPort = PORTA_PORT,
    .delay_time = VL53L1_I2C_DELAY_TIME,
};

// VL53L1设备结构体
VL53L1_Dev_t sensor;

// 高速测量上下文
static struct {
    uint8_t init_done;
    uint8_t measuring;
    uint16_t distance_mm;
    uint8_t data_ready;
    uint32_t last_measure_time;
} vl53_ctx = {0};

/*
 * @brief 初始化VL53L1 - 高速优化版本
 */
void VL53L1_Read_Init(void)
{
    VL53L1_Error status;
    
    // 清零上下文
    memset(&vl53_ctx, 0, sizeof(vl53_ctx));
    memset(&sensor, 0, sizeof(VL53L1_Dev_t));
    
    // 初始化I2C
    SOFT_IIC_Init(&vl53l1_i2c_config);
    VL53L1_I2C_Init(&vl53l1_i2c_config);
    
    // 缩短延时
    delay_ms(VL53L1_INIT_DELAY_MS);
    
    // 高速初始化流程
    do {
        status = VL53L1_DataInit(&sensor);
        if (status != VL53L1_ERROR_NONE) break;
        
        status = VL53L1_StaticInit(&sensor);
        if (status != VL53L1_ERROR_NONE) break;
        
        // 配置为中距离模式
        status = VL53L1_SetDistanceMode(&sensor, VL53L1_DISTANCE_MODE);
        if (status != VL53L1_ERROR_NONE) break;
        
        // 设置时间预算
        status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(&sensor, VL53L1_TIMING_BUDGET_US);
        if (status != VL53L1_ERROR_NONE) break;
        
        // 设置ROI区域
        VL53L1_UserRoi_t roi;
        roi.TopLeftX = VL53L1_ROI_TOP_LEFT_X;
        roi.TopLeftY = VL53L1_ROI_TOP_LEFT_Y;
        roi.BotRightX = VL53L1_ROI_BOTTOM_RIGHT_X;
        roi.BotRightY = VL53L1_ROI_BOTTOM_RIGHT_Y;
        
        status = VL53L1_SetUserROI(&sensor, &roi);
        if (status != VL53L1_ERROR_NONE) {
            // ROI设置失败不影响传感器工作
            status = VL53L1_ERROR_NONE;
        }
        
        // 初始化成功
        vl53_ctx.init_done = 1;
        vl53_ctx.last_measure_time = get_ms();
        
    } while(0);
}

/**
 * @brief 处理函数 - 针对快速测量优化
 */
void VL53L1_Process(void)
{
    uint32_t current_time = get_ms();
    VL53L1_Error status;
    VL53L1_RangingMeasurementData_t measurement_data;
    uint8_t data_ready = 0;
    
    // 检查初始化状态
    if (!vl53_ctx.init_done) {
        return;
    }
    
    // 如果正在测量，检查是否完成
    if (vl53_ctx.measuring) {
        status = VL53L1_GetMeasurementDataReady(&sensor, &data_ready);
        
        if (status == VL53L1_ERROR_NONE && data_ready) {
            // 读取数据
            status = VL53L1_GetRangingMeasurementData(&sensor, &measurement_data);
            
            // 立即停止测量并启动下一次测量，减少间隔
            VL53L1_StopMeasurement(&sensor);
            vl53_ctx.measuring = 0;
            vl53_ctx.last_measure_time = current_time;
            
            if (status == VL53L1_ERROR_NONE) {
                // 接受更多的有效状态
                if (measurement_data.RangeStatus == VL53L1_RANGESTATUS_RANGE_VALID ||
                    measurement_data.RangeStatus == VL53L1_RANGESTATUS_RANGE_VALID_NO_WRAP_CHECK_FAIL) {
                    
                    // 过滤有效距离范围的数据
                    if (measurement_data.RangeMilliMeter <= VL53L1_MAX_DISTANCE_MM && 
                        measurement_data.RangeMilliMeter >= VL53L1_MIN_DISTANCE_MM) {
                        vl53_ctx.distance_mm = measurement_data.RangeMilliMeter;
                        vl53_ctx.data_ready = 1;
                    }
                }
            }
            
            // 立即开始下一次测量，不等待间隔时间
            status = VL53L1_StartMeasurement(&sensor);
            if (status == VL53L1_ERROR_NONE) {
                vl53_ctx.measuring = 1;
            }
            
        } else if (status != VL53L1_ERROR_NONE) {
            // 测量错误，重置状态
            VL53L1_StopMeasurement(&sensor);
            vl53_ctx.measuring = 0;
            vl53_ctx.last_measure_time = current_time;
        }
        
        // 超时处理
        if ((current_time - vl53_ctx.last_measure_time) > VL53L1_TIMEOUT_MS) {
            VL53L1_StopMeasurement(&sensor);
            vl53_ctx.measuring = 0;
            vl53_ctx.last_measure_time = current_time;
        }
        
        return;
    }
    
    // 检查是否需要开始新的测量
    if ((current_time - vl53_ctx.last_measure_time) >= VL53L1_MEASURE_INTERVAL_MS) {
        status = VL53L1_StartMeasurement(&sensor);
        if (status == VL53L1_ERROR_NONE) {
            vl53_ctx.measuring = 1;
        } else {
            vl53_ctx.last_measure_time = current_time;
        }
    }
}

/**
 * @brief 获取距离值
 */
int VL53L1_GetDistance(uint16_t *distance)
{
    if (distance == NULL || !vl53_ctx.init_done) {
        if (distance) *distance = 0;
        return VL53L1_ERROR;
    }
    
    if (vl53_ctx.data_ready) {
        *distance = vl53_ctx.distance_mm;
        vl53_ctx.data_ready = 0;  // 清除标志
        return VL53L1_OK;
    }
    
    *distance = 0;
    return VL53L1_NO_DATA;
}

/**
 * @brief 检查传感器是否就绪
 */
uint8_t VL53L1_IsReady(void)
{
    return vl53_ctx.init_done;
}

/**
 * @brief 强制重新初始化
 */
void VL53L1_Reset(void)
{
    // 停止当前测量
    if (vl53_ctx.measuring) {
        VL53L1_StopMeasurement(&sensor);
    }
    
    vl53_ctx.init_done = 0;
    vl53_ctx.measuring = 0;
    
    // 延时后重新初始化
    delay_ms(50);
    VL53L1_Read_Init();
}
