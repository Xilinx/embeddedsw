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

#include "xil_io.h"
#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xillibpm_api.h"

XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, u32 BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&RpuCore->Core, Id, BaseAddress, Power,
			      Clock, Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (XPM_DEVID_R50_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->Core.Device.Node.BaseAddress +
				     RPU_0_CFG_OFFSET;
	} else {
		RpuCore->ResumeCfg = RpuCore->Core.Device.Node.BaseAddress +
				     RPU_1_CFG_OFFSET;
	}

	RpuCore->Ipi = Ipi;

done:
	return Status;
}

XStatus XPmRpuCore_WakeUp(XPm_RpuCore *RpuCore, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	XPm_ResetNode *Reset;
	u32 addrLow;

	/* Set reset address */
	if (0 == SetAddress) {
		goto done;
	}

	addrLow = (u32) (Address & 0xffff0000ULL);

	/* CFG_VINITHI_MASK mask is common for both processors */
	if (XPM_PROC_RPU_HIVEC_ADDR == addrLow) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			XPM_RPU_VINITHI_MASK);
	} else {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			~XPM_RPU_VINITHI_MASK);
	}

	/* release reset for all resets attached to this core*/
	Reset = RpuCore->Core.Device.Reset;

	while (NULL != Reset) {
		Status = Reset->Ops->SetState(Reset, PM_RESET_ACTION_RELEASE);
		Reset = Reset->NextReset;
	}
done:
	return Status;
}

void XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode)
{
	u32 Val;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	PmIn32(RpuCore->Core.Device.Node.BaseAddress + RPU_GLBL_CNTL_OFFSET,
	       Val);
	if (Mode == XPM_RPU_MODE_SPLIT) {
		Val |= XPM_RPU_SLSPLIT_MASK;
		Val &= ~XPM_RPU_TCM_COMB_MASK;
		Val &= ~XPM_RPU_SLCLAMP_MASK;
	} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
		Val &= ~XPM_RPU_SLSPLIT_MASK;
		Val |= XPM_RPU_TCM_COMB_MASK;
		Val |= XPM_RPU_SLCLAMP_MASK;
	} else {
		/* Required by MISRA */
	}

	PmOut32(RpuCore->Core.Device.Node.BaseAddress + RPU_GLBL_CNTL_OFFSET,
	        Val);
}

XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr)
{
	XStatus Status = XST_SUCCESS;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	/* CFG_VINITHI_MASK mask is common for both processors */
	if (XPM_RPU_BOOTMEM_LOVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			~XPM_RPU_VINITHI_MASK);
	} else if (XPM_RPU_BOOTMEM_HIVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			XPM_RPU_VINITHI_MASK);
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}
