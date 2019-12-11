/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "sleep.h"
#include "xpm_common.h"
#include "xpm_pmc.h"
#include "xpm_domain_iso.h"
#include "xpm_npdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_regs.h"
#include "xpm_clock.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"

#define XPM_NODEIDX_DEV_DDRMC_MIN	XPM_NODEIDX_DEV_DDRMC_0
#define XPM_NODEIDX_DEV_DDRMC_MAX	XPM_NODEIDX_DEV_DDRMC_3

static u32 NpdMemIcAddresses[XPM_NODEIDX_MEMIC_MAX];

static XStatus NpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 NpdPowerUpTime = 0;

	(void)Args;
	(void)NumOfArgs;

	/* Check vccint_soc first to make sure power is on */
	while (XST_SUCCESS != XPmPower_CheckPower(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* Wait for VCCINT_SOC power up */
		usleep(10);
		NpdPowerUpTime++;
		if (NpdPowerUpTime > XPM_POLL_TIMEOUT) {
			/* TODO: Request PMC to power up VCCINT_SOC rail and wait for the acknowledgement.*/
			Status = XST_FAILURE;
			goto done;
		}
	}
	if (PLATFORM_VERSION_SILICON == Platform) {
		/* TODO: This is a temporary fix for MGT boards;
		 * Remove the delay once AMS solution to read rail voltages is finalized.
		 */
		usleep(1000);
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release POR for NoC */
	Status = XPmReset_AssertbyId(PM_RST_NOC_POR, (u32)PM_RESET_ACTION_RELEASE);

done:
	return Status;
}

static void NpdPreBisrReqs(void)
{
	/* Release NPI Reset */
	(void)XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_RELEASE);

	/* Release NoC Reset */
	(void)XPmReset_AssertbyId(PM_RST_NOC, (u32)PM_RESET_ACTION_RELEASE);

	/* Release Sys Resets */
	(void)XPmReset_AssertbyId(PM_RST_SYS_RST_1, (u32)PM_RESET_ACTION_RELEASE);
	(void)XPmReset_AssertbyId(PM_RST_SYS_RST_2, (u32)PM_RESET_ACTION_RELEASE);
	(void)XPmReset_AssertbyId(PM_RST_SYS_RST_3, (u32)PM_RESET_ACTION_RELEASE);

	return;
}

static XStatus NpdInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 i=0;
	XPm_Device *Device;
	u32 BaseAddress;

	(void)Args;
	(void)NumOfArgs;

	/* NPD pre bisr requirements - in case if bisr and mbist was skipped */
	NpdPreBisrReqs();

	if (XST_SUCCESS == XPmPower_CheckPower(	PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* Remove vccaux-soc domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_SOC, FALSE_VALUE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Assert ODISABLE NPP for all NMU and NSU
	 * This step is omitted for SSIT Device Slave SLR for  NSU_1 as config is
	 * done by BOOT ROM
	 */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses) && (0U != NpdMemIcAddresses[i]); i++) {
			if (i == (u32)XPM_NODEIDX_MEMIC_NSU_1 &&
				SlrType < SLR_TYPE_SSIT_DEV_MASTER_SLR) {
				continue;
			}

			PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET,
				PCSR_UNLOCK_VAL);
			PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
				NPI_PCSR_CONTROL_ODISABLE_NPP_MASK);
			PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
				NPI_PCSR_CONTROL_ODISABLE_NPP_MASK);
			PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

	/* Deassert UB_INITSTATE for DDR blocks */
	for (i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID(i));
		if (NULL != Device) {
			BaseAddress = Device->Node.BaseAddress;
			PmOut32(BaseAddress + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
			PmOut32(BaseAddress + NPI_PCSR_MASK_OFFSET,
				NPI_DDRMC_PSCR_CONTROL_UB_INITSTATE_MASK);
			PmOut32(BaseAddress + NPI_PCSR_CONTROL_OFFSET, 0);
			PmOut32(BaseAddress + NPI_PCSR_LOCK_OFFSET, 1);
			/* Only UB0 for non sillicon platforms */
			if (PLATFORM_VERSION_SILICON != Platform) {
				Status = XST_SUCCESS;
				break;
			}
		}
	}
	/* When NPD is powered, copy sysmon data */
	for (i = (u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN; i < (u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MAX; i++) {
		/* Copy_trim< AMS_SAT_N> */
		if (0U != SysmonAddresses[i]) {
			Status = XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[i], PM_POWER_NOC, i-(u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	/* Assert pcomplete to indicate HC is done and NoC is ready to use */
	/* Unlock PCSR Register*/
	PmOut32(NPI_BASEADDR + NPI_NIR_0_OFFSET + NPI_PCSR_LOCK_OFFSET,
		PCSR_UNLOCK_VAL);

	/* Unmask the pcomplete bit */
	PmOut32(NPI_BASEADDR + NPI_NIR_0_OFFSET + NPI_PCSR_MASK_OFFSET,
		NPI_PCSR_CONTROL_PCOMPLETE_MASK);

	/*Assert control on pcomplete bit*/
	PmOut32(NPI_BASEADDR + NPI_NIR_0_OFFSET + NPI_PCSR_CONTROL_OFFSET,
		NPI_PCSR_CONTROL_PCOMPLETE_MASK);

	/*Lock PCSR Register */
	PmOut32(NPI_BASEADDR + NPI_NIR_0_OFFSET + NPI_PCSR_LOCK_OFFSET,
		0x1);

done:
	return Status;
}

static XStatus NpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_Pmc *Pmc;
	u32 RegValue;
	XPm_OutClockNode *Clk;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (SlrType != SLR_TYPE_MONOLITHIC_DEV &&
		SlrType != SLR_TYPE_SSIT_DEV_MASTER_SLR) {
		PmDbg("Skipping Scan-Clear of NPD for Slave SLR\n\r");
		Status = XST_SUCCESS;
		goto done;
	}

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XST_FAILURE;
		goto done;
	}

	PmOut32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_ERR1_STATUS_OFFSET),
		(PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
		PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK));

	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK);

	/* 200 us is not enough and scan clear pass status is updated
		after so increasing delay for scan clear to finish */
	usleep(400);

	/* Enable NPI Clock */
	Clk = (XPm_OutClockNode *)XPmClock_GetByIdx((u32)XPM_NODEIDX_CLK_NPI_REF);
	Status = XPmClock_SetGate((XPm_OutClockNode *)Clk, 1);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release NPI Reset */
	Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmIn32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_ERR1_STATUS_OFFSET),
	       RegValue);
	if (0U != (RegValue & (PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
			     PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK))) {
		Status = XST_FAILURE;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus NpdMbist(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u32 i;
	XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1] = {0};

	(void)Args;
	(void)NumOfArgs;

	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID((u32)XPM_NODEIDX_DEV_DDRMC_MIN + i));
		if (NULL != Device) {
			DdrMcAddresses[i] = Device->Node.BaseAddress;
		}
	}

	/* NPD pre bisr requirements - in case if bisr was skipped */
	NpdPreBisrReqs();

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Deassert PCSR Lock*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses) && (0U != NpdMemIcAddresses[i]); i++) {
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET,
			PCSR_UNLOCK_VAL);
	}

	/* Enable ILA clock for DDR blocks*/
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK);
	}

	/* Trigger Mem clear */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses) && (0U != NpdMemIcAddresses[i]); i++) {
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
	}

	/* Check for Mem clear done */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses) && (0U != NpdMemIcAddresses[i]); i++) {
		Status = XPm_PollForMask(NpdMemIcAddresses[i] +
					 NPI_PCSR_STATUS_OFFSET,
					 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
					 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		Status = XPm_PollForMask(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET,
				 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Check for Mem clear Pass/Fail */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses) && (0U != NpdMemIcAddresses[i]); i++) {
		PmIn32(NpdMemIcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			Status = XST_FAILURE;
			goto done;
		}
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		PmIn32(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			Status = XST_FAILURE;
			goto done;
		}
	}

	/* Disable ILA clock for DDR blocks*/
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK, 0);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}


	/* Unwrite trigger bits */
        for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses) && (0U != NpdMemIcAddresses[i]); i++) {
                PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
                        NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK)
                PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET, 0);
        }
        for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
                PmOut32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
                        NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET, 0);
	}

	/* Assert PCSR Lock*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses) && (0U != NpdMemIcAddresses[i]); i++) {
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus NpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0;
	XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1] = {0};

	(void)Args;
	(void)NumOfArgs;

	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID((u32)XPM_NODEIDX_DEV_DDRMC_MIN + i));
		if (NULL != Device) {
			DdrMcAddresses[i] = Device->Node.BaseAddress;
		}
	}

	/* NPD pre bisr requirements */
	NpdPreBisrReqs();


	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Enable Bisr clock */
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		/* Unlock writes */
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK,
			NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK);
	}

	/* Run BISR */
	Status = XPmBisr_Repair(DDRMC_TAG_ID);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Disable Bisr clock */
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK, 0);
		/* Lock writes */
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

	/* NIDB Lane Repair */
	Status = XPmBisr_NidbLaneRepair();

done:
	return Status;
}

static struct XPm_PowerDomainOps NpdOps = {
	.InitStart = NpdInitStart,
	.InitFinish = NpdInitFinish,
	.ScanClear = NpdScanClear,
	.Mbist = NpdMbist,
	.Bisr = NpdBisr,
};

XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, &NpdOps);
	if (XST_SUCCESS == Status) {
		Npd->BisrDataCopied = 0;
	}

	return Status;
}

XStatus XPmNpDomain_MemIcInit(u32 DeviceId, u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;
	u32 Idx = NODEINDEX(DeviceId);
	u32 Type = NODETYPE(DeviceId);

	if ((((u32)XPM_NODETYPE_MEMIC_SLAVE != Type) &&
	    ((u32)XPM_NODETYPE_MEMIC_MASTER != Type)) ||
	    ((u32)XPM_NODEIDX_MEMIC_MAX <= Idx)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NpdMemIcAddresses[Idx] = BaseAddr;

	Status = XST_SUCCESS;

done:
	return Status;
}
