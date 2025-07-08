#include "gray_detection_app.h"
#include "pca9555.h"
#include "log.h"

static float gray_status_backup = 0.0f; // 初始备份值设为0

// I2C 硬件配置 (与您的代码相同)
static sw_i2c_t pca9555_i2c = {
    .sclPort = PORTA_PORT,
    .sdaPort = PORTA_PORT,
    .sclPin = PORTA_SCL1_PIN,
    .sdaPin = PORTA_SDA1_PIN,
    .sclIOMUX = PORTA_SCL1_IOMUX,
    .sdaIOMUX = PORTA_SDA1_IOMUX,
};

#if !USE_PCA9555
static gpio_struct_t gray_gpio[TRACK_SENSOR_COUNT] = {
	TRACK_PIN_0_PORT, TRACK_PIN_0_PIN,
	TRACK_PIN_1_PORT, TRACK_PIN_1_PIN,
	TRACK_PIN_2_PORT, TRACK_PIN_2_PIN,
	TRACK_PIN_3_PORT, TRACK_PIN_3_PIN,
	TRACK_PIN_4_PORT, TRACK_PIN_4_PIN,
	TRACK_PIN_5_PORT, TRACK_PIN_5_PIN,
	TRACK_PIN_6_PORT, TRACK_PIN_6_PIN,
	TRACK_PIN_7_PORT, TRACK_PIN_7_PIN,
};
#endif

void gray_detection_init(void) {
    SOFT_IIC_Init(&pca9555_i2c);
}

uint16_t gray_read_byte(void) {
#if !USE_PCA9555
		uint16_t data = 0;
		for (int i = 0; i < TRACK_SENSOR_COUNT; i++) {
			uint8_t bit = !DL_GPIO_readPins(gray_gpio[i].port, gray_gpio[i].pin);
			data = (data << 1) | bit;  // Correct version
		}
		return data;	
#else
		return pca9555_read_bit12(&pca9555_i2c, PCA9555_ADDR);
#endif
}

float gray_read_data(void) {
    uint16_t pca_data = gray_read_byte();
    
    const int table_size = sizeof(lookup_table) / sizeof(lookup_table[0]);
    
    // 查找匹配的输入值
    for (int i = 0; i < table_size; i++) {
        if (lookup_table[i].input == pca_data) {
            gray_status_backup = lookup_table[i].output; // 找到匹配项，更新备份值
            return lookup_table[i].output;
        }
    }
    
    // 如果没有找到匹配项 (例如，所有传感器都未检测到，或出现不连续的噪声)
    // 返回上一次有效的值。这有助于在短暂丢失线时保持方向。
    return gray_status_backup;
}