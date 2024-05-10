/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_psfpdomain.h"
#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_psm.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_rail.h"

#define FPD_POLL_TIMEOUT 0x10000U

static XStatus SendFpdHouseCleanReqToPsm(u32 FuncId, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = FuncId;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_IPI_SEND;
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_IPI_STATUS;
	}

done:
	return Status;
}

static XStatus FpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;


	(void)PwrDomain;
	u32 SecLockDownInfo = GetSecLockDownInfoFromArgs(Args, NumOfArgs);
	const XPm_Rail *VccintPsfpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSFP);

	/* Check vccint_fpd first to make sure power is on */
	Status = XPmPower_CheckPower(VccintPsfpRail,
				     PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		DbgErr = XPM_INT_ERR_PSMFW_NOT_PRESENT;
		Status = XST_NOT_ENABLED;
		goto done;
	}
	if (IS_SECLOCKDOWN(SecLockDownInfo)){
		Status = SendFpdHouseCleanReqToPsm((u32)FUNC_SECLOCKDOWN, &DbgErr);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = SendFpdHouseCleanReqToPsm((u32)FUNC_INIT_START, &DbgErr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release POR for PS-FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_POR;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus FpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	Status = XST_SUCCESS;

	return Status;
}

static XStatus FpdHcComplete(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SysmonAddr;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	Status = SendFpdHouseCleanReqToPsm((u32)FUNC_INIT_FINISH, &DbgErr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove FPD SOC domains isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_SOC_ISO;
		goto done;
	}

	SysmonAddr = XPm_GetSysmonByIndex((u32)XPM_NODEIDX_MONITOR_SYSMON_PS_FPD);
	if (0U == SysmonAddr) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Copy sysmon data */
	Status = XPmPowerDomain_ApplyAmsTrim(SysmonAddr, PM_POWER_FPD, 0);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AMS_TRIM;
		goto done;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus FpdScanClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	const XPm_Psm *Psm;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RegVal;
	u32 SecLockDownInfo = GetSecLockDownInfoFromArgs(Args, NumOfArgs);
	u32 PollTimeOut = GetPollTimeOut(SecLockDownInfo, FPD_POLL_TIMEOUT);

	if (!(PM_HOUSECLEAN_CHECK(FPD, SCAN))) {
		PmInfo("Skipping ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	PmInfo("Triggering ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);;
	if (NULL == Psm) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Trigger scan clear, This Register bit is WO type */
	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
		PSM_GLOBAL_SCAN_CLEAR_TRIGGER, PSM_GLOBAL_SCAN_CLEAR_TRIGGER);

	Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
				 PSM_GLOBAL_SCAN_CLEAR_DONE_STATUS, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
		goto done;
	}

	RegVal = XPm_In32(Psm->PsmGlobalBaseAddr +
			      PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET);
	if ((RegVal &
	    (u32)PSM_GLOBAL_SCAN_CLEAR_PASS_STATUS) !=
	    (u32)PSM_GLOBAL_SCAN_CLEAR_PASS_STATUS) {
		DbgErr = XPM_INT_ERR_SCAN_PASS;
		Status = XST_FAILURE;
                goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus FpdBisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	/* Call PSM to execute pre bisr requirements */
	Status = SendFpdHouseCleanReqToPsm((u32)FUNC_BISR, &DbgErr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (PM_HOUSECLEAN_CHECK(FPD, BISR)) {
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Trigger Bisr repair */
		Status = XPmBisr_Repair(FPD_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BISR_REPAIR;
		}
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus TriggerMemClearFpd(const XPm_Psm *Psm, u16 *DbgErr, u32 SecLockDownInfo)
{
	XStatus Status = XST_FAILURE;
	u32 PollTimeOut = GetPollTimeOut(SecLockDownInfo, FPD_POLL_TIMEOUT);

	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_RST_OFFSET,
		PSM_GLOBAL_MBIST_RST_FPD_MASK, PSM_GLOBAL_MBIST_RST_FPD_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_RST_OFFSET),
			PSM_GLOBAL_MBIST_RST_FPD_MASK, PSM_GLOBAL_MBIST_RST_FPD_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_FPDMBISTCLR_RST;
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_SETUP_OFFSET,
		PSM_GLOBAL_MBIST_SETUP_FPD_MASK, PSM_GLOBAL_MBIST_SETUP_FPD_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_SETUP_OFFSET),
			PSM_GLOBAL_MBIST_SETUP_FPD_MASK, PSM_GLOBAL_MBIST_SETUP_FPD_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_FPDMBISTCLR_SETUP;
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_PG_EN_OFFSET,
		PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, PSM_GLOBAL_MBIST_PG_EN_FPD_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_PG_EN_OFFSET),
			PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_FPDMBISTCLR_PGEN;
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_DONE_OFFSET,
				 PSM_GLOBAL_MBIST_DONE_FPD_MASK, PollTimeOut);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MBIST_DONE_TIMEOUT;
		goto done;
	}

	if (PSM_GLOBAL_MBIST_GO_FPD_MASK !=
	    (XPm_In32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_GO_OFFSET) &
	     PSM_GLOBAL_MBIST_GO_FPD_MASK)) {
		*DbgErr = XPM_INT_ERR_MBIST_GO;
		Status = XST_FAILURE;
	}

done:
	if (IS_SECLOCKDOWN(SecLockDownInfo)) {
		if ((*DbgErr == XPM_INT_ERR_MBIST_GO) || \
			(*DbgErr == XPM_INT_ERR_MBIST_DONE_TIMEOUT) || \
			(*DbgErr == XPM_INT_ERR_REG_WRT_FPDMBISTCLR_PGEN) || \
			(*DbgErr == XPM_INT_ERR_REG_WRT_FPDMBISTCLR_SETUP) || \
			(*DbgErr == XPM_INT_ERR_REG_WRT_FPDMBISTCLR_RST)){
			Status = XST_FAILURE;
			}
	}
	return Status;
}

static XStatus FpdMbistClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	const XPm_Psm *Psm;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	u32 SecLockDownInfo = GetSecLockDownInfoFromArgs(Args, NumOfArgs);
        Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, Psm, XPm_Psm, XPmDevice_GetById, PM_DEV_PSM_PROC);
        if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
                DbgErr = XPM_INT_ERR_INVALID_DEVICE;
                goto done;
        }

	Status = SendFpdHouseCleanReqToPsm((u32)FUNC_MBIST_CLEAR, &DbgErr);
	if (XST_SUCCESS != Status) {
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	if (!(PM_HOUSECLEAN_CHECK(FPD, MBIST))) {
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

	Status = TriggerMemClearFpd(Psm, &DbgErr, SecLockDownInfo);
	if (XST_SUCCESS != Status) {
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	/* Unwrite trigger bits */
	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_RST_OFFSET,
		PSM_GLOBAL_MBIST_RST_FPD_MASK, 0);

	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_SETUP_OFFSET,
		PSM_GLOBAL_MBIST_SETUP_FPD_MASK, 0);

	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_PG_EN_OFFSET,
		PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, 0);

	/* Mem clear introduces apu gic ecc error,
	so pulse gic reset as a work around to fix it */
	Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}
	Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

done:
	if (IS_SECLOCKDOWN(SecLockDownInfo)) {
		if ((DbgErr == XPM_INT_ERR_MBIST_GO) || \
			(DbgErr == XPM_INT_ERR_MBIST_DONE_TIMEOUT) || \
			(DbgErr == XPM_INT_ERR_REG_WRT_FPDMBISTCLR_PGEN) || \
			(DbgErr == XPM_INT_ERR_REG_WRT_FPDMBISTCLR_SETUP) || \
			(DbgErr == XPM_INT_ERR_REG_WRT_FPDMBISTCLR_RST) || \
			(DbgErr == XPM_INT_ERR_SRST_FPD)) {
			Status = XST_FAILURE;
			}
	}
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static struct XPm_PowerDomainOps FpdOps = {
	.InitStart = FpdInitStart,
	.InitFinish = FpdInitFinish,
	.ScanClear = FpdScanClear,
	.Bisr = FpdBisr,
	.Mbist = FpdMbistClear,
	.HcComplete = FpdHcComplete,
	/* Mask to indicate which Ops are present */
	.InitMask = (BIT16(FUNC_INIT_START) |
		     BIT16(FUNC_INIT_FINISH) |
		     BIT16(FUNC_SCAN_CLEAR) |
		     BIT16(FUNC_BISR) |
		     BIT16(FUNC_MBIST_CLEAR) |
		     BIT16(FUNC_HOUSECLEAN_COMPLETE))
};

XStatus XPmPsFpDomain_Init(XPm_PsFpDomain *PsFpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent,  const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PsFpd->Domain, Id, BaseAddress, Parent, &FpdOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (1U <= OtherBaseAddressCnt) {
		PsFpd->FpdSlcrBaseAddr = OtherBaseAddresses[0];
		PsFpd->FpdSlcrSecureBaseAddr = OtherBaseAddresses[1];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

	/* Clear FPD section of PMC RAM register reserved for houseclean disable */
	XPm_RMW32(PM_HOUSECLEAN_DISABLE_REG_1, PM_HOUSECLEAN_DISABLE_FPD_MASK, 0U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
