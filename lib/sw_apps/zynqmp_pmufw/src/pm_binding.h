/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

/*********************************************************************
 * Function declarations to be used for integrating
 * power management within PMU firmware.
 *********************************************************************/

#ifndef PM_BINDING_H_
#define PM_BINDING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

/*********************************************************************
 * Enum definitions
 ********************************************************************/
/* Return status for checking whether IPI interrupt is a PM API call */
typedef enum {
	XPFW_PM_IPI_NOT_PM_CALL,
	XPFW_PM_IPI_IS_PM_CALL,
	XPFW_PM_IPI_SRC_UNKNOWN,
} XPfw_PmIpiStatus;

/*********************************************************************
 * Function declarations
 ********************************************************************/
/* Initialize power management firmware */
void XPfw_PmInit(void);

/* Check whether the ipi is power management related */
XPfw_PmIpiStatus XPfw_PmCheckIpiRequest(const u32 isrVal, const u32* apiId);

/* Call from IPI interrupt routine to handle PM API request */
s32 XPfw_PmIpiHandler(const u32 IsrMask, const u32* Payload, u8 Len);

/* Call from GPI2 interrupt routine to handle processor sleep request */
s32 XPfw_PmWfiHandler(const u32 srcMask);

/* Call from GPI1 interrupt routine to handle wake request */
s32 XPfw_PmWakeHandler(const u32 srcMask);

/* Call from DAP event handlers to inform PM about the FPD power state change */
void XPfw_DapFpdWakeEvent(void);

/* Call from DAP event handlers to inform PM about the RPU power state change */
void XPfw_DapRpuWakeEvent(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_BINDING_H_ */
