/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xpm_mem.h"
#include "xplmi.h"

#define XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN	XPM_NODEIDX_MONITOR_SYSMON_NPD_0
#define XPM_NODEIDX_MEMIC_NSU_MIN1		XPM_NODEIDX_MEMIC_NSU_0
#define XPM_NODEIDX_MEMIC_NSU_MAX1		XPM_NODEIDX_MEMIC_NSU_49
#define XPM_NODEIDX_MEMIC_NSU_MIN2		XPM_NODEIDX_MEMIC_NSU_50
#define XPM_NODEIDX_MEMIC_NSU_MAX2		XPM_NODEIDX_MEMIC_NSU_57

static u32 IsCrypto = 0U;

static u32 NpdMemIcAddresses[XPM_NODEIDX_MEMIC_MAX];

static XStatus NpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 NpdPowerUpTime = 0;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_SOC);

	/* Check vccint_soc first to make sure power is on */
	while (TRUE) {
		Status = XPmPower_CheckPower(VccSocRail, PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);
		if (XST_SUCCESS == Status) {
			break;
		}

		NpdPowerUpTime++;
		if (NpdPowerUpTime > XPM_POLL_TIMEOUT) {
			/* TODO: Request PMC to power up VCCINT_SOC rail and wait for the acknowledgement.*/
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
			Status = XST_FAILURE;
			goto done;
		}
		usleep(10);
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
	/* Delay after isolation removal */
	usleep(10);

	/* Release POR for NoC */
	Status = XPmReset_AssertbyId(PM_RST_NOC_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

	/*
	 * If device is xcvm2152, DDRMC5 has crypto blcok so set local flag.
	 * NOTE: This is a temporary solution until topology support is
	 * available.
	 */
	if (PMC_TAP_IDCODE_DEV_SBFMLY_VM2152 == (XPm_GetIdCode() & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {
		IsCrypto = 1U;
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

static void UbInitStateDeassert(void)
{
	const XPm_Device *Device;
	u32 BaseAddress;
	u32 i = 0U;

	for (i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID(i));
		if (NULL != Device) {
			BaseAddress = Device->Node.BaseAddress;

			XPm_UnlockPcsr(BaseAddress);
			PmOut32(BaseAddress + NPI_PCSR_MASK_OFFSET,
				NPI_DDRMC_PSCR_CONTROL_UB_INITSTATE_MASK);
			PmOut32(BaseAddress + NPI_PCSR_CONTROL_OFFSET, 0);
			XPm_LockPcsr(BaseAddress);

			/* Only UB0 for non sillicon platforms */
			if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
				break;
			}
		}
	}
}

static XStatus NpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XStatus SocRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	u32 i=0;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SlrType;
	u32 SysmonAddr;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_SOC);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);

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
			DbgErr = XPM_INT_ERR_VCCAUX_SOC_ISO;
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
			if ((i == (u32)XPM_NODEIDX_MEMIC_NSU_1) &&
			    (SlrType < (u32)SLR_TYPE_SSIT_DEV_MASTER_SLR)) {
				continue;
			}

			XPm_UnlockPcsr(NpdMemIcAddresses[i]);
			PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
				NPI_PCSR_CONTROL_ODISABLE_NPP_MASK);
			PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
				NPI_PCSR_CONTROL_ODISABLE_NPP_MASK);
			XPm_LockPcsr(NpdMemIcAddresses[i]);
	}

	if (1U != IsCrypto) {
		/* Deassert UB_INITSTATE for DDR blocks */
		UbInitStateDeassert();
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

	if (1U != IsCrypto) {
		/* Assert pcomplete to indicate HC is done and NoC is ready to use */
		/* Unlock PCSR Register*/
		XPm_UnlockPcsr(NPI_BASEADDR + NPI_NIR_0_OFFSET);

		/* Unmask the pcomplete bit */
		PmOut32(NPI_BASEADDR + NPI_NIR_0_OFFSET + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_PCOMPLETE_MASK);

		/*Assert control on pcomplete bit*/
		PmOut32(NPI_BASEADDR + NPI_NIR_0_OFFSET + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_PCOMPLETE_MASK);

		/*Lock PCSR Register */
		XPm_LockPcsr(NPI_BASEADDR + NPI_NIR_0_OFFSET);
	}

	Status = XPmPowerDomain_SecureEfuseTransfer(PM_POWER_NOC);

	if (1U == IsCrypto) {
		/* Deassert UB_INITSTATE for DDR blocks */
		UbInitStateDeassert();
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus NpdScanClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	const XPm_Pmc *Pmc;
	u32 RegValue;
	u32 SlrType;
	XPm_OutClockNode *Clk;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	volatile u32 SlrTypeTemp;
	u32 SecLockDownInfo = GetSecLockDownInfoFromArgs(Args, NumOfArgs);

	SlrType = XPm_GetSlrType();
	SlrTypeTemp = ~(XPm_GetSlrType());
	if (((SlrType != SLR_TYPE_MONOLITHIC_DEV) &&
	     (SlrType != SLR_TYPE_SSIT_DEV_MASTER_SLR)) &&
	    ((SlrTypeTemp != ~(SLR_TYPE_MONOLITHIC_DEV)) &&
	     (SlrTypeTemp != ~(SLR_TYPE_SSIT_DEV_MASTER_SLR)))) {
		PmDbg("Skipping Scan-Clear of NPD for Slave SLR\n\r");
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPM_STRICT_CHECK_IF_NULL(StatusTmp, Pmc, XPm_Pmc, XPmDevice_GetById, PM_DEV_PMC_PROC);
	if ((XST_SUCCESS == Status) || (XST_SUCCESS == StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* This block of code is removed during unit tests becuase there currently
	   is no support for write-to-clear register simulation. */
	#ifndef CPPUTEST
	/* PMC_ERR1_STATUS is the write-to-clear register */
	PmOut32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_ERR1_STATUS_OFFSET),
		(PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
		PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK));
	#endif

	if (PM_HOUSECLEAN_CHECK(NPD, SCAN)) {
		PmInfo("Triggering ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/*
		 * This is a workaround for xcvm2152. When NoC ScanClear runs
		 * the NPI bus is corrupted, refer EDT-1070997.
		 */
		if (PMC_TAP_IDCODE_DEV_SBFMLY_VM2152 == (XPm_GetIdCode() & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {
			/* Idle the PMC-NPI AXI bus */
			XPm_RMW32(PMC_INT_REGS_NPI_AXI, PMC_INT_REGS_NPI_AXI_POWER_IDLEREQ_MASK, PMC_INT_REGS_NPI_AXI_POWER_IDLEREQ_MASK);

			/* Wait for bus to Idle, poll for power_idleack and power_idle */
			Status = XPm_PollForMask(PMC_INT_REGS_NPI_AXI, PMC_INT_REGS_NPI_AXI_POWER_IDLE_MASK, XPM_POLL_TIMEOUT);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_NPD_SCANCLEAR_BUS_IDLE;
				goto done;
			}

			/* Assert raw reset to reset the switch connect between PMC and NPI_AXI */
			XPm_RMW32(PMC_INT_REGS_NPI_AXI, PMC_INT_REGS_NPI_AXI_RAW_RST_N_MASK, 0U);
		}

		PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
			PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
			PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK);
		/* Check that the register value written properly or not! */
		PmChkRegMask32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
			 PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
			 PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_REG_WRT_NPDLSCNCLR_TRIGGER;
			XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
		}

		/* 200 us is not enough and scan clear pass status is updated
			after so increasing delay for scan clear to finish */
		usleep(400);

		/* NoC ScanClear workaround for xcvm2152 continued. */
		if (PMC_TAP_IDCODE_DEV_SBFMLY_VM2152 == (XPm_GetIdCode() & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {

			/* Release PMC-NPI AXI bus reset */
			XPm_RMW32(PMC_INT_REGS_NPI_AXI, PMC_INT_REGS_NPI_AXI_RAW_RST_N_MASK, PMC_INT_REGS_NPI_AXI_RAW_RST_N_MASK);

			/* Clear PMC_NPI AXI idle req */
			XPm_RMW32(PMC_INT_REGS_NPI_AXI, PMC_INT_REGS_NPI_AXI_POWER_IDLEREQ_MASK, 0U);

			/* Wait for port to become active */
			Status = XPm_PollForZero(PMC_INT_REGS_NPI_AXI, PMC_INT_REGS_NPI_AXI_POWER_IDLE_MASK, XPM_POLL_TIMEOUT);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_NPD_SCANCLEAR_BUS_ACT;
				goto done;
			}
		}
	} else {
		/* ScanClear is skipped */
		PmInfo("Skipping ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	/* Enable NPI Clock */
	Clk = (XPm_OutClockNode *)XPmClock_GetByIdx((u32)XPM_NODEIDX_CLK_NPI_REF);
	Status = XPmClock_SetGate((XPm_OutClockNode *)Clk, 1);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CLK_ENABLE;
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	/* Release NPI Reset */
	Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	/* This block of code is removed during unit tests becuase there currently
	   is no support for write-to-clear register simulation. */
	#ifndef CPPUTEST
	/*
	 * During NoC ScanClear there is possibility that ScanClear will
	 * trigger PMC error bits. Clear the error bits before checking the
	 * status to clear any false errors.
	 */
	PmOut32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_ERR1_STATUS_OFFSET),
		(PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
		 PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK));
	#endif

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
	if (IS_SECLOCKDOWN(SecLockDownInfo)) {
		if ((DbgErr == XPM_INT_ERR_NOC_DDRMC_STATUS) || \
			(DbgErr == XPM_INT_ERR_CLK_ENABLE) || \
			(DbgErr == XPM_INT_ERR_RST_RELEASE) || \
			(DbgErr == XPM_INT_ERR_REG_WRT_NPDLSCNCLR_TRIGGER)) {
			Status = XST_FAILURE;
		}
	}
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static void TriggerMemClearNpd(const u32 *DdrMcAddresses, const u32 DdrMcAddrLength)
{
	u32 i;

	/*TODO: Recognise NSU nodes using node type from node structure*/
	for (i = 0U; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if ((((u32)XPM_NODEIDX_MEMIC_NSU_MIN1 <= i) &&
		     ((u32)XPM_NODEIDX_MEMIC_NSU_MAX1 >= i)) ||
		    (((u32)XPM_NODEIDX_MEMIC_NSU_MIN2 <= i) &&
		     ((u32)XPM_NODEIDX_MEMIC_NSU_MAX2 >= i)) ||
		    (0U == NpdMemIcAddresses[i])) {
			continue;
		}

		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
	}
	for (i = 0U; i < DdrMcAddrLength; i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);

		if (1U == IsCrypto) {
			XPm_Out32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
				  DDRMC5_UB_PCSR_MEM_CLEAR_TRIGGER_CRYPTO_MASK);
			XPm_Out32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
				  DDRMC5_UB_PCSR_MEM_CLEAR_TRIGGER_CRYPTO_MASK);
		}
	}
}

static XStatus IsMemClearDoneNpd(const u32 *DdrMcAddresses, const u32 DdrMcAddrLength,
				 u16 *DbgErr, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

		Status = XPm_PollForMask(NpdMemIcAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
					 PollTimeOut);
		if (XST_SUCCESS != Status) {
			*DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
			goto done;
		}
	}
	for (i = 0U; i < DdrMcAddrLength; i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}
		Status = XPm_PollForMask(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
					 PollTimeOut);
		if (XST_SUCCESS != Status) {
			*DbgErr = XPM_INT_ERR_DDR_MEM_CLEAR_DONE;
			break;
		}

		if (1U == IsCrypto) {
			Status = XPm_PollForMask(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET,
						 DDRMC5_UB_PCSR_MEM_CLEAR_DONE_CRYPTO_MASK,
						 PollTimeOut);
			if (XST_SUCCESS != Status) {
				*DbgErr = XPM_INT_ERR_DDR_MEM_CLEAR_DONE;
				break;
			}
		}
	}

done:
	return Status;
}

static XStatus IsMemClearPass(const u32 *DdrMcAddresses, const u32 DdrMcAddrLength,
			      u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

		PmIn32(NpdMemIcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			*DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
			goto done;
		}
	}
	for (i = 0U; i < DdrMcAddrLength; i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}
		PmIn32(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			*DbgErr = XPM_INT_ERR_DDR_MEM_CLEAR_PASS;
			goto done;
		}

		if (1U == IsCrypto) {
			if (DDRMC5_UB_PCSR_MEM_CLEAR_PASS_CRYPTO_MASK !=
			    (RegValue & DDRMC5_UB_PCSR_MEM_CLEAR_PASS_CRYPTO_MASK)) {
				*DbgErr = XPM_INT_ERR_DDR_MEM_CLEAR_PASS;
				goto done;
			}
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

static void CleanupMemClearNpd(const u32 *DdrMcAddresses, const u32 DdrMcAddrLength)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

                PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
                PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET, 0);
        }

        for (i = 0U; i < DdrMcAddrLength; i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}
                PmOut32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
                        NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET, 0);

		if (1U == IsCrypto) {
			XPm_Out32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
				  DDRMC5_UB_PCSR_MEM_CLEAR_TRIGGER_CRYPTO_MASK);
			XPm_Out32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET, 0);
		}
	}
}

static void AssertPcsrLockMem(const u32 *DdrMcAddresses, const u32 DdrMcAddrLength)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

		XPm_LockPcsr(NpdMemIcAddresses[i]);
	}

	for (i = 0U; i < DdrMcAddrLength; i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}

		XPm_LockPcsr(DdrMcAddresses[i]);
	}
}

static XStatus NpdMbist(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	u32 i;
	const XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DdrMcAddrLength = ARRAY_SIZE(DdrMcAddresses);
	u32 SecLockDownInfo = GetSecLockDownInfoFromArgs(Args, NumOfArgs);
	u32 PollTimeOut = GetPollTimeOut(SecLockDownInfo, XPM_POLL_TIMEOUT);

	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID((u32)XPM_NODEIDX_DEV_DDRMC_MIN + i));
		if (NULL != Device) {
			DdrMcAddresses[i] = Device->Node.BaseAddress;
		}
	}

	/* NPD pre bisr requirements - in case if bisr was skipped */
	NpdPreBisrReqs();
	if (!(PM_HOUSECLEAN_CHECK(NPD, MBIST))) {
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

	/* Deassert PCSR Lock*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		if (0U == NpdMemIcAddresses[i]) {
			continue;
		}

		XPm_UnlockPcsr(NpdMemIcAddresses[i]);
	}

	/* Enable ILA clock for DDR blocks*/
	for (i = 0U; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}
		XPm_UnlockPcsr(DdrMcAddresses[i]);
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK);
	}

	/* Trigger Mem clear */
	TriggerMemClearNpd(DdrMcAddresses, DdrMcAddrLength);

	/* Check for Mem clear done */
	Status = IsMemClearDoneNpd(DdrMcAddresses, DdrMcAddrLength, &DbgErr,
				PollTimeOut);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check for Mem clear Pass/Fail */
	Status = IsMemClearPass(DdrMcAddresses, DdrMcAddrLength, &DbgErr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Disable ILA clock for DDR blocks*/
	for (i = 0U; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK, 0);
	}


	/* Unwrite trigger bits */
	CleanupMemClearNpd(DdrMcAddresses, DdrMcAddrLength);

	Status = XST_SUCCESS;

done:
	/* Assert PCSR Lock*/
	AssertPcsrLockMem(DdrMcAddresses, DdrMcAddrLength);
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus NpdBisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	u32 i = 0;
	const XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const u32 NpdBisrResetIds[] = {
		PM_RST_NPI,		/* Release NPI Reset */
		PM_RST_NOC,		/* Release NoC Reset */
		PM_RST_SYS_RST_1,	/* Release Sys Resets */
		PM_RST_SYS_RST_2,
		PM_RST_SYS_RST_3,
	};

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

	for (i = 0; i < ARRAY_SIZE(NpdBisrResetIds); i++) {
		Status = XPmReset_AssertbyId(NpdBisrResetIds[i], (u32)PM_RESET_ACTION_RELEASE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_RELEASE;
			goto done;
		}
	}

	if (PM_HOUSECLEAN_CHECK(NPD, BISR)){
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Enable Bisr clock */
		for (i = 0U; i < ARRAY_SIZE(DdrMcAddresses); i++) {
			if (0U == DdrMcAddresses[i]) {
				continue;
			}
			/* Unlock writes */
			XPm_UnlockPcsr(DdrMcAddresses[i]);
			PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
				NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK,
				NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK);
		}

		/* Run BISR */
		Status = XPmBisr_Repair(DDRMC_TAG_ID);
		if (Status != XST_SUCCESS) {
			DbgErr = XPM_INT_ERR_BISR_REPAIR;
			goto fail;
		}

		/* TODO: Add BISR for DDRMC5 and DDRMC5_CRYPTO tags */

		/* Disable Bisr clock */
		for (i = 0U; i < ARRAY_SIZE(DdrMcAddresses); i++) {
			if (0U == DdrMcAddresses[i]) {
				continue;
			}
			PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
				NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK, 0);
			/* Lock writes */
			XPm_LockPcsr(DdrMcAddresses[i]);
		}

		/* NIDB Lane Repair */
		Status = XPmBisr_NidbLaneRepair();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_NIDB_BISR_REPAIR;
		}
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	goto done;

fail:
	for (i = 0U; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		if (0U == DdrMcAddresses[i]) {
			continue;
		}
		/* Lock writes */
		XPm_LockPcsr(DdrMcAddresses[i]);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static const struct XPm_PowerDomainOps NpdOps = {
	.InitStart = NpdInitStart,
	.InitFinish = NpdInitFinish,
	.ScanClear = NpdScanClear,
	.Mbist = NpdMbist,
	.Bisr = NpdBisr,
	/* Mask to indicate which Ops are present */
	.InitMask = (BIT16(FUNC_INIT_START) |
		     BIT16(FUNC_INIT_FINISH) |
		     BIT16(FUNC_SCAN_CLEAR) |
		     BIT16(FUNC_BISR) |
		     BIT16(FUNC_MBIST_CLEAR))
};

XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, &NpdOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

	/* Clear NPD section of PMC RAM register reserved for houseclean disable */
	XPm_RMW32(PM_HOUSECLEAN_DISABLE_REG_1, PM_HOUSECLEAN_DISABLE_NPD_MASK, 0U);

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

/****************************************************************************/
/**
 * @brief  This function checks whether all power domains that depend on NOC
 *	   power domain are powered off.
 *
 * @param  Node		Node pointer of a device or a power domain that depends
 *			on NOC power domain
 *
 * @return XST_SUCCESS if NOC power domain is quiescent, XST_FAILURE if NOC
 *	   power domain is not quiescent due to one of depend power domains
 *	   is not powered off, or the Node is not a depend power domain of NOC
 *
 * @note  None
 *
 ****************************************************************************/
XStatus XPmNpDomain_IsNpdIdle(const XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;
	const XPm_PowerDomain *PowerD;
	const XPm_Power *Power;
	const XPm_Device *Device;
	u32 i;

	/*
	 * This routine is called directly for cases that 'Node' is a device,
	 * so in those cases validate that its parent node is NOC power domain.
	 * For cases that 'Node' is a power domain, that validation is done by
	 * the calling routine 'XPmNpDomain_ClockGate'.
	 */
	if ((u32)XPM_NODECLASS_DEVICE == NODECLASS(Node->Id)) {
		Device = (XPm_Device *)Node;
		if ((u32)PM_POWER_NOC != Device->Power->Node.Id) {
			Status = XST_FAILURE;
			goto done;
		}
		PowerD = (XPm_PowerDomain *)Device->Power;
	} else if (((u32)XPM_NODECLASS_POWER == NODECLASS(Node->Id)) &&
		   ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(Node->Id))) {
		PowerD = (XPm_PowerDomain *)Node;
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check idleness of NOC's dependent power domains */
	for (i = 0U; ((i < (u32)MAX_POWERDOMAINS) && (PowerD->Children[i] != 0U));
	     i++) {
		Power = (XPm_Power *)XPmPower_GetById(PowerD->Children[i]);
		/* Skip the power domain that is related to the caller */
		if (Power->Node.Id == Node->Id) {
			continue;
		}

		/*
		 * The PL_SYSMON power domain depends on NOC power domain but
		 * it does not use the NOC transport layer, therefore it is
		 * excluded from this consideration.
		 */
		if (Power->Node.Id == (u32)PM_POWER_PL_SYSMON) {
			continue;
		}

		/*
		 * If 'UseCount' of a power domain that depends on NOC is not 0,
		 * then the power domain is in use and therefore not idle.
		 */
		if (Power->UseCount != 0U) {
			Status = XST_FAILURE;
			goto done;
		}
	}

done:
	return Status;
}

static XStatus XPmNpDomain_IsParentPowerNoc(const XPm_Node *Node, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	const XPm_Power *Power;

	/* Node's parent power dmain should be NOC */
	if ((u32)XPM_NODECLASS_DEVICE == NODECLASS(Node->Id)) {
		Device = (XPm_Device *)Node;
		if ((u32)PM_POWER_NOC != Device->Power->Node.Id) {
			*DbgErr = XPM_INT_ERR_INVALID_NODE;
			goto done;
		}
	} else if (((u32)XPM_NODECLASS_POWER == NODECLASS(Node->Id)) &&
		   ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(Node->Id))) {
		const XPm_PowerDomain *PowerD = (XPm_PowerDomain *)Node;
		u32 i = 0U;
		while (MAX_POWERDOMAINS > i) {
			Power = (XPm_Power *)XPmPower_GetById(PowerD->Parents[i]);
			i++;
			if ((u32)PM_POWER_NOC == Power->Node.Id) {
				break;
			}
		}

		if (MAX_POWERDOMAINS == i) {
			*DbgErr = XPM_INT_ERR_INVALID_NODE;
			goto done;
		}
	} else {
		*DbgErr = XPM_INT_ERR_INVALID_NODE;
		goto done;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function turns the NOC clock off/on by gateing the clock.
 *
 * @param  Node		Node pointer of a device or a power domain that depends
 *			on NOC power domain
 * @param  State	Requested state for the clock: 0 - Off, 1 - On
 *
 * @return XST_SUCCESS if successful to change the clock state, or if the NOC
 *	   power domain is not idle due to one of it's depend power domain is
 *	   not powered off.  Otherwise, return a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmNpDomain_ClockGate(const XPm_Node *Node, u8 State)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 BaseAddress, Reg;
	u32 SlrType;
	u32 Clock_State;

	/* Return while power domains are being initialized during the boot */
	if (XPlmi_IsLoadBootPdiDone() == FALSE) {
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPmNpDomain_IsParentPowerNoc(Node, &DbgErr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Support monolithic devices for now */
	SlrType = XPm_GetSlrType();
	if (SlrType != SLR_TYPE_MONOLITHIC_DEV) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* If NOC Power Domain is not idle, just return */
	if ((0U == State) && (XPmNpDomain_IsNpdIdle(Node) != XST_SUCCESS)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Unlock NOC_NPS_0 */
	BaseAddress = NOC_PACKET_SWITCH_BASEADDR;
	XPm_UnlockPcsr(BaseAddress);

	/* Current state of the NOC clock */
	Reg = BaseAddress + NOC_NPS_0_REG_CLOCK_MUX_OFFSET;
	Clock_State = XPm_In32(Reg);

	switch (State) {
	case 0U:
		/* If DRAMs are not in self-refresh mode, just return */
		if (XPmDDRDevice_IsInSelfRefresh() != XST_SUCCESS) {
			Status = XST_SUCCESS;
			break;
		}

		/*
		 * Turn off the NOC clock by setting bits[3:2] to 1
		 * and leave bits[1:0] unmodified.
		 */
		XPm_Out32(Reg, (Clock_State | 0xCU));
		Status = XST_SUCCESS;
		break;
	case 1U:
		/* Turn on the NOC clock by clearing bits[3:2] */
		XPm_Out32(Reg, (Clock_State & 0x3U));
		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XST_INVALID_PARAM;
		break;
	}

	/* Lock NOC_NPS_0 */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
