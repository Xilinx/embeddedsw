/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pldomain.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_regs.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xpm_pmc.h"
#include "xpm_debug.h"
#include "xparameters.h"
#include "sleep.h"
#include "xpm_rail.h"

#define XPM_NODEIDX_DEV_GT_MIN		XPM_NODEIDX_DEV_GT_0
/* Modify value of MAX_DEV_GT if we run out */
#define MAX_DEV_GT		52U

#define PLHCLEAN_EARLY_BOOT 0U
#define PLHCLEAN_INIT_NODE  1U

//If TRIM_CRAM[31:0]=0 (FUSE not programmed). Then set rw_read_voltages to 0.61V + 0.625V
#define CRAM_TRIM_RW_READ_VOLTAGE	0x0600019FU
static XCframe CframeIns={0}; /* CFRAME Driver Instance */
static XCfupmc CfupmcIns={0}; /* CFU Driver Instance */
static volatile u32 PlpdHouseCleanBypass = 0;
static volatile u32 PlpdHouseCleanBypassTmp = 0;
u32 HcleanDone = 0;

static XStatus PldInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	return Status;
}


static XStatus PldGtyMbist(u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((BaseAddress + GTY_PCSR_MASK_OFFSET),
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK,
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_GTY_PCSR_MASK;
		goto done;
	}

	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((BaseAddress + GTY_PCSR_CONTROL_OFFSET),
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK,
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_GTY_MEM_CLEAR_TRIGGER_MASK;
		goto done;
	}
	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET, GTY_PCSR_STATUS_MEM_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
		goto done;
	}
	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET, GTY_PCSR_STATUS_MEM_CLEAR_PASS_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS_TIMEOUT;
		goto done;
	}
	/* Unwrite trigger bits */
	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET, 0);
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static void PldApplyTrim(u32 TrimType)
{
        u32 TrimVal;
	XStatus Status = XST_FAILURE;
        Xuint128 VggTrim={0};
	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Platform;

	if (NULL == EfuseCache) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

        /* Read the corresponding efuse registers for TRIM values */
        switch (TrimType)
        {
                /* Read VGG trim efuse registers */
                case XPM_PL_TRIM_VGG:
                {
			PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CFRM_VGG_0_OFFSET,
			       VggTrim.Word0);
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CFRM_VGG_1_OFFSET,
			       VggTrim.Word1);
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CFRM_VGG_2_OFFSET,
			       VggTrim.Word2);
                        XCframe_VggTrim(&CframeIns, &VggTrim);
			Status = XST_SUCCESS;
                }
                break;
                /* Read CRAM trim efuse registers */
                case XPM_PL_TRIM_CRAM:
                {
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CRAM_OFFSET,
			       TrimVal);

			Platform = XPm_GetPlatform();
			/* if eFUSE is not programmed,
			then set rw_read_voltages to 0.61V + 0.625V by writing */
			if ((TrimVal == 0U) && ((u32)PLATFORM_VERSION_SILICON == Platform)) {
				TrimVal = CRAM_TRIM_RW_READ_VOLTAGE;
			}
                        XCframe_CramTrim(&CframeIns, TrimVal);
			Status = XST_SUCCESS;
                }
                break;
                /* Read BRAM trim efuse registers */
                case XPM_PL_TRIM_BRAM:
                {
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_BRAM_OFFSET,
			       TrimVal);
                        XCframe_BramTrim(&CframeIns, TrimVal);
			Status = XST_SUCCESS;
                }
                break;
                /* Read URAM trim efuse registers */
                case XPM_PL_TRIM_URAM:
                {
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_URAM_OFFSET,
			       TrimVal);
                        XCframe_UramTrim(&CframeIns, TrimVal);
			Status = XST_SUCCESS;
                }
                break;
                default:
                {
			DbgErr = XPM_INT_ERR_INVALID_TRIM_TYPE;
			Status = XST_FAILURE;
                        break;
                }
        }
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return;
}

static void PldCfuLock(XPm_PlDomain *Pld, u32 Enable)
{
	static u32 PrevLockState=1U;

	if (1U == Enable) {
		/* Lock CFU writes */
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_PROTECT_OFFSET, PrevLockState);
	} else {
		PmIn32(Pld->CfuApbBaseAddr + CFU_APB_CFU_PROTECT_OFFSET, PrevLockState);
		/* Unlock CFU writes */
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_PROTECT_OFFSET, 0);
	}

	return;
}

static XStatus PldCfuInit(void)
{
	XStatus Status = XST_FAILURE;
	XCfupmc_Config *Config;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (0U != CfupmcIns.IsReady) {
		DbgErr = XPM_INT_ERR_CFU_NOT_READY;
		Status = XST_SUCCESS;
		goto done;
	}
	/*
	 * Initialize the CFU driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCfupmc_LookupConfig((u16)XPAR_XCFUPMC_0_DEVICE_ID);
	if (NULL == Config) {
		DbgErr = XPM_INT_ERR_DEVICE_LOOKUP;
                Status = XST_FAILURE;
		goto done;
	}

	Status = XCfupmc_CfgInitialize(&CfupmcIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		DbgErr = XPM_INT_ERR_CFG_INIT;
		goto done;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCfupmc_SelfTest(&CfupmcIns);
	if (Status != XST_SUCCESS) {
		DbgErr = XPM_INT_ERR_SELF_TEST;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
static XStatus PldCframeInit(void)
{
        XStatus Status = XST_FAILURE;
        XCframe_Config *Config;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

        if (0U != CframeIns.IsReady) {
                Status = XST_SUCCESS;
		goto done;
        }
        /*
         * Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table,
         * then initialize it.
         */
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_DEVICE_ID);
	if (NULL == Config) {
		DbgErr = XPM_INT_ERR_DEVICE_LOOKUP;
                Status = XST_FAILURE;
                goto done;
        }

        Status = XCframe_CfgInitialize(&CframeIns, Config, Config->BaseAddress);
        if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CFG_INIT;
                goto done;
        }

        /*
         * Performs the self-test to check hardware build.
         */
        Status = XCframe_SelfTest(&CframeIns);
        if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SELF_TEST;
                goto done;
        }

done:
	XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}

static XStatus InitGtyAddrArr(u32 *GtArrPtr, const u32 ArrLen)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Idx = 0U;
	u32 i;
	XPm_Device *Device;

	for (i = (u32)XPM_NODEIDX_DEV_GT_MIN; i < (u32)XPM_NODEIDX_DEV_MAX; ++i) {
		Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) ||
			((u32)XPM_NODETYPE_DEV_GT != NODETYPE(Device->Node.Id))) {
			continue;
		}

		if ((NULL == Device->Power) ||
			((u32)PM_POWER_PLD != Device->Power->Node.Id)) {
			continue;
		}

		if (Idx >= ArrLen) {
			DbgErr = XPM_INT_ERR_GTY_INIT;
			goto done;
		}

		GtArrPtr[Idx] = Device->Node.BaseAddress;
		++Idx;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus GtyHouseClean(void)
{
	volatile XStatus Status = XPM_ERR_HC_PL;
	volatile XStatus StatusTmp = XPM_ERR_HC_PL;
	unsigned int i;
	u32 GtyAddresses[MAX_DEV_GT] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Initialize array with GT addresses */
	Status = InitGtyAddrArr(GtyAddresses, ARRAY_SIZE(GtyAddresses));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_GTY_HC;
		goto done;
	}

	for (i = 0; i < ARRAY_SIZE(GtyAddresses); i++) {
		if (0U == GtyAddresses[i]) {
			continue;
		}
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		/* Deassert INITCTRL */
		PmOut32(GtyAddresses[i] + GTY_PCSR_MASK_OFFSET,
			GTY_PCSR_INITCTRL_MASK);
		PmOut32(GtyAddresses[i] + GTY_PCSR_CONTROL_OFFSET, 0);
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1);
	}

	u32 LocalPlpdHCBypass = PlpdHouseCleanBypassTmp; /* Copy volatile to local to avoid MISRA */
	if ((0U == PlpdHouseCleanBypass) && (0U == LocalPlpdHCBypass)) {
		/* Bisr repair - Bisr should be triggered only for Addresses for which repair
		 * data is found and so not calling in loop. Trigger is handled in below routine
		 * */
		Status = XPmBisr_Repair(GTY_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_GTY_BISR_REPAIR;
			goto done;
		}

		Status = XPmBisr_Repair(GTM_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_GTM_BISR_REPAIR;
			goto done;
		}

		Status = XPmBisr_Repair(GTYP_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_GTYP_BISR_REPAIR;
			goto done;
		}

		for (i = 0; i < ARRAY_SIZE(GtyAddresses); i++) {
			if (0U == GtyAddresses[i]) {
				continue;
			}
			PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET,
				PCSR_UNLOCK_VAL);
			/* Mbist */
			XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (PldGtyMbist), (GtyAddresses[i]));
			XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
			/* Required for redundancy */
			if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
				/* Gt Mem clear is found to be failing on some parts.
				 Just print message and return not to break execution */
				PmInfo("ERROR: GT Mem clear Failed\r\n");
				Status = XST_SUCCESS;
				PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1);
				goto done;
			}
			PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1);
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus PlHouseClean(u32 TriggerTime)
{
	XStatus Status = XST_FAILURE;
	XPm_PlDomain *Pld;
	u32 Value;
	u32 Platform = XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;


	if (PLHCLEAN_EARLY_BOOT == TriggerTime) {
		/* Enable ROWON */
		XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
							XCFRAME_CMD_REG_ROWON);

		/* HCLEANR type 3,4,5,6 */
		XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
							XCFRAME_CMD_REG_HCLEANR);

		/* HB BISR REPAIR */
		Status = XPmBisr_Repair(DCMAC_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DCMAC_BISR_REPAIR;
			goto done;
		}
		Status = XPmBisr_Repair(ILKN_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_ILKN_BISR_REPAIR;
			goto done;
		}
		Status = XPmBisr_Repair(MRMAC_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MRMAC_BISR_REPAIR;
			goto done;
		}
		Status = XPmBisr_Repair(SDFEC_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SDFEC_BISR_REPAIR;
			goto done;
		}

		/* BRAM/URAM TRIM */
		PldApplyTrim(XPM_PL_TRIM_BRAM);
		PldApplyTrim(XPM_PL_TRIM_URAM);

		/* BRAM/URAM repair */
		Status = XPmBisr_Repair(BRAM_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BRAM_BISR_REPAIR;
			goto done;
		}
		Status = XPmBisr_Repair(URAM_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_URAM_BISR_REPAIR;
			goto done;
		}

		/* HCLEAN type 0,1,2 */
		XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
								XCFRAME_CMD_REG_HCLEAN);
	} else {

		Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
		if (NULL == Pld) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
			goto done;
		}

		/* Poll for house clean completion */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Waiitng for PL HC complete....", __func__);
		while ((XPm_In32(Pld->CfuApbBaseAddr + CFU_APB_CFU_STATUS_OFFSET) &
			(u32)CFU_APB_CFU_STATUS_HC_COMPLETE_MASK) !=
					(u32)CFU_APB_CFU_STATUS_HC_COMPLETE_MASK) {};
		XPlmi_Printf(DEBUG_INFO, "Done\r\n");

		/* VGG TRIM */
		PldApplyTrim(XPM_PL_TRIM_VGG);

		/* CRAM TRIM */
		PldApplyTrim(XPM_PL_TRIM_CRAM);

		if (PLATFORM_VERSION_SILICON != Platform) {
			Status = XST_SUCCESS;
			goto done;
		}

		/* LAGUNA REPAIR - not needed for now */

		/* There is no status for Bisr done in hard ip. But we must ensure
		 * BISR is complete before scan clear */
		 /*TBD - Wait for how long?? Wei to confirm with DFT guys */

		/* Fake read */
		/* each register is 128 bits long so issue 4 reads */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : CFRAME Fake Read...", __func__);
		PmIn32(Pld->Cframe0RegBaseAddr + 0U, Value);
		PmIn32(Pld->Cframe0RegBaseAddr + 4U, Value);
		PmIn32(Pld->Cframe0RegBaseAddr + 8U, Value);
		PmIn32(Pld->Cframe0RegBaseAddr + 12U, Value);
		XPlmi_Printf(DEBUG_INFO, "Done\r\n");

		/* PL scan clear / MBIST */
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET,
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
		/* Check that the register value written properly or not! */
		PmChkRegRmw32((Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET),
				CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK,
				CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_REG_WRT_PLHOUSECLN_CFU_MASK;
			goto done;
		}

		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET,
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
		/* Check that the register value written properly or not! */
		PmChkRegRmw32((Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET),
				CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK,
				CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_REG_WRT_PLHOUSECLN_CFU_FGCR;
			goto done;
		}

		/* Poll for status */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for Hard Block Scan Clear / MBIST complete...", __func__);
		Status = XPm_PollForMask(Pld->CfuApbBaseAddr + CFU_APB_CFU_STATUS_OFFSET,
				 CFU_APB_CFU_STATUS_SCAN_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			/** HACK: Continuing even if CFI SC is not completed for ES1 */
			if ((PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
				goto done;
			}
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "Done\r\n");
		}
		/* Check if Scan Clear Passed */
		if ((XPm_In32(Pld->CfuApbBaseAddr + CFU_APB_CFU_STATUS_OFFSET) &
			(u32)CFU_APB_CFU_STATUS_SCAN_CLEAR_PASS_MASK) !=
			(u32)CFU_APB_CFU_STATUS_SCAN_CLEAR_PASS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: Hard Block Scan Clear / MBIST FAILED\r\n", __func__);
			/** HACK: Continuing even if CFI SC is not pass for ES1 */
			if ((PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_SCAN_PASS_TIMEOUT;
				goto done;
			}
		}

		/* Unwrite trigger bits for PL scan clear / MBIST */
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET,
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET, 0);

#ifdef PLM_PRINT_PERF_PL
		XPlmi_Printf(DEBUG_GENERAL, "PL House Clean completed\n\r");
#endif
	}

	/* Compilation warning fix */
	(void)Value;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus PldInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XStatus IntRailPwrSts = XST_FAILURE;
	XStatus RamRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	XStatus SocRailPwrSts = XST_FAILURE;
	XPm_PlDomain *Pld;
	u32 PlPowerUpTime=0;
	u32 Platform = XPm_GetPlatform();
	u32 IdCode = XPm_GetIdCode();
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT);
	XPm_Rail *VccRamRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCC_RAM);
	XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);
	XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCC_SOC);
	XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	/* If PL power is still not up, return error as PLD cant
	   be initialized */
	if (1U != HcleanDone) {
		while ((XST_SUCCESS != IntRailPwrSts) ||
		       (XST_SUCCESS != RamRailPwrSts) ||
		       (XST_SUCCESS != AuxRailPwrSts)) {
			IntRailPwrSts = XPmPower_CheckPower(VccintRail,
					PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
			RamRailPwrSts =  XPmPower_CheckPower(VccRamRail,
					PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
			AuxRailPwrSts =  XPmPower_CheckPower(VccauxRail,
					PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);

			/** Wait for PL power up */
			usleep(10);
			PlPowerUpTime++;
			if (PlPowerUpTime > XPM_POLL_TIMEOUT)
			{
				XPlmi_Printf(DEBUG_GENERAL, "ERROR: PL Power Up TimeOut\n\r");
				DbgErr = XPM_INT_ERR_POWER_SUPPLY;
				Status = XST_FAILURE;
				/* TODO: Request PMC to power up all required rails and wait for the acknowledgement.*/
				goto done;
			}
		}

		/* Skip PL release delay if using sysmon */
		if ((NULL != VccintRail) && (XPM_PGOOD_SYSMON == VccintRail->Source)) {
			PmOut32(Pmc->PmcAnalogBaseAddr + PMC_ANLG_CFG_POR_CNT_SKIP_OFFSET, 1U);
		} else {
			/* Delay is required to stabilize the voltage rails */
			usleep(250);
		}

		Status = XPmPlDomain_InitandHouseclean();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DOMAIN_INIT_AND_HC;
			goto done;
		}
	}

	/*
	 * NOTE:
	 *   Refer CR-1072789 and  EDT-1007075
	 * VNPI output reset to VCCINT connected slaves is clamped at the wrong value.
	 * To work around this in XCVC1902, NPI_RESET should be toggled after VCCINT power up.
	 */
	if (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {
		Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_PULSE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_NPI;
			Status = XPM_ERR_RESET;
			goto done;
		}
	}

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}

	/* Remove PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}
	 /* Remove PMC-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_NPI_ISO;
		goto done;
	}

	/* Unlock CFU writes */
	PldCfuLock(Pld, 0U);

	u32 LocalPlpdHCBypass = PlpdHouseCleanBypassTmp; /* Copy volatile to local to avoid MISRA */
	if ((0U == PlpdHouseCleanBypass) && (0U == LocalPlpdHCBypass)) {
		Status = PlHouseClean(PLHCLEAN_INIT_NODE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PL_HC;
			goto done;
		}
	}

	Status = PldCfuInit();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CFU_INIT;
		goto done;
	}

	if ((PLATFORM_VERSION_SILICON == Platform) || (PLATFORM_VERSION_FCV == Platform)) {
		/*House clean GTY*/
		Status = GtyHouseClean();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_GTY_HC;
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s : GTY HC failed", __func__);
		}
	}


	/* Set init_complete */
	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET,
		CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);
	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET,
		CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);

	/* Enable the global signals */
	XCfupmc_SetGlblSigEn(&CfupmcIns, (u8 )TRUE);

	RamRailPwrSts = XPmPower_CheckPower(VccRamRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
	AuxRailPwrSts = XPmPower_CheckPower(VccauxRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);

	if ((XST_SUCCESS == RamRailPwrSts) &&
	    (XST_SUCCESS == AuxRailPwrSts)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_VCCAUX_VCCRAM_ISO;
			goto done;
		}
	}

	RamRailPwrSts = XPmPower_CheckPower(VccRamRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
	SocRailPwrSts =  XPmPower_CheckPower(VccSocRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);
	if ((XST_SUCCESS == RamRailPwrSts) && (XST_SUCCESS == SocRailPwrSts)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_VCCRAM_SOC_ISO;
			goto done;
		}
	} else {
		Status = XST_FAILURE;
	}

	XCfupmc_GlblSeqInit(&CfupmcIns);

	/* Lock CFU writes */
	PldCfuLock(Pld, 1U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
* @brief This function initializes and performs housecleaning for PL domain
*
* @param       None
*
* @return      XST_FAILURE if error / XST_SUCCESS if success
*
*****************************************************************************/

XStatus XPmPlDomain_InitandHouseclean(void)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	XStatus IntRailPwrSts = XST_FAILURE;
	XStatus RamRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	volatile u32 PlatformType = 0xFFU;
	volatile u32 PlatformTypeTmp = 0xFFU;
	u32 PlatformVersion;
	XPm_Pmc *Pmc;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT);
	XPm_Rail *VccRamRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCC_RAM);
	XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);

	/* Skip if already done */
	if (0U != HcleanDone) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Proceed only if vccint, vccaux, vccint_ram is 1 */
	IntRailPwrSts = XPmPower_CheckPower(VccintRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
	RamRailPwrSts = XPmPower_CheckPower(VccRamRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
	AuxRailPwrSts = XPmPower_CheckPower(VccauxRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
	if ((XST_SUCCESS != IntRailPwrSts) ||
	    (XST_SUCCESS != RamRailPwrSts) ||
	    (XST_SUCCESS != AuxRailPwrSts)) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		Status = XST_FAILURE;
		goto done;
	}

#ifdef PLM_PRINT_PERF_PL
	XPlmi_Printf(DEBUG_GENERAL, "PL supply status good\n\r");
#endif

	/* Remove POR for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	PlatformVersion = XPm_GetPlatformVersion();
	PlatformType = XPm_GetPlatform();
	/* Required for redundancy */
	PlatformTypeTmp = XPm_GetPlatform();
	u32 LocalPlatformType = PlatformTypeTmp; /* Copy volatile to local to avoid MISRA */

	/* Check if housecleaning needs to be bypassed */
	if ((PLATFORM_VERSION_FCV == PlatformType) ||
	    (PLATFORM_VERSION_FCV == LocalPlatformType)) {
		PlpdHouseCleanBypass = 1;
		PlpdHouseCleanBypassTmp = 1;
	}

	/* Check for PL POR Status */
	Status = XPm_PollForMask(Pmc->PmcGlobalBaseAddr +
				 PMC_GLOBAL_PL_STATUS_OFFSET,
				 PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK,
				 XPM_POLL_TIMEOUT);
	if(XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_POR_STATUS;
		goto done;
	}

	if ((PLATFORM_VERSION_SILICON == PlatformType) &&
	    (PLATFORM_VERSION_SILICON == LocalPlatformType) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/*
		 * EDT-995767: Theres a bug with ES1, due to which a small
		 * percent (<2%) of device may miss pl_por_b during power,
		 * which could result CFRAME wait up in wrong state. The work
		 * around requires to toggle PL_POR twice after PL supplies is
		 * up.
		 */

		// Disable PUDC_B pin to allow PL_POR to toggle
		XPm_RMW32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PUDC_B_OVERRIDE_OFFSET,
				PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK,
				PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK);

		/* Toggle PL POR */
		Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_PULSE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PL_POR;
			goto done;
		}

		// Reset to allow PUDC_B pin to function
		XPm_RMW32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PUDC_B_OVERRIDE_OFFSET,
				PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK,
				~PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK);

		/*
		 * Clear sticky ERROR and interrupt status (They are not
		 * cleared by PL_POR). Otherwise, once ERROR/interrupt is
		 * enabled by PLM, PLM may behave incorrectly.
		 */
		XPm_Write32(Pmc->PmcGlobalBaseAddr +
				PMC_GLOBAL_GIC_PROXY_BASE_OFFSET +
				GIC_PROXY_GROUP_OFFSET(3U) +
				GIC_PROXY_IRQ_STATUS_OFFSET,
				GICP3_CFRAME_SEU_MASK | GICP3_CFU_MASK);
		XPm_Write32(Pmc->PmcGlobalBaseAddr +
				PMC_GLOBAL_ERR1_STATUS_OFFSET,
				PMC_GLOBAL_ERR1_STATUS_CFU_MASK |
				PMC_GLOBAL_ERR1_STATUS_CFRAME_MASK);
		XPm_Write32(Pmc->PmcGlobalBaseAddr +
				PMC_GLOBAL_ERR2_STATUS_OFFSET,
				PMC_GLOBAL_ERR2_STATUS_CFI_MASK |
				PMC_GLOBAL_ERR2_STATUS_CFRAME_SEU_CRC_MASK |
				PMC_GLOBAL_ERR2_STATUS_CFRAME_SEU_ECC_MASK);
	}

	/* Check for PL POR Status */
	/* This check is repeated due to ES1 workaround where PL POR is toggled again */
	Status = XPm_PollForMask(Pmc->PmcGlobalBaseAddr +
				 PMC_GLOBAL_PL_STATUS_OFFSET,
				 PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK,
				 XPM_POLL_TIMEOUT);
	if(XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_STATUS_TIMEOUT;
		goto done;
	}

#ifdef PLM_PRINT_PERF_PL
	XPlmi_Printf(DEBUG_GENERAL, "PL POR B status good\n\r");
#endif

	/* Remove SRST for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_SRST, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SRST;
		goto done;
	}

	Status = PldCframeInit();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CFRAME_INIT;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_CFRAME,
					  FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_CFRAME_ISO;
		goto done;
	}

	u32 LocalPlpdHCBypass = PlpdHouseCleanBypassTmp; /* Copy volatile to local to avoid MISRA */
	if ((0U == PlpdHouseCleanBypass) && (0U == LocalPlpdHCBypass)) {
		XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (PlHouseClean), (PLHCLEAN_EARLY_BOOT));
		/* Required for redundancy */
		XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
			DbgErr = XPM_INT_ERR_PL_HC;
			goto done;
		}
	}

	/* Set the flag */
	HcleanDone = 1;

done:
	if (XPM_INT_ERR_POWER_SUPPLY != DbgErr) {
		XPm_PrintDbgErr(Status, DbgErr);
	}

	return Status;
}

static struct XPm_PowerDomainOps PldOps = {
	.InitStart = PldInitStart,
	.InitFinish = PldInitFinish,
	.PlHouseclean = NULL,
};

XStatus XPmPlDomain_Init(XPm_PlDomain *PlDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent, u32 *OtherBaseAddresses,
			 u32 OtherBaseAddressCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PlDomain->Domain, Id, BaseAddress, Parent, &PldOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (2U <= OtherBaseAddressCnt) {
		PlDomain->CfuApbBaseAddr = OtherBaseAddresses[0];
		PlDomain->Cframe0RegBaseAddr = OtherBaseAddresses[1];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPlDomain_RetriggerPlHouseClean(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_PlDomain *Pld;

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}
	HcleanDone = 0U;

	Status = XPmPlDomain_InitandHouseclean();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Unlock CFU writes */
        PldCfuLock(Pld, 0U);

	Status = PlHouseClean(PLHCLEAN_INIT_NODE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Lock CFU writes */
        PldCfuLock(Pld, 1U);
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
