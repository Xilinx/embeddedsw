/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_power_handlers.h"
#include "xpm_power_plat.h"
#include "xpm_versal_aiepg2_regs.h"

static void XPm_InterruptPwrUpHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 PwrUp0Status, PwrUp0IntMask, PwrUp1Status, PwrUp1IntMask;

	PwrUp0Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_MASK);
	if (0U != PwrUp0Status){
		Status = XPm_DispatchApuPwrUpHandler(PwrUp0Status, PwrUp0IntMask);
	}

	PwrUp1Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_MASK);
	if (0U != PwrUp1Status){
		Status = XPm_DispatchRpuPwrUpHandler(PwrUp1Status, PwrUp1IntMask);
	}

	if (XST_SUCCESS != Status) {
		PmErr("Error in handling power up interrupt\r\n");
	}
}

static void XPm_InterruptPwrDwnHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 PwrDwn0Status, PwrDwn0IntMask, PwrUp0Status, PwrUp0IntMask;
	u32 PwrDwn1Status, PwrDwn1IntMask, PwrUp1Status, PwrUp1IntMask;

	PwrDwn0Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN0_STATUS);
	PwrDwn0IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN0_INT_MASK);
	PwrUp0Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_MASK);
	PwrDwn1Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS);
	PwrDwn1IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN1_INT_MASK);
	PwrUp1Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_MASK);

	if (0U != PwrDwn0Status){
		Status = XPm_DispatchApuPwrDwnHandler(PwrDwn0Status, PwrDwn0IntMask,
						      PwrUp0Status, PwrUp0IntMask);
	}

	if (0U != PwrDwn1Status){
		Status = XPm_DispatchRpuPwrDwnHandler(PwrDwn1Status, PwrDwn1IntMask,
						      PwrUp1Status, PwrUp1IntMask);
	}

	if (XST_SUCCESS != Status) {
		PmErr("Error in handling power down interrupt\r\n");
	}
}

static void XPm_InterruptWakeupHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 WakeupStatus, WakeupIntMask;

	WakeupStatus = XPm_In32(PSXC_LPX_SLCR_WAKEUP0_IRQ_STATUS);
	WakeupIntMask = XPm_In32(PSXC_LPX_SLCR_WAKEUP0_IRQ_MASK);
	if (0U != WakeupStatus){
		Status = XPm_DispatchApuWakeupHandler(WakeupStatus, WakeupIntMask);
	}

	WakeupStatus = XPm_In32(PSXC_LPX_SLCR_WAKEUP1_IRQ_STATUS);
	WakeupIntMask = XPm_In32(PSXC_LPX_SLCR_WAKEUP1_IRQ_MASK);
	if (0U != WakeupStatus){
		Status = XPm_DispatchRpuWakeupHandler(WakeupStatus, WakeupIntMask);
	}

	if (XST_SUCCESS != Status) {
		PmErr("Error in handling wakeup interrupt\r\n");
	}
}

static void XPm_InterruptPwrCtrlHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 PwrCtrlStatus, PwrCtrlMask;

	PwrCtrlStatus = XPm_In32(PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS);
	PwrCtrlMask = XPm_In32(PSXC_LPX_SLCR_POWER_DWN_IRQ_MASK);
	Status = XPm_DispatchPwrCtrlHandler(PwrCtrlStatus, PwrCtrlMask);
	if (XST_SUCCESS != Status) {
		PmErr("Error in handling power reset interrupt\r\n");
	}
}

/* Structure for Top level interrupt table */
static const struct HandlerTable g_TopLevelInterruptTable[] = {
	{
		PMC_IRQ_PWR_MB_IRQ_PWR_UP_REQ_SHIFT,
		PMC_IRQ_PWR_MB_IRQ_PWR_UP_REQ_MASK,
		XPm_InterruptPwrUpHandler
	},
	{
		PMC_IRQ_PWR_MB_IRQ_PWR_DWN_REQ_SHIFT,
		PMC_IRQ_PWR_MB_IRQ_PWR_DWN_REQ_MASK,
		XPm_InterruptPwrDwnHandler
	},
	{
		PMC_IRQ_PWR_MB_IRQ_WAKE_UP_REQ_SHIFT,
		PMC_IRQ_PWR_MB_IRQ_WAKE_UP_REQ_MASK,
		XPm_InterruptWakeupHandler
	},
	{
		PMC_IRQ_PWR_MB_IRQ_PWR_CTRL_REQ_SHIFT,
		PMC_IRQ_PWR_MB_IRQ_PWR_CTRL_REQ_MASK,
		XPm_InterruptPwrCtrlHandler
	},
};

int XPm_PwrIntrHandler(void *IntrNumber)
{
	u32 IrqReg = XPm_In32(PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ);
	u32 Index;
	PmDbg("Interrupt number = 0x%x\r\n", (u32)IntrNumber);

	for(Index = 0U; Index < ARRAY_SIZE(g_TopLevelInterruptTable); Index++) {
		if ((IrqReg & g_TopLevelInterruptTable[Index].Mask) ==
		     g_TopLevelInterruptTable[Index].Mask) {
			if (NULL != g_TopLevelInterruptTable[Index].Handler) {
				/* Call interrupt handler */
				g_TopLevelInterruptTable[Index].Handler();
			}

			/* ACK the interrupt */
			XPm_Out32(PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ, g_TopLevelInterruptTable[Index].Mask);
		}
	}

	return XST_SUCCESS;
}
