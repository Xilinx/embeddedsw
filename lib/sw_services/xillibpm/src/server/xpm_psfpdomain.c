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

#include "xpm_common.h"
#include "xpm_psfpdomain.h"
#include "xpm_bisr.h"

static XStatus FpdPreHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	(void)Args;
	(void)NumOfArgs;

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = FUNC_INIT_START;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	return Status;
}

static XStatus FpdPostHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	(void)Args;
	(void)NumOfArgs;

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = FUNC_INIT_FINISH;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	return Status;
}

static XStatus FpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	(void)Args;
	(void)NumOfArgs;

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = FUNC_SCAN_CLEAR;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	return Status;
}

static XStatus FpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	(void)Args;
	(void)NumOfArgs;

	/* Bisr is handled by FPD_SLCR registers only so no
	 * need to involve PSM */
	Status = XPmBisr_Repair(FPD_TAG_ID);

	return Status;
}

static XStatus FpdMbistClear(u32 *Args, u32 NumOfArgs)
{
        XStatus Status = XST_SUCCESS;
        u32 Payload[PAYLOAD_ARG_CNT] = {0};

	(void)Args;
	(void)NumOfArgs;

        Payload[0] = PSM_API_FPD_HOUSECLEAN;
        Payload[1] = FUNC_MBIST_CLEAR;

        Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
        if (XST_SUCCESS != Status) {
                goto done;
        }

        Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
        return Status;
}

struct XPm_PowerDomainOps FpdOps = {
	.PreHouseClean = FpdPreHouseclean,
	.PostHouseClean = FpdPostHouseclean,
	.ScanClear = FpdScanClear,
	.Bisr = FpdBisr,
	.Mbist = FpdMbistClear,
};

XStatus XPmPsFpDomain_Init(XPm_PsFpDomain *PsFpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent)
{
	XPmPowerDomain_Init(&PsFpd->Domain, Id, BaseAddress, Parent, &FpdOps);

	return XST_SUCCESS;
}
