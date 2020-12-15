#pragma once

#include "unistd.h"

enum InstructionID {
	CREATETHREAD = 0,
	CREATETHREADWITHSID = 1,
	DELETETHREAD = 2,
	SWITCHTHREAD = 3,
	LOWERSL = 4,
	LOWERNSL = 5,
	RAISESL = 6,
	RAISENSL = 7,
	ATTACH = 8,
	GETLEVEL = 9
};

#define CREATETHREAD() \
    lseek(0x40000000 | CREATETHREAD, 0, 0)

#define CREATE_THREAD_WITH_SID(sid) \
    lseek(0x40000000 | REATETHREADWITHSID, sid, 0)

#define DELETE_THREAD(tid) \
    lseek(0x40000000 | DELETETHREAD, tid, 0)

#define SWITCH_THREAD(tid) \
    lseek(0x40000000 | SWITCHTHREAD, tid, 0)

#define LOWER(level) \
    lseek(0x40000000 | LOWERSL, level, 0)

#define NEW_LOWER() \
    lseek(0x40000000 | LOWERNSL, 0, 0)

#define LEVEL_POP() \
    lseek(0x40000000 | RAISESL, 0, 0)

#define NEW_RAISE() \
    lseek(0x40000000 | RAISENSL, 0, 0)

#define ATTACH(attach_to, to_attach) \
    lseek(0x40000000 | ATTACH, attach_to, to_attach)

#define GET_LEVEL() \
    lseek(0x40000000 | GETLEVEL, 0, 0)

#define SAMPLE_CACHE_USAGE() \
    lseek(0x40000000 | GETLEVEL, 2, 0)

#define GET_CYCLES() \
    lseek(0x40000000 | GETLEVEL, 1, 0)

#define BEGIN_TIME()                       \
	{                                  \
		unsigned int t1 = 0;             \
		__rdtscp(&t1);           \
		auto t = __rdtscp(&t1);

#define END_TIME(ptr)                 \
	__rdtscp(&t1);              \
        *ptr = __rdtscp(&t1) - t; \
    }
