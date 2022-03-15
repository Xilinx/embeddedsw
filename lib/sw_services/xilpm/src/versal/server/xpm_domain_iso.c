/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_domain_iso.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_device.h"
#include "xpm_ipi.h"
#include "xpm_pldomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psm_api.h"
#include "xpm_psm.h"
#include "xpm_debug.h"

/*TODO: Below data should come from topology */
static XPm_Iso XPmDomainIso_List[XPM_NODEIDX_ISO_MAX] = {
	[XPM_NODEIDX_ISO_FPD_PL_TEST] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_FPD_PL_TEST),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_FPD, PM_DEV_PLD_0 },
	},
	[XPM_NODEIDX_ISO_FPD_PL] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_FPD_PL),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_FPD, PM_DEV_PLD_0 },
	},
	[XPM_NODEIDX_ISO_FPD_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_FPD_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_FPD, PM_POWER_NOC},
	},
	[XPM_NODEIDX_ISO_LPD_CPM_DFX] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_CPM_DFX),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_POWER_CPM },
	},
	[XPM_NODEIDX_ISO_LPD_CPM] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_CPM),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_POWER_CPM },
	},
	[XPM_NODEIDX_ISO_LPD_PL_TEST] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_PL_TEST),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_DEV_PLD_0 },
	},
	[XPM_NODEIDX_ISO_LPD_PL] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_PL),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_DEV_PLD_0 },
	},
	[XPM_NODEIDX_ISO_LPD_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_POWER_NOC },
	},
	[XPM_NODEIDX_ISO_PMC_LPD_DFX] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_LPD_DFX),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_POWER_LPD },
	},
	[XPM_NODEIDX_ISO_PMC_LPD] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_LPD),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_POWER_LPD },
	},
	[XPM_NODEIDX_ISO_PMC_PL_CFRAME] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_PL_CFRAME),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_DEV_PLD_0 },
	},
	[XPM_NODEIDX_ISO_PMC_PL_TEST] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_PL_TEST),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_DEV_PLD_0 },
	},
	[XPM_NODEIDX_ISO_PMC_PL] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_PL),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_DEV_PLD_0 },
	},
	[XPM_NODEIDX_ISO_PMC_SOC_NPI] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_SOC_NPI),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_POWER_NOC },
	},
	[XPM_NODEIDX_ISO_PMC_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PMC_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_POWER_NOC },
	},
	[XPM_NODEIDX_ISO_PL_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_NOC },
	},
	[XPM_NODEIDX_ISO_VCCAUX_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_VCCAUX_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_PMC, PM_POWER_NOC },
	},
	[XPM_NODEIDX_ISO_VCCRAM_SOC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_VCCRAM_SOC),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_NOC },
	},
	[XPM_NODEIDX_ISO_VCCAUX_VCCRAM] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_VCCAUX_VCCRAM),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_NOC },
	},
	[XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR),
		.Node.BaseAddress = PCIEA_ATTRIB_0_FABRICEN,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PCIEA_ATTRIB_0_FABRICEN_ATTR_SHIFT),
		.Polarity = (u8)PM_ACTIVE_LOW,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM },
	},
	[XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR),
		.Node.BaseAddress = PCIEA_ATTRIB_1_FABRICEN,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PCIEA_ATTRIB_1_FABRICEN_ATTR_SHIFT),
		.Polarity = (u8)PM_ACTIVE_LOW,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM },
	},
	[XPM_NODEIDX_ISO_PL_CPM_RST_CPI0] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_RST_CPI0),
		.Node.BaseAddress = CPM_CRCPM_RST_CPI0,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(CPM_CRCPM_RST_CPI0_RESET_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM },
	},
	[XPM_NODEIDX_ISO_PL_CPM_RST_CPI1] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_PL_CPM_RST_CPI1),
		.Node.BaseAddress = CPM_CRCPM_RST_CPI1,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(CPM_CRCPM_RST_CPI1_RESET_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM },
	},
        [XPM_NODEIDX_ISO_GEM_TSU_CLK] = {
                .Node.Id = ISOID(XPM_NODEIDX_ISO_GEM_TSU_CLK),
                .Node.BaseAddress = CRL_RCLK_CTRL,
                .Node.State = (u8)PM_ISOLATION_ON,
                .Mask = CRL_RCLK_CTRL_CLKACT_GEM_TSU_MASK,
                .Polarity = (u8)PM_ACTIVE_LOW,
                .DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_LPD },
        },
        [XPM_NODEIDX_ISO_GEM0_TXRX_CLK] = {
                .Node.Id = ISOID(XPM_NODEIDX_ISO_GEM0_TXRX_CLK),
                .Node.BaseAddress = CRL_RCLK_CTRL,
                .Node.State = (u8)PM_ISOLATION_ON,
                .Mask = CRL_RCLK_CTRL_CLKACT_GEM0_TXRX_MASK,
                .Polarity = (u8)PM_ACTIVE_LOW,
                .DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_LPD },
        },
        [XPM_NODEIDX_ISO_GEM1_TXRX_CLK] = {
                .Node.Id = ISOID(XPM_NODEIDX_ISO_GEM1_TXRX_CLK),
                .Node.BaseAddress = CRL_RCLK_CTRL,
                .Node.State = (u8)PM_ISOLATION_ON,
                .Mask = CRL_RCLK_CTRL_CLKACT_GEM1_TXRX_MASK,
                .Polarity = (u8)PM_ACTIVE_LOW,
                .DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_LPD },
        },
	[XPM_NODEIDX_ISO_LPD_CPM5_DFX] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_CPM5_DFX),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_POWER_CPM5 },
	},
	[XPM_NODEIDX_ISO_LPD_CPM5] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_LPD_CPM5),
		.Node.BaseAddress = PMC_GLOBAL_DOMAIN_ISO_CONTROL,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_SHIFT),
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_POWER_CPM5 },
	},
	[XPM_NODEIDX_ISO_CPM5_PL_DFX] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_CPM5_PL_DFX),
		.Node.BaseAddress = 0U,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = 0U,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM5 },
	},
	[XPM_NODEIDX_ISO_CPM5_PL] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_CPM5_PL),
		.Node.BaseAddress = 0U,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = 0U,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM5 },
	},
	[XPM_NODEIDX_ISO_XRAM_PL_AXI0] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_XRAM_PL_AXI0),
		.Node.BaseAddress = XRAM_SLCR_BASEADDR + XRAM_SLCR_PCSR_PCR_OFFSET,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = XRAM_SLCR_PCSR_ODISABLE_PL_AXI0_MASK,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_LPD },
	},
	[XPM_NODEIDX_ISO_XRAM_PL_AXI1] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_XRAM_PL_AXI1),
		.Node.BaseAddress = XRAM_SLCR_BASEADDR + XRAM_SLCR_PCSR_PCR_OFFSET,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = XRAM_SLCR_PCSR_ODISABLE_PL_AXI1_MASK,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles  = { PM_DEV_PLD_0, PM_POWER_LPD },
	},
	[XPM_NODEIDX_ISO_XRAM_PL_AXI2] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_XRAM_PL_AXI2),
		.Node.BaseAddress = XRAM_SLCR_BASEADDR + XRAM_SLCR_PCSR_PCR_OFFSET,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = XRAM_SLCR_PCSR_ODISABLE_PL_AXI2_MASK,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles  = { PM_DEV_PLD_0, PM_POWER_LPD },
	},
	[XPM_NODEIDX_ISO_XRAM_PL_AXILITE] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_XRAM_PL_AXILITE),
		.Node.BaseAddress = XRAM_SLCR_BASEADDR + XRAM_SLCR_PCSR_PCR_OFFSET,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = XRAM_SLCR_PCSR_ODISABLE_PL_AXILITE_MASK,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles  = { PM_DEV_PLD_0, PM_POWER_LPD },
	},
	[XPM_NODEIDX_ISO_XRAM_PL_FABRIC] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_XRAM_PL_FABRIC),
		.Node.BaseAddress = XRAM_SLCR_BASEADDR + XRAM_SLCR_PCSR_PCR_OFFSET,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = XRAM_SLCR_PCSR_FABRICEN_MASK,
		.Polarity = (u8)PM_ACTIVE_LOW,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_LPD },
	},
	[XPM_NODEIDX_ISO_CPM5_GT_DFX] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_CPM5_GT_DFX),
		.Node.BaseAddress = 0U,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = 0U,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_POWER_CPM5 },
	},
	[XPM_NODEIDX_ISO_CPM5_GT] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_CPM5_GT),
		.Node.BaseAddress = 0U,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = 0U,
		.Polarity = (u8)PM_ACTIVE_HIGH,
		.DependencyNodeHandles = { PM_POWER_LPD, PM_POWER_CPM5 },
	},
	[XPM_NODEIDX_ISO_CPM5_PL_PCIEA0_MPIO] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_CPM5_PL_PCIEA0_MPIO),
		.Node.BaseAddress = CPM5_PCIE0_CSR_FABRICEN,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(CPM5_PCIEA_CSR_FABRICEN_ATTR_SHIFT),
		.Polarity = (u8)PM_ACTIVE_LOW,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM5 },
	},
	[XPM_NODEIDX_ISO_CPM5_PL_PCIEA1_MPIO] = {
		.Node.Id = ISOID(XPM_NODEIDX_ISO_CPM5_PL_PCIEA1_MPIO),
		.Node.BaseAddress = CPM5_PCIE1_CSR_FABRICEN,
		.Node.State = (u8)PM_ISOLATION_ON,
		.Mask = BIT(CPM5_PCIEA_CSR_FABRICEN_ATTR_SHIFT),
		.Polarity = (u8)PM_ACTIVE_LOW,
		.DependencyNodeHandles = { PM_DEV_PLD_0, PM_POWER_CPM5 },
	},
};

static XStatus XPmDomainIso_CheckDependencies(u32 IsoIdx)
{
	XStatus Status = XST_FAILURE;
	u32 i=0, NodeId;
	const XPm_PowerDomain *PwrDomainNode;
	const XPm_Device *Device;

	for (i = 0; i < 2U; i++) {
		NodeId = XPmDomainIso_List[IsoIdx].DependencyNodeHandles[i];
		if (NODECLASS(NodeId) == (u32)XPM_NODECLASS_POWER) {
			PwrDomainNode = (XPm_PowerDomain *) XPmPower_GetById(NodeId);
			if ((NULL != PwrDomainNode) &&
			    (PwrDomainNode->Power.Node.State != (u8)XPM_POWER_STATE_ON)  &&
			    (PwrDomainNode->Power.Node.State != (u8)XPM_POWER_STATE_INITIALIZING)) {
				Status = XST_FAILURE;
				goto done;
			}
			Status = XST_SUCCESS;
		} else if (PM_DEV_PLD_0 == NodeId) {
			Device = XPmDevice_GetById(NodeId);
			if ((NULL != Device) &&
				((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
				((u8)XPM_POWER_STATE_INITIALIZING != Device->Node.State)) {
				Status = XST_FAILURE;
			}
		} else {
			Status = XST_FAILURE;
			goto done;
		}
	}
done:
	return Status;
}

static inline void XramIsoUnmask(u32 IsoIdx)
{
	u32 BaseAddr = XRAM_SLCR_BASEADDR + XRAM_SLCR_PCSR_MASK_OFFSET;
	u32 Mask = XPmDomainIso_List[IsoIdx].Mask;
	XPm_RMW32(BaseAddr, Mask, Mask);
}

static void EnablePlXramIso(void)
{
	u32 i;
	u32 IsoIdx = (u32)XPM_NODEIDX_ISO_XRAM_PL_FABRIC;
	u32 Mask = XPmDomainIso_List[IsoIdx].Mask;

	for (i = (u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0;
		 i <= (u32)(XPM_NODEIDX_ISO_XRAM_PL_AXILITE);
		 ++i) {
		if ((u8)PM_ISOLATION_OFF == XPmDomainIso_List[i].Node.State) {
			goto done;
		}
	}

	if ((u8)PM_ISOLATION_ON != XPmDomainIso_List[IsoIdx].Node.State) {
		XramIsoUnmask((u32)XPM_NODEIDX_ISO_XRAM_PL_FABRIC);
		XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, 0);
		XPmDomainIso_List[IsoIdx].Node.State = (u8)PM_ISOLATION_ON;
	}

done:
		return;
}

static void DisablePlXramIso(void)
{
	u32 IsoIdx = (u32)XPM_NODEIDX_ISO_XRAM_PL_FABRIC;
	u32 Mask = XPmDomainIso_List[IsoIdx].Mask;

	if ((u8)PM_ISOLATION_OFF != XPmDomainIso_List[IsoIdx].Node.State) {
		XramIsoUnmask((u32)XPM_NODEIDX_ISO_XRAM_PL_FABRIC);
		XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, Mask);
		XPmDomainIso_List[IsoIdx].Node.State = (u8)PM_ISOLATION_OFF;
	}

	return;
}

static XStatus XPmDomainIso_SendEventToPsm(u32 IsoIdx, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	if (1U != XPmPsm_FwIsPresent()) {
		DbgErr = XPM_INT_ERR_PSMFW_NOT_PRESENT;
		Status = XST_NOT_ENABLED;
		goto done;
	}

	Payload[0U] = PSM_API_DOMAIN_ISO;
	Payload[1U] = IsoIdx;
	Payload[2U] = Enable;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmDomainIso_Control(u32 IsoIdx, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u32 Mask;
	const XPm_Device *Device = NULL;
	u32 WprotReg;

	if (IsoIdx >= (u32)XPM_NODEIDX_ISO_MAX)
	{
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Mask = XPmDomainIso_List[IsoIdx].Mask;
	if ((IsoIdx <= (u32)XPM_NODEIDX_ISO_XRAM_PL_FABRIC) &&
		(IsoIdx >= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0)) {
		Device = XPmDevice_GetById(PM_DEV_XRAM_0);
		if (NULL == Device) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}

		XPmPsLpDomain_UnlockPcsr(Device->Node.BaseAddress);
	}

	/*
	 * Note: XCVC1902 ES1 has a Si errata where stage 1  portion of PCIe design
	 * did not work for certain configurations. This issue is resolved in ES2.
	 * Resolution in ES2 is to invert spare bit w.r.t. PL_CPM_PCIEA0_FabricEn
	 * (corresponds to XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR iso node). In ES1
	 * there's no effect of writing to sparebit.
	 */
	if ((TRUE_VALUE == Enable) || (TRUE_PENDING_REMOVE == Enable)) {
		if (XPmDomainIso_List[IsoIdx].Polarity == (u8)PM_ACTIVE_HIGH) {
			if (((u32)XPM_NODEIDX_ISO_LPD_CPM5_DFX == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_LPD_CPM5 == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_PL == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_PL_DFX == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_GT == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_GT_DFX == IsoIdx)) {
				Status = XPmDomainIso_SendEventToPsm(IsoIdx,
								     TRUE_VALUE);
			} else {
				if (((u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0 <= IsoIdx) &&
				    ((u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE >= IsoIdx)) {
					XramIsoUnmask(IsoIdx);
				}
				XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress,
					  Mask, Mask);
			}
		} else {
			if (((u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0 <= IsoIdx) &&
			    ((u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE >= IsoIdx)) {
				XramIsoUnmask(IsoIdx);
			}
			XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, 0);
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_DMA_ATTR_DMA_SPARE_3_H,
					  PCIEA_ATTRIB_DMA_ATTR_DMA_SPARE_3_H_MASK,
					  PCIEA_ATTRIB_DMA_ATTR_DMA_SPARE_3_H_MASK);
			}
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_0_DPLL,
						PCIEA_ATTRIB_DPLL_DPLL_RESET_MASK, 0x1);
			}
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_1_DPLL,
						PCIEA_ATTRIB_DPLL_DPLL_RESET_MASK, 0x1);
			}

			if ((u32)XPM_NODEIDX_ISO_CPM5_PL_PCIEA0_MPIO == IsoIdx) {
				/* Store CPM5_DMA0_ATTR_WPROTP register value */
				WprotReg = XPm_In32(CPM5_DMA0_ATTR_WPROTP);
				/* Enable attribute writes */
				XPm_Out32(CPM5_DMA0_ATTR_WPROTP, 0x0);

				XPm_RMW32(CPM5_DMA0_ATTRIB_ATTR_DMA_SPARE_3_H,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK);

				/* Restore CPM5_DMA0_ATTR_WPROTP register value */
				XPm_Out32(CPM5_DMA0_ATTR_WPROTP, WprotReg);
			}

			if ((u32)XPM_NODEIDX_ISO_CPM5_PL_PCIEA1_MPIO == IsoIdx) {
				/* Store CPM5_DMA1_ATTR_WPROTP register value */
				WprotReg = XPm_In32(CPM5_DMA1_ATTR_WPROTP);
				/*Enable attribute writes */
				XPm_Out32(CPM5_DMA1_ATTR_WPROTP, 0x0);

				XPm_RMW32(CPM5_DMA1_ATTRIB_ATTR_DMA_SPARE_3_H,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK);

				/* Restore CPM5_DMA1_ATTR_WPROTP register value */
				XPm_Out32(CPM5_DMA1_ATTR_WPROTP, WprotReg);
			}
		}
		/* Mark node state appropriately */
		XPmDomainIso_List[IsoIdx].Node.State = (TRUE_VALUE == Enable) ?
			(u8)PM_ISOLATION_ON : (u8)PM_ISOLATION_REMOVE_PENDING;
		if ((IsoIdx <= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE) &&
			(IsoIdx >= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0)) {
			 EnablePlXramIso();
		}
	} else if(Enable == FALSE_IMMEDIATE) {
		if ((IsoIdx <= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE) &&
			(IsoIdx >= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0)) {
			DisablePlXramIso();
		}
		if (XPmDomainIso_List[IsoIdx].Polarity == (u8)PM_ACTIVE_HIGH) {
			if (((u32)XPM_NODEIDX_ISO_LPD_CPM5_DFX == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_LPD_CPM5 == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_PL == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_PL_DFX == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_GT == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_GT_DFX == IsoIdx)) {
				Status = XPmDomainIso_SendEventToPsm(IsoIdx,
								     FALSE_VALUE);
			} else {
				if (((u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0 <= IsoIdx) &&
				    ((u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE >= IsoIdx)) {
					XramIsoUnmask(IsoIdx);
				}
				XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress,
					  Mask, 0U);
			}
		} else {
			if (((u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0 <= IsoIdx) &&
			    ((u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE >= IsoIdx)) {
				XramIsoUnmask(IsoIdx);
			}
			XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, Mask);
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_DMA_ATTR_DMA_SPARE_3_H,
					  PCIEA_ATTRIB_DMA_ATTR_DMA_SPARE_3_H_MASK,
					  0U);
			}
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_0_DPLL,
						PCIEA_ATTRIB_DPLL_DPLL_RESET_MASK, 0x0);
			}
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_1_DPLL,
						PCIEA_ATTRIB_DPLL_DPLL_RESET_MASK, 0x0);
			}

			if ((u32)XPM_NODEIDX_ISO_CPM5_PL_PCIEA0_MPIO == IsoIdx) {
				/* Store CPM5_DMA0_ATTR_WPROTP register value */
				WprotReg = XPm_In32(CPM5_DMA0_ATTR_WPROTP);
				/* Enable attribute writes */
				XPm_Out32(CPM5_DMA0_ATTR_WPROTP, 0x0);

				XPm_RMW32(CPM5_DMA0_ATTRIB_ATTR_DMA_SPARE_3_H,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK, 0x0);

				/* Restore CPM5_DMA0_ATTR_WPROTP register value */
				XPm_Out32(CPM5_DMA0_ATTR_WPROTP, WprotReg);
			}

			if ((u32)XPM_NODEIDX_ISO_CPM5_PL_PCIEA1_MPIO == IsoIdx) {
				/* Store CPM5_DMA1_ATTR_WPROTP register value */
				WprotReg = XPm_In32(CPM5_DMA1_ATTR_WPROTP);
				/*Enable attribute writes */
				XPm_Out32(CPM5_DMA1_ATTR_WPROTP, 0x0);

				XPm_RMW32(CPM5_DMA1_ATTRIB_ATTR_DMA_SPARE_3_H,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK, 0x0);

				/* Restore CPM5_DMA1_ATTR_WPROTP register value */
				XPm_Out32(CPM5_DMA1_ATTR_WPROTP, WprotReg);
			}
		}
		XPmDomainIso_List[IsoIdx].Node.State = (u8)PM_ISOLATION_OFF;
	} else {
		Status = XPmDomainIso_CheckDependencies(IsoIdx);
		if(XST_SUCCESS != Status)
		{
			/* Mark it pending */
			XPmDomainIso_List[IsoIdx].Node.State = (u8)PM_ISOLATION_REMOVE_PENDING;
			Status = XST_SUCCESS;
			goto done;
		}
		if ((IsoIdx <= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE) &&
			(IsoIdx >= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0)) {
			DisablePlXramIso();
		}
		if (XPmDomainIso_List[IsoIdx].Polarity == (u8)PM_ACTIVE_HIGH) {
			if (((u32)XPM_NODEIDX_ISO_LPD_CPM5_DFX == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_LPD_CPM5 == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_PL == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_PL_DFX == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_GT == IsoIdx) ||
			    ((u32)XPM_NODEIDX_ISO_CPM5_GT_DFX == IsoIdx)) {
				Status = XPmDomainIso_SendEventToPsm(IsoIdx,
								     FALSE_VALUE);
			} else {
				if (((u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0 <= IsoIdx) &&
				    ((u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE >= IsoIdx)) {
					XramIsoUnmask(IsoIdx);
				}
				XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress,
					  Mask, 0U);
			}
		} else {
			if (((u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0 <= IsoIdx) &&
			    ((u32)XPM_NODEIDX_ISO_XRAM_PL_AXILITE >= IsoIdx)) {
				XramIsoUnmask(IsoIdx);
			}
			XPm_RMW32(XPmDomainIso_List[IsoIdx].Node.BaseAddress, Mask, Mask);
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_DMA_ATTR_DMA_SPARE_3_H,
					  PCIEA_ATTRIB_DMA_ATTR_DMA_SPARE_3_H_MASK,
					  0U);
			}
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_0_DPLL,
						PCIEA_ATTRIB_DPLL_DPLL_RESET_MASK, 0x0);
			}
			if ((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR == IsoIdx) {
				XPm_RMW32(PCIEA_ATTRIB_1_DPLL,
						PCIEA_ATTRIB_DPLL_DPLL_RESET_MASK, 0x0);
			}

			if ((u32)XPM_NODEIDX_ISO_CPM5_PL_PCIEA0_MPIO == IsoIdx) {
				/* Store CPM5_DMA0_ATTR_WPROTP register value */
				WprotReg = XPm_In32(CPM5_DMA0_ATTR_WPROTP);
				/* Enable attribute writes */
				XPm_Out32(CPM5_DMA0_ATTR_WPROTP, 0x0);

				XPm_RMW32(CPM5_DMA0_ATTRIB_ATTR_DMA_SPARE_3_H,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK, 0x0);

				/* Restore CPM5_DMA0_ATTR_WPROTP register value */
				XPm_Out32(CPM5_DMA0_ATTR_WPROTP, WprotReg);
			}

			if ((u32)XPM_NODEIDX_ISO_CPM5_PL_PCIEA1_MPIO == IsoIdx) {
				/* Store CPM5_DMA1_ATTR_WPROTP register value */
				WprotReg = XPm_In32(CPM5_DMA1_ATTR_WPROTP);
				/*Enable attribute writes */
				XPm_Out32(CPM5_DMA1_ATTR_WPROTP, 0x0);

				XPm_RMW32(CPM5_DMA1_ATTRIB_ATTR_DMA_SPARE_3_H,
						CPM5_DMA_ATTRIB_ATTR_DMA_SPARE_3_H_MASK, 0x0);

				/* Restore CPM5_DMA1_ATTR_WPROTP register value */
				XPm_Out32(CPM5_DMA1_ATTR_WPROTP, WprotReg);
			}
		}
		XPmDomainIso_List[IsoIdx].Node.State = (u8)PM_ISOLATION_OFF;
	}

	Status = XST_SUCCESS;

done:
	if ((IsoIdx <= (u32)XPM_NODEIDX_ISO_XRAM_PL_FABRIC) &&
		(IsoIdx >= (u32)XPM_NODEIDX_ISO_XRAM_PL_AXI0)) {
		XPmPsLpDomain_LockPcsr(Device->Node.BaseAddress);
	}

	return Status;
}

XStatus XPmDomainIso_ProcessPending()
{
	XStatus Status = XST_FAILURE;
	u32 i;


	for(i=0; i< ARRAY_SIZE(XPmDomainIso_List); i++)
	{
		if (XPmDomainIso_List[i].Node.State == (u8)PM_ISOLATION_REMOVE_PENDING) {
			Status = XPmDomainIso_Control(i, FALSE_VALUE);
		} else {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

XStatus XPmDomainIso_GetState(u32 IsoIdx, XPm_IsoStates *State)
{
	XStatus Status = XST_FAILURE;
	u32 Mask, Base, Polarity;

	if ((IsoIdx >= (u32)XPM_NODEIDX_ISO_MAX) || (NULL == State)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* TODO: Implement GetState for isolations present in PSM */

	Mask = XPmDomainIso_List[IsoIdx].Mask;
	Base = XPmDomainIso_List[IsoIdx].Node.BaseAddress;
	Polarity = XPmDomainIso_List[IsoIdx].Polarity;

	if (Mask == (XPm_In32(Base) & Mask)) {
		*State = (Polarity == (u32)PM_ACTIVE_HIGH)?
			PM_ISOLATION_ON : PM_ISOLATION_OFF;
	} else {
		*State = (Polarity == (u32)PM_ACTIVE_HIGH)?
			PM_ISOLATION_OFF : PM_ISOLATION_ON;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
