/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Definitions of commonly used macros and enums in PMU Power
 * Management (PM).
 *********************************************************************/

#ifndef PM_COMMON_H_
#define PM_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

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
	#define true	(bool)1U
	#define false	(bool)0U
#endif

typedef u32 (*const PmTranHandler)(void);

typedef struct PmNode PmNode;
typedef struct PmMaster PmMaster;
typedef struct PmRequirement PmRequirement;
typedef struct PmPower PmPower;
typedef struct PmPowerClass PmPowerClass;
typedef struct PmClockHandle PmClockHandle;
typedef struct PmSlave PmSlave;
typedef struct PmProc PmProc;
typedef struct PmNodeClass PmNodeClass;
typedef struct PmClockClass PmClockClass;
typedef struct PmSlaveTcm PmSlaveTcm;
typedef struct PmReset PmReset;
typedef struct PmSlaveClass PmSlaveClass;
typedef struct PmWakeEvent PmWakeEvent;

/*********************************************************************
 * Macros
 ********************************************************************/
#define pm_printf	xil_printf

/*
 * PM log levels. The configured log level should be specifid with:
 * -DPM_LOG_LEVEL=X where X is one of the numbers defined below
 */
#define PM_ALERT	1U
#define PM_ERROR	2U
#define PM_WARNING	3U
#define PM_INFO		4U

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
	u32 _ipi_resp_data[XPFW_IPI_MAX_MSG_LEN] = {(arg0), 0U, 0U, 0U, 0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteResponse(PmModPtr, (mask),		\
						 &_ipi_resp_data[0],		\
						 ARRAY_SIZE(_ipi_resp_data))) {	\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}

#define IPI_RESPONSE2(mask, arg0, arg1)				\
{	\
	u32 _ipi_resp_data[XPFW_IPI_MAX_MSG_LEN] = {(arg0), (arg1), 0U, 0U, 0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteResponse(PmModPtr, (mask),		\
						 &_ipi_resp_data[0],		\
						 ARRAY_SIZE(_ipi_resp_data))) {	\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}

#define IPI_RESPONSE3(mask, arg0, arg1, arg2)			\
{	\
	u32 _ipi_resp_data[XPFW_IPI_MAX_MSG_LEN] = {(arg0), (arg1), (arg2), 0U, 0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteResponse(PmModPtr, (mask),		\
						 &_ipi_resp_data[0],		\
						 ARRAY_SIZE(_ipi_resp_data))) {	\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}

#define IPI_RESPONSE4(mask, arg0, arg1, arg2, arg3)		\
{	\
	u32 _ipi_resp_data[XPFW_IPI_MAX_MSG_LEN] = {(arg0), (arg1), (arg2), (arg3),0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteResponse(PmModPtr, (mask),		\
						 &_ipi_resp_data[0],		\
						 ARRAY_SIZE(_ipi_resp_data))) {	\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}

#define IPI_RESPONSE5(mask, arg0, arg1, arg2, arg3, arg4)	\
{	\
	u32 ipi_resp_data[XPFW_IPI_MAX_MSG_LEN] = {(arg0), (arg1), (arg2), (arg3), (arg4), 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteResponse(PmModPtr, (mask),		\
						 &_ipi_resp_data[0],		\
						 ARRAY_SIZE(_ipi_resp_data))) {	\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
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

#define MASK_OF_BITS(bits)		(((u32)1 << (bits)) - 1U)

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

#ifdef __cplusplus
}
#endif

#endif /* PM_COMMON_H_ */
