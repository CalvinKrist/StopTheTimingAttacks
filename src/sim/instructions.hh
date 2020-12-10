#pragma once

#include "cpu/thread_context.hh"

enum class InstructionID {
	CREATETHREAD = 0,
	CREATETHREADWITHSID = 1,
	DELETETHREAD = 2,
	SWITCHTHREAD = 3,
	LOWERSL = 4,
	LOWERNSL = 5,
	RAISESL = 6,
	RAISENSL = 7,
	ATTACH = 8;
};

typedef uint32_t(*InstructionFunc)(uint32_t, uint32_t);

extern InstructionFunc instructions[9];

#define UNUSED_INST_PARAM uint32_t
#define INST_COMMON_PARAMS ThreadContext* context

/**
 * Creates a new thread construct and creates for it a new security level. 
 * Returns the thread identifier created. This should only be called when 
 * the OS creates a new thread.
 */
uint32_t inst_CREATETHREAD(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);

/**
 * Creates a new thread construct. Returns the thread identifier created.
 * This should only be called when the OS creates a new thread.
 */
uint32_t inst_CREATETHREADWITHSID(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM);
uint32_t inst_DELETETHREAD(INST_COMMON_PARAMS, uint32_t TID, UNUSED_INST_PARAM);
uint32_t inst_SWITCHTHREAD(INST_COMMON_PARAMS, uint32_t TID, UNUSED_INST_PARAM);
uint32_t inst_LOWERSL(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM);
uint32_t inst_LOWERNSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);
uint32_t inst_RAISESL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);
uint32_t inst_RAISENSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);
uint32_t inst_ATTACH(INST_COMMON_PARAMS, uint32_t attach_to, uint32_t to_attach);
