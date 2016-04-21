#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int main ()
{
    int32_t cnt, fd, i, flag = 1;
    uint8_t buf[1024];

    if (0 != ece391_getargs (buf, 1024)) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
		return 3;
    }
    
    for (i = 0; i < 1024; i++) {
    	if(buf[i] != ' ' && buf[i] != '\0') {
    		flag = 0;
    		break;
    	}
    }
    
    if (flag) {
    	ece391_fdputs (1, (uint8_t*)"must input a file name\n");
		return 3;
    }
    
    if (-1 == (fd = ece391_open (buf, 1))) {
		return 2;
    }
    
    ece391_fdputs (1, (uint8_t*)"Input text you want to write to the file (press enter to finish):\n");
    
    if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
		ece391_fdputs (1, (uint8_t*)"read from keyboard failed\n");
		return 3;
	}
	
	if (cnt > 0 && '\n' == buf[cnt - 1])
		buf[cnt] = '\0';
	
	if (-1 == ece391_write (fd, buf, cnt)) {
		ece391_fdputs (1, (uint8_t*)"you are not allowed to modify this file\n");
		return 3;
	}
    
    return 0;
}

