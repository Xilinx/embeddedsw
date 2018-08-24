/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * Definitions of commonly used macros and enums in PMU Power
 * Management (PM).
 *********************************************************************/

#ifndef PM_COMMON_H_
#define PM_COMMON_H_

#include "pmu_local.h"
#include "xpfw_default.h"
#include "xil_types.h"

#include "xpfw_ipi_manager.h"
#include "xpfw_mod_pm.h"

/*********************************************************************
 * Typedefs (common use in PMU Power Management)
 ********************************************************************/
/*
 * stdint.h is not available, and pmu-fw framework defined similar
 * macros but as 0 and 1 (signed). PM, as in general, uses unsigned.
 */
#ifndef bool
	typedef u8 bool;
	#define true	1U
	#define false	0U
#endif

typedef u32 (*const PmTranHandler)(void);
typedef struct PmNode PmNode;

/*********************************************************************
 * Macros
 ********************************************************************/
#define pm_printf	xil_printf

/*
 * PM log levels. The configured log level should be specifid with:
 * -DPM_LOG_LEVEL=X where X is one of the numbers defined below
 */
#define PM_ALERT	1
#define PM_ERROR	2
#define PM_WARNING	3
#define PM_INFO		4

#if defined(PM_LOG_LEVEL) && (PM_LOG_LEVEL >= PM_INFO)
	#define PmInfo(...)	pm_printf(__VA_ARGS__)
#else
	#define PmInfo(...)	{}
#endif

#if defined(PM_LOG_LEVEL) && (PM_LOG_LEVEL >= PM_WARNING)
	#define PmWarn(...)	xil_printf(__VA_ARGS__)
#else
	#define PmWarn(...)	{}
#endif

#if defined(PM_LOG_LEVEL) && (PM_LOG_LEVEL >= PM_ERROR)
	#define PmErr(...)	pm_printf("Err: "); xil_printf(__VA_ARGS__)
#else
	#define PmErr(...)	{}
#endif

#if defined(PM_LOG_LEVEL) && (PM_LOG_LEVEL >= PM_ALERT)
	#define PmAlert(...)	pm_printf("Alert: "); xil_printf(__VA_ARGS__)
#else
	#define PmAlert(...)	{}
#endif

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))

/* Enable/disable macros for wake events in GPI1 register */
#define ENABLE_WAKE(mask)   XPfw_RMW32(PMU_LOCAL_GPI1_ENABLE, (mask), (mask));
#define DISABLE_WAKE(mask)  XPfw_RMW32(PMU_LOCAL_GPI1_ENABLE, (mask), ~(mask));

/* Macros for IPI responses (return values and callbacks) */
#define IPI_RESPONSE1(mask, arg0)				\
{	\
	u32 _ipi_resp_data[] = {(arg0)};	\
	XPfw_IpiWriteResponse(PmModPtr, (mask), &_ipi_resp_data[0], ARRAY_SIZE(_ipi_resp_data));	\
}

#define IPI_RESPONSE2(mask, arg0, arg1)				\
{	\
	u32 _ipi_resp_data[] = {(arg0), (arg1)};	\
	XPfw_IpiWriteResponse(PmModPtr, (mask), &_ipi_resp_data[0], ARRAY_SIZE(_ipi_resp_data));	\
}

#define IPI_RESPONSE3(mask, arg0, arg1, arg2)			\
{	\
	u32 _ipi_resp_data[] = {(arg0), (arg1), (arg2)};	\
	XPfw_IpiWriteResponse(PmModPtr, (mask), &_ipi_resp_data[0], ARRAY_SIZE(_ipi_resp_data));	\
}

#define IPI_RESPONSE4(mask, arg0, arg1, arg2, arg3)		\
{	\
	u32 _ipi_resp_data[] = {(arg0), (arg1), (arg2), (arg3)};	\
	XPfw_IpiWriteResponse(PmModPtr, (mask), &_ipi_resp_data[0], ARRAY_SIZE(_ipi_resp_data));	\
}

#define IPI_RESPONSE5(mask, arg0, arg1, arg2, arg3, arg4)	\
{	\
	u32 _ipi_resp_data[] = {(arg0), (arg1), (arg2), (arg3), (arg4)};	\
	XPfw_IpiWriteResponse(PmModPtr, (mask), &_ipi_resp_data[0], ARRAY_SIZE(_ipi_resp_data));	\
}

/* PMU internal capabilities used in definition of slaves' states */
#define PM_CAP_POWER        0x8U
#define PM_CAP_CLOCK        0x10U

/* Default transition latencies in us */
#define PM_DEFAULT_LATENCY 1000U
#define PM_POWER_ISLAND_LATENCY 2000U
#define PM_POWER_DOMAIN_LATENCY 10000U

/* Power consumptions of the slave components */
#define DEFAULT_POWER_ON		100U
#define DEFAULT_POWER_RETENTION		50U
#define DEFAULT_POWER_OFF		0U

/* Type of boot cold vs warm boot */
#define PM_COLD_BOOT	1U
#define PM_WARM_BOOT	2U

/* One (first) u32 is used for API call id coding */
#define PAYLOAD_API_ID			1U
/* Each API can have up to 5 arguments */
#define PAYLOAD_API_ARGS_CNT	5U
/* Number of payload elements (api id and api's arguments) */
#define PAYLOAD_ELEM_CNT		(PAYLOAD_API_ID + PAYLOAD_API_ARGS_CNT)

#define MASK_OF_BITS(bits)		((1 << (bits)) - 1)

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmRegisterContext - A pair of address/value used for saving/restoring context
 *                     of a register
 * @addr        Address of a register
 * @value       Variable to store register content
 */
typedef struct PmRegisterContext {
	const u32 addr;
	u32 value;
} PmRegisterContext;

/**
 * PmMemorySection - Memory region that will be processed by PMUFW
 * @startAddr        Start address of memory region
 * @endAddr          End address of memory region
 */
typedef struct PmMemorySection {
	const u32 startAddr;
	const u32 endAddr;
} PmMemorySection;

/*********************************************************************
 * Function declarations
 ********************************************************************/

#endif /* PM_COMMON_H_ */
