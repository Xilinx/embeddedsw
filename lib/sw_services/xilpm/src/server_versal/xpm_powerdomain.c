/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_npdomain.h"
#include "xpm_core.h"
#include "xpm_psm.h"
#include "xpm_pmc.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_gic_proxy.h"
#include "xpm_regs.h"
#include "xpm_board.h"

extern u32 ResetReason;
extern int XLoader_ReloadImage(u32 ImageId);
static u8 SystemResetFlag;
static u8 DomainPORFlag;

XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    struct XPm_PowerDomainOps *Ops)
{
	u32 InitMask = 0;
	XPmPower_Init(&PowerDomain->Power, Id, BaseAddress, Parent);

	PowerDomain->Children = NULL;
	PowerDomain->DomainOps = Ops;

	if (Ops && Ops->ScanClear) {
		InitMask |= BIT(FUNC_SCAN_CLEAR);
	}

	if (Ops && Ops->Bisr) {
		InitMask |= BIT(FUNC_BISR);
	}

	if (Ops && Ops->Mbist) {
		InitMask |= BIT(FUNC_MBIST_CLEAR);
	}

	if (Ops && Ops->Lbist) {
		InitMask |= BIT(FUNC_LBIST);
	}

	PowerDomain->InitMask = InitMask;

	return XST_SUCCESS;
}

#define BITMASK_LOWER_15_BITS 0x7fff
#define BITMASK_UPPER_17_BITS 0xffff8000
#define GET_DELTA_AT_OFFSET(array, x) (0xf & (array[x / 32] >> (x % 32)))

XStatus XPmPowerDomain_ApplyAmsTrim(u32 DestAddress, u32 PowerDomainId, u32 SateliteIdx)
{
	XStatus Status = XST_SUCCESS;
	u32 EfuseCacheBaseAddress, StartbitOffset, RegValue, TrimVal, DeltaVal = 0;
	int i, Arr[8];

	if (0 == DestAddress) {
		Status = XST_FAILURE;
		goto done;
	}

	XPm_Device *EfuseCache = XPmDevice_GetById(XPM_DEVID_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		Status = XST_FAILURE;
		goto done;
	}
	EfuseCacheBaseAddress = EfuseCache->Node.BaseAddress;

	/* Unlock writes */
	PmOut32(DestAddress + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

	/* Copy EFUSE_CACHE.TSENS_INT_OFFSET_5_0 to dest_reg.EFUSE_CONFIG0[5:0] */
	PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_3_OFFSET, RegValue);
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_INT_OFFSET_5_0_MASK) >> EFUSE_CACHE_TRIM_AMS_3_TSENS_INT_OFFSET_5_0_SHIFT;
	PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_OFFSET_MASK, (TrimVal << EFUSE_CONFIG0_OFFSET_SHIFT));
	/* Copy EFUSE_CACHE.TSENS_SLOPE_5_0      to dest_reg.EFUSE_CONFIG0[11:6] */
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_SLOPE_5_0_MASK) >> EFUSE_CACHE_TRIM_AMS_3_TSENS_SLOPE_5_0_SHIFT;
	PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_SLOPE_MASK, (TrimVal << EFUSE_CONFIG0_SLOPE_SHIFT));

	/* Copy EFUSE_CACHE.IXPCM_PROCESS_15_0   to dest_reg.EFUSE_CONFIG0[31:16] */
	PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_11_OFFSET, RegValue);
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_11_IXPCM_PROCESS_15_0_MASK) >> EFUSE_CACHE_TRIM_AMS_11_IXPCM_PROCESS_15_0_SHIFT;
	PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_PROCESS_MASK, (TrimVal << EFUSE_CONFIG0_PROCESS_SHIFT));

	/* Copy EFUSE_CACHE.RES_PROCESS_6_0     to dest_reg.EFUSE_CONFIG1[6:0] */
	PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_11_OFFSET, RegValue);
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_11_RES_PROCESS_0_MASK) >> EFUSE_CACHE_TRIM_AMS_11_RES_PROCESS_0_SHIFT;
	PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_12_OFFSET, RegValue);
	TrimVal |= (((RegValue & EFUSE_CACHE_TRIM_AMS_12_RES_PROCESS_6_1_MASK) >> EFUSE_CACHE_TRIM_AMS_12_RES_PROCESS_6_1_SHIFT) << 1);
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_RESISTOR_MASK, (TrimVal << EFUSE_CONFIG1_RESISTOR_SHIFT));
	/* Copy EFUSE_CACHE.BJT_PROCESS_3_0      to dest_reg.EFUSE_CONFIG1[10:7] */
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_BJT_PROCESS_3_0_MASK) >> EFUSE_CACHE_TRIM_AMS_12_BJT_PROCESS_3_0_SHIFT;
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_BJT_OFFSET_MASK, (TrimVal << EFUSE_CONFIG1_BJT_OFFSET_SHIFT));
	/* Copy EFUSE_CACHE.TSENS_EXT_OFFSET_5_0 to dest_reg.EFUSE_CONFIG1[16:11] */
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_TSENS_EXT_OFFSET_5_0_MASK) >> EFUSE_CACHE_TRIM_AMS_12_TSENS_EXT_OFFSET_5_0_SHIFT;
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_EXT_OFFSET_MASK, (TrimVal << EFUSE_CONFIG1_EXT_OFFSET_SHIFT));
	/* Copy EFUSE_CACHE.SHARED_SPARE_1_0     to dest_reg.EFUSE_CONFIG1[18:17] */
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_1_0_MASK) >> EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_1_0_SHIFT;
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_ANA_SPARE_MASK, (TrimVal << EFUSE_CONFIG1_ANA_SPARE_SHIFT));
	/* Copy EFUSE_CACHE.SHARED_SPARE_14_2    to dest_reg.EFUSE_CONFIG1[31:19] */
	TrimVal = (RegValue & EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_14_2_MASK) >> EFUSE_CACHE_TRIM_AMS_12_SHARED_SPARE_14_2_SHIFT;
	PmRmw32(DestAddress + EFUSE_CONFIG1_OFFSET,  EFUSE_CONFIG1_DIG_SPARE_MASK, (TrimVal << EFUSE_CONFIG1_DIG_SPARE_SHIFT));

	/* Copy 256 bits of TSENS_DELTA value to array */
	PmIn32(EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_3_OFFSET, RegValue);
	/*Store 17 bits from current register */
	Arr[0] = (RegValue & EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_MASK) >> EFUSE_CACHE_TRIM_AMS_3_TSENS_DELTA_16_0_SHIFT;
	for (i=0; i<8; i++) {
		u32 Address = (EfuseCacheBaseAddress + EFUSE_CACHE_TRIM_AMS_4_OFFSET + (i*4));
		PmIn32(Address, RegValue);
		/* current element already have 17 bits stored from prev register,
		store 15 bits from current register to current element */
		Arr[i] |= (RegValue & BITMASK_LOWER_15_BITS) << 17;
		/* store 17 bits from current register to next element */
		if (i != 7)
			Arr[i+1] = (RegValue & BITMASK_UPPER_17_BITS) >> 15;
	}

	switch (NODEINDEX(PowerDomainId)) {
        case XPM_NODEIDX_POWER_PMC:
			if (SateliteIdx == 0) {
				/* Copy EFUSE_CACHE.TSENS_DELTA_3_0 to PMC_SYSMON.SAT0_EFUSE_CONFIG0[15:12] */
				DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 0);
			} else if (SateliteIdx == 1) {
				/* Copy EFUSE_CACHE.TSENS_DELTA_7_4 to PMC_SYSMON.SAT1_EFUSE_CONFIG0[15:12] */
				DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 4);
			}
			break;
        case XPM_NODEIDX_POWER_LPD:
			/* Copy EFUSE_CACHE.TSENS_DELTA_11_8 to LPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
			DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 8);
			break;
        case XPM_NODEIDX_POWER_FPD:
			/* Copy EFUSE_CACHE.TSENS_DELTA_15_12 to FPD_SYSMON_SAT.EFUSE_CONFIG0[15:12] */
			DeltaVal =  GET_DELTA_AT_OFFSET(Arr, 12);
			break;
        case XPM_NODEIDX_POWER_NOC:
			StartbitOffset = 16+(SateliteIdx*4);
			/* Copy EFUSE_CACHE.TSENS_DELTA_STARTBIT_ENDBIT to AMS_SAT_N.EFUSE_CONFIG0[15:12] */
			DeltaVal =  GET_DELTA_AT_OFFSET(Arr, StartbitOffset);
			break;
		default:
			Status = XST_FAILURE;
			goto done;
	}

	if(DeltaVal != 0)
		PmRmw32(DestAddress + EFUSE_CONFIG0_OFFSET,  EFUSE_CONFIG0_DELTA_MASK, (DeltaVal << EFUSE_CONFIG0_DELTA_SHIFT));

	/* Lock writes */
	PmOut32(DestAddress + NPI_PCSR_LOCK_OFFSET, 1);

done:
	return Status;
}

XStatus XPm_PowerUpLPD(XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;

	if (XPM_POWER_STATE_ON == Node->State) {
		goto done;
	} else {
		/* TODO: LPD CDO should be rexecuted. Right now we don't have separate LPD
		 * CDO so calling house cleaning commands here
		 */
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_INIT_START, NULL, 0);
		if (Status != XST_SUCCESS)
			goto done;
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_SCAN_CLEAR, NULL, 0);
		if (Status != XST_SUCCESS)
			goto done;
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_LBIST, NULL, 0);
		if (Status != XST_SUCCESS)
			goto done;

		/*
		 * Release SRST for PS-LPD
		 */
		Status = XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_PS_SRST),
				     PM_RESET_ACTION_RELEASE);

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_BISR, NULL, 0);
		if (Status != XST_SUCCESS)
			goto done;
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_MBIST_CLEAR, NULL, 0);
		if (Status != XST_SUCCESS)
			goto done;

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_INIT_FINISH, NULL, 0);
	}

done:
	return Status;
}

XStatus XPm_PowerDwnLPD()
{
	XStatus Status = XST_SUCCESS;
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(LPD_ID);

	if (NULL == LpDomain) {
		Status = XST_FAILURE;
		goto done;
	}

	XPm_Device *AmsRoot = XPmDevice_GetById(XPM_DEVID_AMS_ROOT);
	if (NULL == AmsRoot) {
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

	/* Isolate PS_PL */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_PL_TEST, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_PL, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PS_CPM domains */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_CPM_DFX, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_CPM, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate LP-SoC */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PS_PMC domains */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_LPD_DFX, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_LPD, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Assert reset for PS SRST */
	Status = XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_PS_SRST),
				     PM_RESET_ACTION_ASSERT);

	/* Assert POR for PS-LPD */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PS_POR),
				     PM_RESET_ACTION_ASSERT);

	/*TODO: Send PMC_I2C command to turn off PS-LPD power rail */

	LpDomain->LpdBisrFlags &= ~(LPD_BISR_DATA_COPIED | LPD_BISR_DONE);

done:
	return Status;
}

XStatus XPm_PowerUpFPD(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/*
	 *  Power up FPD power rail
	 *  Compiler flag CUSTOM_PMBUS must be set
	 */
	Status = XPmBoard_ControlRail(RAIL_POWER_UP, POWER_RAIL_FPD);
	if (XST_SUCCESS != Status) {
		PmErr("Control power rail for FPD failure during power up\r\n");
		goto done;
	}

	if (XPM_POWER_STATE_ON != Node->State) {
		PmInfo("Reloading FPD CDO\r\n");
		Status = XLoader_ReloadImage(Node->Id);
		if (XST_SUCCESS != Status) {
			PmErr("Error while reloading FPD CDO\r\n");
		}

		XPm_GicProxy.Clear();
	}

done:
	return Status;
}

XStatus XPm_PowerDwnFPD(XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;
	XPm_Core *ApuCore = (XPm_Core *)XPmDevice_GetById(XPM_DEVID_ACPU_0);
	XPm_Device *AmsRoot = XPmDevice_GetById(XPM_DEVID_AMS_ROOT);
	if (NULL == AmsRoot) {
		Status = XST_FAILURE;
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
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate FPD-PL */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_PL, TRUE);
	if (Status != XST_SUCCESS)
		goto done;
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_PL_TEST, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmPsm_SendPowerDownReq(Node->BaseAddress);

	/* Assert SRST for FPD */
	Status = XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_FPD),
				     PM_RESET_ACTION_ASSERT);

	/* Assert POR for FPD */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_FPD_POR),
				     PM_RESET_ACTION_ASSERT);

	/*
	 * Power down FPD power rail
	 * Compiler flag CUSTOM_PMBUS must be set
	 */
	Status = XPmBoard_ControlRail(RAIL_POWER_DOWN, POWER_RAIL_FPD);
	if (XST_SUCCESS != Status) {
		PmErr("Control power rail for FPD failure during power down\r\n");
		goto done;
	}

	/* Enable GIC proxy only if resume path is set */
	if ((NULL != ApuCore) &&
	    (XST_SUCCESS == ApuCore->CoreOps->HasResumeAddr(ApuCore))) {
		XPm_GicProxy.Enable();
	}

done:
	return Status;
}

XStatus XPm_PowerUpPLD(XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;

	if (XPM_POWER_STATE_ON == Node->State) {
		goto done;
	} else {
		/* TODO: PLD CDO should be rexecuted. Right now we don't have separate PLD
		 * CDO so calling house cleaning commands here
		 */
		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_INIT_START, NULL, 0);
		if (Status != XST_SUCCESS)
			goto done;

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_HOUSECLEAN_PL, NULL, 0);
		if (Status != XST_SUCCESS)
			goto done;

		Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_INIT_FINISH, NULL, 0);
	}
done:
	return Status;
}

XStatus XPm_PowerDwnPLD()
{
	XStatus Status = XST_SUCCESS;

	/* Isolate PL-NoC */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate FPD-PL */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_PL, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_PL_TEST, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate LPD-PL */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_PL, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_PL_TEST, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PL-PMC */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_PL, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_PL_TEST, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_PL_CFRAME, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCINT_RAM from VCCINT */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCRAM_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCAUX_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCAUX_VCCRAM, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PL_CPM */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR, TRUE);
	if (Status != XST_SUCCESS)
		goto done;
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR, TRUE);
	if (Status != XST_SUCCESS)
		goto done;
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_RST_CPI0, TRUE);
	if (Status != XST_SUCCESS)
		goto done;
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_RST_CPI1, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Assert POR PL */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PL_POR),
				     PM_RESET_ACTION_ASSERT);

	/* TODO: Send PMC_I2C command to turn of PLD power rail */
done:
	return Status;
}

XStatus XPm_PowerUpME(XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;

	(void)Node;

	/* TODO: Reload ME CDO */

	return Status;
}

XStatus XPm_PowerDwnME()
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Isolate ME */

	/* TODO: Assert POR ME */

	/* TODO: Send PMC_I2C command to turn of ME power rail */

	return Status;
}

XStatus XPm_PowerUpCPM(XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;

	(void)Node;

	/* TODO: Reload CPM CDO */

	return Status;
}

XStatus XPm_PowerDwnCPM()
{
	XStatus Status = XST_SUCCESS;

	/* Isolate LPD-CPM */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_CPM, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_CPM_DFX, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PL_CPM */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_PCIEA0_ATTR, TRUE);
	if (Status != XST_SUCCESS)
		goto done;
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_PCIEA1_ATTR, TRUE);
	if (Status != XST_SUCCESS)
		goto done;
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_RST_CPI0, TRUE);
	if (Status != XST_SUCCESS)
		goto done;
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_CPM_RST_CPI1, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Assert POR for CPM */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_CPM_POR),
				     PM_RESET_ACTION_ASSERT);

	/* TODO: Send PMC_I2C command to turn off CPM power rail */

done:
	return Status;
}

XStatus XPm_PowerUpNoC(XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;

        if (XPM_POWER_STATE_ON == Node->State) {
                goto done;
        } else {
                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_INIT_START, NULL, 0);
                if (Status != XST_SUCCESS)
                        goto done;
                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_SCAN_CLEAR, NULL, 0);
                if (Status != XST_SUCCESS)
                        goto done;

                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_BISR, NULL, 0);
                if (Status != XST_SUCCESS)
                        goto done;
                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_MBIST_CLEAR, NULL, 0);
                if (Status != XST_SUCCESS)
                        goto done;

                Status = XPmPowerDomain_InitDomain((XPm_PowerDomain *)Node, FUNC_INIT_FINISH, NULL, 0);
        }

done:
        return Status;
}

XStatus XPm_PowerDwnNoC()
{
	XStatus Status = XST_SUCCESS;
	XPm_Device *AmsRoot = XPmDevice_GetById(XPM_DEVID_AMS_ROOT);
	XPm_NpDomain *NpDomain = (XPm_NpDomain *)XPmPower_GetById(NPD_NODEID);

	if ((NULL == AmsRoot) || (NULL == NpDomain)) {
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
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate LPD-NoC domain */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PL-NoC domain */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCAUX-NoC domain */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCAUX_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCRAM-NoC domain */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCRAM_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PMC-NoC domain */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_SOC, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PMC-NoC NPI domain */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_SOC_NPI, TRUE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Assert POR for NoC */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NOC_POR),
				     PM_RESET_ACTION_ASSERT);

	/* TODO: Send PMC_I2C command to turn off NoC power rail */

	NpDomain->BisrDataCopied = 0;

done:
	return Status;
}

XStatus XPmPower_CheckPower(u32 VoltageRailMask)
{
	XStatus Status = XST_SUCCESS;
	u32 RegVal;
	XPm_Pmc *Pmc;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(XPM_DEVID_PMC);
	if (NULL == Pmc) {
		goto done;
	}

	PmIn32(Pmc->PmcGlobalBaseAddr + PWR_SUPPLY_STATUS_OFFSET, RegVal);
	if((RegVal & VoltageRailMask) != VoltageRailMask) {
		Status = XST_FAILURE;
		goto done;
	}
done:
	return Status;
}

static void XPmPower_UpdateResetFlags(XPm_PowerDomain *PwrDomain, u32 FuncId)
{
	XPm_ResetNode *Reset;
	u32 ResetId;
	u32 PmcSysResetMask = (CRP_RESET_REASON_SLR_SYS_MASK |
			       CRP_RESET_REASON_SW_SYS_MASK |
			       CRP_RESET_REASON_ERR_SYS_MASK |
			       CRP_RESET_REASON_DAP_SYS_MASK);
	u32 DomainStatusMask = 1 << (NODEINDEX(PwrDomain->Power.Node.Id) - 1);

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
		if (0 == (XPm_In32(XPM_DOMAIN_INIT_STATUS_REG) &
			  DomainStatusMask)) {
			goto done;
		}

		switch (NODEINDEX(PwrDomain->Power.Node.Id)) {
			case XPM_NODEIDX_POWER_LPD:
				ResetId = POR_RSTID(XPM_NODEIDX_RST_PS_POR);
				break;
			case XPM_NODEIDX_POWER_FPD:
				ResetId = POR_RSTID(XPM_NODEIDX_RST_FPD_POR);
				break;
			case XPM_NODEIDX_POWER_NOC:
				ResetId = POR_RSTID(XPM_NODEIDX_RST_NOC_POR);
				break;
			case XPM_NODEIDX_POWER_CPM:
				ResetId = POR_RSTID(XPM_NODEIDX_RST_CPM_POR);
				break;
			default:
				ResetId = 0;
		}

		/* Check for POR reset for a domain is occured or not. */
		if (0 != ResetId) {
			Reset = XPmReset_GetById(ResetId);
			if (XPM_RST_STATE_ASSERTED ==
			    Reset->Ops->GetState(Reset)) {
				DomainPORFlag = 1;
				goto done;
			}
		}

		/* Check for system reset is occured or not. */
		if (0 != (ResetReason & PmcSysResetMask)) {
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
	XStatus Status = XST_SUCCESS;
	struct XPm_PowerDomainOps *Ops = PwrDomain->DomainOps;

	if ((XPM_POWER_STATE_ON == PwrDomain->Power.Node.State) && (Function != FUNC_XPPU_CTRL)) {
		goto done;
	}

	switch (Function) {
	case FUNC_INIT_START:
		PwrDomain->Power.Node.State = XPM_POWER_STATE_INITIALIZING;
		XPmPower_UpdateResetFlags(PwrDomain, FUNC_INIT_START);
		if (Ops && Ops->InitStart) {
			Status = Ops->InitStart(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		break;
	case FUNC_INIT_FINISH:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->InitFinish) {
			Status = Ops->InitFinish(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		PwrDomain->Power.Node.State = XPM_POWER_STATE_ON;
		XPmDomainIso_ProcessPending(PwrDomain->Power.Node.Id);
		XPmPower_UpdateResetFlags(PwrDomain, FUNC_INIT_FINISH);
		break;
	case FUNC_SCAN_CLEAR:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset or POR of a domain */
		if ((1 == SystemResetFlag) || (1 == DomainPORFlag)) {
			goto done;
		}
		if (Ops && Ops->ScanClear) {
			Status = Ops->ScanClear(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_SCAN_CLEAR);
		}
		break;
	case FUNC_BISR:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset */
		if ((1 == SystemResetFlag) &&
		    (PwrDomain->Power.Node.Id != NPD_NODEID)) {
			goto done;
		}
		if (Ops && Ops->Bisr) {
			Status = Ops->Bisr(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_BISR);
		}
		break;
	case FUNC_LBIST:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset or POR of a domain */
		if ((1 == SystemResetFlag) || (1 == DomainPORFlag)) {
			goto done;
		}
		if (Ops && Ops->Lbist) {
			Status = Ops->Lbist(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_LBIST);
		}
		break;
	case FUNC_MBIST_CLEAR:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		/* Skip in case of system reset or POR of a domain */
		if ((1 == SystemResetFlag) || (1 == DomainPORFlag)) {
			goto done;
		}
		if (Ops && Ops->Mbist) {
			Status = Ops->Mbist(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			PwrDomain->InitFlag |= BIT(FUNC_MBIST_CLEAR);
		}
		break;
	case FUNC_HOUSECLEAN_PL:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->PlHouseclean) {
			Status = Ops->PlHouseclean(Args, NumArgs);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		break;
	case FUNC_MEM_INIT:
                if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
                        Status = XST_FAILURE;
                        goto done;
                }
                if (Ops && Ops->MemInit) {
                        Status = Ops->MemInit(Args, NumArgs);
                        if (XST_SUCCESS != Status) {
                                goto done;
                        }
                }
                break;
	case FUNC_HOUSECLEAN_COMPLETE:
                if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
                        Status = XST_FAILURE;
                        goto done;
                }
                if (Ops && Ops->HcComplete) {
                        Status = Ops->HcComplete(Args, NumArgs);
                        if (XST_SUCCESS != Status) {
                                goto done;
                        }
                }
                break;
	case FUNC_XPPU_CTRL:
                if (XPM_POWER_STATE_ON != PwrDomain->Power.Node.State) {
                        Status = XST_FAILURE;
                        goto done;
                }
                if (Ops && Ops->XppuCtrl) {
                        Status = Ops->XppuCtrl(Args, NumArgs);
                        if (XST_SUCCESS != Status) {
                                goto done;
                        }
                }
                break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}
