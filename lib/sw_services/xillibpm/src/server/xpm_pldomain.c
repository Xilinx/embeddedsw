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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xpm_common.h"
#include "xpm_pldomain.h"
#include "xpm_domain_iso.h"
#include "xpm_regs.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xparameters.h"

XCframe CframeIns={0}; /* CFRAME Driver Instance */

u32 GtyBaseAddressList[11] = {
	GTY_NPI_SLAVE_0_BASEADDDRESS,
	GTY_NPI_SLAVE_1_BASEADDDRESS,
	GTY_NPI_SLAVE_2_BASEADDDRESS,
	GTY_NPI_SLAVE_3_BASEADDDRESS,
	GTY_NPI_SLAVE_4_BASEADDDRESS,
	GTY_NPI_SLAVE_5_BASEADDDRESS,
	GTY_NPI_SLAVE_6_BASEADDDRESS,
	GTY_NPI_SLAVE_7_BASEADDDRESS,
	GTY_NPI_SLAVE_8_BASEADDDRESS,
	GTY_NPI_SLAVE_9_BASEADDDRESS,
	GTY_NPI_SLAVE_10_BASEADDDRESS,
};

static XStatus PldPreHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	/* TODO: Proceed only if vccint, vccaux, vccint_ram is 1 */

	/*TODO: Check NoC power state before disabling Isolation */

	/* Remove PL-NoC isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PL_SOC);
	if (XST_SUCCESS != Status)
		goto done;

	/*TODO: Check FPD and LPD  power state before disabling Isolation */

	/* Remove FPD-PL isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_FPD_PL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_FPD_PL_TEST);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove LPD-PL isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_PL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_PL_TEST);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove PL-PMC isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_PL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_PL_TEST);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_PL_CFRAME);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove isolation from VCCINT to VCCINT_RAM*/
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCRAM_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCAUX_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCAUX_VCCRAM);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove POR for PL */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PL_POR),
				     PM_RESET_ACTION_RELEASE);
done:
	return Status;
}

static XStatus PldPostHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	return Status;
}

#ifdef SPP_HACK
static XStatus PldGtyBisr( )
{
	XStatus Status = XST_SUCCESS;

	/* Todo BISR - Add when supported in XilSkey */

	return Status;
}

static XStatus PldCpmBisr( )
{
	XStatus Status = XST_SUCCESS;

	/* Todo BISR - Add when supported in XilSkey */

	return Status;
}

static XStatus PldGtyMbist(u32 BaseAddress)
{
	XStatus Status = XST_SUCCESS;

	PmOut32(BaseAddress + GTY_PCSR_LOCK_OFFSET, 0);
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

	PmOut32(BaseAddress + GTY_PCSR_LOCK_OFFSET, 1);
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

#endif

static XStatus PldHouseClean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

#ifdef SPP_HACK
	u32 Value = 0, i = 0;

	Status = PldCframeInit();
	 if (XST_SUCCESS != Status) {
                goto done;
        }

	/* Enable ROWON */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
                        XCFRAME_CMD_REG_ROWON);

	/* HCLEANR type 3,4,5,6 */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
                        XCFRAME_CMD_REG_HCLEANR);
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
	PmIn32(CFU_APB_CFU_STATUS, Value);
        while ((Value & CFU_APB_CFU_STATUS_HC_COMPLETE_MASK) !=
                                CFU_APB_CFU_STATUS_HC_COMPLETE_MASK);
        while ((Value & CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK) ==
                                CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK);

	/* VGG TRIM */
	PldApplyTrim(XPM_PL_TRIM_VGG);

	/* CRAM TRIM */
	PldApplyTrim(XPM_PL_TRIM_CRAM);

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

	/* LAGUNA REPAIR - not needed for now */

	/* There is no status for Bisr done in hard ip. But we must ensure
	 * BISR is complete before scan clear */
	 /*TBD - Wait for how long?? Wei to confirm with DFT guys */

	/* Fake read */
	/* each register is 128 bits long so issue 4 reads */
	PmIn32(CFRAME0_REG_BASEADDR + 0, Value);
	PmIn32(CFRAME0_REG_BASEADDR + 4, Value);
	PmIn32(CFRAME0_REG_BASEADDR + 8, Value);
	PmIn32(CFRAME0_REG_BASEADDR + 12, Value);

	/* PL scan clear / MBIST */
	PmOut32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
	PmOut32(CFU_APB_CFU_FGCR, CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);

	/* Poll for status */
	Status = XPm_PollForMask(CFU_APB_CFU_STATUS, CFU_APB_CFU_STATUS_SCAN_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XPm_PollForMask(CFU_APB_CFU_STATUS, CFU_APB_CFU_STATUS_SCAN_CLEAR_PASS_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Set init_complete */
	PmOut32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);
	PmOut32(CFU_APB_CFU_FGCR, CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);

	/* Run scan clear on CPM */
	PmOut32(CPM_PCSR_LOCK, 0);
	PmOut32(CPM_PCSR_MASK, CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	PmOut32(CPM_PCSR_PCR, CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	Status = XPm_PollForMask(CPM_PCSR_PSR, CPM_PCSR_PSR_SCAN_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XPm_PollForMask(CPM_PCSR_PSR, CPM_PCSR_PSR_SCAN_CLEAR_PASS_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	PmOut32(CPM_PCSR_LOCK, 1);

	/* Run BISR on GTY, CPM */
	Status = PldGtyBisr();
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = PldCpmBisr();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Run MBIST on GTY */
	for (i=0; i<sizeof(GtyBaseAddressList)/sizeof(GtyBaseAddressList[0]); i++) {
		Status = PldGtyMbist(GtyBaseAddressList[i]);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
done:
#endif
	return Status;
}

struct XPm_PowerDomainOps PldOps = {
	.PreHouseClean = PldPreHouseclean,
	.PostHouseClean = PldPostHouseclean,
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
