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
#include "xpm_debug.h"
#include "xpm_rail.h"
#include "xpm_device.h"

#define XPM_NODEIDX_DEV_DDRMC_MIN	XPM_NODEIDX_DEV_DDRMC_0
#define XPM_NODEIDX_DEV_DDRMC_MAX	XPM_NODEIDX_DEV_DDRMC_3

static u32 NpdMemIcAddresses[XPM_NODEIDX_MEMIC_MAX];

static XStatus NpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 NpdPowerUpTime = 0;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCC_SOC);

	/* Check vccint_soc first to make sure power is on */
	while (XST_SUCCESS != Status) {
		Status = XPmPower_CheckPower(VccSocRail, PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);

		usleep(10);
		NpdPowerUpTime++;
		if (NpdPowerUpTime > XPM_POLL_TIMEOUT) {
			/* TODO: Request PMC to power up VCCINT_SOC rail and wait for the acknowledgement.*/
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
			Status = XST_FAILURE;
			goto done;
		}
	}
	if ((NULL == VccSocRail) || (VccSocRail->Source != XPM_PGOOD_SYSMON)) {
		if (PLATFORM_VERSION_SILICON == XPm_GetPlatform()) {
			/* This is a workaround for MGT boards when sysmon is not used.
			 * The delay is required to wait for power rails to stabilize
			 */
			usleep(1000);
		}
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_NPI_ISO;
		goto done;
	}

	/* Release POR for NoC */
	Status = XPmReset_AssertbyId(PM_RST_NOC_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
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
	XStatus SocRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	u32 i=0;
	XPm_Device *Device;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SlrType;
	u32 SysmonAddr;

	(void)Args;
	(void)NumOfArgs;

	XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCC_SOC);
	XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);

	/* NPD pre bisr requirements - in case if bisr and mbist was skipped */
	NpdPreBisrReqs();

	SocRailPwrSts = XPmPower_CheckPower(VccSocRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);
	AuxRailPwrSts = XPmPower_CheckPower(VccauxRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
	if ((XST_SUCCESS == SocRailPwrSts) && (XST_SUCCESS == AuxRailPwrSts)) {
		/* Remove vccaux-soc domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_SOC, FALSE_VALUE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_VCCAUX_VCCRAM_ISO;
			goto done;
		}
	}

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_ISO;
		goto done;
	}

	/* Assert ODISABLE NPP for all NMU and NSU
	 * This step is omitted for SSIT Device Slave SLR for  NSU_1 as config is
	 * done by BOOT ROM
	 */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
			if (0U == NpdMemIcAddresses[i]) {
				continue;
			}

			SlrType = XPm_GetSlrType();
			if (i == (u32)XPM_NODEIDX_MEMIC_NSU_1 &&
				SlrType < (u32)SLR_TYPE_SSIT_DEV_MASTER_SLR) {
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
			if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
				Status = XST_SUCCESS;
				break;
			}
		}
	}
	/* When NPD is powered, copy sysmon data */
	for (i = (u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN; i < (u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MAX; i++) {
		/* Copy_trim< AMS_SAT_N> */
		SysmonAddr = XPm_GetSysmonByIndex(i);
		if (0U != SysmonAddr) {
			Status = XPmPowerDomain_ApplyAmsTrim(SysmonAddr, PM_POWER_NOC, i-(u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_AMS_TRIM;
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
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus NpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_Pmc *Pmc;
	u32 RegValue;
	u32 SlrType;
	XPm_OutClockNode *Clk;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	SlrType = XPm_GetSlrType();
	if (SlrType != SLR_TYPE_MONOLITHIC_DEV &&
		SlrType != SLR_TYPE_SSIT_DEV_MASTER_SLR) {
		PmDbg("Skipping Scan-Clear of NPD for Slave SLR\n\r");
		Status = XST_SUCCESS;
		goto done;
	}

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* PMC_ERR1_STATUS is the write-to-clear register */
	PmOut32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_ERR1_STATUS_OFFSET),
		(PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
		PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK));

	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
			 PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
			 PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPDLSCNCLR_TRIGGER;
		goto done;
	}

	/* 200 us is not enough and scan clear pass status is updated
		after so increasing delay for scan clear to finish */
	usleep(400);

	/* Enable NPI Clock */
	Clk = (XPm_OutClockNode *)XPmClock_GetByIdx((u32)XPM_NODEIDX_CLK_NPI_REF);
	Status = XPmClock_SetGate((XPm_OutClockNode *)Clk, 1);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CLK_ENABLE;
		goto done;
	}

	/* Release NPI Reset */
	Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

	PmIn32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_ERR1_STATUS_OFFSET),
	       RegValue);
	if (0U != (RegValue & (PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
			     PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK))) {
		DbgErr = XPM_INT_ERR_NOC_DDRMC_STATUS;
		Status = XST_FAILURE;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus NpdMbist(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u32 i;
	XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

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

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Deassert PCSR Lock*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

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
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

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
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

		Status = XPm_PollForMask(NpdMemIcAddresses[i] +
					 NPI_PCSR_STATUS_OFFSET,
					 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
					 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
			goto done;
		}
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		Status = XPm_PollForMask(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET,
				 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DDR_MEM_CLEAR_DONE;
			goto done;
		}
	}

	/* Check for Mem clear Pass/Fail */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

		PmIn32(NpdMemIcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
			Status = XST_FAILURE;
			goto done;
		}
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
		PmIn32(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			DbgErr = XPM_INT_ERR_DDR_MEM_CLEAR_PASS;
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
        for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
			if (0U == NpdMemIcAddresses[i]) {
				continue;
			}

                PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
                PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET, 0);
        }

        for (i = 0; i < ARRAY_SIZE(DdrMcAddresses) && (0U != DdrMcAddresses[i]); i++) {
                PmOut32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
                        NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET, 0);
		}

	/* Assert PCSR Lock*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus NpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0;
	XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

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


	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
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
		DbgErr = XPM_INT_ERR_BISR_REPAIR;
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
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NIDB_BISR_REPAIR;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
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
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, &NpdOps);
	if (XST_SUCCESS == Status) {
		Npd->BisrDataCopied = 0;
	} else {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmNpDomain_MemIcInit(u32 DeviceId, u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;
	u32 Idx = NODEINDEX(DeviceId);
	u32 Type = NODETYPE(DeviceId);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((((u32)XPM_NODETYPE_MEMIC_SLAVE != Type) &&
	    ((u32)XPM_NODETYPE_MEMIC_MASTER != Type)) ||
	    ((u32)XPM_NODEIDX_MEMIC_MAX <= Idx)) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NpdMemIcAddresses[Idx] = BaseAddr;

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
