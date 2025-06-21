#include "pca9555.h"
#include "gray_detection_app.h"
#include "log_config.h"
#include "log.h"

static sw_i2c_t pca9555_i2c = {
	  .sclPort = PORTA_PORT,
    .sdaPort = PORTA_PORT,
    .sclPin = PORTA_SCL1_PIN,
    .sdaPin = PORTA_SDA1_PIN,
    .sclIOMUX = PORTA_SCL1_IOMUX,
    .sdaIOMUX = PORTA_SDA1_IOMUX,
};

void gray_detection_init(void) {
	SOFT_IIC_Init(&pca9555_i2c);
}

void gray_read_data(uint8_t *gray_datas) {
    uint16_t pca_data = pca9555_read_bit12(&pca9555_i2c, PCA9555_ADDR);
	
    // 将 16 位数据按位压入 gray_datas 数组（只取低 12 位）
    for (int i = 0; i < 12; i++) {
        gray_datas[i] = (pca_data >> i) & 0x01; // 提取每一位并存入数组
    }

}    