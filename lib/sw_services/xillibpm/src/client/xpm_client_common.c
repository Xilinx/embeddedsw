/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#include "xpm_client_common.h"
#include "xillibpm_node.h"
#if defined (__aarch64__)
#include <xreg_cortexa53.h>
#elif defined (__arm__)
#include <xreg_cortexr5.h>
#endif

#define PM_AFL0_MASK			(0xFF)

#if defined (__aarch64__)
static struct XPm_Proc Proc_APU0 = {
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU1 = {
	.Ipi = NULL,
};

static struct XPm_Proc *const ProcList[] = {
	&Proc_APU0,
	&Proc_APU1,
};

struct XPm_Proc *PrimaryProc = &Proc_APU0;
#elif defined (__arm__)
#define RPU_GLBL_CTRL_OFFSET		(0x00)
#define RPU_GLBL_CNTL_SLSPLIT_MASK	(0x00000008)

static struct XPm_Proc Proc_RPU0 = {
	.Ipi = NULL,
};

static struct XPm_Proc Proc_RPU1 = {
	.Ipi = NULL,
};

static struct XPm_Proc *const ProcList[] = {
	&Proc_RPU0,
	&Proc_RPU1,
};

struct XPm_Proc *PrimaryProc = &Proc_RPU0;
char ProcName[5] = "RPU";
#endif

/**
 *  XPmClient_SetPrimaryProc() - Set primary processor based on processor ID
 */
void XPmClient_SetPrimaryProc(void)
{
	u32 ProcId;

#if defined (__aarch64__)
	ProcId = (mfcp(MPIDR_EL1) & PM_AFL0_MASK);
#elif defined (__arm__)
	ProcId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) & PM_AFL0_MASK);
	if (!(XPm_Read(XPAR_PSU_RPU_0_S_AXI_BASEADDR + RPU_GLBL_CTRL_OFFSET) &
	      RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		ProcId = 0;
		XPm_Dbg("Running in lock-step mode\r\n");
		memcpy(ProcName, "RPU", sizeof("RPU"));
	} else {
		XPm_Dbg("Running in split mode\r\n");
		if (0 == ProcId) {
			memcpy(ProcName, "RPU0", sizeof("RPU0"));
		} else {
			memcpy(ProcName, "RPU1", sizeof("RPU1"));
		}
	}
#endif

	ProcId &= PM_AFL0_MASK;
	PrimaryProc = ProcList[ProcId];
}
