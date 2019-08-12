/*
 * Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
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
