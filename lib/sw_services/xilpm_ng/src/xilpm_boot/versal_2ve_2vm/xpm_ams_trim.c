/******************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_regs.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_node.h"
#include "xpm_ams_trim.h"
#define BITMASK_LOWER_15_BITS			(0x7fffU)
#define BITMASK_UPPER_17_BITS			(0xffff8000U)

/** Getting 4 bits from x offset of given array of 32-bits number */
#define GET_DELTA_AT_OFFSET(array, x)		(TSENS_DELTA_ELEMENT_MASK & (array[(x) / 32U] >> ((x) % 32U)))

#define TSENS_DELTA_BITS_PER_ELEMENT    4U
#define TSENS_DELTA_ELEMENT_MASK        0xFU
#define TSENS_DELTA_ARRAY_LAST_INDEX    7U
#define TSENS_DELTA_PMC_SAT0_OFFSET     0U
#define TSENS_DELTA_PMC_SAT1_OFFSET     4U
#define TSENS_DELTA_FPD_SAT0_OFFSET     8U
#define TSENS_DELTA_LPD_OFFSET          12U
#define TSENS_DELTA_NOC_BASE_OFFSET     16U
#define TSENS_DELTA_CPM5N_OFFSET        240U
#define TSENS_DELTA_FPD_SAT1_OFFSET     244U
#define TSENS_DELTA_FPD_SAT2_OFFSET     248U
#define TSENS_DELTA_FPD_SAT3_OFFSET     252U

#define EFUSE_TSENS_ARRAY_SIZE          8U
#define EFUSE_REGISTER_OFFSET_BYTES     4U
#define EFUSE_LOWER_15_BITS_SHIFT       17U
#define EFUSE_UPPER_17_BITS_SHIFT       15U

#define CACHE_READ_EMPTY                0x5A5A5A5AU
#define CACHE_READ_VALID                0xA5A5A5A5U

static XStatus AmsTrim_CopyCache(u32 EfuseCacheBaseAddress,  u32 PowerDomainId,
					u32 SateliteIdx, u32 *CacheRead, u32 *DeltaVal)
{
	XStatus Status = XST_FAILURE;
	u32 StartbitOffset, RegValue;
	volatile u32 i;
	u32 Arr[EFUSE_TSENS_ARRAY_SIZE] = {0};

	if (CACHE_READ_EMPTY == *CacheRead) {
		/* Copy 256 bits of TSENS_DELTA value to array */
		PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_3_OFFSET, RegValue);
		/*Store 17 bits from current register */
		Arr[0] = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_MASK) >>
			  EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_SHIFT;

		for (i = 0U; i < EFUSE_TSENS_ARRAY_SIZE; i++) {
			u32 Address = (EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_4_OFFSET + (i * EFUSE_REGISTER_OFFSET_BYTES));
			PmIn32(Address, RegValue);
			/* current element already have 17 bits stored from prev register,
			store 15 bits from current register to current element */
			Arr[i] |= (RegValue & BITMASK_LOWER_15_BITS) << EFUSE_LOWER_15_BITS_SHIFT;
			/* store 17 bits from current register to next element */
			if (i != TSENS_DELTA_ARRAY_LAST_INDEX) {
				Arr[i + 1U] = (RegValue & BITMASK_UPPER_17_BITS) >> EFUSE_UPPER_17_BITS_SHIFT;
			}
		}

		/* Verify loop completed */
		if (i != EFUSE_TSENS_ARRAY_SIZE) {
			Status = XST_FAILURE;
			goto done;
		}

		/* Set cache read to avoid multiple reads */
		*CacheRead = CACHE_READ_VALID;
	}

	switch (NODEINDEX(PowerDomainId)) {
	case (u32)XPM_NODEIDX_POWER_PMC:
		if (0U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_3_0 to PMC_SYSMON.SAT0_EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_PMC_SAT0_OFFSET);
		} else if (1U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_7_4 to PMC_SYSMON.SAT1_EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_PMC_SAT1_OFFSET);
		} else {
			/* Required due to MISRA */
			PmDbg("[%d] Invalid SateliteIdx\r\n", __LINE__);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_LPD:
		/* Copy EFUSE_CACHE.TSENS_DELTA_15_12 to LPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
		*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_LPD_OFFSET);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_CPM5N:
		/* Copy EFUSE_CACHE.TSENS_DELTA_243_240 to CPM5N_SYSMON.EFUSE_CONFIG0[15:12] */
		*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_CPM5N_OFFSET);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_FPD:
		if (0U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_11_8 to FPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_FPD_SAT0_OFFSET);
		} else if (1U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_247_244 to FPD_SYSMON_SAT1.EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_FPD_SAT1_OFFSET);
		} else if (2U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_251_248 to FPD_SYSMON_SAT2.EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_FPD_SAT2_OFFSET);
		} else if (3U == SateliteIdx) {
			/* Copy EFUSE_CACHE.TSENS_DELTA_255_252 to FPD_SYSMON_SAT3.EFUSE_CONFIG0[15:12] */
			*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, TSENS_DELTA_FPD_SAT3_OFFSET);
		} else {
			/* Required due to MISRA */
			PmDbg("[%d] Invalid SateliteIdx\r\n", __LINE__);
		}
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_NOC:
		StartbitOffset = TSENS_DELTA_NOC_BASE_OFFSET + (SateliteIdx * TSENS_DELTA_BITS_PER_ELEMENT);
		/* Copy EFUSE_CACHE.TSENS_DELTA_STARTBIT_ENDBIT to AMS_SAT_N.EFUSE_CONFIG0[15:12] */
		*DeltaVal =  GET_DELTA_AT_OFFSET(Arr, StartbitOffset);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

done:
	return Status;
}

XStatus XPm_ApplyAmsTrim(u32 DestAddress, u32 PowerDomainId, u32 SateliteIdx)
{
	XStatus Status = XST_FAILURE;
	XStatus StatusTmp = XST_FAILURE;
	u32 EfuseCacheBaseAddress, RegValue, DeltaVal = 0;
	static u32 OffsetVal,SlopeVal,ProcessVal,ResistorVal,BjtOffsetVal,ExtOffsetVal,AnaSpareVal,DigSpareVal;
	static u32 CacheRead = CACHE_READ_EMPTY;
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

	if (CACHE_READ_EMPTY == CacheRead) {
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

	XSECURE_TEMPORAL_CHECK(fail, Status, AmsTrim_CopyCache,
                       EfuseCacheBaseAddress, PowerDomainId,
                       SateliteIdx, &CacheRead, &DeltaVal);

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
