/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27
 * 
 * Handles keyboard initialization and interrupts */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"


#define KB_IO_PORT			0x60
#define KB_IRQ				0x01

#define SCANCODE_SIZE		128

#define KEY_RELEASE			0x80
#define RELEASED_KEY		0x7F

#define KEY_TAB				0x0D
#define KEY_ENTER			0x1C
#define KEY_SPACE			0x39
#define KEY_BACKSPACE		0x0E
#define KEY_CAPS_LOCK		0x3A
#define KEY_UP_ARROW		0x48
#define KEY_DOWN_ARROW		0x50
#define KEY_LEFT_ARROW		0x4B
#define KEY_RIGHT_ARROW		0x4D
#define KEY_INSERT			0x52
#define KEY_DELETE			0x53
#define KEY_HOME			0x47
#define KEY_END				0x4F

#define KEY_L_SHIFT			0x2A
#define KEY_R_SHIFT			0x36
#define KEY_CTRL			0x1D
#define KEY_ALT				0x38

#define KEY_F1				0x3B
#define KEY_F2				0x3C
#define KEY_F3				0x3D

#define KEY_C				0x2E
#define KEY_F				0x21
#define KEY_L				0x26
#define KEY_M				0x32
#define KEY_P				0x19
#define KEY_R				0x13
#define KEY_T				0x14
#define KEY_X				0x2D

/* Meta Keys Stuff (Shift, Alt, Ctrl, Caps Lock) */
typedef struct {
	union {
		uint8_t value;
		struct {
			uint8_t caps_lock : 1;
			uint8_t shift : 1;
			uint8_t ctrl : 1;
			uint8_t alt : 1;
			uint8_t reserved : 4;
		} __attribute__((packed));
	};
} special_keys_t;

/* See function header for details */

/* Initialize keyboard */
void keyboard_init(void);

/* Keyboard handler */
void keyboard_handler(void);

#endif /* _KEYBOARD_H */
