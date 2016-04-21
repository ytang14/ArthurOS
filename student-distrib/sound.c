/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27
 *
 * sound */

#include "lib.h"
#include "sound.h"
#include "rtc.h"
#include "filesys.h"
#include "terminal.h"


#define SOUND_CMD_PORT		0x43
#define SOUND_CMD_ADDR		0xB6
#define SOUND_DATA_PORT		0x42
#define SOUND_LOW_MASK		0xFF
#define SOUND_HIGH_SHIFT	8
#define SOUND_IO_PORT		0x61
#define SOUND_IO_BIT		3
#define SOUND_IO_MASK		0xFC
#define SOUND_FREQ			1193182

#define SOUND_HEADER_SIZE	2
#define SOUND_LENGTH_SIZE	2
#define SOUND_FREQ_SIZE		2
#define SOUND_FREQ_OFFSET	0x10
#define SOUND_DATA_OFFSET	0xC0
#define SOUND_DATA_NEXT		2
#define SOUND_DATA_SIZE		300
#define SOUND_SPEED_NUM		8
#define SOUND_SPEED_NEXT	1

#define HIGH_SHIFT			8

uint32_t playing = 0;
int32_t music_rtc = 0;
int32_t orignal_rtc = 0;

/* sound_play
 *	 DESCRIPTION: Play the sound at a given frequency
 *		  INPUTS: freq - frequency needs playing
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
sound_play(uint32_t freq)
{
	playing = 1;

	uint16_t div;
	uint8_t tmp;

	if(freq != 0)
		div = SOUND_FREQ / freq;
	else
		div = SOUND_FREQ / NO_SOUND;

	outb(SOUND_CMD_ADDR, SOUND_CMD_PORT);
	outb(div & SOUND_LOW_MASK, SOUND_DATA_PORT);
	outb(div >> SOUND_HIGH_SHIFT, SOUND_DATA_PORT);
	
	tmp = inb(SOUND_IO_PORT);
	if(tmp != (tmp | SOUND_IO_BIT))
		outb(tmp | SOUND_IO_BIT, SOUND_IO_PORT);
}

/* sound_stop
 *	 DESCRIPTION: Stop playing the sound
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
sound_stop(void)
{
	playing = 0;
	uint8_t tmp = (inb(SOUND_IO_PORT) & SOUND_IO_MASK);
	outb(tmp, SOUND_IO_PORT);
}

/* sound_beep
 *	 DESCRIPTION: Play the sound at a given frequency
 *		  INPUTS: freq - frequency needs playing
 *				  time - duration of the sound
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
sound_beep(uint32_t freq, uint32_t tick_count)
{
	sound_play(freq);
	
	if(music_rtc > current_rtc_freq) {
		orignal_rtc = current_rtc_freq;
		rtc_write(0, (uint8_t*)&music_rtc, 0);
	}
	
	if(music_rtc != 0)
		tick_count = tick_count * (current_rtc_freq / music_rtc);

	int32_t i;
	for(i = 0; i < tick_count; i++)
		rtc_read();

	sound_stop();
}

/* sound_music
 *	 DESCRIPTION: Play the first part of Turkish March by Mozart
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: will disable scheduler and keyboard when playing music */
int32_t
sound_music(const uint8_t* fname)
{
	dentry_t dentry;
	uint8_t sound_header[SOUND_HEADER_SIZE], sound_length[SOUND_LENGTH_SIZE];
	int32_t read, length, i;
	uint8_t sound[SOUND_DATA_SIZE], speed[SOUND_DATA_SIZE], temp[SOUND_DATA_NEXT], temp_freq[SOUND_FREQ_SIZE];
	uint32_t offset = 0, count = 0, stop_flag = 0;
	uint32_t key_freq[KEY_NUM];
	uint8_t music_speed[SOUND_SPEED_NUM];

	/* try to read the audio file */
	read = read_dentry_by_name(fname, &dentry);
	if(read == SUCCESS) {

		/* check if this is a audio file */
		read = read_data(dentry.inode_idx, offset, sound_header, SOUND_HEADER_SIZE);
		if(read == 2 && sound_header[0] == 'S' && sound_header[1] == 'D') {

			/* get the length of the audio */
			offset += SOUND_DATA_NEXT;
			read_data(dentry.inode_idx, offset, sound_length, SOUND_LENGTH_SIZE);

			/* get the rtc frequency which will be used to determine the duration of each note */
			offset += SOUND_DATA_NEXT;
			read_data(dentry.inode_idx, offset, temp_freq, SOUND_FREQ_SIZE);

			/* convert the length and frequency to integers */
			length = ((int32_t)(sound_length[0] << HIGH_SHIFT | sound_length[1])) / SOUND_DATA_NEXT;
			music_rtc = (int32_t)(temp_freq[0] << HIGH_SHIFT | temp_freq[1]);

			/* read the speed data */
			offset += SOUND_DATA_NEXT;
			for(i = 0; i < SOUND_SPEED_NUM; i++) {
				read_data(dentry.inode_idx, offset, temp, SOUND_SPEED_NEXT);
				offset++; 
				music_speed[i] = (int32_t)temp[0];
			}

			/* read the frequency data */
			offset = SOUND_FREQ_OFFSET;
			key_freq[0] = 0;
			for(i = 1; i < KEY_NUM; i++) {
				read_data(dentry.inode_idx, offset, temp, SOUND_DATA_NEXT);
				offset += SOUND_DATA_NEXT; 
				key_freq[i] = (int32_t)(temp[0] << HIGH_SHIFT | temp[1]);
			}
			
			offset = SOUND_DATA_OFFSET;
			while(1) {
				
				/* read the audio data */
				if(length <= SOUND_DATA_SIZE) {
					stop_flag = 1;
					for(i = 0, count = 0; i < length; i++, count++) {
						read_data(dentry.inode_idx, offset, temp, SOUND_DATA_NEXT);
						offset += SOUND_DATA_NEXT; 
						sound[i] = temp[0];
						speed[i] = temp[1];
					}
				} else {
					stop_flag = 0;
					length -= SOUND_DATA_SIZE;
					for(i = 0, count = 0; i < SOUND_DATA_SIZE; i++, count++) {
						read_data(dentry.inode_idx, offset, temp, SOUND_DATA_NEXT);
						offset += SOUND_DATA_NEXT; 
						sound[i] = temp[0];
						speed[i] = temp[1];
					}
				}
				
				/* play the audio */
				for(i = 0; i < count; i++)
					sound_beep(key_freq[sound[i]], music_speed[speed[i]]);

				if(stop_flag)
					break;
			}

			/* restore RTC frequency */
			if(orignal_rtc != 0) {
				rtc_write(0, (uint8_t*)&orignal_rtc, 0);
				orignal_rtc = 0;
			}
			
			music_rtc = 0;

			return SUCCESS;
		}
		return FAILURE;
	}
	return FAILURE;
}
