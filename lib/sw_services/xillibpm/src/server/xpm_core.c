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

#include "xillibpm_psm_api.h"
#include "xpm_core.h"
#include "xpm_psm.h"

XStatus XPmCore_Init(XPm_Core *Core, u32 Id, u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh, struct XPm_CoreOps *Ops)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&Core->Device,
		Id, BaseAddress[0], Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Core->DebugMode = 0;
	Core->ImageId = 0;
	Core->Ipi = IpiCh;
	Core->CoreOps = Ops;
	Core->RegAddress[0] = BaseAddress[1];
	Core->RegAddress[1] = BaseAddress[2];

done:
	return Status;
}

static XStatus XPmCore_Sleep(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	/*
	 * If parent is on, then only send sleep request
	 */
	if (Core->Device.Power->Parent->Node.State == XPM_POWER_STATE_ON) {
		/*
		 * Power down the core
		 */
		Status = XPm_DirectPwrDwn(Core->Device.Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		Status = XST_SUCCESS;
		goto done;
	}

	if (NULL != Core->Device.Power) {
		Status = Core->Device.Power->Node.HandleEvent(&Core->Device.Power->Node, XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL != Core->Device.ClkHandles) {
		Status = XPmClock_Release(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

done:
	return Status;
}

XStatus XPmCore_PwrDwn(XPm_Core *Core, u32 PwrDwnRegOffset)
{
	XStatus Status = XST_FAILURE;
	u32 PwrReq;

	if ((Core->Device.Node.State == XPM_DEVSTATE_PWR_OFF) ||
	    (Core->Device.Node.State == XPM_DEVSTATE_UNUSED)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if(Core->Device.Node.State == XPM_DEVSTATE_SUSPENDING) {
		DISABLE_WFI(Core->SleepMask);
	}

	Status = XPmCore_Sleep(Core);
	if(Status != XST_SUCCESS) {
		goto done;
	}
	Core->Device.Node.State = XPM_DEVSTATE_PWR_OFF;

	/* CLear Power Down Request */
	PmIn32(Core->Device.Node.BaseAddress + PwrDwnRegOffset, PwrReq);
	if (0U != (Core->PwrDwnMask & PwrReq)) {
		PwrReq &= ~Core->PwrDwnMask;
		PmOut32(Core->Device.Node.BaseAddress + PwrDwnRegOffset, PwrReq);
	}

done:
	return Status;
}
