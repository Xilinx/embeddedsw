/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/******************************************************************************/
/**
*
* @file xfih.h
* @{
* @details This file implements core functionality of fault injection hardening
*          (FIH) macros/inline functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 1.0   mmd  09/04/23 Initial version
*
* </pre>
*
* @note
*
*******************************************************************************/

#ifndef XFIH_CORE_H
#define XFIH_CORE_H

/***************************** Include Files **********************************/
#include "xil_types.h"
#include "xfih_config.h"
#include "xfih_platform.h"

/************************* Macros / Inline Functions **************************/

/**
 * @brief FIH value for Success
 *
 * @note at a hamming distance of 18 from @ref XFIH_FAILURE_VAL
 */
#define XFIH_SUCCESS_VAL		0x0U /**< TODO: Updated success value
					to 0 from 0x46E56A7CU. Need to check */

/**
 * @brief FIH value for Failure
 *
 * @note at a hamming distance of 18 from @ref XFIH_SUCCESS_VAL
 */
#define XFIH_FAILURE_VAL		0x973AFB51U

/******************************************************************************/
/******************************************************************************/
#if (XFIH_ENABLE_SECURE_CHECK == FALSE)

/**
 * @brief FIH variable declared as u32 type when fault injection protection
 *        is disabled (@ref XFIH_ENABLE_SECURE_CHECK)
 */
typedef u32 XFih_Var;

/**
 * @brief Macro for Non FIH variable comparison. Enters block next to macro if
 *        comparison passes.
 *
 * @note Used to disable FIH variable comparison for fall-in when secure check
 *       is disabled (@ref XFIH_ENABLE_SECURE_CHECK).
 */
#define XFIH_IF_FAILIN(a, condition, b) \
	if (a condition b)

/**
 * @brief Macro for Non FIH variable comparison. Enters block next to macro if
 *        comparison passes.
 *
 * @note Used to disable FIH variable comparison for fall-out when secure check
 *       is disabled (@ref XFIH_ENABLE_SECURE_CHECK).
 */
#define XFIH_IF_FAILOUT(a, condition, b) \
	if (a condition b)

/**
 * @brief Dummy macro for disabling lockdown on detection of instruction glitch
 *        when secure check is disabled (@ref XFIH_ENABLE_SECURE_CHECK).
 */
#define XFIH_TRIGGER_LOCKDOWN	do{ }while(0)

/**
 * @brief Macro to disable the FIH goto statement when secure check is disabled
 *        (@ref XFIH_ENABLE_SECURE_CHECK).
 */
#define XFIH_GOTO(Label) \
	goto Label

/**
 * @brief Macro to perform volatile assignment of variable when secure check
 *        is disabled (@ref XFIH_ENABLE_SECURE_CHECK).
 */
#define XFIH_CORE_VOLATILE_ASSIGN_XFIH_VAR(FihVar, FihVal) \
	do { \
		XFIH_PLAT_VOLATILE_ASSIGNMENT(FihVar, FihVal); \
	} while(0)

/**
 * @brief Macro for FIH variable assignment without compiler optimization
 */
#define XFIH_CORE_VOLATILE_ASSIGN_U32(FihVar, Val) \
	do { \
		XFIH_PLAT_VOLATILE_ASSIGNMENT(FihVar, Val); \
	} while(0)

/**
 * @brief Macro to read the value from non FIH variable when secure check is
 *        disabled (@ref XFIH_ENABLE_SECURE_CHECK).
 */
#define XFIH_CORE_GET_VAL(FihVar, Val) \
	do { \
		Val = FihVar; \
	} while(0)

#else	/* (XFIH_ENABLE_SECURE_CHECK == FALSE) */

/**
 * @brief Mask used for generating transformed redundant value for FIH variable
 *        (@ref XFih_Var)
 */
#define XFIH_MASK			0xB0551DEAU

/**
 * @brief FIH variable that stores original value and its transformed value.
 *        Masking is used for transformation. Mask is defined by (@ref XFIH_MASK)
 */
typedef struct _XFih_Var {
	u32 Val;
	u32 TransformedVal;
} XFih_Var;

/**
 * @brief Macro to execute the next block if one of the below conditional check
 *        results to TRUE
 *        1. Comparison of FIH non-transformed variables
 *        2. Comparison of FIH transformed variables
 *
 * @note Validation of FIH variables depends on @ref XFIH_ENABLE_VAR_GLITCH_DETECTION
 */
#define XFIH_IF_FAILIN(a, condition, b) \
	XFIH_VALIDATE_VARIABLE(a); \
	XFIH_VALIDATE_VARIABLE(b); \
	if ((a.Val condition b.Val) || ((a.TransformedVal ^ XFIH_MASK) condition (b.TransformedVal ^ XFIH_MASK)))

/**
 * @brief Macro to execute the next block if both below conditional check
 *        results to TRUE
 *        1. Comparison of FIH non-transformed variables
 *        2. Comparison of FIH transformed variables
 *
 * @note
 * 		- Validation of input FIH variables depends on
 *        @ref XFIH_ENABLE_VAR_GLITCH_DETECTION
 */
#define XFIH_IF_FAILOUT(a, condition, b) \
	XFIH_VALIDATE_VARIABLE(a); \
	XFIH_VALIDATE_VARIABLE(b); \
	if ((a.Val condition b.Val) && ((a.TransformedVal ^ XFIH_MASK) condition (b.TransformedVal ^ XFIH_MASK)))

/**
 * @brief Helper macro to trigger secure lockdown using illegal instruction
 *        exception. This provided redundancy by adding additional illegal
 *        instruction exception.
 */
#define XFIH_TRIGGER_LOCKDOWN \
	do { \
		XFIH_PLAT_ILLEGAL_INSTRUCTION_TRAP; \
		XFIH_PLAT_ILLEGAL_INSTRUCTION_TRAP; \
	} while(0)

/**
 * @brief Macro for unconditional branch (goto) with illegal instruction
 *        trap for handling glitch to unconditional branch instruction
 */
#define XFIH_GOTO(Label) \
	do { \
		XFIH_PLAT_UNCONDITIONAL_BRANCH(Label); \
		XFIH_PLAT_ILLEGAL_INSTRUCTION_TRAP; \
	} while(0)

/**
 * @brief Macro for FIH variable assignment without compiler optimization
 */
#define XFIH_CORE_VOLATILE_ASSIGN_XFIH_VAR(FihVar, FihVal) \
	do { \
		XFIH_PLAT_VOLATILE_ASSIGNMENT(FihVar.Val, FihVal.Val); \
		XFIH_PLAT_VOLATILE_ASSIGNMENT(FihVar.TransformedVal, FihVal.TransformedVal); \
	} while(0)

/**
 * @brief Macro for FIH variable assignment without compiler optimization
 */
#define XFIH_CORE_VOLATILE_ASSIGN_U32(FihVar, Val) \
	do { \
		XFIH_PLAT_VOLATILE_ASSIGNMENT(FihVar.Val, Val); \
		XFIH_PLAT_VOLATILE_ASSIGNMENT(FihVar.TransformedVal, Val ^ XFIH_MASK); \
	} while(0)

/**
 * @brief Macro for reading 32-bit value from FIH variable
 */
#define XFIH_CORE_GET_VAL(FihVar, Val) \
	do { \
		Val = FihVar.Val; \
	} while(0)

#endif /* (XFIH_ENABLE_SECURE_CHECK == FALSE) */

/******************************************************************************/
/******************************************************************************/
#if (XFIH_ENABLE_VAR_GLITCH_DETECTION == FALSE)

/**
 * @brief Macro that provides flexibility to enable or disable integrity
 *        validation of FIH variables during comparison
 */
#define XFIH_VALIDATE_VARIABLE(FihVar)

#else /* (XFIH_ENABLE_VAR_GLITCH_DETECTION == FALSE) */

/******************************************************************************/
/**
 *
 * @brief Validates integrity of FIH variable
 *
 * @param[in] FihVar - Temporal redundant variable to be validated
 *                     (@ref XFih_Var)
 *
 * @return	None
 ******************************************************************************/
void XFih_Validate(XFih_Var FihVar);

/**
 * @brief Macro that provides flexibility to enable or disable FIH variable
 *        validation based on @ref XFIH_ENABLE_SECURE_CHECK
 */
#define XFIH_VALIDATE_VARIABLE(FihVar)	XFih_Validate(FihVar)

#endif /* (XFIH_ENABLE_VAR_GLITCH_DETECTION == FALSE) */

/******************************************************************************/
/******************************************************************************/

/**
 * @brief Macro to insert label when assembly code is generated.
 *        This label helps to locate the code using this macro during assembly
 *        analysis
 */
#define XFIH_LABEL(LableName) \
	__asm volatile ("XFIH_LABEL_" LableName "_%=:" ::)

/******************************************************************************/
/******************************************************************************/
#if (XFIH_ENABLE_CFI == FALSE)

/**
 * @brief Dummy macros to disable CFI
 */
#define XFIH_CFI_CORE_DEFINE_VAR(VarName)

#define XFIH_CFI_PRE_CALL_SETUP(CfiCounter) \
	do { } while(0)

#define XFIH_CFI_POST_CALL_CHECK(CfiCounter, ExpectedCfiCounterIncrement) \
	do { } while(0)

#define XFih_CfiStepUp(CfiCounter, Steps) \
	do { } while(0)

#define XFih_CfiCheck(CfiCounter, ExpectedCounterValue) \
	do { } while(0)

#define XFih_CfiResetCounter(CfiCounter) \
	do { } while(0)

#else /* (XFIH_ENABLE_CFI == FALSE) */

#define XFIH_CFI_CORE_DEFINE_VAR(VarName)	u32 VarName

/******************************************************************************/
/**
 *
 * @brief Function to increment the CFI counter
 *
 * @param[in] CfiCounter - Pointer to CFI counter
 * @param[in] Steps - Value to be added to CFI counter
 *
 * @return	None
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XFih_CfiStepUp(u32 *CfiCounter, u32 Steps)
{
	*CfiCounter += Steps;
}

/******************************************************************************/
/**
 *
 * @brief Function to check control flow integrity variable. If integrity check
 *        fails, this function triggers lockdown using @ref XFIH_TRIGGER_LOCKDOWN
 *
 * @param[in] XFihCfiCounter - CFI counter
 * @param[in] ExpectedCounterValue - Value for comparison
 *
 * @return	None
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XFih_CfiCheck(u32 XFihCfiCounter, u32 ExpectedCounterValue)
{
	if (XFihCfiCounter != ExpectedCounterValue) {
		XFIH_TRIGGER_LOCKDOWN;
	}
}

/******************************************************************************/
/**
 *
 * @brief Function to reset CFI Counter
 *
 * @param[in] XFihCfiCounter - Pointer to CFI counter
 *
 * @return	None
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XFih_CfiResetCounter(u32 *XFihCfiCounter)
{
	*XFihCfiCounter = 0U;
}

/**
 * @brief Macro to add label and storing the control flow integrity counter in
 * local variable
 */
#define XFIH_CFI_PRE_CALL_SETUP(CfiCounter) \
	XFIH_LABEL("XFIH_CALL_START"); \
	u32 XOldCfiCounterVal = CfiCounter

/**
 * @brief Macro to perform control flow integrity check
 */
#define XFIH_CFI_POST_CALL_CHECK(CfiCounter, ExpectedCfiCounterIncrement) \
	XFih_CfiCheck(CfiCounter, XOldCfiCounterVal + ExpectedCfiCounterIncrement); \
	XFIH_LABEL("XFIH_CALL_END")

#endif /* (XFIH_ENABLE_CFI == FALSE) */

#endif /* XFIH_CORE_H */
/** @} */
