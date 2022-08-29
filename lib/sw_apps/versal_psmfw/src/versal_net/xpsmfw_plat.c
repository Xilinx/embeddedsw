/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpsmfw_default.h"
#include "xpsmfw_plat.h"
#include "xpsmfw_power.h"

XStatus XPsmfw_PwrUpHandler(void)
{
	u32 PwrUp0Status, PwrUp0IntMask, PwrUp1Status, PwrUp1IntMask;
	XStatus Status = XST_FAILURE;

	PwrUp0Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_INT_MASK);
	if(0U != PwrUp0Status){
		Status = XPsmFw_DispatchPwrUp0Handler(PwrUp0Status, PwrUp0IntMask);
	}
	PwrUp1Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_INT_MASK);
	if(0U != PwrUp1Status){
		Status = XPsmFw_DispatchPwrUp1Handler(PwrUp1Status, PwrUp1IntMask);
	}
	return Status;
}

XStatus XPsmfw_PwrDwnHandler(void)
{
	u32 PwrDwn0Status, PwrDwn0IntMask, PwrUp0Status, PwrUp0IntMask;
	u32 PwrDwn1Status, PwrDwn1IntMask, PwrUp1Status, PwrUp1IntMask;
	XStatus Status = XST_FAILURE;

	PwrDwn0Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN0_STATUS);
	PwrDwn0IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN0_INT_MASK);
	PwrUp0Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_INT_MASK);
	PwrDwn1Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN1_STATUS);
	PwrDwn1IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN1_INT_MASK);
	PwrUp1Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_INT_MASK);
	if(0 != PwrDwn0Status){
		Status = XPsmFw_DispatchPwrDwn0Handler(PwrDwn0Status, PwrDwn0IntMask,
				PwrUp0Status, PwrUp0IntMask);
	}
	if(0 != PwrDwn1Status){
		Status = XPsmFw_DispatchPwrDwn1Handler(PwrDwn1Status, PwrDwn1IntMask,
				PwrUp1Status, PwrUp1IntMask);
	}
	return Status;
}

XStatus XPsmfw_WakeupHandler(void)
{
	u32 WakeupStatus, WakeupIntMask;
	XStatus Status = XST_FAILURE;

	WakeupStatus = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP0_IRQ_STATUS);
	WakeupIntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP0_IRQ_MASK);
	if(0 != WakeupStatus){
		Status = XPsmFw_DispatchAPUWakeupHandler(WakeupStatus, WakeupIntMask);
	}

	WakeupStatus = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP1_IRQ_STATUS);
	WakeupIntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP1_IRQ_MASK);
	if(0 != WakeupStatus){
		Status = XPsmFw_DispatchRPUWakeupHandler(WakeupStatus, WakeupIntMask);
	}
	return Status;
}

XStatus XPsmfw_PwrCtlHandler(void)
{
	u32 PwrCtlStatus, PwrCtlIntMask;
	XStatus Status = XST_FAILURE;

	PwrCtlStatus = XPsmFw_Read32(PSMX_GLOBAL_REG_PWR_CTRL1_IRQ_STATUS);
	PwrCtlIntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_PWR_CTRL1_IRQ_MASK);
	Status = XPsmFw_DispatchPwrCtlHandler(PwrCtlStatus, PwrCtlIntMask);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling power control interrupt\r\n");
	}
	return Status;
}

XStatus XPsmFw_GicP2Handler(void)
{
	/*no operation in versal net*/
	return XST_SUCCESS;
}
