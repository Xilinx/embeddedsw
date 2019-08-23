/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#include "xpm_pmc.h"

XStatus XPmPmc_Init(XPm_Pmc *Pmc, u32 DevcieId, u32 Ipi, u32 *BaseAddress,
		    XPm_Power *Power,  XPm_ClockNode *Clock,
		    XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&Pmc->Core, DevcieId, BaseAddress, Power, Clock,
			      Reset, Ipi, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Pmc->PmcIouSlcrBaseAddr = BaseAddress[0];
	Pmc->PmcGlobalBaseAddr = BaseAddress[1];
	Pmc->PmcAnalogBaseAddr = BaseAddress[2];
done:
	return Status;
}
