/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/******************************************************************************/
/**
*
* @file xfih.h
* @{
* @details This file gives interface to fault injection hardening (FIH) macros
*          and functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 1.0   mmd  06/16/23 Initial release
* 1.1   sak  06/19/23 Added code comments
* 1.2   mmd  09/04/23 Fixed review comments, seperated FIH interface from core
*                     functionality
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XFIH_H
#define XFIH_H

/***************************** Include Files **********************************/
#include "xil_types.h"
#include "xfih_core.h"
#include "xfih_platform.h"

/************************* Macros / Inline Functions **************************/

/**
 * @brief Macro to check FIH variables for equality and enter block if either
 *        original or transformed variable equality check passes
 *
 * @details Fault injection macro for redundant comparison for equality using
 * FIH variable. It enters the block that is just next to this macro call, when
 * equality comparison of either original variables or transformed variables
 * passes.
 */
#define XFIH_IF_FAILIN_EQ(a, b) \
	XFIH_IF_FAILIN (a, ==, b)

/**
 * @brief Macro to check FIH variables for equality and enter block if both
 *        original and transformed variable equality check passes
 *
 * @details Fault injection macro for redundant comparison for equality using
 * FIH variable. It enters the block that is just next to this macro call, when
 * equality comparison of both original variables and transformed variables
 * passes.
 */
#define XFIH_IF_FAILOUT_EQ(a, b) \
	XFIH_IF_FAILOUT (a, ==, b)

/**
 * @brief Macro to check FIH variables for non equality and enter block if
 *        either original or transformed variable non equality check passes
 *
 * @details Fault injection macro for redundant comparison for non equality
 * using FIH variable. It enters the block that is just next to this macro
 * call, when non equality comparison of either original variables or
 * transformed variables passes.
 */
#define XFIH_IF_FAILIN_NEQ(a, b) \
	XFIH_IF_FAILIN (a, !=, b)

/**
 * @brief Macro to check FIH variables for non equality and enter block if both
 *        original and transformed variable non equality check passes
 *
 * @details Fault injection macro for redundant comparison for non equality
 * using FIH variable. It enters the block that is just next to this macro
 * call, when non equality comparison of both original variables and
 * transformed variables passes.
 */
#define XFIH_IF_FAILOUT_NEQ(a, b) \
	XFIH_IF_FAILOUT (a, !=, b)

/**
 * @brief Macro to check FIH variables for equality and jumps to given label if
 *        either original or transformed variable equality check passes. The
 *        macro provides glitch protection for goto and has trap to execute
 *        lockdown on glitch to unconditional branch (goto).
 */
#define XFIH_IF_FAILIN_EQ_GOTO_LABEL(a, b, Label) \
	do {\
		XFIH_IF_FAILIN (a, ==, b) { \
			XFIH_GOTO(Label); \
		} \
	} while (0)

/**
 * @brief Macro to check FIH variables for equality and jumps to given label if
 *        both original or transformed variable equality check passes. The
 *        macro provides glitch protection for goto and has trap to execute
 *        lockdown on glitch to unconditional branch (goto).
 */
#define XFIH_IF_FAILOUT_EQ_GOTO_LABEL(a, b, Label) \
	do {\
		XFIH_IF_FAILOUT (a, ==, b) { \
			XFIH_GOTO(Label); \
		} \
	}  while (0)

/**
 * @brief Macro to check FIH variables for non-equality and jumps to given label
 *        if either original or transformed variable equality check passes. The
 *        macro provides glitch protection for goto and has trap to execute
 *        secure lockdown on glitch to unconditional branch (goto).
 */
#define XFIH_IF_FAILIN_NEQ_GOTO_LABEL(a, b, Label) \
	do {\
		XFIH_IF_FAILIN (a, !=, b) { \
			XFIH_GOTO(Label); \
		} \
	} while (0)

/**
 * @brief Macro to check FIH variables for non-equality and jumps to given label
 *        if both original or transformed variable equality check passes. This
 *        macro provides glitch protection for goto and has trap to execute
 *        secure lockdown on glitch to unconditional branch (goto).
 */
#define XFIH_IF_FAILOUT_NEQ_GOTO_LABEL(a, b, Label) \
	do {\
		XFIH_IF_FAILOUT (a, !=, b) { \
			XFIH_GOTO(Label); \
		} \
	} while (0)

/**
 * @brief Macro to check FIH variables not holding success value. It jumps to
 *        given label if either original or transformed variable do not hold
 *        success value.This macro provides glitch protection for goto and has
 *        trap to execute secure lockdown on glitch to unconditional branch
 *        (goto).
 */
#define XFIH_IF_FAILURE_GOTO_LABEL(a, Label) \
	do {\
		XFIH_LABEL("XFIH_IF_FAILURE_GOTO_LABEL_START"); \
		XFIH_IF_FAILIN(a, !=, XFIH_SUCCESS) { \
			XFIH_GOTO(Label); \
		} \
		XFIH_LABEL("XFIH_IF_FAILURE_GOTO_LABEL_END"); \
	} while (0)

/**
 * @brief Fault injection function call macro It updates the Status variable with
 *        original value if either original or transformed variable do not hold
 *        success value.
 */
#define XFIH_CALL(Func, FihVar, Status, ...) \
	do { \
		XFIH_LABEL("XFIH_CALL_START"); \
		FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE); \
		FihVar = XFih_VolatileAssignS32(Func(__VA_ARGS__)); \
		Status = XFih_GetVal(FihVar); \
		XFIH_IF_FAILIN(FihVar, !=, XFIH_SUCCESS) { \
			Status = XFih_GetVal(FihVar); \
		} \
		XFIH_LABEL("XFIH_CALL_END"); \
	} while (0)

/**
 * @brief Fault injection function call macro It jumps to
 *        given label if either original or transformed variable do not hold
 *        success value.
 */
#define XFIH_CALL_GOTO(Func, FihVar, Status, Label, ...) \
	do { \
		XFIH_LABEL("XFIH_CALL_START"); \
		FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE); \
		FihVar = XFih_VolatileAssignS32(Func(__VA_ARGS__)); \
		Status = XFih_GetVal(FihVar); \
		XFIH_IF_FAILURE_GOTO_LABEL(FihVar, Label);\
		XFIH_LABEL("XFIH_CALL_END"); \
	} while (0)

/**
 * @brief Fault injection function call macro It updates the Status variable with
 *        given error if either original or transformed variable do not hold
 *        success value.
 */
#define XFIH_CALL_WITH_SPECIFIC_ERROR(Func, Error, FihVar, Status, ...) \
	do { \
		XFIH_LABEL("XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR_START"); \
		FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE); \
		FihVar = XFih_VolatileAssignS32(Func(__VA_ARGS__)); \
		Status = XFih_GetVal(FihVar); \
		XFIH_IF_FAILIN(FihVar, !=, XFIH_SUCCESS) { \
			Status = Error; \
		} \
		XFIH_LABEL("XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR_END"); \
	} while (0)

/**
 * @brief Fault injection function call macro It updates the Status variable with
 *        given error and jumps to a given label if either original or transformed
 *        variable do not hold success value.
 */
#define XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Func, Error, FihVar, Status, Label, ...) \
	do { \
		XFIH_LABEL("XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR_START"); \
		FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE); \
		FihVar = XFih_VolatileAssignS32(Func(__VA_ARGS__)); \
		Status = XFih_GetVal(FihVar); \
		XFIH_IF_FAILIN(FihVar, !=, XFIH_SUCCESS) { \
			Status = Error; \
			XFIH_GOTO(Label); \
		} \
		XFIH_LABEL("XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR_END"); \
	} while (0)

/**
 * @brief Fault injection function call macro with lockdown on failure
 */
#define XFIH_CALL_ON_ERROR_LOCKDOWN(Func, Error, ReturnVal, ...) \
	do { \
		XFIH_LABEL("XFIH_CALL_WITH_ON_ERROR_LOCKDOWN_START"); \
		ReturnVal = XFih_VolatileAssignXfihVar(XFIH_FAILURE); \
		ReturnVal = Func(__VA_ARGS__); \
		XFIH_IF_FAILIN(ReturnVal, !=, XFIH_SUCCESS) { \
			XFIH_CALLBACK_BEFORE_LOCKDOWN(Error); \
			XFIH_TRIGGER_LOCKDOWN; \
		} \
		XFIH_LABEL("XFIH_CALL_WITH_ON_ERROR_LOCKDOWN_END"); \
	} while (0)

/******************************************************************************/
/**
 *
 * @brief Macro to define CFI variable.
 *
 * @note Use this macro to define CFI Counter variable. This defines CFI
 *       variable only when CFI is enabled (@ref XFIH_ENABLE_CFI)
 ******************************************************************************/
#define XFIH_CFI_DEFINE_VAR(VarName)	XFIH_CFI_CORE_DEFINE_VAR(VarName)

/******************************************************************************/
/**
 *
 * @brief Function to increment the CFI counter
 *
 * @param[in]	Pointer to CFI counter
 * @param[in]	Value to be added to CFI counter
 *
 * @return	None
 ******************************************************************************/
#define XFIH_CFI_STEP_UP(XFihCfiCounter, Steps) \
	XFih_CfiStepUp(&XFihCfiCounter, Steps)

/******************************************************************************/
/**
 *
 * @brief Macro to check control flow integrity variable. If integrity check
 *        fails, this function triggers secure lock-down using
 *        @ref XFIH_TRIGGER_LOCKDOWN
 *
 * @param[in]	Pointer to CFI counter
 * @param[in]	ExpectedCounterValue expected global counter value
 *
 * @return	None
 ******************************************************************************/
#define XFIH_CFI_CHECK(XFihCfiCounter, ExpectedCounterValue) \
	XFih_CfiCheck(&XFihCfiCounter, ExpectedCounterValue)

/******************************************************************************/
/**
 *
 * @brief Macro to reset CFI Counter
 *
 * @param[in] CFI counter
 *
 * @return	None
 ******************************************************************************/
#define XFIH_CFI_RESET_COUNTER(CfiCounter) \
	XFih_CfiResetCounter(&CfiCounter)

/**
 * @brief Fault injection function call macro with CFI
 */
#define XFIH_CALL_WITH_CFI(Func, CfiCounter, ExpectedCfiIncrement, ReturnVal, ...) \
	do { \
		XFIH_CFI_PRE_CALL_SETUP(CfiCounter); \
		ReturnVal = XFih_VolatileAssignXfihVar(XFIH_FAILURE); \
		ReturnVal = Func(__VA_ARGS__); \
		XFIH_CFI_POST_CALL_CHECK(CfiCounter, ExpectedCfiIncrement); \
	} while (0)

/**
 * @brief Variable holding the success value. Should be used wherever
 *        temporal redundancy is required. The variable type used for holding
 *        success value is decided by @ref XFIH_ENABLE_SECURE_CHECK.
 */
extern XFih_Var XFIH_SUCCESS;

/**
 * @brief Variable holding the failure value. Should be used wherever
 *        temporal redundancy is required. The variable type used for holding
 *        faliure value is decided by @ref XFIH_ENABLE_SECURE_CHECK.
 */
extern XFih_Var XFIH_FAILURE;

/******************************************************************************/
/**
 *
 * @brief This function is used for non-volatile XFih variable assignment without
 *        getting optimized by the compiler. It takes s32 value as input, and
 *        assigns it to @ref XFih_Var
 *
 * @param Val input u32 value to be assigned to @ref XFih_Var
 *
 * @return @ref XFih_Var assigned with input value
 *
 * @note  Usage: x is Xfih_Var type, y is s32 type
 *           x = XFih_VolatileAssignS32(y);
 *           Status = XFih_VolatileAssignS32(XST_FAILURE);
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
XFih_Var XFih_VolatileAssignS32(s32 Val)
{
	XFih_Var ReturnVal;

	XFIH_CORE_VOLATILE_ASSIGN_U32(ReturnVal, Val);

	return ReturnVal;
}

/******************************************************************************/
/**
 *
 * @brief This function is used for non-volatile XFih variable assignment without
 *        getting optimized by the compiler. It takes @ref XFih_Var value as input,
 *        and returns @ref XFih_Var value for assignment
 *
 * @param FihVal Input FihVal value to be assigned to @ref XFih_Var
 *
 * @return @ref XFih_Var assigned with input value
 *
 * @note  Usage: x and y both are XFih_Var type
 *           x = XFih_VolatileAssignXfihVar(y);
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
XFih_Var XFih_VolatileAssignXfihVar(XFih_Var FihVal)
{
	XFih_Var ReturnVal;

	XFIH_CORE_VOLATILE_ASSIGN_XFIH_VAR(ReturnVal, FihVal);

	return ReturnVal;
}


/******************************************************************************/
/**
 *
 * @brief This function is used for non-volatile s32 variable assignment.
 *        The s32 value is assigned to another s32 value. Note that this will
 *        not work if you are assigning back to back same values to same
 *        variable.
 *
 * @param Val Input s32 value to be assigned to s32 varaible
 *
 * @return s32 value for assignment
 *
 * @note  Usage: x and y are of type s32
 *           x = XFih_VolatileAssign(y);
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
s32 XFih_VolatileAssign(s32 Val)
{
	s32 ReturnVal;

	XFIH_PLAT_VOLATILE_ASSIGNMENT(ReturnVal, Val);

	return ReturnVal;
}

/******************************************************************************/
/**
 *
 * @brief Function to get value from temporal redundant variable
 *
 * @param[in]	FihVar - @ref XFih_Var variable to read from
 *
 * @return
 * -	extracted u32 value
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XFih_GetVal(XFih_Var FihVar)
{
	u32 Val;

	XFIH_CORE_GET_VAL(FihVar, Val);

	return Val;
}

/******************************************************************************/
/**
 *
 * @brief Function initializes the fault injection hardened variables to holding
 *        success and failure values.
 *
 * @return	None
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XFih_Init (void)
{
	XFIH_SUCCESS = XFih_VolatileAssignS32(XFIH_SUCCESS_VAL);
	XFIH_FAILURE = XFih_VolatileAssignS32(XFIH_FAILURE_VAL);
}

#endif /* XFIH_H */
/** @} */
