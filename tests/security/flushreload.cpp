#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <cstdio>

#include "security_tools.h"

void mem_flush(const void* p, unsigned int allocation_size){
	const size_t cache_line = 64;
	const char* cp = (const char*) p;
	size_t i = 0;

	if(p == NULL || allocation_size <= 0)
		return;

	for(i = 0; i < allocation_size; i += cache_line){
		asm volatile("clflush (%0)\n\t"
					 :
		: "r"(&cp[i])
			: "memory");
	}

	asm volatile("sfence\n\t"
				 :
	:
		: "memory");
}

int probe(char* adrs){
	volatile unsigned long time;

	asm __volatile__(
		"    mfence             \n"
		"    lfence             \n"
		"    rdtsc              \n"
		"    lfence             \n"
		"    movl %%eax, %%esi  \n"
		"    movl (%1), %%eax   \n"
		"    lfence             \n"
		"    rdtsc              \n"
		"    subl %%esi, %%eax  \n"
		"    clflush 0(%1)      \n"
		: "=a" (time)
		: "c" (adrs)
		: "%esi", "%edx"
	);
	return time;
}

int main(int argc, char** argv){
	auto tid = CREATETHREAD();
	SWITCH_THREAD(tid);
	auto level = GET_LEVEL();

	bool useSecLevels = *argv[1] == '1';
	if(useSecLevels){
		printf("Security enabled\n");
	} else{
		printf("Security disabled\n");
	}

	char* buff = (char*) mmap(0, 64 * 256, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	uint64_t timings[256];
	memset(timings, 0, 256 * 8);
	char tmp = 0;
	short secret = 0x24;
	uint64_t time;

	for(int k = 0; k < 1000; k++){
		mem_flush(buff, 64 * 256);

		if(useSecLevels){
			NEW_RAISE();
		}
		buff[secret * 64] = 1; // secret dependent access!
		if(useSecLevels){
			LOWER(level);
		}

		int best_v = 0;
		for(int i = 0; i < 256; i++){
			timings[i] += probe(&buff[i * 64]);
		}
	}

	uint64_t best_time = 0xFFFFFFFFFFFFFFFF;
	int best_v = 0;
	for(int i = 0; i < 256; i++){
		if(timings[i] < best_time){
			best_time = timings[i];
			best_v = i;
		}
	}
	printf("Guess: %d (at %llu)\n", best_v, best_time);
}
