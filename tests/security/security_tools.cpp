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
	uint64_t time, time2;

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time2);
	printf("First time took %lld cycles; second took %lld (second should be much faster)\n", time, time2);

	auto lower = NEW_LOWER();

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Warmed up took %llu cycles; lower level took %llu (lower should be much slower)\n", time2, time);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time2);
	printf("First time at lower took %llu cycles; second took %llu (second should be much faster)\n", time, time2);

	LEVEL_POP();
	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time);
	printf("Warmed up lower took %llu cycles; back to higher level took %llu (both should be fast)\n", time2, time);

	LOWER(lower);

	BEGIN_TIME();
	for(int i = 0; i < 64 * 1024; i += 64){
		tmp = buff[i];
	}
	END_TIME(&time2);
	printf("Lower again took %llu cycles; higher took %llu (both should be fast)\n", time2, time);
}