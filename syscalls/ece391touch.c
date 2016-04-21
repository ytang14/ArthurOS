#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    int32_t fd, i, flag = 1;
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
    
    return 0;
}

