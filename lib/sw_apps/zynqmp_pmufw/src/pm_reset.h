/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Implementation of reset configuration mechanism within
 * power management.
 *********************************************************************/

#ifndef PM_RESET_H_
#define PM_RESET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "gpio.h"
#include "pm_common.h"

/*********************************************************************
 * Function declarations
 ********************************************************************/
u32 PmResetGetStatusInt(const PmReset* const resetPtr, u32 *status);

s32 PmResetDoAssert(const PmReset *reset, u32 action);
s32 PmResetAssertInt(u32 reset, u32 action);
s32 PmResetSetConfig(const u32 resetId, const u32 permissions);
void PmResetClearConfig(void);

bool PmResetMasterHasAccess(const PmMaster* const m, const PmReset* const r);
PmReset* PmGetResetById(const u32 resetId);

#ifdef __cplusplus
}
#endif

#endif /* PM_RESET_H_ */
