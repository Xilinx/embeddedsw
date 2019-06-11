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
 * Structure definitions
 ********************************************************************/

typedef struct PmReset PmReset;
typedef struct PmMaster PmMaster;

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
