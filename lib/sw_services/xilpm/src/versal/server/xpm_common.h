/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_COMMON_H_
#define XPM_COMMON_H_

#include "xstatus.h"
#include "xil_io.h"
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
 * GCC Specific attribute to suppress unused variable/function warning
 */
#ifndef maybe_unused
#define maybe_unused __attribute__((unused))
#endif

/**
 * Common baseline macro to print debug logs
 */
#define PmPrintCommon(DbgLevel, PrefixStr, ...)						  \
	do {										  \
		if (((DbgLevel) & (XPlmiDbgCurrentTypes & DebugLog.LogLevel)) != FALSE) { \
			xil_printf("%s %s: ", PrefixStr, __func__);			  \
			xil_printf(__VA_ARGS__);					  \
		}									  \
	} while (0)

/* Debug logs */
#define PmAlert(...)	PmPrintCommon(DEBUG_GENERAL,  "ALERT", __VA_ARGS__)
#define PmErr(...)	PmPrintCommon(DEBUG_GENERAL,  "ERR",   __VA_ARGS__)
#define PmWarn(...)	PmPrintCommon(DEBUG_GENERAL,  "WARN",  __VA_ARGS__)
#define PmInfo(...)	PmPrintCommon(DEBUG_INFO,     "INFO",  __VA_ARGS__)
#define PmDbg(...)	PmPrintCommon(DEBUG_DETAILED, "DBG",   __VA_ARGS__)

#ifdef DEBUG_REG_IO

#define PmIn32(ADDR, VAL)					\
	do {							\
		(VAL) = XPm_In32(ADDR);				\
		PmInfo("RD: 0x%08X -> 0x%08X\r\n", ADDR, VAL);	\
	} while (0)

#define PmOut32(ADDR, VAL)					\
	do {							\
		PmInfo("WR: 0x%08X <- 0x%08X\r\n", ADDR, VAL);	\
		XPm_Out32(ADDR, VAL);				\
	} while (0)

#define PmRmw32(ADDR, MASK, VAL)				\
	do {							\
		XPm_RMW32(ADDR, MASK, VAL);			\
		PmInfo("RMW: Addr=0x%08X, Mask=0x%08X, Val=0x%08X, Reg=0x%08X\r\n", \
			ADDR, MASK, VAL, XPm_In32(ADDR));	\
	} while (0)

#else

#define PmIn32(ADDR, VAL)		((VAL) = XPm_In32(ADDR))

#define PmOut32(ADDR, VAL)		XPm_Out32(ADDR, VAL)

#define PmRmw32(ADDR, MASK, VAL)	XPm_RMW32(ADDR, MASK, VAL)

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

#define XPm_Read32				XPm_In32
#define XPm_Write32				XPm_Out32

#define PLATFORM_VERSION_SILICON		(0x0U)
#define PLATFORM_VERSION_SPP			(0x1U)
#define PLATFORM_VERSION_EMU			(0x2U)
#define PLATFORM_VERSION_QEMU			(0x3U)
#define PLATFORM_VERSION_FCV			(0x4U)
#define PLATFORM_VERSION_SILICON_ES1		(0x0U)

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

void *XPm_AllocBytes(u32 Size);

void XPm_Out32(u32 RegAddress, u32 l_Val);

u32 XPm_In32(u32 RegAddress);

u32 XPm_GetPlatform(void);
u32 XPm_GetPlatformVersion(void);
u32 XPm_GetSlrType(void);

/**
 * Read Modify Write a register
 */
void XPm_RMW32(u32 RegAddress, u32 Mask, u32 Value);

/**
 * Wait for a period represented by TimeOut
 *
 */
void XPm_Wait(u32 TimeOutCount);

/**
 * Poll for mask for a period represented by TimeOut
 */
XStatus XPm_PollForMask(u32 RegAddress, u32 Mask, u32 TimeOutCount);
XStatus XPm_PollForZero(u32 RegAddress, u32 Mask, u32 TimeOutCount);

/**
 * Compute parity of a 32-bit word
 */
u32 XPm_ComputeParity(u32 Value);

/* Dump Memory Related Data like Total Mem, Usaged Mem, Free Mem */
void XPm_DumpMemUsage(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_COMMON_H_ */
