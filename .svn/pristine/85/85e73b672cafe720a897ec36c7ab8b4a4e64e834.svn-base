/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Find the font for the Chinese character */

#include "modex.h"
#include "types.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "filesys.h"
#include "other.h"
#include "chinese.h"


uint32_t cn_font_pos = FONT_START;

/* Can store up to 64 Chinese characters without overlapping */
uint8_t cn_font_data3[FONT_TOTAL][FONT_HEIGHT];

/* get_cn_font
 *	 DESCRIPTION: Find the font data for the desired Chinese character
 *		  INPUTS: cn_word - holds the data of the Chinese character
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: use the ASCII 128-255 to store Chinese character font */
int32_t
get_cn_font(uint8_t cn_word[FONT_NUM])
{
	uint32_t cn_offset;
	uint8_t cn_buffer[FONT_SIZE];
	uint8_t* cn_font_file = (uint8_t*)"hzk16";
	int32_t read;
	int32_t i, j, found = 0;
	dentry_t dentry;

	/* calculate the offset of the font */
	cn_offset = (FONT_COUNT * (uint32_t)(cn_word[0] - FONT_OFFSET) + (uint32_t)(cn_word[1] - FONT_OFFSET)) * FONT_SIZE;

	read = read_dentry_by_name(cn_font_file, &dentry);

	if(read == SUCCESS) {
		read = read_data(dentry.inode_idx, cn_offset, cn_buffer, FONT_SIZE);

		if(read == FONT_SIZE) {
		
			for(i = 0; i < FONT_SIZE; i++) {
				if(cn_buffer[i] != 0) {
					found = 1;
					break;
				}
			}

			if(found) {
				/* English fonts are (8 * 16) in size, Chinese characters are (16 * 16),
				 * so we need to split the data read into two parts (left and right) */
				for(i = 0, j = FONT_LEFT; j < FONT_SIZE; j += FONT_NEXT, i++)
					cn_font_data3[cn_font_pos + FONT_LEFT - FONT_START][i] = cn_buffer[j];

				for(i = 0, j = FONT_RIGHT; j < FONT_SIZE; j += FONT_NEXT, i++)
					cn_font_data3[cn_font_pos + FONT_RIGHT - FONT_START][i] = cn_buffer[j];

				return SUCCESS;
			}
			return FAILURE;

		} else
			return FAILURE;
	}

	return FAILURE;
}
