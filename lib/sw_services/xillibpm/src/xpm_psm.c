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
#include "xpm_psm.h"

#define GLOBAL_CNTRL(BASE)	((BASE) + PSM_GLOBAL_CNTRL)
#define PWR_UP_EN(BASE)		((BASE) + PSM_GLOBAL_REQ_PWRUP_EN)
#define PWR_UP_TRIG(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRUP_TRIG)
#define PWR_DN_EN(BASE)		((BASE) + PSM_GLOBAL_REQ_PWRDWN_EN)
#define PWR_DN_TRIG(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRDWN_TRIG)
#define PWR_STAT(BASE)		((BASE) + PSM_GLOBAL_PWR_STATE)

#define PSM_NID		NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_CORE, XPM_NODETYPE_DEV_CORE_PSM, XPM_NODEIDX_DEV_PSM_PROC)

XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&Psm->Core,
		PSM_NID, BaseAddress,
		Power, Clock, Reset, Ipi, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

XStatus XPmPsm_PowerUp(u32 BitMask)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	XPm_Psm *Psm;

	PmInfo("BitMask=0x%08X\n\r", BitMask);

	Psm = (XPm_Psm *)PmDevices[XPM_NODEIDX_DEV_PSM_PROC];
	if (NULL == Psm) {
		goto done;
	}

	if (XPmPsm_FwIsPresent() != TRUE) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	PmOut32(PWR_UP_TRIG(Psm->Core.Device.Node.BaseAddress), BitMask);
	PmOut32(PWR_UP_EN(Psm->Core.Device.Node.BaseAddress), BitMask);
	do {
		PmIn32(PWR_STAT(Psm->Core.Device.Node.BaseAddress), Reg);
	} while ((Reg & BitMask) != BitMask);

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmPsm_PowerDown(u32 BitMask)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	XPm_Psm *Psm;

	PmInfo("BitMask=0x%08X\n\r", BitMask);

	Psm = (XPm_Psm *)PmDevices[XPM_NODEIDX_DEV_PSM_PROC];
	if (NULL == Psm) {
		goto done;
	}

	if (XPmPsm_FwIsPresent() != TRUE) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	PmOut32(PWR_DN_TRIG(Psm->Core.Device.Node.BaseAddress), BitMask);
	PmOut32(PWR_DN_EN(Psm->Core.Device.Node.BaseAddress), BitMask);
	do {
		PmIn32(PWR_STAT(Psm->Core.Device.Node.BaseAddress), Reg);
	} while (0 != (Reg & BitMask));

	Status = XST_SUCCESS;

done:
	return Status;
}

u32 XPmPsm_FwIsPresent(void)
{
	u32 Reg = FALSE;
	XPm_Psm *Psm;

	Psm = (XPm_Psm *)PmDevices[XPM_NODEIDX_DEV_PSM_PROC];
	if (NULL == Psm) {
		goto done;
	}

	PmIn32(GLOBAL_CNTRL(Psm->Core.Device.Node.BaseAddress), Reg)
	if (PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK ==
		(Reg & PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)) {
		Reg = TRUE;
	}

done:
	return Reg;
}
