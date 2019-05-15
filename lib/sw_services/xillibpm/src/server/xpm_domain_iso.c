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

#include "xpm_domain_iso.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_subsystem.h"

/*TODO: Below data should come from topology */
#define PMC_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_PMC, XPM_NODEIDX_POWER_PMC)
#define NPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_NOC, XPM_NODEIDX_POWER_NOC)
#define FPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_PS_FULL, XPM_NODEIDX_POWER_FPD)
#define LPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_PS_LOW, XPM_NODEIDX_POWER_LPD)
#define CPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_CPM, XPM_NODEIDX_POWER_CPM)
#define ME_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_ME, XPM_NODEIDX_POWER_ME)

XPm_Iso XPmDomainIso_List[XPM_NODEIDX_ISO_MAX] = {
	[XPM_NODEIDX_ISO_FPD_PL_TEST] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_FPD_PL_TEST),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_SHIFT,
		.DependencyNodeHandles = { FPD_NODEID, XPM_SUBSYSID_PL },
	},
	[XPM_NODEIDX_ISO_FPD_PL] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_FPD_PL),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_SHIFT,
		.DependencyNodeHandles = { FPD_NODEID, XPM_SUBSYSID_PL },
	},
	[XPM_NODEIDX_ISO_FPD_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_FPD_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_SHIFT,
		.DependencyNodeHandles = { FPD_NODEID, NPD_NODEID},
	},
	[XPM_NODEIDX_ISO_LPD_CPM_DFX] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_CPM_DFX),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_SHIFT,
		.DependencyNodeHandles = { LPD_NODEID, CPD_NODEID },
	},
	[XPM_NODEIDX_ISO_LPD_CPM] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_CPM),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_SHIFT,
		.DependencyNodeHandles = { LPD_NODEID, CPD_NODEID },
	},
	[XPM_NODEIDX_ISO_LPD_PL_TEST] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_PL_TEST),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_SHIFT,
		.DependencyNodeHandles = { LPD_NODEID, XPM_SUBSYSID_PL },
	},
	[XPM_NODEIDX_ISO_LPD_PL] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_PL),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_SHIFT,
		.DependencyNodeHandles = { LPD_NODEID, XPM_SUBSYSID_PL },
	},
	[XPM_NODEIDX_ISO_LPD_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_SHIFT,
		.DependencyNodeHandles = { LPD_NODEID, NPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PMC_LPD_DFX] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_LPD_DFX),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, LPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PMC_LPD] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_LPD),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, LPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PMC_PL_CFRAME] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_PL_CFRAME),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, XPM_SUBSYSID_PL },
	},
	[XPM_NODEIDX_ISO_PMC_PL_TEST] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_PL_TEST),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, XPM_SUBSYSID_PL },
	},
	[XPM_NODEIDX_ISO_PMC_PL] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_PL),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, XPM_SUBSYSID_PL },
	},
	[XPM_NODEIDX_ISO_PMC_SOC_NPI] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_SOC_NPI),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, NPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PMC_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, NPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PL_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_SHIFT,
		.DependencyNodeHandles = { XPM_SUBSYSID_PL, NPD_NODEID },
	},
	[XPM_NODEIDX_ISO_VCCAUX_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_VCCAUX_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_SHIFT,
		.DependencyNodeHandles = { PMC_NODEID, NPD_NODEID },
	},
	[XPM_NODEIDX_ISO_VCCRAM_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_VCCRAM_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_SHIFT,
		.DependencyNodeHandles = { XPM_SUBSYSID_PL, NPD_NODEID },
	},
	[XPM_NODEIDX_ISO_VCCAUX_VCCRAM] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_VCCAUX_VCCRAM),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_SHIFT,
		.DependencyNodeHandles = { XPM_SUBSYSID_PL, NPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR),
		.Node.BaseAddress = PCIEA_ATTRIB_0_FABRICEN,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PCIEA_ATTRIB_0_FABRICEN_ATTR_SHIFT,
		.DependencyNodeHandles = { XPM_SUBSYSID_PL, CPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR),
		.Node.BaseAddress = PCIEA_ATTRIB_1_FABRICEN,
		.Node.State = PM_ISOLATION_ON,
		.Offset = PCIEA_ATTRIB_1_FABRICEN_ATTR_SHIFT,
		.DependencyNodeHandles = { XPM_SUBSYSID_PL, CPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PL_CPM_RST_CPI0] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_RST_CPI0),
		.Node.BaseAddress = CPM_CRCPM_RST_CPI0,
		.Node.State = PM_ISOLATION_ON,
		.Offset = CPM_CRCPM_RST_CPI0_RESET_SHIFT,
		.DependencyNodeHandles = { XPM_SUBSYSID_PL, CPD_NODEID },
	},
	[XPM_NODEIDX_ISO_PL_CPM_RST_CPI1] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_RST_CPI1),
		.Node.BaseAddress = CPM_CRCPM_RST_CPI1,
		.Node.State = PM_ISOLATION_ON,
		.Offset = CPM_CRCPM_RST_CPI1_RESET_SHIFT,
		.DependencyNodeHandles = { XPM_SUBSYSID_PL, CPD_NODEID },
	},
};

static XStatus XPmDomainIso_CheckDependencies(u32 IsoIdx)
{
	XStatus Status = XST_SUCCESS;
	u32 i=0, NodeId;
	XPm_PowerDomain *PwrDomainNode;
	XPm_Subsystem *Subsystem;

	for( i=0; i<2; i++) {
		NodeId = XPmDomainIso_List[IsoIdx].DependencyNodeHandles[i];
		if (NODECLASS(NodeId) == XPM_NODECLASS_POWER) {
			PwrDomainNode = (XPm_PowerDomain *) PmPowers[NODEINDEX(NodeId)];
			if(PwrDomainNode->Power.Node.State != XPM_POWER_STATE_ON  && PwrDomainNode->Power.Node.State != XPM_POWER_STATE_INITIALIZING) {
				Status = XST_FAILURE;
				goto done;
			}
		} else if (NodeId == XPM_SUBSYSID_PL) {
			/* Right now as we dont have
			 * init finish for PLD, we assume PL is there when we see
			 * pm_iso_control commands */
			 Subsystem = XPmSubsystem_GetById(NodeId);
			 if(Subsystem->State != ONLINE) {
				Status = XST_FAILURE;
				goto done;
			}
		} else {
			Status = XST_FAILURE;
			goto done;
		}
	}
done:
	return Status;
}

XStatus XPmDomainIso_Control(u32 IsoIdx, u32 Enable)
{
	XStatus Status = XST_SUCCESS;
	u32 Mask;

	if (IsoIdx >= XPM_NODEIDX_ISO_MAX)
	{
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Mask = BIT(XPmDomainIso_List[IsoIdx].Offset);

	if(Enable == TRUE) {
		XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, Mask);
		XPmDomainIso_List[IsoIdx].Node.State = PM_ISOLATION_ON;
	} else if(Enable == FALSE_IMMEDIATE) {
		XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, 0);
		XPmDomainIso_List[IsoIdx].Node.State = PM_ISOLATION_OFF;
	} else {
		Status = XPmDomainIso_CheckDependencies(IsoIdx);
		if(XST_SUCCESS != Status)
		{
			/* Mark it pending */
			XPmDomainIso_List[IsoIdx].Node.State = PM_ISOLATION_REMOVE_PENDING;
			Status = XST_SUCCESS;
			goto done;
		}

		XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, 0);
		XPmDomainIso_List[IsoIdx].Node.State = PM_ISOLATION_OFF;
	}
done:
	return Status;
}

XStatus XPmDomainIso_ProcessPending(u32 PowerDomainId)
{
	XStatus Status = XST_SUCCESS;
	u32 i;

	(void)PowerDomainId;

	for(i=0; i< ARRAY_SIZE(XPmDomainIso_List); i++)
	{
		if(XPmDomainIso_List[i].Node.State == PM_ISOLATION_REMOVE_PENDING)
		{
			Status = XPmDomainIso_Control(i, FALSE);
		}
	}
	return Status;
}
