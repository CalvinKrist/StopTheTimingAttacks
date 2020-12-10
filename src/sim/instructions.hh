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
	ATTACH = 8
};

typedef uint32_t(*InstructionFunc)(uint32_t, uint32_t);

extern InstructionFunc instructions[9];

#define UNUSED_INST_PARAM uint32_t
#define INST_COMMON_PARAMS ThreadContext* context

/**
 * \brief Creates a new thread representation. This should be called whenever a new thread
 *        in a new address space is created.
 * 
 * \note This should only be called from within the TCB. This is currently not enforced.
 * 
 * \return The thread identifier assigned to the new thread.
 */
uint32_t inst_CREATETHREAD(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);

/**
 * \brief Creates a new thread representation. This should be called whenever a thread
 *        in an existing address space is created. The newly created thread will begin at
 *        the given security level.
 * 
 * \note This should only be called from within the TCB. This is currently not enforced.
 * 
 * \param SID The security identifier with which the thread should be created.
 * 
 * \return The thread identifier assigned to the new thread.
 */
uint32_t inst_CREATETHREADWITHSID(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM);

/**
 * \brief Deletes an existing thread representation. The thread must not be the currently
 *        executing thread, and any further usage of any CS 6501 instructions will not
 *        work when using this thread's identifier (until such a time as it is reissued).
 * 
 * \note This should only be called from within the TCB. This is currently not enforced.
 * 
 * \param TID The thread identifier of the thread to be deleted.
 * 
 * \return -1 if the thread identifier was invalid,
 *         -2 if the thread ID was the current thread's ID,
 *         0 if the thread was successfully deleted
 */
uint32_t inst_DELETETHREAD(INST_COMMON_PARAMS, uint32_t TID, UNUSED_INST_PARAM);

/**
 * \brief Switches the current active thread to the thread referenced by the given 
 *        thread identifier. 
 * 
 * \note This should only be called from within the TCB. This is currently not enforced.
 * 
 * \param TID The identifier of the thread to set as the current thread.
 * 
 * \return -1 if the thread identifier was invalid,
 *		   0 if the switch was successful
 */
uint32_t inst_SWITCHTHREAD(INST_COMMON_PARAMS, uint32_t TID, UNUSED_INST_PARAM);

/**
 * \brief Lowers the current thread's security to a security level specified by the
 *        given security level identifier. Requires that the specified security level be
 *        lower than the current thread's security level. This pushes the current security
 *        level to the security level stack.
 * 
 * \param SID The security identifier to which the current thread's security level should 
 *            be lowered.
 * 
 * \return -1 if the security identifier was invalid,
 *         -2 if the security level was not lower than the current security level,
 *         the new security level if the security level was successfully lowered
 */
uint32_t inst_LOWERSL(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM);

/**
 * \brief Lowers the current thread's security to a new security level immediately lower
 *        than the current thread's level. This pushes the current security level to the 
 *        security level stack.
 *
 * \return -1 if an unspecified error occured,
 *         the new security level if the security level was successfully lowered
 */
uint32_t inst_LOWERNSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);

/**
 * \brief Raises the current thread's security to the security level at the top of the
 *        security level stack. Requires that at least one security level have been 
 *        pushed to the stack and not already popped by this function.
 *
 * \return -1 if an unspecified error occured,
 *         the new security level if the security level was successfully raised
 */
uint32_t inst_RAISESL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);

/**
 * \brief Raises the current thread's security to a new security level immediately higher
 *        than the current thread's level. This new security level is incomparable to all
 *        levels not the current level and not lower than the current level.
 *
 * \return -1 if an unspecified error occured,
 *         the new security level if the security level was successfully lowered
 */
uint32_t inst_RAISENSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM);

/**
 * \brief Specifies that one security level should be placed directly below another level.
 *        Requires that the level being placed below another be below the current security
 *        level and that the two levels for which a relationship is being defined be 
 *        incomparable.
 * 
 * \param attach_to The security level which is being placed above the other level
 * \param to_attach The security level which is being placed below the other level
 *  
 * \return -1 if a security identifier was invalid,
 *         -2 if to_attach was not lower than the current thread's security level,
 *         -4 if the two security levels already had a relationship,
 *         0 if the new relationship was successfully defined
 */
uint32_t inst_ATTACH(INST_COMMON_PARAMS, uint32_t attach_to, uint32_t to_attach);