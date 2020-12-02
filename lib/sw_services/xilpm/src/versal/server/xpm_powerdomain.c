/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_notifier.h"
#include "xpm_npdomain.h"
#include "xpm_core.h"
#include "xpm_psm.h"
#include "xpm_pmc.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_pldomain.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_gic_proxy.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_debug.h"

#define SYSMON_CHECK_POWER_TIMEOUT	2000000U

static u8 SystemResetFlag;
static u8 DomainPORFlag;
static u32 PsmApuPwrState;

static const char *PmInitFunctions[FUNC_MAX_COUNT_PMINIT] = {
	[FUNC_INIT_START]		= "INIT_START",
	[FUNC_INIT_FINISH]		= "INIT_FINISH",
	[FUNC_SCAN_CLEAR]		= "SCAN_CLEAR",
	[FUNC_BISR]			= "BISR",
	[FUNC_LBIST]			= "LBIST",
	[FUNC_MEM_INIT]			= "MEM_INIT",
	[FUNC_MBIST_CLEAR]		= "MBIST_CLEAR",
	[FUNC_HOUSECLEAN_PL]		= "HOUSECLEAN_PL",
	[FUNC_HOUSECLEAN_COMPLETE]	= "HOUSECLEAN_COMPLETE",
	[FUNC_XPPU_CTRL]		= "XPPU_CTRL",
	[FUNC_XMPU_CTRL]		= "XMPU_CTRL",
};

XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    struct XPm_PowerDomainOps *Ops)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	u16 InitMask = 0;
	Status = XPmPower_Init(&PowerDomain->Power, Id, BaseAddress, Parent);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	PowerDomain->Children = NULL;
	PowerDomain->DomainOps = Ops;

	if ((NULL != Ops) && (NULL != Ops->ScanClear)) {
		InitMask |= BIT(FUNC_SCAN_CLEAR);
	}

	if ((NULL != Ops) && (NULL != Ops->Bisr)) {
		InitMask |= BIT(FUNC_BISR);
	}

	if ((NULL != Ops) && (NULL != Ops->Mbist)) {
		InitMask |= BIT(FUNC_MBIST_CLEAR);
	}

	if ((NULL != Ops) && (NULL != Ops->Lbist)) {
		InitMask |= BIT(FUNC_LBIST);
	}

	PowerDomain->InitMask = InitMask;
	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

#define BITMASK_LOWER_15_BITS			(0x7fffU)
#define BITMASK_UPPER_17_BITS			(0xffff8000U)
#define GET_DELTA_AT_OFFSET(array, x)		(0xfU & (array[(x) / 32U] >> ((x) % 32U)))

XStatus XPmPowerDomain_ApplyAmsTrim(u32 DestAddress, u32 PowerDomainId, u32 SateliteIdx)
{
	XStatus Status = XST_FAILURE;
	u32 EfuseCacheBaseAddress, StartbitOffset, RegValue, i, DeltaVal = 0;
	static u32 OffsetVal,SlopeVal,ProcessVal,ResistorVal,BjtOffsetVal,ExtOffsetVal,AnaSpareVal,DigSpareVal,Arr[8];
	static u32 CacheRead=0;
	static u32 BipSelVal, TsensSelVal, TsensBiasVal;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (0U == DestAddress) {
		DbgErr = XPM_INT_ERR_INVALID_ADDR;
		goto done;
	}

	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}
	EfuseCacheBaseAddress = EfuseCache->Node.BaseAddress;

	/* Unlock writes */
	PmOut32(DestAddress + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

	if (0U == CacheRead) {
		/* Read EFUSE_CACHE.TSENS_INT_OFFSET_5_0*/
		PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_3_OFFSET, RegValue);
		OffsetVal = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_INT_OFFSET_5_0_MASK) >> EFUSE_CACHE_TRIM_AMS_3_TSENS_INT_OFFSET_5_0_SHIFT;
		/* Read EFUSE_CACHE.TSENS_SLOPE_5_0 */
		SlopeVal = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_SLOPE_5_0_MASK) >> EFUSE_CACHE_TRIM_AMS_3_TSENS_SLOPE_5_0_SHIFT;
		/* Read EFUSE_CACHE.IXPCM_PROCESS_15_0 */
		PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_11_OFFSET, RegValue);
		ProcessVal = (RegValue & EFUSE_CACHE_TRIM_AMS_11_IXPCM_PROCESS_15_0_MASK) >> EFUSE_CACHE_TRIM_AMS_11_IXPCM_PROCESS_15_0_SHIFT;
		/* Read EFUSE_CACHE.RES_PROCESS_6_0 */
		PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_11_OFFSET, RegValue);
		ResistorVal = (RegValue & EFUSE_CACHE_TRIM_AMS_11_RES_PROCESS_0_MASK) >> EFUSE_CACHE_TRIM_AMS_11_RES_PROCESS_0_SHIFT;
		PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_12_OFFSET, RegValue);
		ResistorVal |= (((RegValue & EFUSE_CACHE_TRIM_AMS_12_RES_PROCESS_6_1_MASK) >> EFUSE_CACHE_TRIM_AMS_12_RES_PROCESS_6_1_SHIFT) << 1);
		/* Read EFUSE_CACHE.BJT_PROCESS_3_0 */
		BjtOffsetVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_BJT_PROCESS_3_0_MASK) >> EFUSE_CACHE_TRIM_AMS_12_BJT_PROCESS_3_0_SHIFT;
		/* Read EFUSE_CACHE.TSENS_EXT_OFFSET_5_0*/
		ExtOffsetVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_TSENS_EXT_OFFSET_5_0_MASK) >> EFUSE_CACHE_TRIM_AMS_12_TSENS_EXT_OFFSET_5_0_SHIFT;
		/* Read EFUSE_CACHE.SHARED_SPARE_1_0 */
		AnaSpareVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_1_0_MASK) >> EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_1_0_SHIFT;
		/* Read EFUSE_CACHE.SHARED_SPARE_14_2 */
		DigSpareVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_14_2_MASK) >> EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_14_2_SHIFT;
		TsensSelVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_2_MASK) >> EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_2_SHIFT;
		BipSelVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_6_MASK) >> EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_6_SHIFT;
		TsensBiasVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_4_3_MASK) >> EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_4_3_SHIFT;
	}

	/* Copy EFUSE_CACHE.TSENS_INT_OFFSET_5_0 to dest_reg.EFUSE_CONFIG0[5:0] */
	PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_OFFSET_MASK, (OffsetVal << EFUSE_CONFIG0_OFFSET_SHIFT));
	/* Copy EFUSE_CACHE.TSENS_SLOPE_5_0      to dest_reg.EFUSE_CONFIG0[11:6] */
	PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_SLOPE_MASK, (SlopeVal << EFUSE_CONFIG0_SLOPE_SHIFT));
	/* Copy EFUSE_CACHE.IXPCM_PROCESS_15_0   to dest_reg.EFUSE_CONFIG0[31:16] */
	PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_PROCESS_MASK, (ProcessVal << EFUSE_CONFIG0_PROCESS_SHIFT));
	/* Copy EFUSE_CACHE.RES_PROCESS_6_0     to dest_reg.EFUSE_CONFIG1[6:0] */
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_RESISTOR_MASK, (ResistorVal << EFUSE_CONFIG1_RESISTOR_SHIFT));
	/* Copy EFUSE_CACHE.BJT_PROCESS_3_0      to dest_reg.EFUSE_CONFIG1[10:7] */
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_BJT_OFFSET_MASK, (BjtOffsetVal << EFUSE_CONFIG1_BJT_OFFSET_SHIFT));
	/* Copy EFUSE_CACHE.TSENS_EXT_OFFSET_5_0 to dest_reg.EFUSE_CONFIG1[16:11] */
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_EXT_OFFSET_MASK, (ExtOffsetVal << EFUSE_CONFIG1_EXT_OFFSET_SHIFT));
	/* Copy EFUSE_CACHE.SHARED_SPARE_1_0     to dest_reg.EFUSE_CONFIG1[18:17] */
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_ANA_SPARE_MASK, (AnaSpareVal << EFUSE_CONFIG1_ANA_SPARE_SHIFT));
	/* Copy EFUSE_CACHE.SHARED_SPARE_14_2    to dest_reg.EFUSE_CONFIG1[31:19] */
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_DIG_SPARE_MASK, (DigSpareVal << EFUSE_CONFIG1_DIG_SPARE_SHIFT));
	/* Copy EFUSE_CACHE.TRIM_AMS_12.SHARED_SPARE_2 to dest_reg.CAL_SM_BIP_TSENS[1]  */
	PmRmw32(DestAddress + CAL_SM_BIP_TSENS_OFFSET,  CAL_SM_BIP_TSENS_TSENS_MASK, (TsensSelVal << CAL_SM_BIP_TSENS_TSENS_SHIFT));
	/* Copy EFUSE_CACHE.TRIM_AMS_12.SHARED_SPARE_6  to dest_reg.CAL_SM_BIP_TSENS[0] */
	PmRmw32(DestAddress + CAL_SM_BIP_TSENS_OFFSET,  CAL_SM_BIP_TSENS_BIP_MASK, (BipSelVal << CAL_SM_BIP_TSENS_BIP_SHIFT));
	/* Copy EFUSE_CACHE.TRIM_AMS_12.SHARED_SPARE_3_4 to dest_reg.TSENS_BIAS_CTRL[1:0] */
	PmRmw32(DestAddress + TSENS_BIAS_CTRL_OFFSET,  TSENS_BIAS_VAL_MASK, (TsensBiasVal << TSENS_BIAS_VAL_SHIFT));

	if (0U == CacheRead) {
		/* Copy 256 bits of TSENS_DELTA value to array */
		PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_3_OFFSET, RegValue);
		/*Store 17 bits from current register */
		Arr[0] = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_MASK) >> EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_SHIFT;
		for (i = 0; i < 8U; i++) {
			u32 Address = (EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_4_OFFSET + (i*4U));
			PmIn32(Address, RegValue);
			/* current element already have 17 bits stored from prev register,
			store 15 bits from current register to current element */
			Arr[i] |= (RegValue & BITMASK_LOWER_15_BITS) << 17;
			/* store 17 bits from current register to next element */
			if (i != 7U) {
				Arr[i + 1U] = (RegValue & BITMASK_UPPER_17_BITS) >> 15;
			}
		}

		/* Set cache read to avoid multiple reads */
		CacheRead = 1;
	}

	switch (NODEINDEX(PowerDomainId)) {
	case (u32)XPM_NODEIDX_POWER_PMC:
		if (0U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_3_0 to PMC_SYSMON.SAT0_EFUSE_CONFIG0[15:12] */
			DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 0U);
		} else if (1U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_7_4 to PMC_SYSMON.SAT1_EFUSE_CONFIG0[15:12] */
			DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 4U);
		} else {
			/* Required due to MISRA */
			PmDbg("[%d] Invalid SateliteIdx\r\n", __LINE__);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_LPD:
		/* Copy EFUSE_CACHE.TSENS_DELTA_11_8 to LPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
		DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 8U);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_FPD:
		/* Copy EFUSE_CACHE.TSENS_DELTA_15_12 to FPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
		DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 12U);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_NOC:
		StartbitOffset = 16U + (SateliteIdx * 4U);
		/* Copy EFUSE_CACHE.TSENS_DELTA_STARTBIT_ENDBIT to AMS_SAT_N.EFUSE_CONFIG0[15:12] */
		DeltaVal =  GET_DELTA_AT_OFFSET(Arr, StartbitOffset);
		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (0U != DeltaVal) {
		PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_DELTA_MASK, (DeltaVal << EFUSE_CONFIG0_DELTA_SHIFT));
	}

	/* Lock writes */
	PmOut32(DestAddress + NPI_PCSR_LOCK_OFFSET, 1);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpLPD(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((u8)XPM_POWER_STATE_ON == Node->State) {
		Status = XST_SUCCESS;
		goto done;
	} else {
		/* TODO: LPD CDO should be rexecuted. Right now we don't have separate LPD
		 * CDO so calling house cleaning commands here
		 */
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_INIT_START, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_INIT_START;
			goto done;
		}
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_SCAN_CLEAR, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_SCAN_CLEAR;
			goto done;
		}
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_LBIST, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_LBIST;
			goto done;
		}

		/*
		 * Release SRST for PS-LPD
		 */
		Status = XPmReset_AssertbyId(PM_RST_PS_SRST, (u32)PM_RESET_ACTION_RELEASE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_RELEASE;
			goto done;
		}

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_BISR, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_BISR;
			goto done;
		}
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_MBIST_CLEAR, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_MBIST_CLEAR;
			goto done;
		}

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_INIT_FINISH, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerDwnLPD(void)
{
	XStatus Status = XST_FAILURE;
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	u32 DbgErr = (u32)XPM_INT_ERR_UNDEFINED;
	XPm_Core *RpuCore0;
	XPm_Core *RpuCore1;
	int HasResumeAddrStatus = XST_FAILURE;
	u32 i;

	RpuCore0 = (XPm_Core *)XPmDevice_GetById(PM_DEV_RPU0_0);
	if (NULL != RpuCore0) {
		HasResumeAddrStatus = XPmCore_HasResumeAddr(RpuCore0);
		if (XST_SUCCESS == HasResumeAddrStatus) {
			/* Enable GIC proxy only if resume path is set */
			XPm_GicProxy.Enable();
		} else {
			DbgErr = (u32)XPM_INT_ERR_INVALID_ADDR;
		}
	}

	RpuCore1 = (XPm_Core *)XPmDevice_GetById(PM_DEV_RPU0_1);
	if (NULL != RpuCore1) {
		HasResumeAddrStatus = XPmCore_HasResumeAddr(RpuCore1);
		if (XST_SUCCESS == HasResumeAddrStatus) {
			/* Enable GIC proxy only if resume path is set */
			XPm_GicProxy.Enable();
		} else {
			DbgErr = (u32)XPM_INT_ERR_INVALID_ADDR;
		}
	}

	const u32 LpdPlIso[] = {
		(u32)XPM_NODEIDX_ISO_LPD_PL_TEST,
		(u32)XPM_NODEIDX_ISO_LPD_PL
	};
	const u32 LpdPlIsoErr[] = {
		(u32)XPM_INT_ERR_LPD_PL_TEST_ISO,
		(u32)XPM_INT_ERR_LPD_PL_ISO
	};

	if (NULL == LpDomain) {
		DbgErr = (u32)XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	XPm_Device *AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	if (NULL == AmsRoot) {
		DbgErr = (u32)XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock configuration and system registers for write operation */
	PmOut32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_LOCK_OFFSET,
		PCSR_UNLOCK_VAL);

	/* Disable the SSC interface to PS LPD satellite */
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
		AMS_ROOT_TOKEN_MNGR_BYPASS_LPD_MASK,
		AMS_ROOT_TOKEN_MNGR_BYPASS_LPD_MASK);

	/* Lock configuration and system registers */
	PmOut32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_LOCK_OFFSET, 1);

	/**
	 * Isolate LPD <-> PL interface and mark the isolations pending for removal
	 * only if appropriate based on the initial boot time configuration.
	 */
	for (i = 0; i < ARRAY_SIZE(LpdPlIso) && i < ARRAY_SIZE(LpdPlIsoErr); i++) {
		XPm_IsoStates IsoState;

		Status = XPmDomainIso_GetState(LpdPlIso[i], &IsoState);
		if (XST_SUCCESS != Status) {
			DbgErr = LpdPlIsoErr[i];
			goto done;
		}

		u32 Enable = (IsoState == PM_ISOLATION_ON) ?
					TRUE_VALUE : TRUE_PENDING_REMOVE;

		/* Isolate LPD-PL */
		Status = XPmDomainIso_Control(LpdPlIso[i], Enable);
		if (XST_SUCCESS != Status) {
			DbgErr = LpdPlIsoErr[i];
			goto done;
		}
	}

	/* Isolate PS_CPM domains */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM_DFX, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_LPD_CPM_DFX_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_LPD_CPM_ISO;
		goto done;
	}

	/* Isolate LP-SoC */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_LPD_SOC_ISO;
		goto done;
	}

	/* Isolate PS_PMC domains */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD_DFX, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_PMC_LPD_DFX_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_PMC_LPD_ISO;
		goto done;
	}

	/* Assert reset for PS SRST */
	Status = XPmReset_AssertbyId(PM_RST_PS_SRST, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_PS_SRST;
	}

	/* Skip PS-POR and LPD power rail handling in case of user PS-SRST */
	if (0U == UserAssertPsSrst) {
		/* Assert POR for PS-LPD */
		Status = XPmReset_AssertbyId(PM_RST_PS_POR, (u32)PM_RESET_ACTION_ASSERT);
		if (XST_SUCCESS != Status) {
			DbgErr = (u32)XPM_INT_ERR_PS_POR;
		}

		/*TODO: Send PMC_I2C command to turn off PS-LPD power rail */
	}

	LpDomain->LpdBisrFlags &= (u8)(~(LPD_BISR_DATA_COPIED | LPD_BISR_DONE));

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpFPD(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	if ((u8)XPM_POWER_STATE_ON != Node->State) {
		/* Restore the PSM APU power state register */
		PmOut32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_APU_POWER_STATUS_INIT_OFFSET, PsmApuPwrState);

		PmInfo("Reloading FPD CDO\r\n");
		Status = XPm_RestartCbWrapper(Node->Id);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RELOAD_IMAGE;
		}

		XPm_GicProxy.Clear();
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerDwnFPD(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	XPm_Psm *Psm;
	XPm_Core *ApuCore;
	int HasResumeAddrStatus = XST_FAILURE;
	u32 DbgErr = (u32)XPM_INT_ERR_UNDEFINED;
	u32 i;

	const u32 FpdPlIso[] = {
		(u32)XPM_NODEIDX_ISO_FPD_PL,
		(u32)XPM_NODEIDX_ISO_FPD_PL_TEST
	};
	const u32 FpdPlIsoErr[] = {
		(u32)XPM_INT_ERR_FPD_PL_ISO,
		(u32)XPM_INT_ERR_FPD_PL_TEST_ISO
	};

	XPm_Device *AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	if (NULL == AmsRoot) {
		DbgErr = (u32)XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	/* Unlock configuration and system registers for write operation */
	PmOut32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_LOCK_OFFSET,
		PCSR_UNLOCK_VAL);

	/* Disable the SSC interface to PS FPD satellite */
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
		AMS_ROOT_TOKEN_MNGR_BYPASS_FPD_MASK,
		AMS_ROOT_TOKEN_MNGR_BYPASS_FPD_MASK);

	/* Lock configuration and system registers */
	PmOut32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_LOCK_OFFSET, 1);

	/* Isolate FPD-NoC */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_FPD_SOC_ISO;
		goto done;
	}

	/**
	 * Isolate FPD <-> PL interface and mark the isolations pending for removal
	 * only if appropriate based on the initial boot time configuration.
	 */
	for (i = 0; i < ARRAY_SIZE(FpdPlIso) && i < ARRAY_SIZE(FpdPlIsoErr); i++) {
		XPm_IsoStates IsoState;

		Status = XPmDomainIso_GetState(FpdPlIso[i], &IsoState);
		if (XST_SUCCESS != Status) {
			DbgErr = FpdPlIsoErr[i];
			goto done;
		}

		u32 Enable = (IsoState == PM_ISOLATION_ON)?
				TRUE_VALUE : TRUE_PENDING_REMOVE;

		/* Isolate FPD-PL */
		Status = XPmDomainIso_Control(FpdPlIso[i], Enable);
		if (XST_SUCCESS != Status) {
			DbgErr = FpdPlIsoErr[i];
			goto done;
		}
	}

	Status = XPmPsm_SendPowerDownReq(Node->BaseAddress);

	/* Assert SRST for FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD, (u32)PM_RESET_ACTION_ASSERT);

	/* Assert POR for FPD */
	Status = XPmReset_AssertbyId(PM_RST_FPD_POR, (u32)PM_RESET_ACTION_ASSERT);

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		DbgErr = (u32)XPM_INT_ERR_INVALID_PROC;
		Status = XST_FAILURE;
		goto done;
	}

	ApuCore = (XPm_Core *)XPmDevice_GetById(PM_DEV_ACPU_0);
	if (NULL != ApuCore) {
		HasResumeAddrStatus = XPmCore_HasResumeAddr(ApuCore);
		if (XST_SUCCESS == HasResumeAddrStatus) {
			/* Enable GIC proxy only if resume path is set */
			XPm_GicProxy.Enable();

			/* Store the PSM APU power state register */
			PmIn32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_APU_POWER_STATUS_INIT_OFFSET, PsmApuPwrState);
		} else {
			DbgErr = (u32)XPM_INT_ERR_INVALID_ADDR;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpPLD(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((u8)XPM_POWER_STATE_ON == Node->State) {
		Status = XST_SUCCESS;
		goto done;
	} else {
		/* TODO: PLD CDO should be rexecuted. Right now we don't have separate PLD
		 * CDO so calling house cleaning commands here
		 */
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_INIT_START, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_INIT_START;
			goto done;
		}

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_HOUSECLEAN_PL, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_HOUSECLEAN_PL;
			goto done;
		}

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_INIT_FINISH, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
		}
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerDwnPLD(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Isolate PL-NoC */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}

	/* Isolate FPD-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL_TEST, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_TEST_ISO;
		goto done;
	}

	/* Isolate LPD-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_PL_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL_TEST, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_PL_TEST_ISO;
		goto done;
	}

	/* Isolate PL-PMC */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_TEST, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_TEST_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_CFRAME, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_CFRAME_ISO;
		goto done;
	}

	/* Isolate VCCINT_RAM from VCCINT */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCRAM_SOC_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCAUX_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCAUX_VCCRAM_ISO;
		goto done;
	}

	/* Isolate PL_CPM */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_PCIEA0_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_PCIEA1_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_RST_CPI0, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_RST_CPI0_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_RST_CPI1, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_RST_CPI1_ISO;
		goto done;
	}
	/* Reset Houseclean flag for PL */
	HcleanDone = 0U;

	/* Assert POR PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
	}

	/* TODO: Send PMC_I2C command to turn of PLD power rail */
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpME(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	(void)Node;

	/* TODO: Reload ME CDO */
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnME(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Isolate ME */

	/* TODO: Assert POR ME */

	/* TODO: Send PMC_I2C command to turn of ME power rail */

	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpCPM(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	(void)Node;

	/* TODO: Reload CPM CDO */
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnCPM(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Isolate LPD-CPM */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM_ISO;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM_DFX, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM_DFX_ISO;
		goto done;
	}

	/* Isolate PL_CPM */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_PCIEA0_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_PCIEA1_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_RST_CPI0, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_RST_CPI0_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_CPM_RST_CPI1, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM_RST_CPI1_ISO;
		goto done;
	}

	/* Assert POR for CPM */
	Status = XPmReset_AssertbyId(PM_RST_CPM_POR, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}

	/* TODO: Send PMC_I2C command to turn off CPM power rail */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpNoC(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

        if ((u8)XPM_POWER_STATE_ON == Node->State) {
			Status = XST_SUCCESS;
                goto done;
        } else {
                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_INIT_START, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_INIT_START;
			goto done;
		}
                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_SCAN_CLEAR, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_SCAN_CLEAR;
			goto done;
		}

                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_BISR, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_BISR;
			goto done;
		}
                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_MBIST_CLEAR, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_MBIST_CLEAR;
			goto done;
		}

                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, (u32)FUNC_INIT_FINISH, NULL, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
		}
        }

done:
	XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}

XStatus XPm_PowerDwnNoC(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	XPm_NpDomain *NpDomain = (XPm_NpDomain *)XPmPower_GetById(PM_POWER_NOC);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((NULL == AmsRoot) || (NULL == NpDomain)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock configuration and system registers for write operation */
	PmOut32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_LOCK_OFFSET,
		PCSR_UNLOCK_VAL);

	/* PL satellite depends on NPD and not PLD so disable the SSC interface to PL satellite
		while powering down NPD*/
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
		AMS_ROOT_TOKEN_MNGR_BYPASS_PL_MASK,
		AMS_ROOT_TOKEN_MNGR_BYPASS_PL_MASK);

	/* Lock configuration and system registers */
	PmOut32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_LOCK_OFFSET, 1);

	/* Isolate FPD-NoC domain */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_SOC_ISO;
		goto done;
	}

	/* Isolate LPD-NoC domain */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_SOC_ISO;
		goto done;
	}

	/* Isolate PL-NoC domain */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}

	/* Isolate VCCAUX-NoC domain */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCAUX_ISO;
		goto done;
	}

	/* Isolate VCCRAM-NoC domain */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCRAM_SOC_ISO;
		goto done;
	}

	/* Isolate PMC-NoC domain */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_ISO;
		goto done;
	}

	/* Isolate PMC-NoC NPI domain */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_NPI_ISO;
		goto done;
	}

	/* Assert POR for NoC */
	Status = XPmReset_AssertbyId(PM_RST_NOC_POR, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
	}

	/* TODO: Send PMC_I2C command to turn off NoC power rail */

	NpDomain->BisrDataCopied = 0;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  Check power rail if minimum operational voltage has been reached
 *		   using Sysmon
 *
 * @param  PowerRail: Pointer to power rail node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
static XStatus XPmPower_SysmonCheckPower(XPm_Rail *Rail)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RailVoltage = 0;
	u32 LowThreshVal = 0;

	/**
	 * Index is stored as the BaseAddress and is used to calculate the SysMon
	 * SUPPLYn and NewDataFlag registers
	 */
	u32 Index = Rail->Power.Node.BaseAddress;
	u32 SysmonSupplyReg = (u32)PMC_SYSMON_SUPPLY0 + (Index * 4U);
	u32 SysmonLowThReg = (u32)PMC_SYSMON_SUPPLY0_TH_LOWER + (Index * 4U);
	u32 NewDataFlagReg = (u32)PMC_SYSMON_NEW_DATA_FLAG0 + ((Index/32U) * 4U);
	u32 Offset = Index % 32U;

	/* Wait for New Data Flag */
	Status = XPm_PollForMask(NewDataFlagReg, BIT32(Offset),
								SYSMON_CHECK_POWER_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NEW_DATA_FLAG_TIMEOUT;
		goto done;
	}

	PmIn32(SysmonLowThReg, LowThreshVal);
	PmIn32(SysmonSupplyReg, RailVoltage);
	if (RailVoltage < LowThreshVal) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		Status = XST_FAILURE;
	}

	/* Unlock Root SysMon registers */
	PmOut32((PMC_SYSMON_BASEADDR + AMS_ROOT_REG_PCSR_LOCK_OFFSET),
			PCSR_UNLOCK_VAL);

	/* Clear New Data Flag */
	PmOut32(NewDataFlagReg, BIT32(Offset));

	/* Lock Root SysMon registers */
	PmOut32((PMC_SYSMON_BASEADDR + AMS_ROOT_REG_PCSR_LOCK_OFFSET), 1U);

done:

	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Check power rail power using power detectors. This check uses the
 *		   PMC_GLOBAL_PWR_SUPPLY_STATUS registers
 *
 * @param  VoltageRailMask: Mask of PMC_GLOBAL_PWR_SUPPLY_STATUS registers for
 *		   rails to be checked
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note This function uses power detectors which are not very reliable. It
 *		 should only be used when no other methods are available i.e. sysmon
 *
 ****************************************************************************/
static XStatus XPmPower_DetectorCheckPower(u32 VoltageRailMask)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;
	XPm_Pmc *Pmc;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmIn32(Pmc->PmcGlobalBaseAddr + PWR_SUPPLY_STATUS_OFFSET, RegVal);
	if((RegVal & VoltageRailMask) != VoltageRailMask) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;

}

XStatus XPmPower_CheckPower(XPm_Rail *Rail, u32 VoltageRailMask)
{
	XStatus Status = XST_FAILURE;
	u32 Source;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Platform = XPm_GetPlatform();

	if ((NULL == Rail) || (PLATFORM_VERSION_QEMU == Platform)) {
		Status = XPmPower_DetectorCheckPower(VoltageRailMask);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		}
		goto done;
	}

	Source = (u32)Rail->Source;
	if ((u32)XPM_PGOOD_SYSMON == Source) {
		Status = XPmPower_SysmonCheckPower(Rail);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		}
	} else {
		DbgErr = XPM_INT_ERR_RAIL_SOURCE;
	}

done:
	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;
}

static void XPmPower_UpdateResetFlags(XPm_PowerDomain *PwrDomain,
				      enum XPmInitFunctions FuncId)
{
	XPm_ResetNode *Reset;
	u32 ResetId;
	u32 PmcSysResetMask = (CRP_RESET_REASON_SLR_SYS_MASK |
			       CRP_RESET_REASON_SW_SYS_MASK |
			       CRP_RESET_REASON_ERR_SYS_MASK |
			       CRP_RESET_REASON_DAP_SYS_MASK);
	u32 DomainStatusMask = (u32)1U << (NODEINDEX(PwrDomain->Power.Node.Id) - 1U);

	/* Clear System Reset and domain POR reset flags */
	SystemResetFlag = 0;
	DomainPORFlag = 0;

	if (FUNC_INIT_FINISH == FuncId) {
		/*
		 * Mark domain init status bit in DomainInitStatusReg if
		 * initialization is done.
		 */
		if (PwrDomain->InitFlag == PwrDomain->InitMask) {
			PmRmw32(XPM_DOMAIN_INIT_STATUS_REG, DomainStatusMask,
				DomainStatusMask);
		}
	} else if (FUNC_INIT_START == FuncId) {

		PwrDomain->InitFlag = 0;

		/*
		 * All sequences should be executed on PMC_POR. During PMC_POR
		 * power domain bit in XPM_DOMAIN_INIT_STATUS_REG is 0. So
		 * don't set DomainPORFlag or SystemResetFlag flags.
		 */
		if (0U == (XPm_In32(XPM_DOMAIN_INIT_STATUS_REG) &
			  DomainStatusMask)) {
			goto done;
		}

		switch (NODEINDEX(PwrDomain->Power.Node.Id)) {
		case (u32)XPM_NODEIDX_POWER_LPD:
			ResetId = PM_RST_PS_POR;
			break;
		case (u32)XPM_NODEIDX_POWER_FPD:
			ResetId = PM_RST_FPD_POR;
			break;
		case (u32)XPM_NODEIDX_POWER_NOC:
			ResetId = PM_RST_NOC_POR;
			break;
		case (u32)XPM_NODEIDX_POWER_CPM:
			ResetId = PM_RST_CPM_POR;
			break;
		default:
			ResetId = 0;
			break;
		}

		/* Check for POR reset for a domain is occurred or not. */
		if (0U != ResetId) {
			Reset = XPmReset_GetById(ResetId);
			if (XPM_RST_STATE_ASSERTED ==
			    Reset->Ops->GetState(Reset)) {
				DomainPORFlag = 1;
				goto done;
			}
		}

		/* Check for system reset is occurred or not. */
		if (0U != (ResetReason & PmcSysResetMask)) {
			SystemResetFlag = 1;
		}
	} else {
		/* Required by MISRA */
	}

done:
	return;
}

XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  u32 *Args, u32 NumArgs)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	struct XPm_PowerDomainOps *Ops = PwrDomain->DomainOps;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	PmDbg("%s for PwrDomain 0x%x Start\r\n", PmInitFunctions[Function],
						  PwrDomain->Power.Node.Id);

	if (((u8)XPM_POWER_STATE_ON == PwrDomain->Power.Node.State)
	&&  ((u32)FUNC_XPPU_CTRL != Function)
	&&  ((u32)FUNC_XMPU_CTRL != Function)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check PL power up at every init node command to see if we can run Pl houseclean*/
	(void)XPmPlDomain_InitandHouseclean();

	switch (Function) {
	case (u32)FUNC_INIT_START:
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_INITIALIZING;
		XPmPower_UpdateResetFlags(PwrDomain, FUNC_INIT_START);
		if ((NULL != Ops) && (NULL != Ops->InitStart)) {
			Status = Ops->InitStart(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_START;
				goto done;
			}
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_INIT_FINISH:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->InitFinish)) {
			Status = Ops->InitFinish(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
				goto done;
			}
		}
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_ON;
		XPmNotifier_Event(PwrDomain->Power.Node.Id, (u32)EVENT_STATE_CHANGE);
		if (PM_POWER_PLD == PwrDomain->Power.Node.Id) {
			/* Request PLD0 device once PL is housecleaned. */
			Status = XPmDevice_Request(PM_SUBSYS_PMC, PM_DEV_PLD_0,
						   XPM_MAX_CAPABILITY,
						   XPM_MAX_QOS);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_REQ_PL_DEVICE;
				break;
			}
		} else if (PM_POWER_ME == PwrDomain->Power.Node.Id) {
			/* Request AIE device once AIE intialization is done. */
			Status = XPmDevice_Request(PM_SUBSYS_PMC, PM_DEV_AIE,
						   XPM_MAX_CAPABILITY,
						   XPM_MAX_QOS);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_REQ_ME_DEVICE;
				break;
			}
		} else {
			/* Required for MISRA */
		}

		Status = XPmDomainIso_ProcessPending(PwrDomain->Power.Node.Id);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DOMAIN_ISO;
			goto done;
		}

		XPmPower_UpdateResetFlags(PwrDomain, FUNC_INIT_FINISH);

		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_SCAN_CLEAR:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset or POR of a domain */
		/* HACK: Don't skip scanclear for AIE */
		if (((1U == SystemResetFlag) || (1U == DomainPORFlag)) &&
		     (PwrDomain->Power.Node.Id != PM_POWER_ME)) {
			Status = XST_SUCCESS;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->ScanClear)) {
			Status = Ops->ScanClear(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_SCAN_CLEAR;
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_SCAN_CLEAR);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_BISR:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset */
		if ((1U == SystemResetFlag) &&
		    (PwrDomain->Power.Node.Id != PM_POWER_NOC)) {
			Status = XST_SUCCESS;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->Bisr)) {
			Status = Ops->Bisr(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_BISR;
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_BISR);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_LBIST:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset or POR of a domain */
		if ((1U == SystemResetFlag) || (1U == DomainPORFlag)) {
			Status = XST_SUCCESS;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->Lbist)) {
			XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (Ops->Lbist), (Args), (NumArgs));
			XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
			/* Required for redundancy */
			if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
				DbgErr = XPM_INT_ERR_FUNC_LBIST;
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_LBIST);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_MBIST_CLEAR:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset or POR of a domain */
		if ((1U == SystemResetFlag) || (1U == DomainPORFlag)) {
			Status = XST_SUCCESS;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->Mbist)) {
			XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (Ops->Mbist), (Args), (NumArgs));
			XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
			/* Required for redundancy */
			if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
				DbgErr = XPM_INT_ERR_FUNC_MBIST_CLEAR;
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_MBIST_CLEAR);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_HOUSECLEAN_PL:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->PlHouseclean)) {
			Status = Ops->PlHouseclean(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_HOUSECLEAN_PL;
				goto done;
			}
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_MEM_INIT:
                if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
                        Status = XST_FAILURE;
                        goto done;
                }
                if ((NULL != Ops) && (NULL != Ops->MemInit)) {
                        Status = Ops->MemInit(Args, NumArgs);
                        if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_MEM_INIT;
                                goto done;
                        }
                }
		Status = XST_SUCCESS;
                break;
	case (u32)FUNC_HOUSECLEAN_COMPLETE:
                if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
                        Status = XST_FAILURE;
                        goto done;
                }
                if ((NULL != Ops) && (NULL != Ops->HcComplete)) {
                        Status = Ops->HcComplete(Args, NumArgs);
                        if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_HOUSECLEAN_COMPLETE;
                                goto done;
                        }
                }
		Status = XST_SUCCESS;
                break;
	case (u32)FUNC_XPPU_CTRL:
		if ((u8)XPM_POWER_STATE_ON != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->XppuCtrl)) {
			Status = Ops->XppuCtrl(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_XPPU_CTRL;
				goto done;
			}
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_XMPU_CTRL:
		if ((u8)XPM_POWER_STATE_ON != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->XmpuCtrl)) {
			Status = Ops->XmpuCtrl(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_XMPU_CTRL;
				goto done;
			}
		}
		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_FUNC;
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);

	if (XST_SUCCESS == Status) {
		PmDbg("%s for PwrDomain 0x%x completed successfully\r\n",
		      PmInitFunctions[Function], PwrDomain->Power.Node.Id);
	}

	return Status;
}
