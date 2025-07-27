// beep.h - 添加状态机相关定义
#ifndef BEEP_H__
#define BEEP_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 音量级别定义
typedef enum {
    BEEP_VOLUME_MUTE = 0,      // 静音
    BEEP_VOLUME_LOW = 10,      // 低音量 (10%)
    BEEP_VOLUME_MEDIUM = 20,   // 中音量 (20%) 
    BEEP_VOLUME_HIGH = 35,     // 高音量 (35%)
    BEEP_VOLUME_MAX = 50       // 最大音量 (50%)
} beep_volume_level_t;

// 音符索引定义
typedef enum {
    P = 0,      // Pause 休止符
    // 低音区
    L1,  L1_,   L2,  L2_,   L3,   L4,  L4_,   L5,  L5_,   L6,  L6_,   L7,
    // 中音区
    M1,  M1_,   M2,  M2_,   M3,   M4,  M4_,   M5,  M5_,   M6,  M6_,   M7,
    // 高音区
    H1,  H1_,   H2,  H2_,   H3,   H4,  H4_,   H5,  H5_,   H6,  H6_,   H7,
    NOTE_MAX    // 音符数量上限
} note_index_t;

// 音乐播放状态机结构体
typedef struct {
    const uint8_t *music_data;  // 音乐数据
    uint16_t music_length;      // 音乐长度
    uint16_t current_index;     // 当前索引
    uint32_t note_start_time;   // 音符开始时间
    uint32_t note_duration;     // 音符持续时间
    bool is_playing;            // 是否正在播放
} music_player_t;

// 基础函数
void beep_init(void);
void beep_control(bool state);
void beep_on(void);
void beep_off(void);
void beep_set_volume(beep_volume_level_t volume);
void play_note(uint8_t note_index, uint32_t duration_ms);
void play_music(const uint8_t *music, uint16_t length);

// 状态机函数
void music_player_init(void);
void music_player_start(const uint8_t *music, uint16_t length);
void music_player_stop(void);
void music_player_update(void);
bool music_player_is_playing(void);

// 外部需要实现的函数
extern uint32_t get_ms(void);

// 音乐数据
extern const uint8_t music_example_1[];
extern size_t music_example_1_size;
extern const uint8_t music_example_2[];
extern size_t music_example_2_size;

#ifdef __cplusplus
}
#endif

#endif // BEEP_H__