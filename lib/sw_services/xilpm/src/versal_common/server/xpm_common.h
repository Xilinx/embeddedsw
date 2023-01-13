/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_COMMON_H_
#define XPM_COMMON_H_

#include "xstatus.h"
#include "xil_io.h"
#include "xil_util.h"
#include "xpm_err.h"
#include "xplmi_debug.h"
#include "xpm_common_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This macro defines "always false" value which is of boolean type.
 * The purpose of macro is to have boolean value which can be used
 * at loop condition instead of "0U" which is non-boolean value.
 */
#define XPM_FALSE_COND		(0U != 0U)

/**
 * GCC Specific attribute to suppress unused variable/function warning
 */
#ifndef maybe_unused
#define maybe_unused __attribute__((unused))
#endif

#define XPM_ALERT_VAL	0x10U
#define XPM_ERR_VAL	0x20U
#define XPM_WARN_VAL	0x30U
#define XPM_INFO_VAL	0x40U
#define XPM_DBG_VAL	0x50U

#define XPM_ALERT   (DEBUG_GENERAL  | XPM_ALERT_VAL)
#define XPM_ERR     (DEBUG_GENERAL  | XPM_ERR_VAL)
#define XPM_WARN    (DEBUG_GENERAL  | XPM_WARN_VAL)
#define XPM_INFO    (DEBUG_INFO     | XPM_INFO_VAL)
#define XPM_DBG     (DEBUG_DETAILED | XPM_DBG_VAL)

#define XPM_DEBUG_MASK	0x70U
#define XPM_DEBUG_SHIFT	4U

/**
 * Common baseline macro to print debug logs
 */

void XPm_Printf(u32 DebugType, const char *Fnstr, const char8 *Ctrl1, ...);

#define PmPrintCommon(DbgLevel, ...)					\
	do {								\
		if (((DbgLevel) & (XPlmiDbgCurrentTypes)) != (u8)FALSE) {\
			XPm_Printf(DbgLevel, __func__,  __VA_ARGS__);\
		}\
	} while (XPM_FALSE_COND)

/* Debug logs */
#define PmAlert(...)	PmPrintCommon(XPM_ALERT, __VA_ARGS__)
#define PmErr(...)	PmPrintCommon(XPM_ERR, __VA_ARGS__)
#define PmWarn(...)	PmPrintCommon(XPM_WARN, __VA_ARGS__)
#define PmInfo(...)	PmPrintCommon(XPM_INFO, __VA_ARGS__)
#define PmDbg(...)	PmPrintCommon(XPM_DBG, __VA_ARGS__)

#ifndef MIO_FLUSH_DEBUG
#define MIO_FLUSH_DEBUG DEBUG_INFO
#endif

#ifdef DEBUG_REG_IO

#define PmIn32(ADDR, VAL)					\
	do {							\
		(VAL) = XPm_In32(ADDR);				\
		PmInfo("RD: 0x%08X -> 0x%08X\r\n", ADDR, VAL);	\
	} while (XPM_FALSE_COND)

#define PmOut32(ADDR, VAL)					\
	do {							\
		PmInfo("WR: 0x%08X <- 0x%08X\r\n", ADDR, VAL);	\
		XPm_Out32(ADDR, VAL);				\
	} while (XPM_FALSE_COND)

#define PmRmw32(ADDR, MASK, VAL)				\
	do {							\
		XPm_RMW32(ADDR, MASK, VAL);			\
		PmInfo("RMW: Addr=0x%08X, Mask=0x%08X, Val=0x%08X, Reg=0x%08X\r\n", \
			ADDR, MASK, VAL, XPm_In32(ADDR));	\
	} while (XPM_FALSE_COND)								\

#define PmIn64(ADDR, VAL)					\
	do {							\
		(VAL) = XPm_In64(ADDR);				\
		PmInfo("RD: 0x%016X -> 0x%08X\r\n", ADDR, VAL);	\
	} while (XPM_FALSE_COND)								\

#define PmOut64(ADDR, VAL)					\
	do {							\
		PmInfo("WR: 0x%016X <- 0x%08X\r\n", ADDR, VAL);	\
		XPm_Out64(ADDR, VAL);				\
	} while (XPM_FALSE_COND)								\

#define PmRmw64(ADDR, MASK, VAL)				\
	do {							\
		XPm_RMW64(ADDR, MASK, VAL);			\
		PmInfo("RMW: Addr=0x%016X, Mask=0x%08X, Val=0x%08X, Reg=0x%08X\r\n", \
			ADDR, MASK, VAL, XPm_In64(ADDR));	\
	} while (XPM_FALSE_COND)								\

#else

#define PmIn32(ADDR, VAL)		((VAL) = XPm_In32(ADDR))

#define PmOut32(ADDR, VAL)		XPm_Out32(ADDR, VAL)

#define PmRmw32(ADDR, MASK, VAL)	XPm_RMW32(ADDR, MASK, VAL)

#define PmIn64(ADDR, VAL)		((VAL) = XPm_In64(ADDR))

#define PmOut64(ADDR, VAL)		XPm_Out64(ADDR, VAL)

#define PmRmw64(ADDR, MASK, VAL)	XPm_RMW64(ADDR, MASK, VAL)

#endif

#define BIT(n)					(1U << (n))
#define BIT8(n)					((u8)1U << (n))
#define BIT16(n)				((u16)1U << (n))
#define BIT32(n)				((u32)1U << (n))
// set the first n bits to 1, rest to 0
#define BITMASK(n)				(u32)((1ULL << (n)) - 1ULL)
// set width specified bits at offset to 1, rest to 0
#define BITNMASK(offset, width) 		(BITMASK(width) << (offset))

#define ARRAY_SIZE(x)				(sizeof(x) / sizeof((x)[0]))

#define MIN(x, y)				(((x) < (y)) ? (x) : (y))

#define XPm_Read32				XPm_In32
#define XPm_Write32				XPm_Out32

#define PLATFORM_VERSION_SILICON		(0x0U)
#define PLATFORM_VERSION_SPP			(0x1U)
#define PLATFORM_VERSION_EMU			(0x2U)
#define PLATFORM_VERSION_QEMU			(0x3U)
#define PLATFORM_VERSION_FCV			(0x4U)

/*
 * SSIT forwarding API supports CmdType. In case of certain APIs which do not
 * pass this argument this macro can be passed in place.
 */
#define NO_HEADER_CMDTYPE     0xFFU

/**
 * Adds redundancy while comparing the return value of called function.
 * based on NULL == RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to strictly check for && as per the
 * implementation.
 *
 * @RET1        Variable to store the return value of function to be execute
 *              at at RHS during comparison.
 * @TYPE        Type of pointer which returns by the function to be execute at
 *              RHS during comparison.
 * @FUNC        The function to be execute at RHS during comparison.
 * @param       Other params are arguments to the called function
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_STRICT_CHECK_IF_NULL(STSTMP, RET1, TYPE, FUNC, ...) \
	({                                                      \
		volatile const TYPE *RET2 = (NULL);             \
		RET1 = (NULL);                                  \
		STSTMP = XST_FAILURE;                           \
		RET1 = (TYPE *)(FUNC(__VA_ARGS__));             \
		RET2 = (TYPE *)(FUNC(__VA_ARGS__));             \
		if (((NULL) == RET1) && ((NULL) == RET2)) {     \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on NULL != RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to strictly check for && as per the
 * implementation.
 *
 * @RET1	Variable to store the return value of function to be execute
 *		at at RHS during comparison.
 * @TYPE	Type of pointer which returns by the function to be execute at
 *		RHS during comparison.
 * @FUNC	The function to be execute at RHS during comparison.
 * @param	Other params are arguments to the called function
 *
 * @return	XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_STRICT_CHECK_IF_NOT_NULL(STSTMP, RET1, TYPE, FUNC, ...)     \
	({                                                      \
		volatile const TYPE *RET2 = NULL;               \
		RET1 = NULL;                                    \
		STSTMP = XST_FAILURE;                           \
		RET1 = (TYPE *)(FUNC(__VA_ARGS__));             \
		RET2 = (TYPE *)(FUNC(__VA_ARGS__));             \
		if (((NULL) != RET1) && ((NULL) != RET2)) {     \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS == RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to strictly check for && as per the
 * implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @TYPE        Return type of function to be execute at RHS during comparison.
 * @FUNC        The function to be execute at RHS during comparison.
 * @param       Other params are arguments to the called function
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_STRICT_CHECK_IF_EQUAL_FOR_FUNC(STSTMP, LHS, TYPE, FUNC, ...)\
	({                                                      \
		volatile TYPE RET1 = ~(LHS);                    \
		volatile TYPE RET2 = ~(LHS);                    \
		STSTMP = XST_FAILURE;                           \
		RET1 = (FUNC(__VA_ARGS__));                     \
		RET2 = (FUNC(__VA_ARGS__));                     \
		if (((LHS) == RET1) && ((LHS) == RET2)) {       \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS != RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to strictly check for && as per the
 * implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @TYPE        Return type of function to be execute at RHS during comparison.
 * @FUNC        The function to be execute at RHS during comparison.
 * @param       Other params are arguments to the called function
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_STRICT_CHECK_IF_NOTEQUAL_FOR_FUNC(STSTMP, LHS, TYPE, FUNC, ...)\
	({                                                      \
		volatile TYPE RET1 = LHS;                       \
		volatile TYPE RET2 = LHS;                       \
		STSTMP = XST_FAILURE;                           \
		RET1 = (FUNC(__VA_ARGS__));                     \
		RET2 = (FUNC(__VA_ARGS__));                     \
		if (((LHS) != RET1) && ((LHS) != RET2)) {       \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS == RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to strictly check for && as per the
 * implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @RHS         Right hand side value wants to compare.
 * @TYPE        Data type of LHS and RHS.
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_STRICT_CHECK_IF_EQUAL(STSTMP, LHS, RHS, TYPE)               \
	({                                                      \
		volatile TYPE RET1 = LHS;                       \
		volatile TYPE RET2 = ~(LHS);                    \
		STSTMP = XST_FAILURE;                           \
		if ((RET1 == (RHS)) && (RET2 == (~(RHS)))) {    \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS != RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to strictly check for && as per the
 * implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @RHS         Right hand side value wants to compare.
 * @TYPE        Data type of LHS and RHS.
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_STRICT_CHECK_IF_NOTEQUAL(STSTMP, LHS, RHS, TYPE)            \
	({                                                      \
		volatile TYPE RET1 = LHS;                       \
		volatile TYPE RET2 = ~(LHS);                    \
		STSTMP = XST_FAILURE;                           \
		if ((RET1 != (RHS)) && (RET2 != (~(RHS)))) {    \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on NULL == RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to check for || as per implementation.
 *
 * @RET1        Variable to store the return value of function to be execute
 *              at at RHS during comparison.
 * @TYPE        Type of pointer which returns by the function to be execute at
 *              RHS during comparison.
 * @FUNC        The function to be execute at RHS during comparison.
 * @param       Other params are arguments to the called function
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_CHECK_IF_NULL(STSTMP, RET1, TYPE, FUNC, ...)        \
	({                                                      \
		volatile const TYPE *RET2 = (NULL);             \
		RET1 = (NULL);                                  \
		STSTMP = XST_FAILURE;                           \
		RET1 = (TYPE *)(FUNC(__VA_ARGS__));             \
		RET2 = (TYPE *)(FUNC(__VA_ARGS__));             \
		if (((NULL) == RET1) || ((NULL) == RET2)) {     \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on NULL != RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to check for || as per implementation.
 *
 * @RET1        Variable to store the return value of function to be execute
 *              at at RHS during comparison.
 * @TYPE        Type of pointer which returns by the function to be execute at
 *              RHS during comparison.
 * @FUNC        The function to be execute at RHS during comparison.
 * @param       Other params are arguments to the called function
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_CHECK_IF_NOT_NULL(STSTMP, RET1, TYPE, FUNC, ...)    \
	({                                                      \
		volatile const TYPE *RET2 = NULL;               \
		RET1 = NULL;                                    \
		STSTMP = XST_FAILURE;                           \
		RET1 = (TYPE *)(FUNC(__VA_ARGS__));             \
		RET2 = (TYPE *)(FUNC(__VA_ARGS__));             \
		if (((NULL) != RET1) || ((NULL) != RET2)) {     \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS == RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to check for || as per implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @TYPE        Return type of function to be execute at RHS during comparison.
 * @FUNC        The function to be execute at RHS during comparison.
 * @param       Other params are arguments to the called function
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_CHECK_IF_EQUAL_FOR_FUNC(STSTMP, LHS, TYPE, FUNC, ...)\
	({                                                      \
		volatile TYPE RET1 = ~(LHS);                    \
		volatile TYPE RET2 = ~(LHS);                    \
		STSTMP = XST_FAILURE;                           \
		RET1 = (FUNC(__VA_ARGS__));                     \
		RET2 = (FUNC(__VA_ARGS__));                     \
		if (((LHS) == RET1) || ((LHS) == RET2)) {       \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS != RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to check for || as per implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @TYPE        Return type of function to be execute at RHS during comparison.
 * @FUNC        The function to be execute at RHS during comparison.
 * @param       Other params are arguments to the called function
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_CHECK_IF_NOTEQUAL_FOR_FUNC(STSTMP, LHS, TYPE, FUNC, ...)\
	({                                                      \
		volatile TYPE RET1 = LHS;                       \
		volatile TYPE RET2 = LHS;                       \
		STSTMP = XST_FAILURE;                           \
		RET1 = (FUNC(__VA_ARGS__));                     \
		RET2 = (FUNC(__VA_ARGS__));                     \
		if (((LHS) != RET1) || ((LHS) != RET2)) {       \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS == RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to check for || as per implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @RHS         Right hand side value wants to compare.
 * @TYPE        Data type of LHS and RHS.
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_CHECK_IF_EQUAL(STSTMP, LHS, RHS, TYPE)              \
	({                                                      \
		volatile TYPE RET1 = LHS;                       \
		volatile TYPE RET2 = ~(LHS);                    \
		STSTMP = XST_FAILURE;                           \
		if ((RET1 == (RHS)) || (RET2 == (~(RHS)))) {    \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

/**
 * Adds redundancy while comparing the return value of called function.
 * based on LHS != RHS comparison wants to skip some part from execution.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to".
 * This MACRO will require when need to check for || as per implementation.
 *
 * @LHS         Left hand side value wants to compare.
 * @RHS         Right hand side value wants to compare.
 * @TYPE        Data type of LHS and RHS.
 *
 * @return      XST_SUCCESS if condition become true else XST_FAILURE
 **/
#define XPM_CHECK_IF_NOTEQUAL(STSTMP, LHS, RHS, TYPE)           \
	({                                                      \
		volatile TYPE RET1 = LHS;                       \
		volatile TYPE RET2 = ~(LHS);                    \
		STSTMP = XST_FAILURE;                           \
		if ((RET1 != (RHS)) || (RET2 != (~(RHS)))) {    \
			STSTMP = XST_SUCCESS;                   \
		}                                               \
		STSTMP;                                         \
	})

#define XPM_GOTO_LABEL_ON_CONDITION(condition, label) { \
	if (condition) { \
		goto label;\
	}\
}

void *XPm_AllocBytes(u32 SizeInBytes);

void XPm_Out32(u32 RegAddress, u32 l_Val);

u32 XPm_In32(u32 RegAddress);

u32 XPm_GetPlatform(void);
u32 XPm_GetPmcVersion(void);
u32 XPm_GetSlrType(void);
u32 XPm_GetIdCode(void);
u32 XPm_GetPlatformVersion(void);

/**
 * Read Modify Write a register
 */
void XPm_RMW32(u32 RegAddress, u32 Mask, u32 Value);

void XPm_Out64(u64 RegAddress, u32 Value);
u32 XPm_In64(u64 RegAddress);
void XPm_RMW64(u64 RegAddress, u32 Mask, u32 Value);

/**
 * Wait for a period represented by TimeOut
 *
 */
void XPm_Wait(u32 TimeOutCount);

/**
 * Poll for mask for a period represented by TimeOut
 */
maybe_unused static inline XStatus XPm_PollForMask(u32 RegAddress, u32 Mask,
				      u32 TimeOut)
{
	return XPlmi_UtilPoll(RegAddress, Mask, Mask, TimeOut, NULL);
}

/**
 * Poll for mask for a period represented by TimeOut for 64 bit registers
 */
maybe_unused static inline XStatus XPm_PollForMask64(u64 RegAddress, u32 Mask,
					u32 TimeOut)
{
	return XPlmi_UtilPoll64(RegAddress, Mask, Mask, TimeOut);
}

/**
 * Poll for zero for a period represented by TimeOut
 */
maybe_unused static inline XStatus XPm_PollForZero(u32 RegAddress, u32 Mask,
				      u32 TimeOut)
{
	return XPlmi_UtilPoll(RegAddress, Mask, 0, TimeOut, NULL);
}

/**
 * Compute parity of a 32-bit word
 */
u32 XPm_ComputeParity(u32 CalParity);

/* Dump Memory Related Data like Total Mem, Usaged Mem, Free Mem */
void XPm_DumpMemUsage(void);

#define PMC_VERSION_SILICON_ES1			(0x10U)
#define PMC_VERSION_SILICON_PROD		(0x20U)

/* NPI PCSR related general functions */
void XPm_UnlockPcsr(u32 BaseAddr);
void XPm_LockPcsr(u32 BaseAddr);

#ifdef __cplusplus
}
#endif

#endif /* XPM_COMMON_H_ */
