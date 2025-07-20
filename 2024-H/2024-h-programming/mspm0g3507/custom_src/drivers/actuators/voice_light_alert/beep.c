#include "beep.h"
#include "ti_msp_dl_config.h"
#include "delay.h"
#include <math.h>

#define LFCLK_FREQ 32768
#define BASE_TEMPO 125  // 基准节拍时间(ms)

// 全局变量
static uint8_t current_volume = BEEP_VOLUME_MAX;
static music_player_t g_music_player = {0};

// 频率表
static const uint16_t MusicalNote[] = {
    0,    // P  - Pause (静音)
    // 低音区
    131,  // C3  - L1
    139,  // C#3 - L1_
    147,  // D3  - L2
    156,  // D#3 - L2_
    165,  // E3  - L3
    175,  // F3  - L4
    185,  // F#3 - L4_
    196,  // G3  - L5
    208,  // G#3 - L5_
    220,  // A3  - L6
    233,  // A#3 - L6_
    247,  // B3  - L7
    // 中音区
    262,  // C4  - M1  (中央C)
    277,  // C#4 - M1_
    294,  // D4  - M2
    311,  // D#4 - M2_
    330,  // E4  - M3
    349,  // F4  - M4
    370,  // F#4 - M4_
    392,  // G4  - M5
    415,  // G#4 - M5_
    440,  // A4  - M6  (标准音A)
    466,  // A#4 - M6_
    494,  // B4  - M7
    // 高音区
    523,  // C5  - H1
    554,  // C#5 - H1_
    587,  // D5  - H2
    622,  // D#5 - H2_
    659,  // E5  - H3
    698,  // F5  - H4
    740,  // F#5 - H4_
    784,  // G5  - H5
    831,  // G#5 - H5_
    880,  // A5  - H6
    932,  // A#5 - H6_
    988   // B5  - H7
};

void BEEP_PWM_setPeriod(uint32_t newPeriod) {
    newPeriod = fmin(LFCLK_FREQ, fmax(1, newPeriod));
    
    DL_TimerG_stopCounter(BEEP_PWM_INST);
    DL_TimerG_setLoadValue(BEEP_PWM_INST, newPeriod);
    
    uint32_t duty_cycle;
    if (current_volume == 0) {
        duty_cycle = 0;
    } else {
        duty_cycle = (newPeriod * current_volume) / 100;
    }
    
    DL_TimerG_setCaptureCompareValue(BEEP_PWM_INST, duty_cycle, DL_TIMER_CC_1_INDEX);
    DL_TimerG_startCounter(BEEP_PWM_INST);
}

void beep_init(void) {
    beep_off();
    music_player_init();
}

void beep_control(bool state) {
    state ? beep_on() : beep_off();
}

void beep_on(void) {
    DL_GPIO_setPins(PORTA_PORT, DL_GPIO_PIN_27);
    DL_TimerG_startCounter(BEEP_PWM_INST);
}

void beep_off(void) {
    DL_GPIO_clearPins(PORTA_PORT, DL_GPIO_PIN_27);
    DL_TimerG_stopCounter(BEEP_PWM_INST);
}

void beep_set_volume(beep_volume_level_t volume) {
    current_volume = volume;
}

static void play_sound(uint16_t frq) {
    if(frq != 0) {
        uint32_t period = LFCLK_FREQ / frq;
        BEEP_PWM_setPeriod(period);
    } else {
        beep_off();
    }
}

// 阻塞式播放单个音符
void play_note(uint8_t note_index, uint32_t duration_ms) {
    if(note_index < sizeof(MusicalNote)/sizeof(MusicalNote[0])) {
        play_sound(MusicalNote[note_index]);
        delay_ms(duration_ms);
        beep_off();
    } else {
        beep_off();
    }	
}

// 阻塞式播放音乐
void play_music(const uint8_t *music, uint16_t length) {
    if (music == NULL) return;
    
    for (size_t i = 0; i < length; i += 2) {
        if (i + 1 < length) {
            play_note(music[i], music[i + 1] * BASE_TEMPO);
        }
    }
}

// ========== 状态机实现 ==========

// 初始化音乐播放器
void music_player_init(void) {
    g_music_player.music_data = NULL;
    g_music_player.music_length = 0;
    g_music_player.current_index = 0;
    g_music_player.note_start_time = 0;
    g_music_player.note_duration = 0;
    g_music_player.is_playing = false;
}

// 开始播放音乐
void music_player_start(const uint8_t *music, uint16_t length) {
    if (music == NULL || length < 2) return;
    
    g_music_player.music_data = music;
    g_music_player.music_length = length;
    g_music_player.current_index = 0;
    g_music_player.is_playing = true;
    g_music_player.note_start_time = get_ms();
    g_music_player.note_duration = 0; // 立即开始播放第一个音符
}

// 停止播放
void music_player_stop(void) {
    g_music_player.is_playing = false;
    beep_off();
}

// 检查是否正在播放
bool music_player_is_playing(void) {
    return g_music_player.is_playing;
}

// 状态机更新 - 在5ms定时器中调用
void music_player_update(void) {
		uint32_t current_ms = get_ms();
    if (!g_music_player.is_playing) {
        return;
    }
    
    // 检查当前音符是否播放完成
    if (current_ms - g_music_player.note_start_time >= g_music_player.note_duration) {
        // 播放下一个音符
        if (g_music_player.current_index + 1 < g_music_player.music_length) {
            uint8_t note = g_music_player.music_data[g_music_player.current_index];
            uint8_t duration = g_music_player.music_data[g_music_player.current_index + 1];
            
            // 检查结束标记
            if (note == 0xFF) {
                music_player_stop();
                return;
            }
            
            // 播放音符
            if (note < NOTE_MAX) {
                play_sound(MusicalNote[note]);
            }
            
            // 更新状态
            g_music_player.note_start_time = current_ms;
            g_music_player.note_duration = duration * BASE_TEMPO;
            g_music_player.current_index += 2;
        } else {
            // 音乐播放完成
            music_player_stop();
        }
    }
}

//春日影
const uint8_t music_example_1[] = {
    // ========== 前奏 ==========
    // 第1-2小节：3 2 1 2 | 3. 4 3 2. | 3 2 1 2 | 3. 4 3 2. |
    M5, 4, M4, 2, M3, 4, M4, 2,
    M5, 3, M6, 1, M5, 2, M4, 6,
    M5, 4, M4, 2, M3, 4, M4, 2,
    M5, 3, M6, 1, M5, 2, M4, 6,
    
    // 第3-4小节：重复
    M5, 4, M4, 2, M3, 4, M4, 2,
    M5, 3, M6, 1, M5, 2, M4, 6,
    M5, 4, M4, 2, M3, 4, M4, 2,
    M5, 3, M6, 1, M5, 2, M4, 4, M3, 1, M4, 1,
    
    // ========== 主歌A段 ==========
    // 第5小节：3 3 2 4 3 2 | 2 2 1 1 4 3 2 | 2 1 2 3. |
    M5, 2, M5, 2, M4, 2, M6, 2, M5, 2, M4, 2,
    M4, 2, M4, 2, M3, 1, M3, 1, M6, 2, M5, 2, M4, 2,
    M4, 4, M3, 1, M4, 1, M5, 6,
    
    // 第6小节：0. 3 5 1' | 7 1' 7 1' | 7 6 5 5 2 4 | 4 3 3 5 | 4 3 2 3 5 |
    P, 6, M5, 2, M7, 2, H3, 2,
    H2, 4, H3, 2, H2, 4, H3, 2,
    H2, 1, H1, 1, M7, 4, M7, 2, M4, 2, M6, 2,
    M6, 4, M5, 2, M5, 4, M7, 2,
    M6, 2, M5, 2, M4, 2, M5, 4, M7, 2,
    
    // 第7小节：1. 0 1 | 2 1. 1 1 5 1 | 4 4 4 3 2 2 1 | 1. 0. |
    M3, 6, P, 4, M3, 2,
    M4, 2, M3, 3, M3, 1, M3, 2, M7, 2, M3, 2,
    M6, 4, M5, 2, M4, 4, M3, 2,
    M3, 6, P, 6,
    
    // ========== 副歌 ==========
    // 第8小节：6' 5' 5' 5' 4' 4' | 3' 2' 2' 2' 0 5 |
    H7, 2, H6, 2, H6, 2, H6, 2, H5, 2, H5, 2,
    H4, 2, H2, 2, H2, 2, H2, 2, P, 2, M7, 2,
    
    // 第9小节：5 4 4 4 4 3 2 | 2 1 7- 1. 0. |
    M7, 2, M6, 1, M6, 1, M6, 2, M6, 2, M5, 2, M4, 2,
    M4, 4, M3, 1, L7, 1, M3, 3, P, 3,
    
    // 第10小节：重复副歌变奏
    H7, 2, H6, 2, H6, 2, H6, 2, H5, 2, H5, 2,
    H4, 2, H2, 2, H2, 2, H2, 2, P, 2, M5, 2,
    M5, 2, M5, 1, M5, 1, M5, 1, M5, 1, M5, 2, M4, 2, M5, 2,
    
    // ========== 间奏 ==========
    // 第11小节：2' 1' 1' 0 1' |
    H2, 4, H3, 2, H3, 2, P, 2, H3, 2,
    
    // 第12小节：7 6 6. | 0 6 6 5 4 4 |
    H2, 4, H1, 2, H1, 6,
    P, 4, H1, 2, H1, 2, M7, 2, M6, 1, M6, 1,
    
    // 第13小节：4. 3 4 5 | 5. 0. |
    M6, 6, M5, 1, M6, 1, M7, 10, P, 6,
    
    // ========== 桥段 ==========
    // 第14小节：快速音阶
    M5, 1, M4, 1, M5, 1, M4, 1, M5, 1, M6, 1, M7, 2, P, 2, M6, 1, M7, 1,
    H1, 2, P, 2, H1, 1, H2, 1, H3, 2, P, 2, H2, 1, H3, 1,
    M7, 4, P, 1, M3, 1, M7, 2, M6, 2, M6, 2,
    M5, 4, M5, 1, M6, 1, M7, 4, P, 2,
    
    // 第15小节：重复桥段变奏
    M5, 1, M4, 1, M5, 1, M4, 1, M5, 1, M6, 1, M7, 2, P, 2, M6, 1, M7, 1,
    H1, 2, P, 2, M7, 1, H1, 1, H2, 2, P, 3, M5, 1,
    H5, 2, H5, 2, P, 1, M5, 1, H6, 2, H5, 2, H4, 2,
    H4, 4, H3, 1, H2, 1, H3, 2, P, 2, M7, 1, H3, 1,
    
    // ========== 尾奏 ==========
    H4, 4, H3, 2, H3, 2, P, 2, M7, 2,
    H4, 4, H3, 2, H3, 2, P, 2, M7, 1, H3, 1,
    H4, 3, H5, 1, H4, 2, H3, 2, P, 2, H3, 2,
    H2, 4, H1, 2, H1, 2, P, 2, M7, 2,
    
    // 渐慢结束
    M7, 2, M6, 2, M6, 2, M5, 2, M4, 2,
    M5, 6, P, 6,
    M5, 2, M6, 2, M5, 2, M6, 2, M5, 2, M4, 2,
    M3, 6, M3, 6,
    
    // 最终和弦
    P, 4, M7, 2, H3, 2,
    H3, 8, P, 4,
    H3, 2, H4, 2, H3, 2, H4, 2, H3, 2, H4, 2,
    H3, 12, P, 20,
    
    0xFF  // 结束标记
};

//天空之城
const uint8_t music_example_2[] = {
    //第1行
    P,    4,    P,    4,    P,    4,    M6,    2,    M7,    2,
    H1,    4+2,M7,    2,    H1,    4,    H3,    4,
    M7,    4+4+4,        M3,    2,    M3,    2,
    //第2行
    M6,    4+2,M5,    2,    M6,    4,    H1,    4,
    M5,    4+4+4,        M3,    4,
    M4,    4+2,M3,    2,    M4,    4,     H1,    4,
    //第3行
    M3,    4+2,P,    2,    H1,    2,    H1,    2,    H1,    2,
    M7,    4+2,M4_,2,    M4_,4,    M7,    4,
    M7,    4+4,P,    4,    M6,    2,    M7,    2,
    //第4行
    H1, 4+2,M7,    2,    H1,    4,    H3,    4,
    M7,    4+4+4,    M3,    2,    M3,    2,
    M6,    4+2,    M5,    2,    M6,    4, H1,    4,
    //第5行
    M5,    4+4+4,    M2,    2,    M3,    2,
    M4,    4,    H1,    2,    M7,    2+2,    H1,    2+4,
    H2,    2,    H2,    2,    H3,    2,    H1,    2+4+4,
    //第6行
    H1,    2,    M7,    2,    M6,    2,    M6,    2,    M7,    4,    M5_,4,
    M6,    4+4+4,    H1,    2,    H2,    2,
    H3,    4+2,H2,    2,    H3,    4,    H5,    4,
    //第7行
    H2,    4+4+4,    M5,    2,    M5,    2,
    H1,    4+2,    M7,    2,    H1,    4,    H3,    4,
    H3,    4+4+4+4,
    //第8行
    M6,    2,    M7,    2, H1,    4,    M7,    4,    H2,    2,    H2,    2,
    H1,    4+2,M5,    2+4+4,
    H4,    4,    H3,    4,    H2,    4,    H1,    4,
    //第9行
    H3,    4+4+4,    H3,    4,
    H6,    4+4,    H5,    4,    H5,    4,
    H3,    2,    H2,    2,    H1,    4+4,    P,    2,    H1,    2,
    //第10行
    H2,    4,    H1,    2,    H2,    2,    H2,    4,    H5,    4,
    H3,    4+4+4,    H3,    4,
    H6,    4+4,    H5,    4+4,
    //第11行
    H3,    2,    H2,    2,    H1,    4+4,    P,    2,    H1,    2,
    H2,    4,    H1,    2,    H2,    2+4,    M7,    4,
    M6,    4+4+4,    M6,    2,    M7,    2,
};

size_t music_example_1_size = sizeof(music_example_1) / sizeof(music_example_1[0]);
size_t music_example_2_size = sizeof(music_example_2) / sizeof(music_example_2[0]);