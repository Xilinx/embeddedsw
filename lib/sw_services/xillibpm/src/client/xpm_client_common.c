/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
#include <xil_cache.h>
#include "xillibpm_node.h"
#if defined (__aarch64__)
#include <xreg_cortexa53.h>
#elif defined (__arm__)
#include <xreg_cortexr5.h>
#endif

#define XPM_ARRAY_SIZE(x)		(sizeof(x) / sizeof(x[0]))

#define APU_DEVID(IDX)			NODEID(XPM_NODECLASS_DEVICE, \
					       XPM_NODESUBCL_DEV_CORE, \
					       XPM_NODETYPE_DEV_CORE_APU, (IDX))

#define RPU_DEVID(IDX)			NODEID(XPM_NODECLASS_DEVICE, \
					       XPM_NODESUBCL_DEV_CORE, \
					       XPM_NODETYPE_DEV_CORE_RPU, (IDX))

#define PM_AFL0_MASK			(0xFF)

#if defined (__aarch64__)
#define APU_PWRCTRL_OFFSET		(0x90)
#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK	(0x00000001)
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK	(0x00000002)

static struct XPm_Proc Proc_APU0 = {
	.DevId = APU_DEVID(XPM_NODEIDX_DEV_ACPU_0),
	.PwrCtrl = XPAR_PSV_APU_0_S_AXI_BASEADDR + APU_PWRCTRL_OFFSET,
	.PwrDwnMask = APU_0_PWRCTL_CPUPWRDWNREQ_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU1 = {
	.DevId = APU_DEVID(XPM_NODEIDX_DEV_ACPU_1),
	.PwrCtrl = XPAR_PSV_APU_0_S_AXI_BASEADDR + APU_PWRCTRL_OFFSET,
	.PwrDwnMask = APU_1_PWRCTL_CPUPWRDWNREQ_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc *const ProcList[] = {
	&Proc_APU0,
	&Proc_APU1,
};

struct XPm_Proc *PrimaryProc = &Proc_APU0;
#elif defined (__arm__)
#define RPU_0_PWRDWN_OFFSET		(0x108)
#define RPU_1_PWRDWN_OFFSET		(0x208)
#define RPU_PWRDWN_EN_MASK		(0x1)
#define RPU_GLBL_CTRL_OFFSET		(0x00)
#define RPU_GLBL_CNTL_SLSPLIT_MASK	(0x00000008)

static struct XPm_Proc Proc_RPU0 = {
	.DevId = RPU_DEVID(XPM_NODEIDX_DEV_RPU0_0),
	.PwrCtrl = XPAR_PSV_RPU_0_S_AXI_BASEADDR + RPU_0_PWRDWN_OFFSET,
	.PwrDwnMask = RPU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_RPU1 = {
	.DevId = RPU_DEVID(XPM_NODEIDX_DEV_RPU0_1),
	.PwrCtrl = XPAR_PSV_RPU_0_S_AXI_BASEADDR + RPU_1_PWRDWN_OFFSET,
	.PwrDwnMask = RPU_PWRDWN_EN_MASK,
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
 *  XPm_SetPrimaryProc() - Set primary processor based on processor ID
 */
void XPm_SetPrimaryProc(void)
{
	u32 ProcId;

#if defined (__aarch64__)
	ProcId = (mfcp(MPIDR_EL1) & PM_AFL0_MASK);
#elif defined (__arm__)
	ProcId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) & PM_AFL0_MASK);
	if (!(XPm_Read(XPAR_PSV_RPU_0_S_AXI_BASEADDR + RPU_GLBL_CTRL_OFFSET) &
	      RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		ProcId = 0;
		memcpy(ProcName, "RPU", sizeof("RPU"));
		XPm_Dbg("Running in lock-step mode\r\n");
	} else {
		if (0 == ProcId) {
			memcpy(ProcName, "RPU0", sizeof("RPU0"));
		} else {
			memcpy(ProcName, "RPU1", sizeof("RPU1"));
		}
		XPm_Dbg("Running in split mode\r\n");
	}
#endif

	ProcId &= PM_AFL0_MASK;
	PrimaryProc = ProcList[ProcId];
}

struct XPm_Proc *XPm_GetProcByDeviceId(u32 DeviceId)
{
	struct XPm_Proc *Proc = NULL;
	u8 Idx;

	for (Idx = 0; Idx < XPM_ARRAY_SIZE(ProcList); Idx++) {
		if (DeviceId == ProcList[Idx]->DevId) {
			Proc = ProcList[Idx];
			break;
		}
	}

	return Proc;
}

void XPm_ClientSuspend(const struct XPm_Proc *const Proc)
{
	u32 PwrDwnReg;

	/* Disable interrupts at processor level */
	XpmDisableInterrupts();

	/* Set powerdown request */
	PwrDwnReg = XPm_Read(Proc->PwrCtrl);
	PwrDwnReg |= Proc->PwrDwnMask;
	XPm_Write(Proc->PwrCtrl, PwrDwnReg);
}

void XPm_ClientWakeUp(const struct XPm_Proc *const Proc)
{
	if (NULL != Proc) {
		u32 Val;

		Val = XPm_Read(Proc->PwrCtrl);
		Val &= ~Proc->PwrDwnMask;
		XPm_Write(Proc->PwrCtrl, Val);
	}
}

void XPm_ClientSuspendFinalize(void)
{
	u32 CtrlReg;

	/* Flush the data cache only if it is enabled */
#ifdef __aarch64__
	CtrlReg = mfcp(SCTLR_EL3);
	if (XREG_CONTROL_DCACHE_BIT & CtrlReg) {
		Xil_DCacheFlush();
	}
#else
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	if (XREG_CP15_CONTROL_C_BIT & CtrlReg) {
		Xil_DCacheFlush();
	}
#endif

	XPm_Dbg("Going to WFI...\n");
	__asm__("wfi");
	XPm_Dbg("WFI exit...\n");
}

void XPm_ClientAbortSuspend(void)
{
	u32 PwrDwnReg;

	/* Clear powerdown request */
	PwrDwnReg = XPm_Read(PrimaryProc->PwrCtrl);
	PwrDwnReg &= ~PrimaryProc->PwrDwnMask;
	XPm_Write(PrimaryProc->PwrCtrl, PwrDwnReg);

	/* Enable interrupts at processor level */
	XpmEnableInterrupts();
}
