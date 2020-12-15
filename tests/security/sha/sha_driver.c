/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"

#include "../security_tools_gcc.h"

const int UMD = 0;
const int UMS  = 1;
int mode = 0; // 0= UMD 1=UMS

int main(int argc, char **argv)
{
	SWITCH_THREAD(CREATETHREAD());

    FILE *fin;

    mode = *argv[1] - 0x30;
	if(mode == UMD)
		printf("Testing UMD\n");
	if(mode == UMS)
		printf("Testing UMS\n");
   
	int level = (int) GET_LEVEL();
	for(int i = 0; i < 10; i++) {
		SAMPLE_CACHE_USAGE();
		fin = fopen(argv[2], "rb");
		SHA_INFO sha_info;
    	if(mode == UMD)
    		NEW_RAISE(); // Create new higher level
		sha_stream(&sha_info, fin);
		sha_print(&sha_info);
		fclose(fin);
		if(mode == UMD)
			LOWER(level); // Push higher level to stack, return back to the lower level.
	}
		SAMPLE_CACHE_USAGE();
    return(0);
}
