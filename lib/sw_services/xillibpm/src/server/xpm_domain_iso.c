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

#include "xpm_domain_iso.h"
#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_core.h"

u32 XPmDomainIso_MaskList[XPM_DOMAIN_ISO_MAX] = {
	[XPM_DOMAIN_ISO_FPD_PL_TEST] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_MASK,
	[XPM_DOMAIN_ISO_FPD_PL] =  PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK,
	[XPM_DOMAIN_ISO_FPD_SOC] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_MASK,
	[XPM_DOMAIN_ISO_LPD_CPM_DFX] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_MASK,
	[XPM_DOMAIN_ISO_LPD_CPM] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_MASK,
	[XPM_DOMAIN_ISO_LPD_PL_TEST] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_MASK,
	[XPM_DOMAIN_ISO_LPD_PL] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_MASK,
	[XPM_DOMAIN_ISO_LPD_SOC] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_MASK,
	[XPM_DOMAIN_ISO_PMC_LPD_DFX] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_MASK,
	[XPM_DOMAIN_ISO_PMC_LPD] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_MASK,
	[XPM_DOMAIN_ISO_PMC_PL_CFRAME] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK,
	[XPM_DOMAIN_ISO_PMC_PL_TEST] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK,
	[XPM_DOMAIN_ISO_PMC_PL] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_MASK,
	[XPM_DOMAIN_ISO_PMC_SOC_NPI] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_MASK,
	[XPM_DOMAIN_ISO_PMC_SOC] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_MASK,
	[XPM_DOMAIN_ISO_PL_SOC] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_MASK,
	[XPM_DOMAIN_ISO_VCCAUX_SOC] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_MASK,
	[XPM_DOMAIN_ISO_VCCRAM_SOC] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_MASK,
	[XPM_DOMAIN_ISO_VCCAUX_VCCRAM] = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_MASK,
};

XStatus XPmDomainIso_Control(enum XPmDomainIso IsoId, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Pmc;

	if (IsoId >= XPM_DOMAIN_ISO_MAX)
		goto done;

	Pmc = (XPm_Core *)PmDevices[XPM_NODEIDX_DEV_PMC_PROC];
	if (NULL == Pmc) {
		goto done;
	}

	if(Enable)
		XPm_RMW32((Pmc->RegAddress[0] + DOMAIN_ISO_CTRL_OFFSET),
			XPmDomainIso_MaskList[IsoId], XPmDomainIso_MaskList[IsoId]);
	else {
		/* Check power status before removing isolation */
		XPm_RMW32((Pmc->RegAddress[0] + DOMAIN_ISO_CTRL_OFFSET),
		  XPmDomainIso_MaskList[IsoId], 0);
	}

	Status = XST_SUCCESS;
done:
	return Status;
}
