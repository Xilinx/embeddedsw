/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#include "xpm_common.h"
#include "xpm_pldomain.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_regs.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xparameters.h"

#define XPM_NODEIDX_DEV_GT_MIN		XPM_NODEIDX_DEV_GT_0
#define XPM_NODEIDX_DEV_GT_MAX		XPM_NODEIDX_DEV_GT_10

//If TRIM_CRAM[31:0]=0 (FUSE not programmed). Then set rw_read_voltages to 0.61V + 0.625V
#define CRAM_TRIM_RW_READ_VOLTAGE	0x185

XCframe CframeIns={0}; /* CFRAME Driver Instance */
XCfupmc CfupmcIns={0}; /* CFU Driver Instance */
u32 PlpdHouseCleanBypass = 0;

static XStatus PldInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	if (XST_SUCCESS == XPmPower_CheckPower(	PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCAUX_VCCRAM, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (XST_SUCCESS == XPmPower_CheckPower(	PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCRAM_SOC, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	XCfupmc_GlblSeqInit(&CfupmcIns);

done:
	return Status;
}


static XStatus PldGtyMbist(u32 BaseAddress)
{
	XStatus Status = XST_SUCCESS;

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
done:
	return Status;
}

static void PldApplyTrim(u32 TrimType)
{
        u32 TrimVal;
        Xuint128 VggTrim={0};

        /* Read the corresponding efuse registers for TRIM values */
        switch (TrimType)
        {
                /* Read VGG trim efuse registers */
                case XPM_PL_TRIM_VGG:
                {
			PmIn32(EFUSE_CACHE_TRIM_CFRM_VGG_0, VggTrim.Word0);
                        PmIn32(EFUSE_CACHE_TRIM_CFRM_VGG_1, VggTrim.Word1);
                        PmIn32(EFUSE_CACHE_TRIM_CFRM_VGG_2, VggTrim.Word2);
                        XCframe_VggTrim(&CframeIns, &VggTrim);
                }
                break;
                /* Read CRAM trim efuse registers */
                case XPM_PL_TRIM_CRAM:
                {
                        PmIn32(EFUSE_CACHE_TRIM_CRAM, TrimVal);
			/* if eFUSE is not programmed,
			then set rw_read_voltages to 0.61V + 0.625V by writing */
			if ((TrimVal == 0) && (PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion))
				TrimVal = CRAM_TRIM_RW_READ_VOLTAGE;
                        XCframe_CramTrim(&CframeIns, TrimVal);
                }
                break;
                /* Read BRAM trim efuse registers */
                case XPM_PL_TRIM_BRAM:
                {
                        PmIn32(EFUSE_CACHE_TRIM_BRAM, TrimVal);
                        XCframe_BramTrim(&CframeIns, TrimVal);
                }
                break;
                /* Read URAM trim efuse registers */
                case XPM_PL_TRIM_URAM:
                {
                        PmIn32(EFUSE_CACHE_TRIM_URAM, TrimVal);
                        XCframe_UramTrim(&CframeIns, TrimVal);
                }
                break;
                default:
                {
                        break;
                }
        }
}

XStatus PldCfuInit()
{
	XStatus Status;
	XCfupmc_Config *Config;

	if(CfupmcIns.IsReady)
	{
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
static XStatus PldCframeInit()
{
        XStatus Status;
        XCframe_Config *Config;

        if(CframeIns.IsReady) {
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

static XStatus GtyHouseClean()
{
	XStatus Status = XST_SUCCESS;
	unsigned int i;
	XPm_Device *Device;
	u32 GtyAddresses[XPM_NODEIDX_DEV_GT_MAX - XPM_NODEIDX_DEV_GT_MIN + 1];

	for (i = 0; i < ARRAY_SIZE(GtyAddresses); i++) {
		Device = XPmDevice_GetById(GT_DEVID(XPM_NODEIDX_DEV_GT_MIN + i));
		GtyAddresses[i] = Device->Node.BaseAddress;
	}

	for (i = 0; i < ARRAY_SIZE(GtyAddresses); i++) {
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		/* Deassert INITCTRL */
		PmOut32(GtyAddresses[i] + GTY_PCSR_MASK_OFFSET,
			GTY_PCSR_INITCTRL_MASK);
		PmOut32(GtyAddresses[i] + GTY_PCSR_CONTROL_OFFSET, 0);
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1);
	}
	if(!PlpdHouseCleanBypass) {
		/* Bisr repair - Bisr should be triggered only for Addresses for which repair
		 * data is found and so not calling in loop. Trigger is handled in below routine
		 * */
		Status = XPmBisr_Repair(GTY_TAG_ID);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		for (i = 0; i < ARRAY_SIZE(GtyAddresses); i++) {
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
done:
	return Status;
}

static XStatus PldInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 PlPowerUpTime=0;

	(void)Args;
	(void)NumOfArgs;

	/* Reset Bypass flag */
	PlpdHouseCleanBypass = 0;

	/* Proceed only if vccint, vccaux, vccint_ram is 1 */
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

	/* Remove POR for PL */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PL_POR),
				     PM_RESET_ACTION_RELEASE);

	/* Toggle PS POR */
	if((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/* EDT-995767: Theres a bug with ES1, due to which a small percent (<2%) of device
		may miss pl_por_b during power, which could result CFRAME wait up in wrong state.
		The work around requires to toggle PL_POR twice after PL supplies is up. */
		Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PL_POR),
				     PM_RESET_ACTION_PULSE);
	}

        /* Check for PL PowerUp */
        Status = XPm_PollForMask(PMC_GLOBAL_PL_STATUS,
                     PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK, XPM_POLL_TIMEOUT);
        if(XST_SUCCESS != Status) {
		goto done;
        }

	/* Remove SRST for PL */
	Status = XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_PL_SRST),
				     PM_RESET_ACTION_RELEASE);

	 /* Remove PL-SOC isolation */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PL_SOC, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	 /* Remove PMC-SOC isolation */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = PldCfuInit();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = PldCframeInit();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Enable the global signals */
	XCfupmc_SetGlblSigEn(&CfupmcIns, (u8 )TRUE);

done:
	return Status;
}

static XStatus PldHouseClean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 Value = 0;

	/* If Arg0 is set, bypass houseclean */
	if(NumOfArgs && Args[0] == 1)
		PlpdHouseCleanBypass = 1;

	if (PLATFORM_VERSION_SILICON == Platform) {
		/*House clean GTY*/
		Status = GtyHouseClean();
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s : GTY HC failed", __func__);
		}
	}

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_PL_CFRAME, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

//#ifndef PLPD_HOUSECLEAN_BYPASS
	if(!PlpdHouseCleanBypass)
	{
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

		/* BRAM/URAM TRIM */
		PldApplyTrim(XPM_PL_TRIM_BRAM);
		PldApplyTrim(XPM_PL_TRIM_URAM);

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

		 /* Poll for house clean completion */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Waiitng for PL HC complete....", __func__);
		while ((Xil_In32(CFU_APB_CFU_STATUS) &
						CFU_APB_CFU_STATUS_HC_COMPLETE_MASK) !=
								CFU_APB_CFU_STATUS_HC_COMPLETE_MASK);
		XPlmi_Printf(DEBUG_INFO, "Done\r\n");

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : CFRAME_BUSY to go low...", __func__);
		while ((Xil_In32(CFU_APB_CFU_STATUS) &
						CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK) ==
								CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK);
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
		PmIn32(CFRAME0_REG_BASEADDR + 0, Value);
		PmIn32(CFRAME0_REG_BASEADDR + 4, Value);
		PmIn32(CFRAME0_REG_BASEADDR + 8, Value);
		PmIn32(CFRAME0_REG_BASEADDR + 12, Value);
		XPlmi_Printf(DEBUG_INFO, "Done\r\n");

		/* Unlock CFU writes */
		PmOut32(CFU_APB_CFU_PROTECT, 0);

		/* PL scan clear / MBIST */
		PmOut32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
		PmOut32(CFU_APB_CFU_FGCR, CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);

		/* Lock CFU writes */
		PmOut32(CFU_APB_CFU_PROTECT, 1);

		/* Poll for status */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for Hard Block Scan Clear / MBIST complete...", __func__);
		Status = XPm_PollForMask(CFU_APB_CFU_STATUS, CFU_APB_CFU_STATUS_SCAN_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			/** HACK: Continuing even if CFI SC is not completed */
			Status = XST_SUCCESS;
			//Status = XST_FAILURE;
			//goto done;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "Done\r\n");
		}
		/* Check if Scan Clear Passed */
		if ((XPm_In32(CFU_APB_CFU_STATUS) & CFU_APB_CFU_STATUS_SCAN_CLEAR_PASS_MASK) !=
			CFU_APB_CFU_STATUS_SCAN_CLEAR_PASS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: Hard Block Scan Clear / MBIST FAILED\r\n", __func__);
			/** HACK: Continuing even if CFI SC is not pass */
			Status = XST_SUCCESS;
			//Status = XST_FAILURE;
			//goto done;
		}
	}
//#endif /* PLPD_HOUSECLEAN_BYPASS */

	/* Unlock CFU writes */
	PmOut32(CFU_APB_CFU_PROTECT, 0);

	/* Set init_complete */
	PmOut32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);
	PmOut32(CFU_APB_CFU_FGCR, CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);

	/* Lock CFU writes */
	PmOut32(CFU_APB_CFU_PROTECT, 1);

	/* Compilation warning fix */
	(void)Value;

done:
	return Status;
}

struct XPm_PowerDomainOps PldOps = {
	.InitStart = PldInitStart,
	.InitFinish = PldInitFinish,
	.PlHouseclean = PldHouseClean,
};

static XStatus (*HandlePowerEvent)(XPm_Node *Node, u32 Event);

static XStatus HandlePlDomainEvent(XPm_Node *Node, u32 Event)
{
	u32 Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	PmDbg("State=%d, Event=%d\n\r", Node->State, Event);

	switch (Node->State)
	{
		case XPM_POWER_STATE_ON:
			if (XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				Power->UseCount++;
			} else if (XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				Power->UseCount--;
				Node->State = XPM_POWER_STATE_OFF;
			} else {
				Status = XST_FAILURE;
			}
			break;
		case XPM_POWER_STATE_OFF:
			if (XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				Power->UseCount++;
				Node->State = XPM_POWER_STATE_ON;
			} else if (XPM_POWER_EVENT_PWR_DOWN == Event) {
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
			 XPm_Power *Parent)
{
	XPmPowerDomain_Init(&PlDomain->Domain, Id, BaseAddress, Parent, &PldOps);
	PlDomain->Domain.Power.Node.State = XPM_POWER_STATE_OFF;
	PlDomain->Domain.Power.UseCount = 1;

	HandlePowerEvent = PlDomain->Domain.Power.Node.HandleEvent;
	PlDomain->Domain.Power.Node.HandleEvent =
		HandlePlDomainEvent;

	return XST_SUCCESS;
}
