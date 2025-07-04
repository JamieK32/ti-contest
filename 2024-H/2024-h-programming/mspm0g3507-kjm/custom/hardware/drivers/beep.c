#include "beep.h"
#include "ti_msp_dl_config.h"
#include "delay.h"

#define LFCLK_FREQ 32768

// 全局音量变量
static uint8_t current_volume = BEEP_VOLUME_LOW;

static const uint16_t MusicalNote[] = {
    0, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976
};

void BEEP_PWM_setPeriod(uint32_t newPeriod) {
    newPeriod = fmin(LFCLK_FREQ, fmax(1, newPeriod));
    
    DL_TimerG_stopCounter(BEEP_PWM_INST);
    DL_TimerG_setLoadValue(BEEP_PWM_INST, newPeriod);
    
    // 计算占空比，音量为0时完全关闭
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

// 修正后的play_sound函数
static void play_sound(uint16_t frq) {
    if(frq != 0) {
        uint32_t period = LFCLK_FREQ / frq;
        BEEP_PWM_setPeriod(period);
    } else {
				beep_off();
    }
}

// 播放音符的便捷函数
void play_note(uint8_t note_index, uint32_t duration_ms) {
    if(note_index < sizeof(MusicalNote)/sizeof(MusicalNote[0])) {
        play_sound(MusicalNote[note_index]);
				delay_ms(duration_ms);
        beep_off(); // 播放完成后关闭
    } else {
				beep_off();
		}	
}

// 播放音乐函数
void play_music(const uint8_t *music, uint16_t length) {
    if (music == NULL) return;
    
    for (size_t i = 0; i < length; i += 2) {
        if (i + 1 < length) {  // 确保不越界
            play_note(music[i], music[i + 1] * 125);
        }
    }
}

const uint8_t music_example_1[] = { // 春日影
    // Music1
    M5, 4, M4, 2, M3, 4, M4, 2, 
    M5, 3, M6, 1, M5, 2, M4, 5, 
    M5, 4, M4, 2, M3, 4, M4, 2, 
    M5, 3, M6, 1, M5, 2, M4, 5, 
    M5, 4, M4, 2, M3, 4, M4, 2, 
    M5, 3, M6, 1, M5, 2, M4, 5, 
    M5, 4, M4, 2, M3, 4, M4, 2, 
    M5, 3, M6, 1, M5, 2, M4, 5, 
    L7, 1, M1, 1, 
    M2, 2, M2, 2, M1, 2, M3, 2, M2, 2, M1, 2, 
    M1, 2, M1, 2, L7, 1, L7, 1, M3, 2, M2, 2, M1, 2, 
    M1, 4, L7, 1, M1, 1, M2, 5, 
    P, 5, M2, 2, M5, 2, H1, 2, 
    H7, 4, H1, 2, H7, 4, H1, 2, H7, 1, H6, 1, M5, 4, 
    M5, 2, M1, 2, M3, 2, M3, 4, M2, 2, M2, 4, L2, 2, 
    M3, 2, M2, 2, M1, 2, M2, 4, M5, 2, 
    L7, 5, P, 4, L7, 2, 
    M1, 2, L7, 3, L7, 1, L7, 2, M5, 2, L7, 2, 
    M3, 4, M2, 2, M1, 4, L7, 2, 
    // Music2
    L7, 5, P, 4, L7, 1, M1, 1, 
    M2, 2, M2, 2, M1, 2, M3, 2, M2, 2, M1, 2, 
    M1, 2, M1, 2, L7, 2, M3, 2, M2, 2, M1, 2, 
    M1, 4, L7, 1, M1, 1, M2, 5, 
    P, 5, M2, 2, M5, 2, H1, 2, 
    H7, 4, H1, 2, H7, 4, H1, 2, H7, 1, H6, 1, M5, 4, 
    M5, 2, M1, 2, M3, 2, M3, 4, M2, 2, M2, 4, L2, 2, 
    M3, 2, M2, 2, M1, 2, M2, 4, M5, 2, 
    L7, 5, P, 4, L7, 1, L7, 1, 
    M1, 2, L7, 4, L7, 2, M5, 2, L7, 2, 
    M3, 2, M3, 1, M3, 1, M2, 1, M1, 1, M1, 4, L7, 2, 
    L7, 3, P, 3, P, 5, 
    H5, 2, H4, 2, H4, 2, H4, 2, H3, 2, H3, 2, 
    H2, 2, M7, 2, M7, 2, M7, 2, P, 2, M5, 2, 
    M5, 2, M3, 1, M3, 1, M3, 2, M3, 2, M2, 2, M1, 2, 
    M1, 4, L7, 1, L6, 1, L7, 3, P, 3, 
    H5, 2, H4, 2, H4, 2, H4, 2, H3, 2, H3, 2, 
    H2, 2, M7, 2, M7, 2, M7, 2, P, 2, M2, 2, 
    M2, 2, M2, 1, M2, 1, M2, 1, M2, 1, M2, 2, M1, 2, M2, 2, 
    // Music3
    M7, 4, M6, 2, M6, 2, P, 2, M6, 2, 
    H7, 4, H6, 2, H7, 5, 
    P, 4, H6, 2, H6, 2, M5, 2, M3, 1, M3, 1, 
    M3, 5, M2, 1, M3, 1, M5, 9, P, 5, 
    M2, 1, M1, 1, M2, 1, M1, 1, M2, 1, M3, 1, M5, 2, 
    P, 2, M3, 1, M5, 1, H6, 2, 
    P, 2, H6, 1, H7, 1, H7, 2, P, 2, M7, 1, M1, 1, 
    M5, 4, P, 1, L7, 1, M5, 2, M3, 2, M3, 2, 
    M2, 4, M2, 1, M3, 1, M5, 4, P, 2, 
    M2, 1, M1, 1, M2, 1, M1, 1, M2, 1, M3, 1, M5, 2, 
    P, 2, M3, 1, M5, 1, H6, 2, 
    P, 2, M4, 1, H6, 1, H7, 2, 
    P, 3, M2, 1, H1, 2, H1, 2, P, 1, M2, 1, H2, 2, 
    H1, 2, M7, 2, M7, 4, M6, 1, H6, 1, H1, 2, 
    P, 2, M5, 1, H1, 1, H3, 4, H2, 2, H2, 2, 
    P, 2, M5, 1, M7, 1, 0xFF
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