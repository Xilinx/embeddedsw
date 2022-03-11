/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_pmcdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_debug.h"
#include "xplmi.h"
#include "xpm_regs.h"

#define NUM_PMC_MIO		52U
#define PMC_IOU_SCLR_GPIO_MUX	0x60U

static XStatus (*HandlePowerEvent)(XPm_Node *Node, u32 Event);

static XStatus HandlePmcDomainEvent(XPm_Node *Node, u32 Event)
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
			} else {
				/* Required by MISRA */
			}
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

	return Status;
}

static XStatus PmcMioFlush(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	u32 SaveIouSclrMioSettings[NUM_PMC_MIO];
	u32 SaveData0, SaveData1, IsFlushed = 0U;
	u32 SaveDirm0, SaveDirm1, SaveTri0, SaveTri1, SaveOen0, SaveOen1;
	/* Extract information from RTCA register */
	u32 PMC_MIO_0_FLUSH_MASK =  Xil_In32(XPLMI_RTCFG_MIO_WA_BANK_500_ADDR);
	u32 PMC_MIO_1_FLUSH_MASK =  Xil_In32(XPLMI_RTCFG_MIO_WA_BANK_501_ADDR);
	/* Save GPIO reset vallue */
	u32 SaveRst = Xil_In32(CRP_RST_GPIO);
	XPlmi_Printf(MIO_FLUSH_DEBUG,"Starting PMC MIO flush ...\n\r");
	/* Deassert GPIO reset */
	Xil_Out32(CRP_RST_GPIO, 0U);
	/* Set all 52 MIO pins to GPIO. But saving their state */
	for (u32 i = 0U; i < NUM_PMC_MIO; i++){
		u32 PinAddr = PMC_IOU_SLCR_BASEADDR + (i << 2U);
		/* Saving all MIO mux settings to local memory */
		SaveIouSclrMioSettings[i] = Xil_In32(PinAddr);
		/* Inspect mask to skip certain pins. */
		if (i < 26U) {
			IsFlushed = (PMC_MIO_0_FLUSH_MASK >> i ) & 1U;
		}else{
			IsFlushed = (PMC_MIO_1_FLUSH_MASK >> (i-26U)) & 1U;
		}
		if (IsFlushed != 0U){
			/* Only go with the one that in the mask */
			XPm_Out32(PinAddr, PMC_IOU_SCLR_GPIO_MUX);
		}else{
			/* Skip the one that not in mask */
			 XPlmi_Printf(MIO_FLUSH_DEBUG,"##########Skipping pin %d ...\n\r",i);
		}
	}
	/* Saving tristate data */
	SaveTri0 = Xil_In32(PMC_IOU_SLCR_MIO_MST_TRI0);
	SaveTri1 = Xil_In32(PMC_IOU_SLCR_MIO_MST_TRI1);
	/* Saving OE data */
	SaveOen0 = Xil_In32(PMC_GPIO_OEN_0_ADDR);
	SaveOen1 = Xil_In32(PMC_GPIO_OEN_1_ADDR);
	/* Saving direction data */
	SaveDirm0 = Xil_In32(PMC_GPIO_DIRM_0_ADDR);
	SaveDirm1 = Xil_In32(PMC_GPIO_DIRM_1_ADDR);
	/* Assert MST TRI0 and MST_TR1 */
	XPm_Out32(PMC_IOU_SLCR_MIO_MST_TRI0, SaveTri0 | PMC_MIO_0_FLUSH_MASK);
	XPm_Out32(PMC_IOU_SLCR_MIO_MST_TRI1, SaveTri1 | PMC_MIO_1_FLUSH_MASK);
	/* Save TX DATA */
	SaveData0 = Xil_In32(PMC_GPIO_DATA_0_ADDR);
	SaveData1 = Xil_In32(PMC_GPIO_DATA_1_ADDR);
	/* Deassert Tristate to allow PMC_GPIO controller to control the OEN */
	XPm_Out32(PMC_IOU_SLCR_MIO_MST_TRI0, SaveTri0 & (~PMC_MIO_0_FLUSH_MASK));
	XPm_Out32(PMC_IOU_SLCR_MIO_MST_TRI1, SaveTri1 & (~PMC_MIO_1_FLUSH_MASK));
	/* Set all MIO to GPIO output */
	XPm_Out32(PMC_GPIO_DIRM_0_ADDR, SaveDirm0 | PMC_MIO_0_FLUSH_MASK);
	XPm_Out32(PMC_GPIO_DIRM_1_ADDR, SaveDirm1 | PMC_MIO_1_FLUSH_MASK);
	/* Set TX Data to zero */
	XPm_Out32(PMC_GPIO_DATA_0_ADDR, SaveData0 & (~PMC_MIO_0_FLUSH_MASK));
	XPm_Out32(PMC_GPIO_DATA_1_ADDR, SaveData1 & (~PMC_MIO_1_FLUSH_MASK));
	/* Assert OE */
	XPm_Out32(PMC_GPIO_OEN_0_ADDR, SaveOen0 | PMC_MIO_0_FLUSH_MASK);
	XPm_Out32(PMC_GPIO_OEN_1_ADDR, SaveOen1 | PMC_MIO_1_FLUSH_MASK);
	/* Deassert OE */
	XPm_Out32(PMC_GPIO_OEN_0_ADDR, SaveOen0 & (~PMC_MIO_0_FLUSH_MASK));
	XPm_Out32(PMC_GPIO_OEN_1_ADDR, SaveOen1 & (~PMC_MIO_1_FLUSH_MASK));
	/* Restore OE */
	XPm_Out32(PMC_GPIO_OEN_0_ADDR, SaveOen0);
	XPm_Out32(PMC_GPIO_OEN_1_ADDR, SaveOen1);
	/* Restore DIR */
	XPm_Out32(PMC_GPIO_DIRM_0_ADDR, SaveDirm0);
	XPm_Out32(PMC_GPIO_DIRM_1_ADDR, SaveDirm1);
	/* Restore TX_DATA */
	XPm_Out32(PMC_GPIO_DATA_0_ADDR, SaveData0);
	XPm_Out32(PMC_GPIO_DATA_1_ADDR, SaveData1);
	/* Restore TriSTate */
	XPm_Out32(PMC_IOU_SLCR_MIO_MST_TRI0, SaveTri0);
	XPm_Out32(PMC_IOU_SLCR_MIO_MST_TRI1, SaveTri1);
	/* Restore MIO Mux */
	for (u32 i = 0U; i < NUM_PMC_MIO; i++){
		XPm_Out32(PMC_IOU_SLCR_BASEADDR + (i << 2U), SaveIouSclrMioSettings[i]);
	}
	/* Restore reset for GPIO */
	Xil_Out32(CRP_RST_GPIO, SaveRst);
	/* Print debug */
	XPlmi_Printf(MIO_FLUSH_DEBUG,"PMC flush MIO done.\n\r");
	/* Done and return */
	return XST_SUCCESS;
}

static const struct XPm_PowerDomainOps PmcOps = {
	.MioFlush = PmcMioFlush
};

XStatus XPmPmcDomain_Init(XPm_PmcDomain *PmcDomain, u32 Id, XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PmcDomain->Domain, Id, 0x00000000, Parent,
				     &PmcOps);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	PmcDomain->Domain.Power.Node.State = (u8)XPM_POWER_STATE_ON;
	PmcDomain->Domain.Power.UseCount = 1;

	HandlePowerEvent = PmcDomain->Domain.Power.HandleEvent;
	PmcDomain->Domain.Power.HandleEvent = HandlePmcDomainEvent;

	/* For all domain, rail stats are updated with init node finish. For
	pmc domain, init node commands are not received so update here */
	Status = XPmPower_UpdateRailStats(&PmcDomain->Domain,
					  (u8)XPM_POWER_STATE_ON);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_RAIL_CONTROL;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
