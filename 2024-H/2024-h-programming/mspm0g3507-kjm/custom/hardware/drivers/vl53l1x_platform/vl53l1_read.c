#include "vl53l1_read.h"

// I2C配置
static sw_i2c_t vl53l1_i2c_config = {
    .sdaIOMUX = PORTA_SDA1_IOMUX,
    .sdaPin = PORTA_SDA1_PIN,
    .sdaPort = PORTA_PORT,
    .sclIOMUX = PORTA_SCL1_IOMUX,
    .sclPin = PORTA_SCL1_PIN,
    .sclPort = PORTA_PORT,
    .delay_time = 1,
};

// VL53L1设备结构体
VL53L1_Dev_t sensor;

// 内部上下文
static VL53L1_Context_t ctx = {0};

/*
 * @brief 初始化VL53L1
 */
void VL53L1_Read_Init(void)
{
    // 初始化sensor结构体
    memset(&sensor, 0, sizeof(VL53L1_Dev_t));    
    ctx.state = STATE_INIT;
    ctx.distance_mm = 0;
    ctx.tick_count = 0;
    ctx.last_measure_time = 0;
    ctx.data_ready = 0;
    ctx.init_done = 0;
}

/**
 * @brief 处理函数 - 每10ms调用一次
 */
/**
 * @brief 处理函数 - 每10ms调用一次
 */
void VL53L1_Process(void)
{
    static uint8_t init_step = 0;
    static uint32_t wait_start_time = 0;  // 改为等待开始的时间戳
    static uint32_t error_start_time = 0;
    VL53L1_Error status;
    VL53L1_RangingMeasurementData_t measurement_data;
    uint8_t data_ready = 0;
    
    ctx.tick_count = get_ms();
    
    switch (ctx.state) {
        case STATE_INIT:
            // 重置静态变量
            if (init_step == 0) {
                wait_start_time = 0;
                error_start_time = 0;
            }
            
            switch (init_step) {
                case 0:
                    SOFT_IIC_Init(&vl53l1_i2c_config);
                    VL53L1_I2C_Init(&vl53l1_i2c_config);
                    init_step++;
                    break;
                    
                case 1:
                    status = VL53L1_DataInit(&sensor);
                    if (status == VL53L1_ERROR_NONE) init_step++;
                    else { ctx.state = STATE_ERROR; init_step = 0; }
                    break;
                    
                case 2:
                    status = VL53L1_StaticInit(&sensor);
                    if (status == VL53L1_ERROR_NONE) init_step++;
                    else { ctx.state = STATE_ERROR; init_step = 0; }
                    break;
                    
                case 3:
                    status = VL53L1_SetDistanceMode(&sensor, VL53L1_DISTANCEMODE_MEDIUM);
                    if (status == VL53L1_ERROR_NONE) init_step++;
                    else { ctx.state = STATE_ERROR; init_step = 0; }
                    break;
                    
                case 4:
                    status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(&sensor, 50000);
                    if (status == VL53L1_ERROR_NONE) {
                        ctx.state = STATE_IDLE;
                        ctx.init_done = 1;
                        ctx.last_measure_time = ctx.tick_count;
                        init_step = 0;
                    } else {
                        ctx.state = STATE_ERROR;
                        init_step = 0;
                    }
                    break;
            }
            break;
            
        case STATE_IDLE:
            // 每50ms自动测量一次
            if ((ctx.tick_count - ctx.last_measure_time) >= 50) {
                status = VL53L1_StartMeasurement(&sensor);
                if (status == VL53L1_ERROR_NONE) {
                    ctx.state = STATE_MEASURING;
                    ctx.last_measure_time = ctx.tick_count;
                    wait_start_time = ctx.tick_count;  // 记录等待开始时间
                } else {
                    ctx.state = STATE_ERROR;
                }
            }
            break;
            
        case STATE_MEASURING:
            // 等待测量完成，1秒超时
            if ((ctx.tick_count - wait_start_time) < 1000) {
                status = VL53L1_GetMeasurementDataReady(&sensor, &data_ready);
                if (status == VL53L1_ERROR_NONE && data_ready) {
                    // 读取数据
                    status = VL53L1_GetRangingMeasurementData(&sensor, &measurement_data);
                    VL53L1_StopMeasurement(&sensor);
                    
                    if (status == VL53L1_ERROR_NONE) {
                        if (measurement_data.RangeStatus == VL53L1_RANGESTATUS_RANGE_VALID) {
                            ctx.distance_mm = measurement_data.RangeMilliMeter;
                            ctx.data_ready = 1;
                        }
                    }
                    ctx.state = STATE_IDLE;
                    wait_start_time = 0;  // 重置等待时间
                } else if (status != VL53L1_ERROR_NONE) {
                    VL53L1_StopMeasurement(&sensor);
                    ctx.state = STATE_ERROR;
                    wait_start_time = 0;
                }
                // 如果既没有数据ready也没有错误，继续等待
            } else {
                // 超时处理
                VL53L1_StopMeasurement(&sensor);
                ctx.state = STATE_IDLE;
                wait_start_time = 0;
            }
            break;
            
        case STATE_ERROR:
            // 错误状态，等待5秒后重新初始化
            if (error_start_time == 0) {
                error_start_time = ctx.tick_count;
            }
            
            if ((ctx.tick_count - error_start_time) >= 5000) {
                ctx.state = STATE_INIT;
                ctx.init_done = 0;
                error_start_time = 0;
            }
            break;
    }
}
/**
 * @brief 获取距离值
 * @param distance 输出距离值(毫米)
 * @return VL53L1_OK=有新数据, VL53L1_NO_DATA=无新数据, VL53L1_ERROR=错误
 */
VL53L1_Status_t VL53L1_GetDistance(uint16_t *distance)
{
    if (distance == NULL) {
        return VL53L1_ERROR;
    }
    
    // 检查状态
    if (ctx.state == STATE_ERROR || !ctx.init_done) {
        *distance = 0;
        return VL53L1_ERROR;
    }
    
    // 检查是否有新数据
    if (ctx.data_ready) {
        *distance = ctx.distance_mm;
        ctx.data_ready = 0;  // 清除标志
        return VL53L1_OK;
    }
    
    *distance = 0;
    return VL53L1_NO_DATA;
}