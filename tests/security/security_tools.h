#pragma once

#include "unistd.h"

enum class InstructionID {
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
    lseek(0x40000000 | static_cast<int>(InstructionID::CREATETHREAD), 0, 0)

#define CREATE_THREAD_WITH_SID(sid) \
    lseek(0x40000000 | static_cast<int>(InstructionID::CREATETHREADWITHSID), sid, 0)

#define DELETE_THREAD(tid) \
    lseek(0x40000000 | static_cast<int>(InstructionID::DELETETHREAD), tid, 0)

#define SWITCH_THREAD(tid) \
    lseek(0x40000000 | static_cast<int>(InstructionID::SWITCHTHREAD), tid, 0)

#define LOWER(level) \
    lseek(0x40000000 | static_cast<int>(InstructionID::LOWERSL), level, 0)

#define NEW_LOWER() \
    lseek(0x40000000 | static_cast<int>(InstructionID::LOWERNSL), 0, 0)

#define LEVEL_POP() \
    lseek(0x40000000 | static_cast<int>(InstructionID::RAISESL), 0, 0)

#define NEW_RAISE() \
    lseek(0x40000000 | static_cast<int>(InstructionID::RAISENSL), 0, 0)

#define ATTACH(attach_to, to_attach) \
    lseek(0x40000000 | static_cast<int>(InstructionID::ATTACH), attach_to, to_attach)

#define GET_LEVEL() \
    lseek(0x40000000 | static_cast<int>(InstructionID::GETLEVEL), 0, 0)

#define SAMPLE_CACHE_USAGE() \
    lseek(0x40000000 | static_cast<int>(InstructionID::GETLEVEL), 2, 0)

#define GET_CYCLES() \
    lseek(0x40000000 | static_cast<int>(InstructionID::GETLEVEL), 1, 0)

#define BEGIN_TIME()                       \
	{                                  \
		uint32_t __t1;             \
		__rdtscp(&__t1);           \
		auto __t = __rdtscp(&__t1);

#define END_TIME(ptr)                 \
	__rdtscp(&__t1);              \
        *ptr = __rdtscp(&__t1) - __t; \
    }
	
