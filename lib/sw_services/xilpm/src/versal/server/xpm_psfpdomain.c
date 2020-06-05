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

static XStatus FpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	(void)Args;
	(void)NumOfArgs;

	/* Check vccint_fpd first to make sure power is on */
	if (XST_SUCCESS != XPmPower_CheckPower(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_MASK)) {
		PmErr("FPD power is OFF\r\n");
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		PmErr("FPD is not enabled\r\n");
		Status = XST_NOT_ENABLED;
		goto done;
	}

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = (u32)FUNC_INIT_START;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		PmErr("IpiSend failed\r\n");
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		PmErr("IpiReadStatus failed\r\n");
		goto done;
	}
	/* Release POR for PS-FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to reset release of FPD_POR\r\n", Status);
	}
done:
	if (XST_SUCCESS != Status) {
		PmErr("Return: 0x%x\r\n", Status);
	}
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

	(void)Args;
	(void)NumOfArgs;

	/* Release SRST for PS-FPD - in case Bisr and Mbist are skipped */
	Status = XPmReset_AssertbyId(PM_RST_FPD, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to reset release of FPD\r\n");
	}

	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = (u32)FUNC_INIT_FINISH;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		PmErr("IpiSend failed for FPD\r\n");
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		PmErr("IpiReadStatus failed for FPD\r\n");
		goto done;
	}

	/* Remove FPD SOC domains isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to remove FPD SOC domains isolation\r\n");
		goto done;
	}

	/* Copy sysmon data */
	Status = XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[XPM_NODEIDX_MONITOR_SYSMON_PS_FPD], PM_POWER_FPD, 0);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to copy sysmon data\r\n\r\n");
	}
done:
	if (XST_SUCCESS != Status) {
		PmErr("Return: 0x%x\r\n", Status);
	}
	return Status;
}

static XStatus FpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_Psm *Psm;

	(void)Args;
	(void)NumOfArgs;

        if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
                Status = XST_SUCCESS;
                goto done;
        }

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);;
	if (NULL == Psm) {
		PmErr("Invalid PSM_PROC\r\n");
		Status = XST_FAILURE;
		goto done;
	}

        /* Trigger scan clear */
        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
		PSM_GLOBAL_SCAN_CLEAR_TRIGGER, PSM_GLOBAL_SCAN_CLEAR_TRIGGER);

        Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
				 PSM_GLOBAL_SCAN_CLEAR_DONE_STATUS, 0x10000U);
        if (XST_SUCCESS != Status) {
		PmErr("Timeout during ScanClear_Done for FPD\r\n");
                goto done;
        }

        Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
				 PSM_GLOBAL_SCAN_CLEAR_PASS_STATUS, 0x10000U);
        if (XST_SUCCESS != Status) {
		PmErr("Timeout during ScanClear_Pass for FPD\r\n");
                goto done;
        }

	/* Unwrite trigger bits */
        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_SCAN_CLEAR_FPD_OFFSET,
		PSM_GLOBAL_SCAN_CLEAR_TRIGGER, 0);

done:
	if (XST_SUCCESS != Status) {
		PmErr("Return: 0x%x\r\n", Status);
	}
        return Status;
}

static XStatus FpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	(void)Args;
	(void)NumOfArgs;

	/* Release SRST for PS-FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD, (u32)PM_RESET_ACTION_RELEASE);

	/* Call PSM to execute pre bisr requirements */
	Payload[0] = PSM_API_FPD_HOUSECLEAN;
	Payload[1] = (u32)FUNC_BISR;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		PmErr("IpiSend failed for FPD\r\n");
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		PmErr("IpiReadStatus failed for FPD\r\n");
		goto done;
	}

	/* Trigger Bisr repair */
	Status = XPmBisr_Repair(FPD_TAG_ID);
	if (XST_SUCCESS != Status) {
		PmErr("BisrRep failed for FPD_TAG_ID\r\n");
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("Return: 0x%x\r\n", Status);
        }
	return Status;
}

static XStatus FpdMbistClear(u32 *Args, u32 NumOfArgs)
{
        XStatus Status = XST_FAILURE;
        u32 Payload[PAYLOAD_ARG_CNT] = {0};
	XPm_Psm *Psm;

	(void)Args;
	(void)NumOfArgs;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);;
	if (NULL == Psm) {
		PmErr("Invalid PSM_PROC\r\n");
		Status = XST_FAILURE;
		goto done;
	}

	/* Release SRST for PS-FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD, (u32)PM_RESET_ACTION_RELEASE);
        if (XST_SUCCESS != Status) {
		PmErr("Failed to reset release of FPD\r\n");
        }

        Payload[0] = PSM_API_FPD_HOUSECLEAN;
        Payload[1] = (u32)FUNC_MBIST_CLEAR;

        Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
        if (XST_SUCCESS != Status) {
		PmErr("IpiSend faild for FPD\r\n");
                goto done;
        }

        Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
        if (XST_SUCCESS != Status) {
		PmErr("IpiReadStatus faild for FPD\r\n");
        }

        if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
                Status = XST_SUCCESS;
                goto done;
        }

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_RST_OFFSET,
		PSM_GLOBAL_MBIST_RST_FPD_MASK, PSM_GLOBAL_MBIST_RST_FPD_MASK);

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_SETUP_OFFSET,
		PSM_GLOBAL_MBIST_SETUP_FPD_MASK, PSM_GLOBAL_MBIST_SETUP_FPD_MASK);

        PmRmw32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_PG_EN_OFFSET,
		PSM_GLOBAL_MBIST_PG_EN_FPD_MASK, PSM_GLOBAL_MBIST_PG_EN_FPD_MASK);

        Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_DONE_OFFSET,
				 PSM_GLOBAL_MBIST_DONE_FPD_MASK, 0x10000U);
        if (XST_SUCCESS != Status) {
		PmErr("Timeout during Mbist_Done for FPD\r\n");
                goto done;
        }

        if (PSM_GLOBAL_MBIST_GO_FPD_MASK !=
            (XPm_In32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_MBIST_GO_OFFSET) &
             PSM_GLOBAL_MBIST_GO_FPD_MASK)) {
		PmErr("MBIST_GO_FPD failed\r\n");
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
		PmErr("MBIST_GO_FPD failed\r\n");
		goto done;
	}

	/* EDT-997247: Mem clear introduces apu gic ecc error,
	so pulse gic reset as a work around to fix it */
	Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to reset assert of ACPU_GIC\r\n");
		goto done;
	}
	Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to reset release of ACPU_GIC\r\n");
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("Return: 0x%x\r\n", Status);
	}
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
	XStatus Status;
	Status = XPmProt_CommonXmpuCtrl(Args, NumOfArgs);
	if (XST_SUCCESS != Status) {
		PmErr("Return: 0x%x\r\n", Status);
	}
        return Status;
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

	Status = XPmPowerDomain_Init(&PsFpd->Domain, Id, BaseAddress, Parent, &FpdOps);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (1U <= OtherBaseAddressCnt) {
		PsFpd->FpdSlcrBaseAddr = OtherBaseAddresses[0];
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

done:
	return Status;
}
