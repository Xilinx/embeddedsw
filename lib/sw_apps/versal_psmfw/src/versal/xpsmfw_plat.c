/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpsmfw_default.h"
#include "xpsmfw_plat.h"
#include "xpsmfw_power.h"

XStatus XPsmfw_PwrUpHandler(void)
{
	u32 PwrUpStatus, PwrUpIntMask;
	PwrUpStatus = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_STATUS);
	PwrUpIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_INT_MASK);
	return XPsmFw_DispatchPwrUpHandler(PwrUpStatus, PwrUpIntMask);
}

XStatus XPsmfw_PwrDwnHandler(void)
{
	u32 PwrDwnStatus, PwrDwnIntMask, PwrUpStatus, PwrUpIntMask;
	PwrDwnStatus = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS);
	PwrDwnIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_INT_MASK);
	PwrUpStatus = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_STATUS);
	PwrUpIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_INT_MASK);
	return XPsmFw_DispatchPwrDwnHandler(PwrDwnStatus, PwrDwnIntMask,
			PwrUpStatus, PwrUpIntMask);
}

XStatus XPsmfw_WakeupHandler(void)
{
	u32 WakeupStatus, WakeupIntMask;
	WakeupStatus = XPsmFw_Read32(PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS);
	WakeupIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_WAKEUP_IRQ_MASK);
	return XPsmFw_DispatchWakeupHandler(WakeupStatus, WakeupIntMask);
}

XStatus XPsmfw_PwrCtlHandler(void)
{
	u32 PwrCtlStatus, PwrCtlIntMask;

	PwrCtlStatus = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS);
	PwrCtlIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_MASK);
	return XPsmFw_DispatchPwrCtlHandler(PwrCtlStatus, PwrCtlIntMask);
}

XStatus XPsmFw_GicP2Handler(void)
{
	u32 GicP2IrqStatus;
	u32 GicP2IrqMask;

	GicP2IrqStatus = XPsmFw_Read32(PSM_GLOBAL_GICP2_IRQ_STATUS);
	GicP2IrqMask = XPsmFw_Read32(PSM_GLOBAL_GICP2_IRQ_MASK);
	return XPsmFw_DispatchGicP2Handler(GicP2IrqStatus, GicP2IrqMask);
}
