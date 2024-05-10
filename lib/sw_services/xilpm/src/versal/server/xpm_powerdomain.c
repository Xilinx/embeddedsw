/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
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
#include "xpm_psfpdomain.h"
#include "xpm_pldomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_gic_proxy.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_pldevice.h"
#ifdef PLM_ENABLE_STL
#include "xstl_plminterface.h"
#endif

#define SYSMON_CHECK_POWER_TIMEOUT	2000000U
#define NUM_PLD0_PWR_DOMAIN_DEPENDENCY	1U
#define PWR_DOMAIN_UNUSED_BITMASK	0U

/* If Sysmon low threshold registers are not used they will be programmed to 0.
 * SysMon data uses Modified Floating point Data format. Bits [18:17] indicate
 * the exponenent offset used to calculate the actual voltage from the register
 * reading and is always programmed to 01. We must account for this when
 * reading the sysmon registers even when they are programmed to zero.
 */
#define SYSMON_THRESH_NOT_PROGRAMMED	0x20000U

static u8 SystemResetFlag;
static u8 DomainPORFlag;
static u32 PsmApuPwrState;

#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
static u32 RequestDomainId = 0U; /* Request domain to keep power up */
#endif

/*
 * Power rail index map
 */
#define RAILIDX(Idx) \
		((u32)(Idx) - (u32)XPM_NODEIDX_POWER_VCCINT_PMC)

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
	[FUNC_MIO_FLUSH]		= "MIO_FLUSH",
	[FUNC_MEM_CTRLR_MAP]		= "MEM_CTRLR_MAP",
};

XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    const struct XPm_PowerDomainOps *Ops)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPower_Init(&PowerDomain->Power, Id, BaseAddress, Parent);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	PowerDomain->DomainOps = Ops;
	if (NULL != Parent) {
		PowerDomain->Parents[0] = Parent->Node.Id;
	}

	/* Set houseclean disable mask to default */
	PowerDomain->HcDisableMask = (u32)HOUSECLEAN_DISABLE_DEFAULT_MASK;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPowerDomain_AddParent(u32 Id, const u32 *ParentNodes, u32 NumParents)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PowerD;
	XPm_PowerDomain *ParentPowerD;
	u32 i, j;

	PowerD = (XPm_PowerDomain *)XPmPower_GetById(Id);

	if (NULL == PowerD) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if ((MAX_POWERDOMAINS - 1U) < NumParents) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (i = 0; i < NumParents; i++) {
		PowerD->Parents[i + 1U] = ParentNodes[i];

		/* Add Id as child of each parent node */
		ParentPowerD = (XPm_PowerDomain *)XPmPower_GetById(ParentNodes[i]);
		if (NULL == ParentPowerD) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		for (j = 0U; j < MAX_POWERDOMAINS; j++) {
			if (ParentPowerD->Children[j] != 0U) {
				continue;
			}

			ParentPowerD->Children[j] = Id;
			break;
		}

		if (MAX_POWERDOMAINS == j) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

#define BITMASK_LOWER_15_BITS			(0x7fffU)
#define BITMASK_UPPER_17_BITS			(0xffff8000U)
#define GET_DELTA_AT_OFFSET(array, x)		(0xfU & (array[(x) / 32U] >> ((x) % 32U)))

static XStatus XPmPowerDomain_CopyCache(u32 EfuseCacheBaseAddress,  u32 PowerDomainId,
					u32 SateliteIdx, u32 *CacheRead, u32 *DeltaVal)
{
	XStatus Status = XST_FAILURE;
	u32 StartbitOffset, RegValue, i;
	u32 Arr[8] = {0};

	if (0U == *CacheRead) {
		/* Copy 256 bits of TSENS_DELTA value to array */
		PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_3_OFFSET, RegValue);
		/*Store 17 bits from current register */
		Arr[0] = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_MASK) >>
			  EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_SHIFT;
		for (i = 0U; i < 8U; i++) {
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
		*CacheRead = 1;
	}

	switch (NODEINDEX(PowerDomainId)) {
	case (u32)XPM_NODEIDX_POWER_PMC:
		if (0U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_3_0 to PMC_SYSMON.SAT0_EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 0U);
		} else if (1U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_7_4 to PMC_SYSMON.SAT1_EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 4U);
		} else {
			/* Required due to MISRA */
			PmDbg("[%d] Invalid SateliteIdx\r\n", __LINE__);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_LPD:
		/* Copy EFUSE_CACHE.TSENS_DELTA_11_8 to LPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
		*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 8U);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_FPD:
		/* Copy EFUSE_CACHE.TSENS_DELTA_15_12 to FPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
		*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 12U);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_NOC:
		StartbitOffset = 16U + (SateliteIdx * 4U);
		/* Copy EFUSE_CACHE.TSENS_DELTA_STARTBIT_ENDBIT to AMS_SAT_N.EFUSE_CONFIG0[15:12] */
		*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, StartbitOffset);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

XStatus XPmPowerDomain_ApplyAmsTrim(u32 DestAddress, u32 PowerDomainId, u32 SateliteIdx)
{
	XStatus Status = XST_FAILURE;
	u32 EfuseCacheBaseAddress, RegValue, DeltaVal = 0;
	static u32 OffsetVal,SlopeVal,ProcessVal,ResistorVal,BjtOffsetVal,ExtOffsetVal,AnaSpareVal,DigSpareVal;
	static u32 CacheRead=0;
	static u32 BipSelVal, TsensSelVal, TsensBiasVal;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (0U == DestAddress) {
		DbgErr = XPM_INT_ERR_INVALID_ADDR;
		goto done;
	}

	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}
	EfuseCacheBaseAddress = EfuseCache->Node.BaseAddress;

	/* Unlock writes */
	XPm_UnlockPcsr(DestAddress);

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

	Status = XPmPowerDomain_CopyCache(EfuseCacheBaseAddress, PowerDomainId,
					  SateliteIdx, &CacheRead, &DeltaVal);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto fail;
	}

	if (0U != DeltaVal) {
		PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_DELTA_MASK, (DeltaVal << EFUSE_CONFIG0_DELTA_SHIFT));
	}

fail:
	/* Lock writes */
	XPm_LockPcsr(DestAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpLPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((u8)XPM_POWER_STATE_ON == Node->State) {
		Status = XST_SUCCESS;
		goto done;
	} else {
		PmInfo("Reloading LPD Image\r\n");
		Status = XPm_RestartCbWrapper(Node->Id);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RELOAD_IMAGE;
			goto done;
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
	const XPm_Core *RpuCore0;
	const XPm_Core *RpuCore1;
	XStatus HasResumeAddrStatus = XST_FAILURE;
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

	const XPm_Device *AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	if (NULL == AmsRoot) {
		DbgErr = (u32)XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock configuration and system registers for write operation */
	XPm_UnlockPcsr(AmsRoot->Node.BaseAddress);

	/* Disable the SSC interface to PS LPD satellite */
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
		AMS_ROOT_TOKEN_MNGR_BYPASS_LPD_MASK,
		AMS_ROOT_TOKEN_MNGR_BYPASS_LPD_MASK);

	/* Lock configuration and system registers */
	XPm_LockPcsr(AmsRoot->Node.BaseAddress);

	/**
	 * Isolate LPD <-> PL interface and mark the isolations pending for removal
	 * only if appropriate based on the initial boot time configuration.
	 */
	for (i = 0; i < MIN(ARRAY_SIZE(LpdPlIso), ARRAY_SIZE(LpdPlIsoErr)); i++) {
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

		/* Power down LPD power rail */
		Status = XPmPower_UpdateRailStats(&LpDomain->Domain,
						  (u8)XPM_POWER_STATE_OFF);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_LPD_RAIL_CONTROL;
			goto done;
		}
	}

	LpDomain->LpdBisrFlags &= (u8)(~(LPD_BISR_DATA_COPIED | LPD_BISR_DONE));

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Psm *Psm;

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

XStatus XPm_PowerDwnFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	const XPm_Psm *Psm;
	const XPm_PsFpDomain *FpDomain = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);
	const XPm_Core *ApuCore;
	XStatus HasResumeAddrStatus = XST_FAILURE;
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

	const XPm_Device *AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	if (NULL == AmsRoot) {
		DbgErr = (u32)XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	/* Unlock configuration and system registers for write operation */
	XPm_UnlockPcsr(AmsRoot->Node.BaseAddress);

	/* Disable the SSC interface to PS FPD satellite */
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
		AMS_ROOT_TOKEN_MNGR_BYPASS_FPD_MASK,
		AMS_ROOT_TOKEN_MNGR_BYPASS_FPD_MASK);

	/* Lock configuration and system registers */
	XPm_LockPcsr(AmsRoot->Node.BaseAddress);

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
	for (i = 0; i < MIN(ARRAY_SIZE(FpdPlIso), ARRAY_SIZE(FpdPlIsoErr)); i++) {
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

	/* Power down FPD power rail */
	Status = XPmPower_UpdateRailStats(&FpDomain->Domain,
					  (u8)XPM_POWER_STATE_OFF);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_RAIL_CONTROL;
		goto done;
	}

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
		Status = XPmNpDomain_ClockGate(Node, 1);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
			goto done;
		}

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

XStatus XPm_PowerDwnPLD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PlDomain *PldDomain = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	u32 Platform = XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();

	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unset CFG_POR_CNT_SKIP to enable PL_POR counting */
	PmOut32(Pmc->PmcAnalogBaseAddr + PMC_ANLG_CFG_POR_CNT_SKIP_OFFSET,
		0U);

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

	/* PMC PL CFRAME isolation should never be enabled for ES1 due to
	   silicon issue so enable only for non ES1 platform */
	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    ((u32)PLATFORM_VERSION_SILICON_ES1 != PlatformVersion)) {
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_CFRAME, TRUE_VALUE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PMC_PL_CFRAME_ISO;
			goto done;
		}
	}

	/* Isolate VCCINT_RAM from VCCINT */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCRAM_SOC_ISO;
		goto done;
	}

	/* Reset Houseclean flag for PL */
	HcleanDone = 0U;

	/* Assert POR PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
	}

	/* Power down PLD power rail */
	Status = XPmPower_UpdateRailStats(&PldDomain->Domain,
					  (u8)XPM_POWER_STATE_OFF);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLD_RAIL_CONTROL;
		goto done;
	}

	Status = XPmNpDomain_ClockGate(Node, 0);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpME(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Node;
	Status = XPmNpDomain_ClockGate(Node, 1);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
	}

	/* TODO: Reload ME CDO */

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerDwnME(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* TODO: Isolate ME */

	/* TODO: Assert POR ME */

	/* TODO: Send PMC_I2C command to turn of ME power rail */

	Status = XPmNpDomain_ClockGate(Node, 0);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpCPM(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Node;
	Status = XPmNpDomain_ClockGate(Node, 1);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
	}

	/* TODO: Reload CPM CDO */

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpCPM5(const XPm_Node *Node)
{
	(void)Node;
	return XST_SUCCESS;
}

XStatus XPm_PowerDwnCPM5(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_CpmDomain *Cpm;

	if (NULL == Node) {
		PmErr("Invalid Node\r\n");
		goto done;
	}
	Cpm = (XPm_CpmDomain *)Node;

	/* Note: Assumption is made that idling is already taken care of */

	/** Assert Reset to CPM5 **/
	/* Unlock the PCSR*/
	XPm_UnlockPcsr(Cpm->CpmPcsrBaseAddr);
	/* Assert the INITSTATE bit and release open the clock gates in CPM */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		CPM_PCSR_MASK_SCAN_CLEAR_INITSTATE_WEN_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET),
			CPM_PCSR_MASK_SCAN_CLEAR_INITSTATE_WEN_MASK,
			CPM_PCSR_MASK_SCAN_CLEAR_INITSTATE_WEN_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_MASK;
		goto done;
	}
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET,
		CPM_PCSR_PCR_INITSTATE_MASK);
	/* Assert lpd_cpm_por_n reset to CPM5. (Write 1 to CRL.RST_OCM2.POR) */
	PmRmw32(CRL_RST_OCM2_CTRL, CRL_RST_OCM2_CTRL_POR_MASK, 0x1);
	/* Lock PCSR*/
	XPm_LockPcsr(Cpm->CpmPcsrBaseAddr);

	/** Turn on isolation **/
	/* Isolate PL-CPM5 */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_CPM5_PL, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM5_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_CPM5_PL_DFX, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_CPM5_DFX_ISO;
		goto done;
	}
	/* Isolate LPD-CPM5 */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM5, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM5_ISO;
		goto done;
	}
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM5_DFX, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM5_DFX_ISO;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);

	return Status;
}

XStatus XPm_PowerDwnCPM(const XPm_Node *Node)
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

	Status = XPmNpDomain_ClockGate(Node, 0);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
	}

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
	const XPm_Device *AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	const XPm_NpDomain *NpDomain = (XPm_NpDomain *)XPmPower_GetById(PM_POWER_NOC);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((NULL == AmsRoot) || (NULL == NpDomain)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock configuration and system registers for write operation */
	XPm_UnlockPcsr(AmsRoot->Node.BaseAddress);

	/* PL satellite depends on NPD and not PLD so disable the SSC interface to PL satellite
		while powering down NPD*/
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
		AMS_ROOT_TOKEN_MNGR_BYPASS_PL_MASK,
		AMS_ROOT_TOKEN_MNGR_BYPASS_PL_MASK);

	/* Lock configuration and system registers */
	XPm_LockPcsr(AmsRoot->Node.BaseAddress);

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
		DbgErr = XPM_INT_ERR_VCCAUX_SOC_ISO;
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

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief This function is used if SysMon lower threshold registers are not
 *        programmed. Hardcoded minimum voltage values or EFUSE are used.
 *
 * @param  Rail: Pointer to power rail node
 * @param  RailVoltage: Current Sysmon voltage reading
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note If the lower threshold registers are programmed the PDI will be device
 *       dependent. Errors are returned to indicate mismatch in device and boot
 *       image.
 *****************************************************************************/
static XStatus SysmonVoltageCheck(const XPm_Rail *Rail, u32 RailVoltage)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeIndex;

	/**
	 * Hardcoded voltages used when sysmon lower threshold values are not used.
	 * Second array element is placeholder for when EFUSE is blown.
	 */
	const u32 RailVoltageTable[8][2] = {
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PMC)] = {0x2547AU, 0U},   /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCAUX_PMC)] = {0x2b333U, 0U},   /* 1.4V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PSLP)] = {0x2547AU, 0U},  /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PSFP)] = {0x2547AU, 0U},  /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_SOC)] = {0x25F5CU, 0U},   /* 0.745V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_RAM)] = {0x25F5CU, 0U},   /* 0.745V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCAUX)] = {0x2b333U, 0U},       /* 1.4V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PL)] = {0x2547AU, 0U},    /* 0.66V */
	};

	NodeIndex = NODEINDEX(Rail->Power.Node.Id);

	/**
	 * Check if current rail voltage reading is below the required minimum
	 * voltage for proper operation.
	 */
	if (RailVoltage < RailVoltageTable[RAILIDX(NodeIndex)][0]) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		Status = XPM_ERR_RAIL_VOLTAGE;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Check power rail if minimum operational voltage has been reached
 *		   using Sysmon
 *
 * @param  Rail: Pointer to power rail node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
static XStatus XPmPower_SysmonCheckPower(const XPm_Rail *Rail)
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
		Status = XPM_ERR_NEW_DATA_FLAG_TIMEOUT;
		goto done;
	}

	PmIn32(SysmonLowThReg, LowThreshVal);
	PmIn32(SysmonSupplyReg, RailVoltage);

	/* If lower threshold values are not programmed, use hardcoded voltages */
	if (SYSMON_THRESH_NOT_PROGRAMMED == LowThreshVal) {
		PmDbg("Using hardcoded voltages for power rail checks\r\n");
		Status = SysmonVoltageCheck(Rail, RailVoltage);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		PmDbg("Using Sysmon lower threshold voltages for power rail checks\r\n");
		if (RailVoltage < LowThreshVal) {
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
			Status = XPM_ERR_RAIL_VOLTAGE;
		}
	}

	/* Unlock Root SysMon registers */
	XPm_UnlockPcsr(PMC_SYSMON_BASEADDR);

	/* Clear New Data Flag */
	PmOut32(NewDataFlagReg, BIT32(Offset));

	/* Lock Root SysMon registers */
	XPm_LockPcsr(PMC_SYSMON_BASEADDR);

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
	const XPm_Pmc *Pmc;
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

XStatus XPmPower_CheckPower(const XPm_Rail *Rail, u32 VoltageRailMask)
{
	XStatus Status = XST_FAILURE;
	u32 Source;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Platform = XPm_GetPlatform();

	if ((NULL == Rail) || (XPM_PGOOD_SYSMON != Rail->Source) ||
	    (PLATFORM_VERSION_QEMU == Platform)) {
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

#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
/****************************************************************************/
/**
 * @brief  Initialize domainctrl node base class
 *
 * @param  DomainCtrl: Pointer to power domainctrl struct
 * @param  DomainCtrlId: Node Id assigned to a Power DomainCtrl node
 * @param  Args: Arguments for power domain control
 * @param  NumArgs: Number of arguments for power domain
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Args is dependent on the domainctrl type. Passed arguments will be
 *		 different for mode type.
 *
 ****************************************************************************/
XStatus XPmDomainCtrl_Init(XPm_DomainCtrl *DomainCtrl, u32 DomainCtrlId, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u32 Mode, CmdLen, ArgIndex = 1U;
	u8 ModeId;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device* Device;

	if ((u32)XPM_NODEIDX_POWER_MAX <= NODEINDEX(DomainCtrlId)) {
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((3U < NumArgs) &&
		(2U == Args[ArgIndex])) { /* GPIO Type */
		ArgIndex++;
		DomainCtrl->ParentId = Args[ArgIndex];
		ArgIndex++;
		/* Check number of modes */
		if (MAX_DOMAIN_CONTROL_MODES != Args[ArgIndex]) {
			DbgErr = XPM_INT_ERR_INVALID_PARAM;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		/* Format as below:
		 * add node domain controller, type, gpio id, num_modes,
		 * mode0 id | mode1 id
		 *
		 * e.g.
		 * pm_add_node 0x4534053 0x2 0x18224023 0x2 0x300 0x0040 0x100000 0x000000
		 *                                          0x301 0x0040 0x100000 0x100000
		 * arg0: power domain controller id = 0x4534053 (PL Domain Controller)
		 * arg1: type = 2 (GPIO)
		 * arg2: GPIO id = 0x18224023 (PM_DEV_GPIO)
		 * arg3: num of modes = 0x02 (0:OFF; 1:ON;)
		 * arg4: len | mode0 = 0x300 0x0040 0x100000 0x000000
		 * arg5: len | mode1 = 0x301 0x0040 0x100000 0x100000
		 */
		for (Mode = 0; MAX_DOMAIN_CONTROL_MODES > Mode; Mode++) {
			/* 4U: Command len | modid + offset + mask + value */
			ArgIndex++;
			if ((ArgIndex + 4U) > NumArgs) {
				Status = XST_INVALID_PARAM;
				goto done;
			}
			CmdLen = (u8)((Args[ArgIndex] >> 8U) & 0xFFU);
			CmdLen = (u8)(CmdLen * 4U);	/* 4U: Num bytes in 32-bit word */
			ModeId = (u8)(Args[ArgIndex] & 0xFFU);

			if ((sizeof(XPmDomainCtrl_GPIO) != CmdLen) || (1U < ModeId)) {
				Status = XST_INVALID_PARAM;
				goto done;
			}
			ArgIndex++;
			DomainCtrl->GpioCtrl[ModeId].Offset = (u16)Args[ArgIndex];
			ArgIndex++;
			DomainCtrl->GpioCtrl[ModeId].Mask = Args[ArgIndex];
			ArgIndex++;
			DomainCtrl->GpioCtrl[ModeId].Value = Args[ArgIndex];
		}

		/* Get parent power domain and save it.
		 * This is to request GPIO device during power domain init.
		 * Request device prevents power domain powering down and
		 * keeps the GPIO configuration (direction, oen) live.
		 */
		Device = (XPm_Device *)XPmDevice_GetById(DomainCtrl->ParentId);
		if (NULL == Device) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		RequestDomainId = Device->Power->Node.Id;
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (NULL == XPmPower_GetById(DomainCtrlId)) {
		Status = XPmPower_Init(&DomainCtrl->Power, DomainCtrlId, BaseAddress, NULL);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		}
	}
	DomainCtrl->Power.Node.State = (u8)XPM_POWER_STATE_ON;
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  DomainCtrl control
 *
 * @param  DomainCtrlId: Domain Control ID
 * @param  State: Domain power state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
static XStatus XPmDomainCtrl_Control(u32 DomainCtrlId, u8 State)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u8 Mode, PowerState;
	u32 NodeIdx;
	u32 BaseAddress = 0U;
	const XPm_Rail *Rail;
	XPm_DomainCtrl *DomainCtrl;

	DomainCtrl = (XPm_DomainCtrl *)XPmPower_GetById(DomainCtrlId);

	if ((u8)XPM_POWER_STATE_ON == State) {
		Mode = 1U;
		PowerState = (u8)XPM_POWER_STATE_ON;
		PmDbg("Turning %x domain on now\r\n", DomainCtrl->Power.Node.Id);
	} else {
		/* Turn off domain if it is the last one */
		if (1U >= DomainCtrl->Power.UseCount) {
			Mode = 0U;
			DomainCtrl->Power.UseCount = 0U;
			PowerState = (u8)XPM_POWER_STATE_OFF;
			PmDbg("Turning %x domain off now\r\n", DomainCtrl->Power.Node.Id);
		} else {
			DomainCtrl->Power.UseCount--;
			Status = XST_SUCCESS;
			goto done;
		}
	}

	/* Get the GPIO device base address */
	Status = XPm_GetDeviceBaseAddr(DomainCtrl->ParentId, &BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Set GPIO value */
	XPm_RMW32(BaseAddress + DomainCtrl->GpioCtrl[Mode].Offset,
		DomainCtrl->GpioCtrl[Mode].Mask, DomainCtrl->GpioCtrl[Mode].Value);

	if ((u8)XPM_POWER_STATE_ON == PowerState) {
		/* Check if the power is on */
		NodeIdx = NODEINDEX(DomainCtrl->Power.Node.Id);

		if ((u32)XPM_NODEIDX_POWER_FPD_DOMAIN_CTRL == NodeIdx) {
			Rail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSFP);
			Status = XPmPower_CheckPower(Rail,
							PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_MASK);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_POWER_SUPPLY;
				goto done;
			}
		} else if ((u32)XPM_NODEIDX_POWER_PLD_DOMAIN_CTRL == NodeIdx) {
			Rail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PL);
			Status = XPmPower_CheckPower(Rail,
							PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_POWER_SUPPLY;
				goto done;
			}
		} else {
			/* Required by MISRA */
		}
	}

	if ((u8)XPM_POWER_STATE_ON == PowerState) {
		DomainCtrl->Power.UseCount++;
	} else {
		/* Required by MISRA */
	}
	DomainCtrl->Power.Node.State = PowerState;
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
#endif

XStatus XPmPower_UpdateRailStats(const XPm_PowerDomain *PwrDomain, u8 State)
{
	XStatus Status = XST_FAILURE;
	u32 i=0, j=0;
	const XPm_PowerDomain *ParentDomain;
	XPm_Rail *ParentRail;

#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
	u8 ParentDomainCtrl = 0U;

	/* Scan parents for a domain control node */
	for (i = 0; ((i < MAX_POWERDOMAINS) && (0U != PwrDomain->Parents[i])); i++) {
		if ((u32)XPM_NODESUBCL_POWER_DOMAIN_CTRL == NODESUBCLASS(PwrDomain->Parents[i])) {
			ParentDomainCtrl = 1U;	/* Parent domain control exists */
			break;
		} else {
			/* Required by MISRA */
		}
	}
#endif

	/* Update rail node usecounts */
	for (i = 0; ((i < MAX_POWERDOMAINS) && (0U != PwrDomain->Parents[i])); i++) {
		/* If power domain is the parent, scan through its rails */
		if ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(PwrDomain->Parents[i])) {
			ParentDomain = (XPm_PowerDomain *)XPmPower_GetById(PwrDomain->Parents[i]);
			for (j = 0; ((j < MAX_POWERDOMAINS) && (0U != ParentDomain->Parents[j])); j++) {
				if ((u32)XPM_NODESUBCL_POWER_RAIL == NODESUBCLASS(ParentDomain->Parents[j])) {
					ParentRail = (XPm_Rail *)XPmPower_GetById(ParentDomain->Parents[j]);
					if ((u8)XPM_POWER_STATE_ON == State) {
						ParentRail->Power.UseCount++;
					} else {
						ParentRail->Power.UseCount--;
					}
				}
			}
		} else if ((u32)XPM_NODESUBCL_POWER_RAIL == NODESUBCLASS(PwrDomain->Parents[i])) {
			ParentRail = (XPm_Rail *)XPmPower_GetById(PwrDomain->Parents[i]);
			if ((u8)XPM_POWER_STATE_ON == State) {
				if (PM_POWER_PMC == PwrDomain->Power.Node.Id) {
					ParentRail->Power.Node.State = (u8)XPM_POWER_STATE_ON;
				} else if ((u8)XPM_POWER_STATE_ON != ParentRail->Power.Node.State) {
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
					/* If a parent domain control exists, then set the rail state on */
					if (0U != ParentDomainCtrl) {
						ParentRail->Power.Node.State = (u8)XPM_POWER_STATE_ON;
					} else
#endif
					{
						PmDbg("Turning %x rail on now\r\n", ParentRail->Power.Node.Id);
						Status = XPmRail_Control(ParentRail,
									(u8)XPM_POWER_STATE_ON, 1U);
						if (XST_SUCCESS != Status) {
							goto done;
						}
					}
				} else {
					/* Required by MISRA */
				}
				ParentRail->Power.UseCount++;
			} else {
				ParentRail->Power.UseCount--;
				if (ParentRail->Power.UseCount <= 0U) {
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
					/* If a parent domain control exists, then set rail state off */
					if (0U != ParentDomainCtrl) {
						ParentRail->Power.Node.State = (u8)XPM_POWER_STATE_OFF;
					} else
#endif
					{
						PmDbg("Turning %x rail off now\r\n", ParentRail->Power.Node.Id);
						Status = XPmRail_Control(ParentRail,
									(u8)XPM_POWER_STATE_OFF, 0U);
						if (XST_SUCCESS != Status) {
							goto done;
						}
					}
				}
			}
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
		} else if ((u32)XPM_NODESUBCL_POWER_DOMAIN_CTRL == NODESUBCLASS(PwrDomain->Parents[i])) {
			Status = XPmDomainCtrl_Control(PwrDomain->Parents[i], State);
			if (XST_SUCCESS != Status) {
				goto done;
			}
#endif
		} else {
			PmDbg("Power parent error.\r\n");
			Status = XST_FAILURE;
			goto done;
		}
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

static void XPmPower_UpdateResetFlags(XPm_PowerDomain *PwrDomain,
				      enum XPmInitFunctions FuncId)
{
	const XPm_ResetNode *Reset;
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
		if ((NULL != PwrDomain->DomainOps) &&
		    (PwrDomain->InitFlag == PwrDomain->DomainOps->InitMask)) {
			PmRmw32(XPM_DOMAIN_INIT_STATUS_REG, DomainStatusMask,
				DomainStatusMask);
		}
	} else if (FUNC_INIT_START == FuncId) {
		/*
		 * Reset flags to indicate Ops are yet to be performed.
		 */
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

static u32 XPmPowerDomain_SkipOp(const XPm_PowerDomain *PwrDomain,
				 u32 Function)
{
	u16 Skip;

	switch (Function) {
	case (u32)FUNC_INIT_START:
	case (u32)FUNC_INIT_FINISH:
	case (u32)FUNC_SCAN_CLEAR:
	case (u32)FUNC_BISR:
	case (u32)FUNC_LBIST:
	case (u32)FUNC_MEM_INIT:
	case (u32)FUNC_MBIST_CLEAR:
	case (u32)FUNC_HOUSECLEAN_PL:
	case (u32)FUNC_HOUSECLEAN_COMPLETE:
		/* Skip if it has been executed before */
		Skip = (PwrDomain->InitFlag >> Function) & 1U;
		break;
	default:
		/* Do not skip by default */
		Skip = 0U;
		break;
	}

	return Skip;
}

XStatus XPmPowerDomain_SecureEfuseTransfer(const u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	volatile u32 RegVal;

	const XPm_Device *AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	if (NULL == AmsRoot) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock configuration and system registers for write operation */
	XPm_UnlockPcsr(AmsRoot->Node.BaseAddress);

	if ((PM_POWER_PLD == NodeId) || (PM_POWER_NOC == NodeId)) {
		/* Enable the SSC interface to PL satellite */
		PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
			AMS_ROOT_TOKEN_MNGR_BYPASS_PL_MASK, 0U);
		PmChkRegMask32(AmsRoot->Node.BaseAddress +
			       AMS_ROOT_TOKEN_MNGR_OFFSET,
			       AMS_ROOT_TOKEN_MNGR_BYPASS_PL_MASK,
			       0U, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_DISABLE_PL_SSC_INTERFACE;
			goto lock;
		}
	}

	/* Assert SECURE_EFUSE_START */
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_MASK_OFFSET,
		AMS_ROOT_SECURE_EFUSE_START_MASK,
		AMS_ROOT_SECURE_EFUSE_START_MASK);
	PmChkRegMask32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_MASK_OFFSET,
		       AMS_ROOT_SECURE_EFUSE_START_MASK,
		       AMS_ROOT_SECURE_EFUSE_START_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_SECURE_EFUSE_START_PCSR_MASK;
		goto lock;
	}

	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_CONTROL_OFFSET,
		AMS_ROOT_SECURE_EFUSE_START_MASK,
		AMS_ROOT_SECURE_EFUSE_START_MASK);

	/* Wait for SECURE_EFUSE_DONE to assert */
	Status = XPm_PollForMask(AmsRoot->Node.BaseAddress +
				 AMS_ROOT_REG_ISR_OFFSET,
				 AMS_ROOT_REG_ISR_SECURE_EFUSE_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SECURE_EFUSE_DONE_TIMEOUT;
		goto deassert_start_bit;
	}

	/* Clear interrupt status for SECURE_EFUSE_DONE */
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_ISR_OFFSET,
		AMS_ROOT_REG_ISR_SECURE_EFUSE_DONE_MASK,
		AMS_ROOT_REG_ISR_SECURE_EFUSE_DONE_MASK);
	PmChkRegMask32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_ISR_OFFSET,
		       AMS_ROOT_REG_ISR_SECURE_EFUSE_DONE_MASK, 0U, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_CLEAR_SECURE_EFUSE_DONE_ISR;
		goto deassert_start_bit;
	}

	/* Check for SECURE_EFUSE_ERR  */
	RegVal = XPm_In32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_ISR_OFFSET);
	if (AMS_ROOT_REG_ISR_SECURE_EFUSE_ERR_MASK == (RegVal &
	    AMS_ROOT_REG_ISR_SECURE_EFUSE_ERR_MASK)) {
		Status = XST_FAILURE;
		DbgErr = XPM_INT_ERR_SECURE_EFUSE_ERR;
	}

deassert_start_bit:
	/* De-assert SECURE_EFUSE_START */
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_MASK_OFFSET,
		AMS_ROOT_SECURE_EFUSE_START_MASK,
		AMS_ROOT_SECURE_EFUSE_START_MASK);
	PmChkRegMask32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_MASK_OFFSET,
		       AMS_ROOT_SECURE_EFUSE_START_MASK,
		       AMS_ROOT_SECURE_EFUSE_START_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_DE_ASSERT_SECURE_EFUSE_START_PCSR_MASK;
		goto lock;
	}

	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_REG_PCSR_CONTROL_OFFSET,
		AMS_ROOT_SECURE_EFUSE_START_MASK, 0U);

lock:
	/* Lock configuration and system registers */
	XPm_LockPcsr(AmsRoot->Node.BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);

	return Status;
}

XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  const u32 *Args, u32 NumArgs)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	const struct XPm_PowerDomainOps *Ops = PwrDomain->DomainOps;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const u32 PldPwrNodeDependency[NUM_PLD0_PWR_DOMAIN_DEPENDENCY] = {PM_POWER_PLD};
	const XPm_PlDevice *PlDevice;

	PmDbg("%s for PwrDomain 0x%x Start\r\n", PmInitFunctions[Function],
						  PwrDomain->Power.Node.Id);

	/*
	 * Skip running a domain operation in either case:
	 *   - If the domain is already powered on
	 *   - If the operation has been executed before
	 *   - AND NOT power node is PMC
	 * We still want to run MIO FLUSH in the case of power domain is on like PMC domain.
	 */
	if (((PM_POWER_PMC != PwrDomain->Power.Node.Id) && ((u8)XPM_POWER_STATE_ON == PwrDomain->Power.Node.State)) ||
	    (0U != XPmPowerDomain_SkipOp(PwrDomain, Function))) {
		PmInfo("Skipping %s for 0x%x\r\n",
				PmInitFunctions[Function],
				PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	switch (Function) {
	case (u32)FUNC_INIT_START:
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_INITIALIZING;
		Status = XPmPower_UpdateRailStats(PwrDomain,
						  (u8)XPM_POWER_STATE_ON);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PWR_DOMAIN_RAIL_CONTROL;
			goto done;
		}

		XPmPower_UpdateResetFlags(PwrDomain, FUNC_INIT_START);
		if ((NULL != Ops) && (NULL != Ops->InitStart)) {
			Status = Ops->InitStart(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_START;
				goto done;
			}
			PwrDomain->InitFlag |= BIT16(FUNC_INIT_START);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_INIT_FINISH:
#ifdef PLM_ENABLE_STL
		(void)XStl_PlmStartupPreCdoTask(PwrDomain->Power.Node.Id);
#endif
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			Status = XST_FAILURE;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->InitFinish)) {
			Status = Ops->InitFinish(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
				goto done;
			}
			PwrDomain->InitFlag |= BIT16(FUNC_INIT_FINISH);
		}
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_ON;
		XPmNotifier_Event(PwrDomain->Power.Node.Id, (u32)EVENT_STATE_CHANGE);
		/*
		 * Note: Fallback mechanism for PLD topology. In case PL topology
		 * is not enabled in vivado, run pld0 init node commands after plpd
		 * init node finish. Determine this by checking the state of pld0,
		 * wf/powerbitmask values. If state is unused, wf/powerbitmask are 0U,
		 * it means PL topology is not active
		 */
		if (PM_POWER_PLD == PwrDomain->Power.Node.Id) {
			PlDevice = (XPm_PlDevice *)XPmDevice_GetById(PM_DEV_PLD_0);
			if (NULL == PlDevice) {
				DbgErr = XST_DEVICE_NOT_FOUND;
				Status = XST_FAILURE;
				goto done;
			}
			if (((u8)XPM_DEVSTATE_UNUSED == PlDevice->Device.Node.State) &&
				(PWR_DOMAIN_UNUSED_BITMASK == PlDevice->WfPowerBitMask) &&
				(PWR_DOMAIN_UNUSED_BITMASK == PlDevice->PowerBitMask)) {
				Status = XPm_InitNode(PM_DEV_PLD_0, (u32)FUNC_INIT_START,
					  PldPwrNodeDependency, NUM_PLD0_PWR_DOMAIN_DEPENDENCY);
				if (XST_SUCCESS != Status) {
					DbgErr = XPM_INT_ERR_PLDEVICE_INITNODE;
					break;
				}
				Status = XPm_InitNode(PM_DEV_PLD_0, (u32)FUNC_INIT_FINISH,
					  PldPwrNodeDependency, NUM_PLD0_PWR_DOMAIN_DEPENDENCY);
				if (XST_SUCCESS != Status) {
					DbgErr = XPM_INT_ERR_PLDEVICE_INITNODE;
					break;
				}
			}
		} else if (PM_POWER_ME == PwrDomain->Power.Node.Id) {
			/* Request AIE device once AIE initialization is done. */
			Status = XPmDevice_Request(PM_SUBSYS_PMC, PM_DEV_AIE,
						   XPM_MAX_CAPABILITY,
						   XPM_MAX_QOS,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_REQ_ME_DEVICE;
				break;
			}
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
		} else if (PM_POWER_LPD == PwrDomain->Power.Node.Id) {
			if (PM_POWER_LPD == RequestDomainId) {
				/* Request LPD GPIO device domainctrl initialization is done.
				 * This is required to keep the LPD power domain up.
				 */
				Status = XPmDevice_Request(PM_SUBSYS_PMC, PM_DEV_GPIO,
							XPM_MAX_CAPABILITY,
							XPM_MAX_QOS,
							XPLMI_CMD_SECURE);
				if (XST_SUCCESS != Status) {
					DbgErr = XPM_INT_ERR_REQ_GPIO;
					break;
				}
			}
#endif
		} else {
			/* Required for MISRA */
		}

		Status = XPmDomainIso_ProcessPending();
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
			Status = Ops->ScanClear(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_SCAN_CLEAR;
				goto done;
			}
			PwrDomain->InitFlag |= BIT16(FUNC_SCAN_CLEAR);
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
			Status = Ops->Bisr(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_BISR;
				goto done;
			}
			PwrDomain->InitFlag |= BIT16(FUNC_BISR);
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
			XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (Ops->Lbist), (PwrDomain), (Args), (NumArgs));
			XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
			/* Required for redundancy */
			if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
				DbgErr = XPM_INT_ERR_FUNC_LBIST;
				goto done;
			}
			PwrDomain->InitFlag |= BIT16(FUNC_LBIST);
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
			XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (Ops->Mbist), (PwrDomain), (Args), (NumArgs));
			XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
			/* Required for redundancy */
			if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
				DbgErr = XPM_INT_ERR_FUNC_MBIST_CLEAR;
				goto done;
			}
			PwrDomain->InitFlag |= BIT16(FUNC_MBIST_CLEAR);
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
			Status = Ops->PlHouseclean(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_HOUSECLEAN_PL;
				goto done;
			}
			PwrDomain->InitFlag |= BIT16(FUNC_HOUSECLEAN_PL);
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
                        Status = Ops->MemInit(PwrDomain, Args, NumArgs);
                        if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_MEM_INIT;
                                goto done;
                        }
			PwrDomain->InitFlag |= BIT16(FUNC_MEM_INIT);
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
                        Status = Ops->HcComplete(PwrDomain, Args, NumArgs);
                        if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_HOUSECLEAN_COMPLETE;
                                goto done;
                        }
			PwrDomain->InitFlag |= BIT16(FUNC_HOUSECLEAN_COMPLETE);
                }
		Status = XST_SUCCESS;
                break;
	case (u32)FUNC_MIO_FLUSH:
		if ((NULL != Ops) && (NULL != Ops->MioFlush)) {
			Status = Ops->MioFlush(PwrDomain, Args, NumArgs);
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
