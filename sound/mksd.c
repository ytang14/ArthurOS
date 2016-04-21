#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mksd.h"

int main(void)
{
	FILE *sdfp;
	char* file = "new.sd";
	int offset, i, j, length;
	unsigned short audio_size;
	unsigned char byte_buffer[8];
	unsigned short word_buffer[88] = {
		  28,   29,   31,   33,   35,   37,   39,   41,
		  44,   46,   49,   52,   55,   58,	  62,   65,
		  69,   73,   78,   82,   87,   92,   98,  104,
		 110,  117,  123,  131,	 139,  147,  156,  165,
		 175,  185,  196,  208,  220,  233,  247,  262,
		 277,  294,	 311,  330,	 349,  370,  392,  415,
		 440,  466,  494,  523,  554,  587,  622,  659,
		 698,  740,  784,  831,  880,  932,  988, 1047,
		1109, 1175, 1245, 1319, 1397, 1480,	1568, 1661,
		1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637,
		2794, 2960, 3136, 3322,	3520, 3729, 3951, 4186
	};

	sdfp = fopen(file, "wb");

	byte_buffer[0] = 'S';
	byte_buffer[1] = 'D';
	offset = 0x00;
	fseek(sdfp, offset, SEEK_SET);
	fwrite(byte_buffer, sizeof(char), 2, sdfp);

	
	for(i = 0, j = 0; i < 8; i++) {
		byte_buffer[i] = j;
		if(j == 0)
			j = 1;
		else
			j = j * 2;
	}
	offset = 0x06;
	fseek(sdfp, offset, SEEK_SET);
	fwrite(byte_buffer, sizeof(char), 8, sdfp);

	offset = 0x10;
	fseek(sdfp, offset, SEEK_SET);
	for(i = 0; i <  88; i++) {
		byte_buffer[0] = (word_buffer[i] & 0xFF00) >> 8;
		byte_buffer[1] = (word_buffer[i] & 0x00FF);
		fwrite(byte_buffer, sizeof(char), 2, sdfp);
	}

	offset = 0x02;
	length = sizeof(note_data);
	audio_size = (unsigned short)(length * 2);
	fseek(sdfp, offset, SEEK_SET);
	byte_buffer[0] = (audio_size  & 0xFF00) >> 8;
	fwrite(byte_buffer, sizeof(char), 1, sdfp);
	byte_buffer[0] = (audio_size  & 0x00FF);
	fwrite(byte_buffer, sizeof(char), 1, sdfp);

	offset = 0x04;
	fseek(sdfp, offset, SEEK_SET);
	byte_buffer[0] = (rtc_freq  & 0xFF00) >> 8;
	fwrite(byte_buffer, sizeof(char), 1, sdfp);
	byte_buffer[0] = (rtc_freq  & 0x00FF);
	fwrite(byte_buffer, sizeof(char), 1, sdfp);
	
	offset = 0xC0;
	fseek(sdfp, offset, SEEK_SET);
	for(i = 0; i < length; i++) {
		fwrite(&note_data[i], sizeof(char), 1, sdfp);
		fwrite(&speed_data[i], sizeof(char), 1, sdfp);
	}
	
	return 0;
}
