#ifndef BEEP_H__
#define BEEP_H__

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

// 音量级别定义
typedef enum {
    BEEP_VOLUME_MUTE = 0,      // 静音
    BEEP_VOLUME_LOW = 10,      // 低音量 (10%)
    BEEP_VOLUME_MEDIUM = 20,   // 中音量 (20%) 
    BEEP_VOLUME_HIGH = 35,     // 高音量 (35%)
    BEEP_VOLUME_MAX = 50       // 最大音量 (50%)
} beep_volume_level_t;

enum {
    P = 0, L1, L1_, L2, L2_, L3, L4, L4_, L5, L5_,
    L6, L6_, L7, M1, M1_, M2, M2_, M3, M4, M4_,
    M5, M5_, M6, M6_, M7, H1, H1_, H2, H2_, H3,
    H4, H4_, H5, H5_, H6, H6_, H7
};

void beep_init(void);
void beep_control(bool state);
void beep_on(void);
void beep_off(void);
void play_music(const uint8_t *music, uint16_t length);

extern const uint8_t music_example_1[];
extern size_t music_example_1_size;
extern const uint8_t music_example_2[];
extern size_t music_example_2_size;

#endif
