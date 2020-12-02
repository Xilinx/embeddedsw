/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_psfpdomain.h"
#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_psm.h"
#include "xpm_device.h"
#include "xpm_prot.h"
#include "xpm_debug.h"
#include "xpm_rail.h"

static XStatus FpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	XPm_Rail *VccintPsfpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSFP);

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

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = (u32)FUNC_INIT_START;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_SEND;
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_STATUS;
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

static XStatus FpdInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)Args;
	(void)NumOfArgs;

	Status = XST_SUCCESS;

	return Status;
}

static XStatus FpdHcComplete(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SysmonAddr;

	(void)Args;
	(void)NumOfArgs;

	/* Release SRST for PS-FPD - in case Bisr and Mbist are skipped */
	Status = XPmReset_AssertbyId(PM_RST_FPD, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = (u32)FUNC_INIT_FINISH;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_SEND;
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_STATUS;
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

static XStatus FpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_Psm *Psm;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

        if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
                Status = XST_SUCCESS;
                goto done;
        }

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
				 PSM_GLOBAL_SCAN_CLEAR_DONE_STATUS, 0x10000U);
        if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
                goto done;
        }

        Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
				 PSM_GLOBAL_SCAN_CLEAR_PASS_STATUS, 0x10000U);
        if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SCAN_PASS_TIMEOUT;
                goto done;
        }

	/* Unwrite trigger bits */
        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
		PSM_GLOBAL_SCAN_CLEAR_TRIGGER, 0);

done:
	XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}

static XStatus FpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	/* Release SRST for PS-FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

	/* Call PSM to execute pre bisr requirements */
	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = (u32)FUNC_BISR;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_SEND;
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_STATUS;
		goto done;
	}

	/* Trigger Bisr repair */
	Status = XPmBisr_Repair(FPD_TAG_ID);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_REPAIR;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus FpdMbistClear(u32 *Args, u32 NumOfArgs)
{
        XStatus Status = XST_FAILURE;
        u32 Payload[PAYLOAD_ARG_CNT] = {0};
	XPm_Psm *Psm;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);;
	if (NULL == Psm) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Release SRST for PS-FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SRST_FPD;
		goto done;
	}

        Payload[0] = PSM_API_FPD_HOUSECLEAN;
        Payload[1] = (u32)FUNC_MBIST_CLEAR;

        Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
        if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_SEND;
                goto done;
        }

        Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_IPI_STATUS;
		goto done;
	}

        if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
                Status = XST_SUCCESS;
                goto done;
        }

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_RST_OFFSET,
		PSM_GLOBAL_MBIST_RST_FPD_MASK, PSM_GLOBAL_MBIST_RST_FPD_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_RST_OFFSET),
		      PSM_GLOBAL_MBIST_RST_FPD_MASK, PSM_GLOBAL_MBIST_RST_FPD_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_FPDMBISTCLR_RST;
		goto done;
	}

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_SETUP_OFFSET,
		PSM_GLOBAL_MBIST_SETUP_FPD_MASK, PSM_GLOBAL_MBIST_SETUP_FPD_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_SETUP_OFFSET),
		      PSM_GLOBAL_MBIST_SETUP_FPD_MASK, PSM_GLOBAL_MBIST_SETUP_FPD_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_FPDMBISTCLR_SETUP;
		goto done;
	}

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_PG_EN_OFFSET,
		PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, PSM_GLOBAL_MBIST_PG_EN_FPD_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_PG_EN_OFFSET),
		      PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_FPDMBISTCLR_PGEN;
		goto done;
	}

        Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_DONE_OFFSET,
				 PSM_GLOBAL_MBIST_DONE_FPD_MASK, 0x10000U);
        if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MBIST_DONE_TIMEOUT;
                goto done;
        }

        if (PSM_GLOBAL_MBIST_GO_FPD_MASK !=
            (XPm_In32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_GO_OFFSET) &
             PSM_GLOBAL_MBIST_GO_FPD_MASK)) {
		DbgErr = XPM_INT_ERR_MBIST_GO;
                Status = XST_FAILURE;
        }

	/* Unwrite trigger bits */
	PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_RST_OFFSET,
		PSM_GLOBAL_MBIST_RST_FPD_MASK, 0);

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_SETUP_OFFSET,
		PSM_GLOBAL_MBIST_SETUP_FPD_MASK, 0);

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_PG_EN_OFFSET,
		PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, 0);

	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MBIST_GOOD;
		goto done;
	}

	/* EDT-997247: Mem clear introduces apu gic ecc error,
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
	XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}

/****************************************************************************/
/**
 * @brief  This function configures xmpu for FPD_SLAVES
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus FpdXmpuCtrl(u32 *Args, u32 NumOfArgs)
{
	return XPmProt_CommonXmpuCtrl(Args, NumOfArgs);
}

static struct XPm_PowerDomainOps FpdOps = {
	.InitStart = FpdInitStart,
	.InitFinish = FpdInitFinish,
	.ScanClear = FpdScanClear,
	.Bisr = FpdBisr,
	.Mbist = FpdMbistClear,
	.HcComplete = FpdHcComplete,
	.XmpuCtrl = FpdXmpuCtrl,
};

XStatus XPmPsFpDomain_Init(XPm_PsFpDomain *PsFpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent,  u32 *OtherBaseAddresses,
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
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
