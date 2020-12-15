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
	int _tmp;
	BEGIN_TIME();
	printf("Hell%llu, w%drld!\n", 0, 0);
	SAMPLE_CACHE_USAGE();
	END_TIME(&_tmp);

	char* buff = (char*)mmap(0, 64 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	char tmp = 0;
	uint64_t time;

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tCold start time: %llu cycles\n", GET_LEVEL(), time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tWarmed up time: %llu cycles\n", GET_LEVEL(), time);

	auto lower = NEW_LOWER();

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tCold lowered time: %llu\n", GET_LEVEL(), time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tWarmed up lowered time: %llu\n", GET_LEVEL(), time);

	LEVEL_POP();
	
	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tStill warm, but raised: %llu\n", GET_LEVEL(), time);

	NEW_RAISE();

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tStill warm, but raised again: %lld\n", GET_LEVEL(), time);

	NEW_LOWER();

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tLowered but incomparable (cold): %lld\n", GET_LEVEL(), time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
		SAMPLE_CACHE_USAGE();
	}
	END_TIME(&time);
	printf("%d\tLowered but incomparable (warmed up): %lld\n", GET_LEVEL(), time);
}
