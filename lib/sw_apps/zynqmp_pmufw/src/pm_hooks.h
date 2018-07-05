/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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

#ifndef PM_HOOKS_H_
#define PM_HOOKS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_common.h"
#include "pm_slave.h"

#ifdef ENABLE_DDR_SR_WR
/*
 * For DDR status PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7 is used.
 */
#define XPFW_DDR_STATUS_REGISTER_OFFSET		(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7)
/*
 * DDR controller initialization flag mask
 *
 * This flag signals whether DDR controller have been initialized or not. It is
 * used by FSBL to inform PMU that DDR controller is initialized. When booting
 * with DDR in self refresh mode, PMU must wait until DDR controller have been
 * initialized by the FSBL before it can bring the DDR out of self refresh mode.
 */
#define DDRC_INIT_FLAG_MASK		BIT(4U)
/*
 * DDR self refresh mode indication flag mask
 *
 * This flag indicates whether DDR is in self refresh mode or not. It is used
 * by PMU to signal FSBL in order to skip over DDR phy and ECC initialization
 * at boot time.
 */
#define DDR_STATUS_FLAG_MASK		BIT(3U)
#endif

#define POS_DDR_REQS_SIZE	1U

/**
 * PmPosRequirement - Power Off Suspend requirements
 * @slave	Slave for which the requirements are set
 * @caps	Capabilities of the slave that are required by the system
 */
typedef struct PmPosRequirement {
	PmSlave* const slave;
	u32 caps;
} PmPosRequirement;

extern PmPosRequirement pmPosDdrReqs_g[POS_DDR_REQS_SIZE];

s32 PmHookPosSaveDdrContext(void);
void PmHookFinalizePowerOffSuspend(void);
void PmHookPowerDownLpd(void);
void PmHookInitPowerOffSuspend(void);
u32 PmHookGetBootType(void);
s32 PmHookRestoreDdrContext(void);
void PmHookPowerOffSuspendDdrReady(void);
#ifdef ENABLE_DDR_SR_WR
void PmHookSystemStart(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_HOOKS_H_ */
