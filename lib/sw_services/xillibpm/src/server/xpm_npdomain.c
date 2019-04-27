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

#include "sleep.h"
#include "xpm_common.h"
#include "xpm_core.h"
#include "xpm_domain_iso.h"
#include "xpm_npdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_regs.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"

u32 NpdDdrMcAddressList[] = { 	0xF6110000, //DDRMC_UB_0
				0xF6280000, //DDRMC_UB_1
				0xF63F0000, //DDRMC_UB_2
				0xF6560000, //DDRMC_UB_3
			};

static XStatus NpdPreHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	/* Check vccint_soc first to make sure power is on */
	if (XST_SUCCESS != XPmPower_CheckPower(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* TODO: Request PMC to power up VCCINT_SOC rail and wait for the acknowledgement.*/
		goto done;
	}

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release POR for NoC */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NOC_POR),
				     PM_RESET_ACTION_RELEASE);

done:
	return Status;
}

static void NpdPreBisrReqs()
{
	/* Release NPI Reset */
	XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NPI),
			PM_RESET_ACTION_RELEASE);

	/* Release NoC Reset */
	XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NOC),
			PM_RESET_ACTION_RELEASE);

	/* Release Sys Resets */
	XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_SYS_RST_1),
			PM_RESET_ACTION_RELEASE);
	XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_SYS_RST_2),
			PM_RESET_ACTION_RELEASE);
	XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_SYS_RST_3),
			PM_RESET_ACTION_RELEASE);

	return;
}

static XStatus NpdPostHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 i=0;

	(void)Args;
	(void)NumOfArgs;

	/* NPD pre bisr requirements - in case if bisr and mbist was skipped */
	NpdPreBisrReqs();

	if (XST_SUCCESS == XPmPower_CheckPower(	PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* Remove vccaux-soc domain isolation */
		Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCAUX_SOC, FALSE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_SOC, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*TODO: Pass these addresses from CDO later */
	u32 NpdNmuAddressList[] = { 	0xF6010000, //NOC_NMU_0
					0xF6012000, //NOC_NMU_1
					0xF6620000, //NOC_NMU_10
					0xF6650000, //NOC_NMU_11
					0xF6680000, //NOC_NMU_12
					0xF66A0000, //NOC_NMU_13
					0xF66D0000, //NOC_NMU_14
					0xF66F0000, //NOC_NMU_15
					0xF6720000, //NOC_NMU_16
					0xF6790000, //NOC_NMU_17
					0xF67C0000, //NOC_NMU_18
					0xF67E0000, //NOC_NMU_19
					0xF6014000, //NOC_NMU_2
					0xF6810000, //NOC_NMU_20
					0xF6830000, //NOC_NMU_21
					0xF6860000, //NOC_NMU_22
					0xF6AC0000, //NOC_NMU_23
					0xF6AF0000, //NOC_NMU_24
					0xF6B20000, //NOC_NMU_25
					0xF6B40000, //NOC_NMU_26
					0xF6B70000, //NOC_NMU_27
					0xF6B90000, //NOC_NMU_28
					0xF6BC0000, //NOC_NMU_29
					0xF6016000, //NOC_NMU_3
					0xF6C30000, //NOC_NMU_30
					0xF6C60000, //NOC_NMU_31
					0xF6C90000, //NOC_NMU_32
					0xF6CC0000, //NOC_NMU_33
					0xF6CF0000, //NOC_NMU_34
					0xF6D10000, //NOC_NMU_35
					0xF6D40000, //NOC_NMU_36
					0xF6D60000, //NOC_NMU_37
					0xF6D90000, //NOC_NMU_38
					0xF6E00000, //NOC_NMU_39
					0xF60E0000, //NOC_NMU_4
					0xF6E30000, //NOC_NMU_40
					0xF6E60000, //NOC_NMU_41
					0xF6E90000, //NOC_NMU_42
					0xF6EC0000, //NOC_NMU_43
					0xF6EE0000, //NOC_NMU_44
					0xF6F10000, //NOC_NMU_45
					0xF6F30000, //NOC_NMU_46
					0xF6F60000, //NOC_NMU_47
					0xF6FD0000, //NOC_NMU_48
					0xF7000000, //NOC_NMU_49
					0xF60E2000, //NOC_NMU_5
					0xF7020000, //NOC_NMU_50
					0xF7050000, //NOC_NMU_51
					0xF7070000, //NOC_NMU_52
					0xF70B0000, //NOC_NMU_53
					0xF60F0000, //NOC_NMU_6
					0xF60F2000, //NOC_NMU_7
					0xF60F4000, //NOC_NMU_8
					0xF60F6000, //NOC_NMU_9
					};
	u32 NpdNsuAddressList[] = { 	0xF6020000, //NOC_NSU_0
					0xF6022000, //NOC_NSU_1
					0xF66D2000, //NOC_NSU_10
					0xF66F2000, //NOC_NSU_11
					0xF6722000, //NOC_NSU_12
					0xF6792000, //NOC_NSU_13
					0xF67C2000, //NOC_NSU_14
					0xF67E2000, //NOC_NSU_15
					0xF6812000, //NOC_NSU_16
					0xF6832000, //NOC_NSU_17
					0xF6862000, //NOC_NSU_18
					0xF6AC2000, //NOC_NSU_19
					0xF60D0000, //NOC_NSU_2
					0xF6AF2000, //NOC_NSU_20
					0xF6B22000, //NOC_NSU_21
					0xF6B42000, //NOC_NSU_22
					0xF6B72000, //NOC_NSU_23
					0xF6B92000, //NOC_NSU_24
					0xF6BC2000, //NOC_NSU_25
					0xF6C32000, //NOC_NSU_26
					0xF6C62000, //NOC_NSU_27
					0xF6C92000, //NOC_NSU_28
					0xF6CC2000, //NOC_NSU_29
					0xF60D2000, //NOC_NSU_3
					0xF6CF2000, //NOC_NSU_30
					0xF6D12000, //NOC_NSU_31
					0xF6D42000, //NOC_NSU_32
					0xF6D62000, //NOC_NSU_33
					0xF6D92000, //NOC_NSU_34
					0xF6E02000, //NOC_NSU_35
					0xF6E32000, //NOC_NSU_36
					0xF6E62000, //NOC_NSU_37
					0xF6E92000, //NOC_NSU_38
					0xF6EC2000, //NOC_NSU_39
					0xF60E4000, //NOC_NSU_4
					0xF6EE2000, //NOC_NSU_40
					0xF6F12000, //NOC_NSU_41
					0xF6F32000, //NOC_NSU_42
					0xF6F62000, //NOC_NSU_43
					0xF6FD2000, //NOC_NSU_44
					0xF7002000, //NOC_NSU_45
					0xF7022000, //NOC_NSU_46
					0xF7052000, //NOC_NSU_47
					0xF7072000, //NOC_NSU_48
					0xF70B2000, //NOC_NSU_49
					0xF60E6000, //NOC_NSU_5
					0xF6622000, //NOC_NSU_6
					0xF6652000, //NOC_NSU_7
					0xF6682000, //NOC_NSU_8
					0xF66A2000, //NOC_NSU_9
					};

	/* Assert ODISABLE NPP for all NMU and NSU*/
	for(i=0; i<ARRAY_SIZE(NpdNmuAddressList); i++)
	{
		PmOut32(NpdNmuAddressList[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmOut32(NpdNmuAddressList[i] + NPI_PCSR_MASK_OFFSET, NPI_PCSR_CONTROL_ODISABLE_NPP_MASK)
		PmOut32(NpdNmuAddressList[i] + NPI_PCSR_CONTROL_OFFSET, NPI_PCSR_CONTROL_ODISABLE_NPP_MASK);
		PmOut32(NpdNmuAddressList[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}
	for(i=0; i<ARRAY_SIZE(NpdNsuAddressList); i++)
	{
		PmOut32(NpdNsuAddressList[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmOut32(NpdNsuAddressList[i] + NPI_PCSR_MASK_OFFSET, NPI_PCSR_CONTROL_ODISABLE_NPP_MASK)
		PmOut32(NpdNsuAddressList[i] + NPI_PCSR_CONTROL_OFFSET, NPI_PCSR_CONTROL_ODISABLE_NPP_MASK);
		PmOut32(NpdNsuAddressList[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

	/* Deassert UB_INITSTATE for DDR blocks */
	for(i=0; i<ARRAY_SIZE(NpdDdrMcAddressList); i++)
	{
		PmOut32(NpdDdrMcAddressList[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmOut32(NpdDdrMcAddressList[i] + NPI_PCSR_MASK_OFFSET, NPI_DDRMC_PSCR_CONTROL_UB_INITSTATE_MASK)
		PmOut32(NpdDdrMcAddressList[i] + NPI_PCSR_CONTROL_OFFSET, 0);
		PmOut32(NpdDdrMcAddressList[i] + NPI_PCSR_LOCK_OFFSET, 1);
		/* Ony UB0 for non sillicon platforms */
		if (PLATFORM_VERSION_SILICON != Platform) {
			Status = XST_SUCCESS;
			goto done;
		}
	}
done:
	return Status;
}

static XStatus NpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	XPm_Core *Pmc;
	u32 RegValue;

	Pmc = (XPm_Core *)PmDevices[XPM_NODEIDX_DEV_PMC_PROC];
	if (NULL == Pmc) {
		Status = XST_FAILURE;
		goto done;
	}

	PmOut32((Pmc->RegAddress[0] + PMC_GLOBAL_ERR1_STATUS_OFFSET),
		(PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
		PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK));

	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK);

	usleep(200);

	PmIn32((Pmc->RegAddress[0] + PMC_GLOBAL_ERR1_STATUS_OFFSET),
	       RegValue);
	if (0 != (RegValue & (PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
			     PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK))) {
		Status = XST_FAILURE;
	}

done:
	return Status;
}

static XStatus NpdMbist(u32 *AddressList, u32 NumOfAddress)
{
	XStatus Status = XST_SUCCESS;
	u32 RegValue;
	u32 i;

	/* NPD pre bisr requirements - in case if bisr was skipped */
	NpdPreBisrReqs();

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Deassert PCSR Lock*/
	for(i=0; i<NumOfAddress; i++)
	{
		PmOut32(AddressList[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
	}

	/* Enable ILA clock for DDR blocks*/
	for(i=0; i<ARRAY_SIZE(NpdDdrMcAddressList); i++)
	{
		PmRmw32(NpdDdrMcAddressList[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET, NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK, NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK);
	}

	for(i=0; i<NumOfAddress; i++)
	{
		PmOut32(AddressList[i] + NPI_PCSR_MASK_OFFSET, NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK)
		PmOut32(AddressList[i] + NPI_PCSR_CONTROL_OFFSET, NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
	}
	for(i=0; i<NumOfAddress; i++)
	{
		Status = XPm_PollForMask(AddressList[i] + NPI_PCSR_STATUS_OFFSET,
				 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	for(i=0; i<NumOfAddress; i++)
	{
		PmIn32(AddressList[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			Status = XST_FAILURE;
			goto done;
		}
	}

	/* Disable ILA clock for DDR blocks*/
	for(i=0; i<ARRAY_SIZE(NpdDdrMcAddressList); i++)
	{
		PmRmw32(NpdDdrMcAddressList[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET, NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK, 0);
	}

	/* Assert PCSR Lock*/
	for(i=0; i<NumOfAddress; i++)
	{
		PmOut32(AddressList[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}
done:
	return Status;
}

static XStatus NpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 i = 0;

	(void)Args;
	(void)NumOfArgs;

	/* NPD pre bisr requirements */
	NpdPreBisrReqs();


	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Enable Bisr clock */
	for(i=0; i<ARRAY_SIZE(NpdDdrMcAddressList); i++)
	{
		/* Unlock writes */
		PmOut32(NpdDdrMcAddressList[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmRmw32(NpdDdrMcAddressList[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET, NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK, NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK);
	}

	/* Run BISR */
	Status = XPmBisr_Repair(DDRMC_TAG_ID);

	/* Disable Bisr clock */
	for(i=0; i<ARRAY_SIZE(NpdDdrMcAddressList); i++)
	{
		PmRmw32(NpdDdrMcAddressList[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET, NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK, 0);
		/* Lock writes */
		PmOut32(NpdDdrMcAddressList[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

done:
	return Status;
}

struct XPm_PowerDomainOps NpdOps = {
	.PreHouseClean = NpdPreHouseclean,
	.PostHouseClean = NpdPostHouseclean,
	.ScanClear = NpdScanClear,
	.Mbist = NpdMbist,
	.Bisr = NpdBisr,
};

XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, &NpdOps);

	return XST_SUCCESS;
}
