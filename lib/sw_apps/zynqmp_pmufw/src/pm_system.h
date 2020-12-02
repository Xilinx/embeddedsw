/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Contains system-level PM functions
 ********************************************************************/

#ifndef PM_SYSTEM_H_
#define PM_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "pm_master.h"
#include "pm_slave.h"

#define PM_SUSPEND_TYPE_REGULAR		0U
#define PM_SUSPEND_TYPE_POWER_OFF	1U

/*********************************************************************
 * Function declarations
 ********************************************************************/
s32 PmSystemRequirementAdd(void);

void PmSystemPrepareForRestart(const PmMaster* const master);
void PmSystemRestartDone(const PmMaster* const master);
bool PmSystemDetectPowerOffSuspend(const PmMaster* const master);
s32 PmSystemPreparePowerOffSuspend(void);
s32 PmSystemFinalizePowerOffSuspend(void);
s32 PmSystemResumePowerOffSuspend(void);
u32 PmSystemSuspendType(void);
void PmSystemSetSuspendType(u32 type);

#ifdef __cplusplus
}
#endif

#endif /* PM_SYSTEM_H_ */
