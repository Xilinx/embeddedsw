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
#include "xparameters.h"
#include "sleep.h"

#define XPM_NODEIDX_DEV_GT_MIN		XPM_NODEIDX_DEV_GT_0
#define XPM_NODEIDX_DEV_GT_MAX		XPM_NODEIDX_DEV_GT_10
#define XPM_NODEIDX_DEV_GTM_MIN		XPM_NODEIDX_DEV_GTM_0
#define XPM_NODEIDX_DEV_GTM_MAX		XPM_NODEIDX_DEV_GTM_4
#define XPM_NODEIDX_DEV_GTYP_MIN		XPM_NODEIDX_DEV_GTYP_0
#define XPM_NODEIDX_DEV_GTYP_MAX		XPM_NODEIDX_DEV_GTYP_2

#define PLHCLEAN_EARLY_BOOT 0U
#define PLHCLEAN_INIT_NODE  1U

//If TRIM_CRAM[31:0]=0 (FUSE not programmed). Then set rw_read_voltages to 0.61V + 0.625V
#define CRAM_TRIM_RW_READ_VOLTAGE	0x0600019FU
static XCframe CframeIns={0}; /* CFRAME Driver Instance */
static XCfupmc CfupmcIns={0}; /* CFU Driver Instance */
static u32 PlpdHouseCleanBypass = 0;
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

	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET, GTY_PCSR_STATUS_MEM_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET, GTY_PCSR_STATUS_MEM_CLEAR_PASS_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* Unwrite trigger bits */
	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET, 0);
done:
	return Status;
}

static void PldApplyTrim(u32 TrimType)
{
        u32 TrimVal;
        Xuint128 VggTrim={0};
	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);

	if (NULL == EfuseCache) {
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
                }
                break;
                /* Read CRAM trim efuse registers */
                case XPM_PL_TRIM_CRAM:
                {
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CRAM_OFFSET,
			       TrimVal);
			/* if eFUSE is not programmed,
			then set rw_read_voltages to 0.61V + 0.625V by writing */
			if ((TrimVal == 0U) && (PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
				TrimVal = CRAM_TRIM_RW_READ_VOLTAGE;
			}
                        XCframe_CramTrim(&CframeIns, TrimVal);
                }
                break;
                /* Read BRAM trim efuse registers */
                case XPM_PL_TRIM_BRAM:
                {
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_BRAM_OFFSET,
			       TrimVal);
                        XCframe_BramTrim(&CframeIns, TrimVal);
                }
                break;
                /* Read URAM trim efuse registers */
                case XPM_PL_TRIM_URAM:
                {
                        PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_URAM_OFFSET,
			       TrimVal);
                        XCframe_UramTrim(&CframeIns, TrimVal);
                }
                break;
                default:
                {
			/* Added due to MISRA */
			PmDbg("[%d] Default case in switch\r\n", __LINE__);
                        break;
                }
        }

done:
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

	if (0U != CfupmcIns.IsReady) {
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
                Status = XST_FAILURE;
		goto done;
	}

	Status = XCfupmc_CfgInitialize(&CfupmcIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCfupmc_SelfTest(&CfupmcIns);
	if (Status != XST_SUCCESS) {
		goto done;
	}

done:
	return Status;
}
static XStatus PldCframeInit(void)
{
        XStatus Status = XST_FAILURE;
        XCframe_Config *Config;

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
                Status = XST_FAILURE;
                goto done;
        }

        Status = XCframe_CfgInitialize(&CframeIns, Config, Config->BaseAddress);
        if (XST_SUCCESS != Status) {
                goto done;
        }

        /*
         * Performs the self-test to check hardware build.
         */
        Status = XCframe_SelfTest(&CframeIns);
        if (XST_SUCCESS != Status) {
                goto done;
        }

done:
        return Status;
}

static XStatus GtyHouseClean(void)
{
	XStatus Status = XPM_ERR_HC_PL;
	unsigned int i;
	XPm_Device *Device;
	u32 GtyAddresses[XPM_NODEIDX_DEV_GT_MAX - XPM_NODEIDX_DEV_GT_MIN + 1 +
			XPM_NODEIDX_DEV_GTM_MAX - XPM_NODEIDX_DEV_GTM_MIN + 1 +
			XPM_NODEIDX_DEV_GTYP_MAX - XPM_NODEIDX_DEV_GTYP_MIN + 1] = {0};
	u32 GtySize = (u32)XPM_NODEIDX_DEV_GT_MAX - (u32)XPM_NODEIDX_DEV_GT_MIN + 1U;
	u32 GtmSize = (u32)XPM_NODEIDX_DEV_GTM_MAX - (u32)XPM_NODEIDX_DEV_GTM_MIN + 1U;
	u32 GtypSize = (u32)XPM_NODEIDX_DEV_GTYP_MAX - (u32)XPM_NODEIDX_DEV_GTYP_MIN + 1U;

	/* Store GTY Addresses */
	for (i = 0; i < GtySize; i++) {
		Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_GT_MIN + i);
		if (NULL != Device) {
			GtyAddresses[i] = Device->Node.BaseAddress;
		}
	}

	/* Store GTM Addresses */
	for (i = 0; i < GtmSize; i++) {
		Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_GTM_MIN + i);
		if (NULL != Device) {
			GtyAddresses[i + GtySize] = Device->Node.BaseAddress;
		}
	}

	/* Store GTYP Addresses */
	for (i = 0; i <  GtypSize; i++) {
		Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_GTYP_MIN + i);
		if (NULL != Device) {
			GtyAddresses[i + GtySize + GtmSize] = Device->Node.BaseAddress;
		}
	}

	for (i = 0; i < ARRAY_SIZE(GtyAddresses) && (0U != GtyAddresses[i]); i++) {
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		/* Deassert INITCTRL */
		PmOut32(GtyAddresses[i] + GTY_PCSR_MASK_OFFSET,
			GTY_PCSR_INITCTRL_MASK);
		PmOut32(GtyAddresses[i] + GTY_PCSR_CONTROL_OFFSET, 0);
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1);
	}
	if (0U == PlpdHouseCleanBypass) {
		/* Bisr repair - Bisr should be triggered only for Addresses for which repair
		 * data is found and so not calling in loop. Trigger is handled in below routine
		 * */
		Status = XPmBisr_Repair(GTY_TAG_ID);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPmBisr_Repair(GTM_TAG_ID);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPmBisr_Repair(GTYP_TAG_ID);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		for (i = 0; i < ARRAY_SIZE(GtyAddresses) && (0U != GtyAddresses[i]); i++) {
			PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET,
				PCSR_UNLOCK_VAL);
			/* Mbist */
			Status = PldGtyMbist(GtyAddresses[i]);
			if (XST_SUCCESS != Status) {
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
	return Status;
}

static XStatus PlHouseClean(u32 TriggerTime)
{
	XStatus Status = XST_FAILURE;
	XPm_PlDomain *Pld;
	u32 Value;

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
			goto done;
		}
		Status = XPmBisr_Repair(ILKN_TAG_ID);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPmBisr_Repair(MRMAC_TAG_ID);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPmBisr_Repair(SDFEC_TAG_ID);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* BRAM/URAM repair */
		Status = XPmBisr_Repair(BRAM_TAG_ID);
		if (XST_SUCCESS != Status) {
					goto done;
			}
		Status = XPmBisr_Repair(URAM_TAG_ID);
		if (XST_SUCCESS != Status) {
					goto done;
			}

		/* HCLEAN type 0,1,2 */
		XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
								XCFRAME_CMD_REG_HCLEAN);
	} else {

		Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
		if (NULL == Pld) {
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

		/* BRAM/URAM TRIM */
		PldApplyTrim(XPM_PL_TRIM_BRAM);
		PldApplyTrim(XPM_PL_TRIM_URAM);

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
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET,
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);

		/* Poll for status */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for Hard Block Scan Clear / MBIST complete...", __func__);
		Status = XPm_PollForMask(Pld->CfuApbBaseAddr + CFU_APB_CFU_STATUS_OFFSET,
				 CFU_APB_CFU_STATUS_SCAN_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			/** HACK: Continuing even if CFI SC is not completed for ES1 */
			if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
				Status = XST_SUCCESS;
			} else {
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
			if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
				Status = XST_SUCCESS;
			} else {
				goto done;
			}
		}

		/* Unwrite trigger bits for PL scan clear / MBIST */
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET,
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET, 0);
	}

	/* Compilation warning fix */
	(void)Value;

done:
	return Status;
}

static XStatus PldInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_PlDomain *Pld;
	u32 PlPowerUpTime=0;

	(void)Args;
	(void)NumOfArgs;

	/* If PL power is still not up, return error as PLD cant
	   be initialized */
	if (1U != HcleanDone) {
		/* Check if vccint, vccaux, vccint_ram is 1 */
		while (XST_SUCCESS != XPmPower_CheckPower(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK)) {
			/** Wait for PL power up */
			usleep(10);
			PlPowerUpTime++;
			if (PlPowerUpTime > XPM_POLL_TIMEOUT)
			{
				XPlmi_Printf(DEBUG_GENERAL, "ERROR: PL Power Up TimeOut\n\r");
				Status = XST_FAILURE;
				/* TODO: Request PMC to power up all required rails and wait for the acknowledgement.*/
				goto done;
			}
		}
		/* Add delay to stabilize the voltage rails. Need to check exact amount of time. */
		usleep(250);
		Status = XPmPlDomain_InitandHouseclean();
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Remove PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	 /* Remove PMC-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Unlock CFU writes */
	PldCfuLock(Pld, 0U);

	if(0U == PlpdHouseCleanBypass) {
		Status = PlHouseClean(PLHCLEAN_INIT_NODE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = PldCfuInit();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (PLATFORM_VERSION_SILICON == Platform) {
		/*House clean GTY*/
		Status = GtyHouseClean();
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s : GTY HC failed", __func__);
		}
	}

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Set init_complete */
	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET,
		CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);
	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET,
		CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);

	/* Enable the global signals */
	XCfupmc_SetGlblSigEn(&CfupmcIns, (u8 )TRUE);

	if (XST_SUCCESS == XPmPower_CheckPower(	PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK |
					PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (XST_SUCCESS == XPmPower_CheckPower(	PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	XCfupmc_GlblSeqInit(&CfupmcIns);

	/* Lock CFU writes */
	PldCfuLock(Pld, 1U);

done:
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
	XStatus Status = XST_FAILURE;
	u32 Version;
	XPm_Pmc *Pmc;
	u32 VoltageRailMask = (PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK |
			       PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK |
			       PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);

	/* Skip if already done */
	if (0U != HcleanDone) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Proceed only if vccint, vccaux, vccint_ram is 1 */
	Status = XPmPower_CheckPower(VoltageRailMask);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove POR for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XST_FAILURE;
		goto done;
	}

	PmIn32(PMC_TAP_VERSION, Version);
	PlatformVersion = ((Version & PMC_TAP_VERSION_PLATFORM_VERSION_MASK) >>
			   PMC_TAP_VERSION_PLATFORM_VERSION_SHIFT);
	Platform = ((Version & PMC_TAP_VERSION_PLATFORM_MASK) >>
		    PMC_TAP_VERSION_PLATFORM_SHIFT);

	/* Check if housecleaning needs to be bypassed */
	if (PLATFORM_VERSION_FCV == Platform) {
		PlpdHouseCleanBypass = 1;
	}


	if((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/*
		 * EDT-995767: Theres a bug with ES1, due to which a small
		 * percent (<2%) of device may miss pl_por_b during power,
		 * which could result CFRAME wait up in wrong state. The work
		 * around requires to toggle PL_POR twice after PL supplies is
		 * up.
		 */
		/* Toggle PL POR */
		Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_PULSE);
		if (XST_SUCCESS != Status) {
			goto done;
		}

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

	/* Check for PL PowerUp */
	Status = XPm_PollForMask(Pmc->PmcGlobalBaseAddr +
				 PMC_GLOBAL_PL_STATUS_OFFSET,
				 PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK,
				 XPM_POLL_TIMEOUT);
	if(XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove SRST for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_SRST, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = PldCframeInit();
	if (XST_SUCCESS != Status) {
			goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_CFRAME,
					  FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if(0U == PlpdHouseCleanBypass) {
		Status = PlHouseClean(PLHCLEAN_EARLY_BOOT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the flag */
	HcleanDone = 1;

done:
	return Status;
}

static struct XPm_PowerDomainOps PldOps = {
	.InitStart = PldInitStart,
	.InitFinish = PldInitFinish,
	.PlHouseclean = NULL,
};

static XStatus (*HandlePowerEvent)(XPm_Node *Node, u32 Event);

static XStatus HandlePlDomainEvent(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	PmDbg("State=%d, Event=%d\n\r", Node->State, Event);

	switch (Node->State)
	{
		case (u8)XPM_POWER_STATE_ON:
			if ((u32)XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				Power->UseCount++;
			} else if ((u32)XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				Power->UseCount--;
				Node->State = (u8)XPM_POWER_STATE_OFF;
			} else {
				Status = XST_FAILURE;
			}
			break;
		case (u8)XPM_POWER_STATE_OFF:
			if ((u32)XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				Power->UseCount++;
				Node->State = (u8)XPM_POWER_STATE_ON;
			} else if ((u32)XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				Power->UseCount--;
			} else {
				Status = XST_FAILURE;
			}
			break;
		default:
			PmWarn("Wrong state %d for event %d\n",
			       Node->State, Event);
			break;
	}

	return Status;
}

XStatus XPmPlDomain_Init(XPm_PlDomain *PlDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent, u32 *OtherBaseAddresses,
			 u32 OtherBaseAddressCnt)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPowerDomain_Init(&PlDomain->Domain, Id, BaseAddress, Parent, &PldOps);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PlDomain->Domain.Power.Node.State = (u8)XPM_POWER_STATE_OFF;
	PlDomain->Domain.Power.UseCount = 1;

	HandlePowerEvent = PlDomain->Domain.Power.HandleEvent;
	PlDomain->Domain.Power.HandleEvent = HandlePlDomainEvent;

	/* Make sure enough base addresses are being passed */
	if (2U <= OtherBaseAddressCnt) {
		PlDomain->CfuApbBaseAddr = OtherBaseAddresses[0];
		PlDomain->Cframe0RegBaseAddr = OtherBaseAddresses[1];
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

done:
	return Status;
}
