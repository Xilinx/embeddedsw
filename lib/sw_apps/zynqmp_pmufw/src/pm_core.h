/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Contains the function to call for processing a PM API call.
 * The function (PmProcessRequest) is called from interrupt handler
 * stubs, implemented in pm_binding files. The request is further
 * processed according to the master that initiated request and
 * API call's payload read from master's IPI buffer.
 ********************************************************************/

#ifndef PM_CORE_H_
#define PM_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_master.h"
#include "xil_types.h"

/*********************************************************************
 * Function declarations
 ********************************************************************/
void PmProcessRequest(PmMaster *const master, const u32 *pload);
void PmResetAssert(const PmMaster *const master, const u32 reset,
		   const u32 action);
void PmShutdownInterruptHandler(void);
void PmKillBoardPower(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_CORE_H_ */
