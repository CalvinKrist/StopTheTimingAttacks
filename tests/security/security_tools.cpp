#include <x86intrin.h>
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

int main(){
	auto tid = CREATETHREAD();
	SWITCH_THREAD(tid);

	char* buff = (char*)mmap(0, 64 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	char tmp = 0;
	uint64_t time;

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Cold start time: %llu cycles\n", time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Warmed up time: %llu cycles\n", time);

	auto lower = NEW_LOWER();

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Cold lowered time: %llu\n", time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Warmed up lowered time: %llu\n", time);

	LEVEL_POP();
	
	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Still warm, but raised: %llu\n", time);

	int level = GET_LEVEL();
	NEW_RAISE();

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Still warm, but raised again: %lld\n", time);

	NEW_LOWER();

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Lowered but incomparable (cold): %lld\n", time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Lowered but incomparable (warmed up): %lld\n", time);
}