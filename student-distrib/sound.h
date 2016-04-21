/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27
 *
 * sound */

#ifndef _SOUND_H
#define _SOUND_H

#include "types.h"

#define KEY_NUM			89
#define NO_SOUND		30000
#define HALT_TIME		1
#define HALT_SOUND		1000

extern uint32_t piano_key_freq[KEY_NUM];
extern uint32_t playing;
extern int32_t music_rtc;
extern int32_t orignal_rtc;

void sound_play(uint32_t freq);
void sound_stop(void);
void sound_beep(uint32_t freq, uint32_t tick_count);
int32_t sound_music(const uint8_t* fname);
void sound_music2(void);

#endif /* _SOUND_H */
