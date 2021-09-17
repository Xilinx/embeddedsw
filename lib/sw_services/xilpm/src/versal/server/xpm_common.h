/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_COMMON_H_
#define XPM_COMMON_H_

#include "xstatus.h"
#include "xil_io.h"
#include "xil_util.h"
#include "xpm_err.h"
#include "xplmi_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hack: These will increase code size.  Define them as needed. */
#define xSELF_TEST
#define xSELF_TEST_DEVICE_REQUEST
#define xSELF_TEST_PIN_API
#define xSELF_TEST_RESET_API
#define xSELF_TEST_CLOCK_API
#define xDEBUG_REG_IO

#define TRUE	1U
#define FALSE	0U

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

#define PmChkRegMask32(ADDR, MASK, VAL, STATUS)				\
	do {									\
		if (((u32)(VAL) & (u32)(MASK)) != ((u32)(XPm_In32((ADDR))) & (u32)(MASK))) {	\
			(STATUS) = XPM_REG_WRITE_FAILED;			\
		}								\
	} while (XPM_FALSE_COND)								\

#define PmChkRegOut32(ADDR, VAL, STATUS)					\
	do {									\
		if ((u32)(VAL) != XPm_In32((ADDR))) {				\
			(STATUS) = XPM_REG_WRITE_FAILED;			\
		}								\
	} while (XPM_FALSE_COND)								\

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

#define PLATFORM_VERSION_SILICON_ES1		(0x0U)
#define PLATFORM_VERSION_SILICON_ES2		(0x1U)

#define PMC_VERSION_SILICON_ES1			(0x10U)
#define PMC_VERSION_SILICON_PROD		(0x20U)

#define XPM_PMC_TAP_IDCODE_SBFMLY_SHIFT		(18U)
#define XPM_PMC_TAP_IDCODE_DEV_SHIFT		(12U)
#define XPM_PMC_TAP_IDCODE_SBFMLY_S		((u32)2U << XPM_PMC_TAP_IDCODE_SBFMLY_SHIFT)
#define XPM_PMC_TAP_IDCODE_SBFMLY_SV		((u32)3U << XPM_PMC_TAP_IDCODE_SBFMLY_SHIFT)
/* VC1902 */
#define PMC_TAP_IDCODE_DEV_VC1902		((u32)0x28U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VC1902	(XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VC1902)
/* VE2302 */
#define PMC_TAP_IDCODE_DEV_VE2302		((u32)0x8U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VE2302	(XPM_PMC_TAP_IDCODE_SBFMLY_SV | PMC_TAP_IDCODE_DEV_VE2302)
/* VC1702 */
#define PMC_TAP_IDCODE_DEV_VC1702		((u32)0x18U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VC1702	(XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VC1702)
/* VE1752 */
#define PMC_TAP_IDCODE_DEV_VE1752       ((u32)0x1AU << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VE1752    (XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VE1752)

#define SLR_TYPE_MONOLITHIC_DEV			(0x7U)
#define SLR_TYPE_SSIT_DEV_MASTER_SLR		(0x6U)
#define SLR_TYPE_SSIT_DEV_SLAVE_1_SLR_TOP	(0x5U)
#define SLR_TYPE_SSIT_DEV_SLAVE_1_SLR_NTOP	(0x4U)
#define SLR_TYPE_SSIT_DEV_SLAVE_2_SLR_TOP	(0x3U)
#define SLR_TYPE_SSIT_DEV_SLAVE_2_SLR_NTOP	(0x2U)
#define SLR_TYPE_SSIT_DEV_SLAVE_3_SLR_TOP	(0x1U)
#define SLR_TYPE_INVALID			(0x0U)

#ifdef STDOUT_BASEADDRESS
#if (STDOUT_BASEADDRESS == 0xFF000000U)
#define NODE_UART PM_DEV_UART_0 /* Assign node ID with UART0 device ID */
#elif (STDOUT_BASEADDRESS == 0xFF010000U)
#define NODE_UART PM_DEV_UART_1 /* Assign node ID with UART1 device ID */
#endif
#endif

void *XPm_AllocBytes(u32 SizeInBytes);

void XPm_Out32(u32 RegAddress, u32 l_Val);

u32 XPm_In32(u32 RegAddress);

u32 XPm_GetPlatform(void);
u32 XPm_GetPlatformVersion(void);
u32 XPm_GetPmcVersion(void);
u32 XPm_GetSlrType(void);
u32 XPm_GetIdCode(void);

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
static inline XStatus XPm_PollForMask(u32 RegAddress, u32 Mask,
				      u32 TimeOut)
{
	return XPlmi_UtilPoll(RegAddress, Mask, Mask, TimeOut);
}

/**
 * Poll for mask for a period represented by TimeOut for 64 bit registers
 */
static inline XStatus XPm_PollForMask64(u64 RegAddress, u32 Mask,
					u32 TimeOut)
{
	return XPlmi_UtilPoll64(RegAddress, Mask, Mask, TimeOut);
}

/**
 * Poll for zero for a period represented by TimeOut
 */
static inline XStatus XPm_PollForZero(u32 RegAddress, u32 Mask,
				      u32 TimeOut)
{
	return XPlmi_UtilPoll(RegAddress, Mask, 0, TimeOut);
}

/**
 * Compute parity of a 32-bit word
 */
u32 XPm_ComputeParity(u32 CalParity);

/* Dump Memory Related Data like Total Mem, Usaged Mem, Free Mem */
void XPm_DumpMemUsage(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_COMMON_H_ */
