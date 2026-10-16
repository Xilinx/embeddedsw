/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_power_handlers.h"
#include "xpm_power_core.h"
#include "xpm_runtime_core.h"
#if defined(XILPM_NG_FPD_OFF)
#include "xpm_runtime_device.h"
#include "xpm_requirement.h"
#endif

CREATE_TABLE_APU_PWRUPDOWNHANDLER(4, 2)
CREATE_TABLE_RPU_PWRUPDOWNHANDLER(5, 2)

static XStatus XPm_DispatchApuPwrUpHandler(u32 PwrUpStatus, u32 PwrUpIntMask);
static XStatus XPm_DispatchRpuPwrUpHandler(u32 PwrUpStatus, u32 PwrUpIntMask);
static XStatus XPm_DispatchApuPwrDwnHandler(u32 PwrDwnStatus, u32 PwrDwnIntMask,
                                            u32 PwrUpStatus, u32 PwrUpIntMask);
static XStatus XPm_DispatchRpuPwrDwnHandler(u32 PwrDwnStatus, u32 PwrDwnIntMask,
                                            u32 PwrUpStatus, u32 PwrUpIntMask);
static XStatus XPm_DispatchApuWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask);
static XStatus XPm_DispatchRpuWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask);
static XStatus XPm_DispatchPwrCtrlHandler(u32 PwrCtrlStatus, u32 PwrCtrlMask);
#if defined(XILPM_NG_FPD_OFF)
static XStatus ApplyScheduledReqmOnSuspend(const XPm_Core *Core);
#endif

CREATE_TABLE_RPU_WAKEUP_HANDLER(5, 2)
CREATE_TABLE_APU_WAKEUP_HANDLER(4, 2)
CREATE_TABLE_APU_SLEEP_HANDLER(4, 2)
CREATE_TABLE_RPU_SLEEP_HANDLER(5, 2)
/****************************************************************************/
/**
 * @brief Service APU and RPU power-up request interrupts.
 *
 * Reads both power-up status and interrupt-mask registers and dispatches each
 * nonzero status value. Logs an error if the final dispatch attempted fails or
 * if neither status register requests a dispatch.
 *
 * @return None.
 ****************************************************************************/
static void XPm_InterruptPwrUpHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 PwrUp0Status, PwrUp0IntMask, PwrUp1Status, PwrUp1IntMask;

	/* Snapshot the APU request bank and its interrupt mask */
	PwrUp0Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_MASK);
	if (0U != PwrUp0Status){
		Status = XPm_DispatchApuPwrUpHandler(PwrUp0Status, PwrUp0IntMask);
	}

	/* Snapshot the RPU request bank and its interrupt mask */
	PwrUp1Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_MASK);
	if (0U != PwrUp1Status){
		Status = XPm_DispatchRpuPwrUpHandler(PwrUp1Status, PwrUp1IntMask);
	}

	/* No request leaves the initial failure status unchanged */
	if (XST_SUCCESS != Status) {
		PmErr("Error in handling power up interrupt\r\n");
	}
}

/****************************************************************************/
/**
 * @brief Service APU and RPU power-down request interrupts.
 *
 * Reads both power-down and power-up status and mask register sets and
 * dispatches each nonzero power-down status. Logs an error if the final
 * dispatch attempted fails or if neither power-down status requests a dispatch.
 *
 * @return None.
 ****************************************************************************/
static void XPm_InterruptPwrDwnHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 PwrDwn0Status, PwrDwn0IntMask, PwrUp0Status, PwrUp0IntMask;
	u32 PwrDwn1Status, PwrDwn1IntMask, PwrUp1Status, PwrUp1IntMask;

	/* Snapshot both request directions for the APU bank */
	PwrDwn0Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN0_STATUS);
	PwrDwn0IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN0_INT_MASK);
	PwrUp0Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_MASK);
	/* Take the same paired snapshot for the RPU bank */
	PwrDwn1Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS);
	PwrDwn1IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN1_INT_MASK);
	PwrUp1Status = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPm_In32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_MASK);

	/* Dispatch only banks with a pending power-down request */
	if (0U != PwrDwn0Status){
		Status = XPm_DispatchApuPwrDwnHandler(PwrDwn0Status, PwrDwn0IntMask,
						      PwrUp0Status, PwrUp0IntMask);
	}

	if (0U != PwrDwn1Status){
		Status = XPm_DispatchRpuPwrDwnHandler(PwrDwn1Status, PwrDwn1IntMask,
						      PwrUp1Status, PwrUp1IntMask);
	}

	/* No request leaves the initial failure status unchanged */
	if (XST_SUCCESS != Status) {
		PmErr("Error in handling power down interrupt\r\n");
	}
}

/****************************************************************************/
/**
 * @brief Service APU and RPU wakeup request interrupts.
 *
 * Reads both wakeup status and mask registers and dispatches each nonzero
 * status value. Logs an error if the final dispatch attempted fails or if
 * neither status register requests a dispatch.
 *
 * @return None.
 ****************************************************************************/
static void XPm_InterruptWakeupHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 WakeupStatus, WakeupIntMask;

	/* Service the APU wakeup request bank first */
	WakeupStatus = XPm_In32(PSXC_LPX_SLCR_WAKEUP0_IRQ_STATUS);
	WakeupIntMask = XPm_In32(PSXC_LPX_SLCR_WAKEUP0_IRQ_MASK);
	if (0U != WakeupStatus){
		Status = XPm_DispatchApuWakeupHandler(WakeupStatus, WakeupIntMask);
	}

	/* Reuse the snapshot variables for the RPU wakeup bank */
	WakeupStatus = XPm_In32(PSXC_LPX_SLCR_WAKEUP1_IRQ_STATUS);
	WakeupIntMask = XPm_In32(PSXC_LPX_SLCR_WAKEUP1_IRQ_MASK);
	if (0U != WakeupStatus){
		Status = XPm_DispatchRpuWakeupHandler(WakeupStatus, WakeupIntMask);
	}

	if (XST_SUCCESS != Status) {
		PmErr("Error in handling wakeup interrupt\r\n");
	}
}

/****************************************************************************/
/**
 * @brief Service core power-control interrupts.
 *
 * Reads the power-down IRQ status and mask registers and dispatches them to the
 * APU/RPU power-control handler, which powers down matching cores and
 * acknowledges serviced bits. A dispatch failure is logged. Status remains
 * XST_FAILURE when no bit matches, so an idle interrupt also logs an error.
 *
 * @return None.
 ****************************************************************************/
static void XPm_InterruptPwrCtrlHandler(void)
{
	XStatus Status = XST_FAILURE;
	u32 PwrCtrlStatus, PwrCtrlMask;
	/* The dispatcher performs the per-core match on this IRQ snapshot */
	PwrCtrlStatus = XPm_In32(PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS);
	PwrCtrlMask = XPm_In32(PSXC_LPX_SLCR_POWER_DWN_IRQ_MASK);
	Status = XPm_DispatchPwrCtrlHandler(PwrCtrlStatus, PwrCtrlMask);
	if (XST_SUCCESS != Status) {
		PmErr("Error in handling power reset interrupt\r\n");
	}
}

/*****************************************************************************/
/**
 * @brief Top level power interrupt handler that dispatches power up, power
 *        down, wakeup and power control requests to their handlers and ACKs
 *        the serviced interrupts.
 *
 * @param IntrNumber	Interrupt number that triggered the handler
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
int XPm_PwrIntrHandler(void *IntrNumber)
{
	/* Structure for Top level interrupt table */
	static const struct HandlerTable g_TopLevelInterruptTable[] = {
		{
			PMC_IRQ_PWR_MB_IRQ_PWR_UP_REQ_SHIFT,
			PMC_IRQ_PWR_MB_IRQ_PWR_UP_REQ_MASK,
			&XPm_InterruptPwrUpHandler
		},
		{
			PMC_IRQ_PWR_MB_IRQ_PWR_DWN_REQ_SHIFT,
			PMC_IRQ_PWR_MB_IRQ_PWR_DWN_REQ_MASK,
			&XPm_InterruptPwrDwnHandler
		},
		{
			PMC_IRQ_PWR_MB_IRQ_WAKE_UP_REQ_SHIFT,
			PMC_IRQ_PWR_MB_IRQ_WAKE_UP_REQ_MASK,
			&XPm_InterruptWakeupHandler
		},
		{
			PMC_IRQ_PWR_MB_IRQ_PWR_CTRL_REQ_SHIFT,
			PMC_IRQ_PWR_MB_IRQ_PWR_CTRL_REQ_MASK,
			&XPm_InterruptPwrCtrlHandler
		},
	};
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
/*****************************************************************************/
/**
 * @brief APU power up request interrupt handler
 *
 * @param PwrUpStatus	Interrupt status indicating a request to power up core.
 * @param PwrUpIntMask	Interrupt mask
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
static XStatus XPm_DispatchApuPwrUpHandler(u32 PwrUpStatus, u32 PwrUpIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAY_SIZE(ApuPwrUpDwnHandlerTable); Index++) {
		/* Check for PwrUpStatus 1 and PwrUpIntMask is 0.
		 * PwrUpStatus indicates that there is an interrupt request to power up the core.
		 * PwrUpIntMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 */
		if ((CHECK_BIT(PwrUpStatus, ApuPwrUpDwnHandlerTable[Index].PwrUpMask)) &&
		    !(CHECK_BIT(PwrUpIntMask, ApuPwrUpDwnHandlerTable[Index].PwrUpMask))) {
			/* Call power up handler */
			Status = XPmPower_ACpuReqPwrUp(ApuPwrUpDwnHandlerTable[Index].Args);

			/* Ack the service */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP0_STATUS, ApuPwrUpDwnHandlerTable[Index].PwrUpMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_DIS, ApuPwrUpDwnHandlerTable[Index].PwrUpMask);
		} else if (CHECK_BIT(PwrUpStatus, ApuPwrUpDwnHandlerTable[Index].PwrUpMask)){
			/* Ack the service if status is 1 but interrupt is not enabled */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP0_STATUS, ApuPwrUpDwnHandlerTable[Index].PwrUpMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_DIS, ApuPwrUpDwnHandlerTable[Index].PwrUpMask);
			Status = XST_SUCCESS;
		}
		else {
			/* No action required for this table entry */
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief RPU power up request interrupt handler
 *
 * @param PwrUpStatus	Interrupt status indicating a request to power up core.
 * @param PwrUpIntMask	Interrupt mask
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
static XStatus XPm_DispatchRpuPwrUpHandler(u32 PwrUpStatus, u32 PwrUpIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAY_SIZE(RpuPwrUpDwnHandlerTable); Index++) {
		/* Check for PwrUpStatus 1 and PwrUpIntMask is 0.
		 * PwrUpStatus indicates that there is an interrupt request to power up the core.
		 * PwrUpIntMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 */
		if ((CHECK_BIT(PwrUpStatus, RpuPwrUpDwnHandlerTable[Index].PwrUpMask)) &&
		    !(CHECK_BIT(PwrUpIntMask, RpuPwrUpDwnHandlerTable[Index].PwrUpMask))) {
			/* Call power up handler */
			Status = XPmPower_RpuReqPwrUp(RpuPwrUpDwnHandlerTable[Index].Args);

			/* Ack the service */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS, RpuPwrUpDwnHandlerTable[Index].PwrUpMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_DIS, RpuPwrUpDwnHandlerTable[Index].PwrUpMask);
		} else if (CHECK_BIT(PwrUpStatus, RpuPwrUpDwnHandlerTable[Index].PwrUpMask)){
			/* Ack the service if status is 1 but interrupt is not enabled */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS, RpuPwrUpDwnHandlerTable[Index].PwrUpMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_DIS, RpuPwrUpDwnHandlerTable[Index].PwrUpMask);
			Status = XST_SUCCESS;
		}
		else {
			/* No action required for this table entry */
		}
	}

	return Status;

}

/*****************************************************************************/
/**
 * @brief APU power down request interrupt handler
 *
 * @param PwrDwnStatus	Interrupt status indicating a request to power down core
 * @param PwrDwnIntMask	Power down interrupt mask
 * @param PwrUpStatus	Interrupt status indicating a request to power up core.
 * @param PwrUpIntMask	Power up interrupt mask
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
static XStatus XPm_DispatchApuPwrDwnHandler(u32 PwrDwnStatus, u32 PwrDwnIntMask,
				     u32 PwrUpStatus, u32 PwrUpIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAY_SIZE(ApuPwrUpDwnHandlerTable); Index++) {
		/* Check for PwrDwnStatus 1 and PwrDwnIntMask is 0.
		 * PwrDwnStatus indicates that there is an interrupt request to power down the core.
		 * PwrDwnIntMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 *
		 * We must also check that there is not simultaneously a request to power up the core.
		 * Check that PwrUpStatus is not set and that power up is masked with PwrUpIntMask.
		 */
		if ((CHECK_BIT(PwrDwnStatus, ApuPwrUpDwnHandlerTable[Index].PwrDwnMask)) &&
		    !(CHECK_BIT(PwrDwnIntMask, ApuPwrUpDwnHandlerTable[Index].PwrDwnMask)) &&
		    !(CHECK_BIT(PwrUpStatus, ApuPwrUpDwnHandlerTable[Index].PwrUpMask)) &&
		    (CHECK_BIT(PwrUpIntMask, ApuPwrUpDwnHandlerTable[Index].PwrUpMask))) {
			/* Call power down handler */
			Status = XPmPower_ACpuReqPwrDwn(ApuPwrUpDwnHandlerTable[Index].Args);

			/* Ack the service */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN0_STATUS, ApuPwrUpDwnHandlerTable[Index].PwrDwnMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN0_INT_DIS, ApuPwrUpDwnHandlerTable[Index].PwrDwnMask);
		} else if (CHECK_BIT(PwrDwnStatus, ApuPwrUpDwnHandlerTable[Index].PwrDwnMask)) {
			/* Ack the service  if power up and power down interrupt arrives simultaneously */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN0_STATUS, ApuPwrUpDwnHandlerTable[Index].PwrDwnMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN0_INT_DIS, ApuPwrUpDwnHandlerTable[Index].PwrDwnMask);
			Status = XST_SUCCESS;
		}
		else {
			/* No action required for this table entry */
		}
	}

	return Status;
}



/*****************************************************************************/
/**
 * @brief RPU power down request interrupt handler
 *
 * @param PwrDwnStatus	Interrupt status indicating a request to power down core
 * @param PwrDwnIntMask	Power down interrupt mask
 * @param PwrUpStatus	Interrupt status indicating a request to power up core.
 * @param PwrUpIntMask	Power up interrupt mask
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
static XStatus XPm_DispatchRpuPwrDwnHandler(u32 PwrDwnStatus, u32 PwrDwnIntMask,
				     u32 PwrUpStatus, u32 PwrUpIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAY_SIZE(RpuPwrUpDwnHandlerTable); Index++) {
		/* Check for PwrDwnStatus 1 and PwrDwnIntMask is 0.
		 * PwrDwnStatus indicates that there is an interrupt request to power down the core.
		 * PwrDwnIntMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 *
		 * We must also check that there is not simultaneously a request to power up the core.
		 * Check that PwrUpStatus is not set and that power up is masked with PwrUpIntMask.
		 */
		if ((CHECK_BIT(PwrDwnStatus, RpuPwrUpDwnHandlerTable[Index].PwrDwnMask)) &&
		    !(CHECK_BIT(PwrDwnIntMask, RpuPwrUpDwnHandlerTable[Index].PwrDwnMask)) &&
		    !(CHECK_BIT(PwrUpStatus, RpuPwrUpDwnHandlerTable[Index].PwrUpMask)) &&
		    (CHECK_BIT(PwrUpIntMask, RpuPwrUpDwnHandlerTable[Index].PwrUpMask))) {
			/* Call power down handler */
			Status = XPmPower_RpuReqPwrDwn(RpuPwrUpDwnHandlerTable[Index].Args);

			/* Ack the service */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS, RpuPwrUpDwnHandlerTable[Index].PwrDwnMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_INT_DIS, RpuPwrUpDwnHandlerTable[Index].PwrDwnMask);
		} else if (CHECK_BIT(PwrDwnStatus, RpuPwrUpDwnHandlerTable[Index].PwrDwnMask)) {
			/* Ack the service  if power up and power down interrupt arrives simultaneously */
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS, RpuPwrUpDwnHandlerTable[Index].PwrDwnMask);
			XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_INT_DIS, RpuPwrUpDwnHandlerTable[Index].PwrDwnMask);
			Status = XST_SUCCESS;
		}
		else {
			/* No action required for this table entry */
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief Dispatch unmasked APU wakeup requests.
 *
 * For each requested and unmasked APU core, obtains the core's saved resume
 * address, requests wakeup, then clears and disables the serviced IRQ bit.
 *
 * @param WakeupStatus Wakeup request status bits for APU cores.
 * @param WakeupIntMask Wakeup interrupt mask bits; a clear bit is unmasked.
 *
 * @return Status from the last successful wakeup request, XST_INVALID_PARAM if
 *         a requested core is absent, an error from XPmCore_RequestWakeup, or
 *         XST_FAILURE if no unmasked request is handled.
 *****************************************************************************/
static XStatus XPm_DispatchApuWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;
	XPm_Core *Core;
	u64 ResumeAddr;

	for (Index = 0U; Index < ARRAY_SIZE(ApuWakeupHandlerTable); Index++) {
		/* Check for WakeupStatus 1 and WakeupIntMask is 0.
		 * WakeupStatus indicates that there is an interrupt request to wakeup the core.
		 * WakeupIntMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 */
		if ((CHECK_BIT(WakeupStatus, ApuWakeupHandlerTable[Index].Mask)) &&
		    !(CHECK_BIT(WakeupIntMask, ApuWakeupHandlerTable[Index].Mask))) {
			/* Resolve the core represented by the matching table entry */
			Core = (XPm_Core *)XPmDevice_GetById(ApuWakeupHandlerTable[Index].DeviceId);
			if (NULL == Core) {
				Status = XST_INVALID_PARAM;
				goto done;
			}

			ResumeAddr = Core->ResumeAddr;

			Status = XPmCore_RequestWakeup(Core, 1U, ResumeAddr);
			if (XST_SUCCESS != Status) {
				PmErr("Failed to wakeup core 0x%x!\r\n", Core->Device.Node.Id);
				goto done;
			}

			/* Ack the service */
			XPm_Out32(PSXC_LPX_SLCR_WAKEUP0_IRQ_STATUS, ApuWakeupHandlerTable[Index].Mask);
			XPm_Out32(PSXC_LPX_SLCR_WAKEUP0_IRQ_DIS, ApuWakeupHandlerTable[Index].Mask);
		}
	}

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief RPU wakeup interrupt handler
 *
 * @param WakeupStatus	Interrupt status indicating a request to wakeup core.
 * @param WakeupIntMask	Interrupt mask
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
static XStatus XPm_DispatchRpuWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;
	XPm_Core *Core;
	u64 ResumeAddr;

	for (Index = 0U; Index < ARRAY_SIZE(RpuWakeupHandlerTable); Index++) {
		/* Check for WakeupStatus 1 and WakeupIntMask is 0.
		 * WakeupStatus indicates that there is an interrupt request to wakeup the core.
		 * WakeupIntMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 */
		if ((CHECK_BIT(WakeupStatus, RpuWakeupHandlerTable[Index].Mask)) &&
		    !(CHECK_BIT(WakeupIntMask, RpuWakeupHandlerTable[Index].Mask))) {
			Core = (XPm_Core *)XPmDevice_GetById(RpuWakeupHandlerTable[Index].DeviceId);
			if (NULL == Core) {
				Status = XST_INVALID_PARAM;
				goto done;
			}

			ResumeAddr = Core->ResumeAddr;
			/* Call power up handler */
			Status = XPmCore_RequestWakeup(Core, 1U, ResumeAddr);
			if (XST_SUCCESS != Status) {
				PmErr("Failed to wakeup core 0x%x!\r\n", Core->Device.Node.Id);
				goto done;
			}

			/* Ack the service */
			XPm_Out32(PSXC_LPX_SLCR_WAKEUP1_IRQ_STATUS, RpuWakeupHandlerTable[Index].Mask);
			XPm_Out32(PSXC_LPX_SLCR_WAKEUP1_IRQ_DIS, RpuWakeupHandlerTable[Index].Mask);
		}
	}

done:
	return Status;
}

#if defined(XILPM_NG_FPD_OFF)
/*****************************************************************************/
/**
 * @brief Applies the requirements a subsystem scheduled during suspend.
 *
 * @param[in] Core	Core that has just powered down
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
static XStatus ApplyScheduledReqmOnSuspend(const XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = XPmDevice_GetSubsystemIdOfCore(&Core->Device);
	XPm_Subsystem *Subsystem = NULL;

	if (INVALID_SUBSYSID == SubsystemId) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	/* Requirements are only scheduled while the subsystem is suspending */
	if ((u8)SUSPENDING != Subsystem->State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/*
	 * Linux only lowered each device to PM_CAP_UNUSABLE while it was still
	 * running, which left the scheduled capability of 0 in Next. Applying it
	 * now takes every device to UNUSED, and the resulting power parent
	 * release is what lets FPD reach UseCount 0.
	 */
	Status = XPmRequirement_UpdateScheduled(Subsystem, 0U);

done:
	return Status;
}
#endif /* XILPM_NG_FPD_OFF */

/*****************************************************************************/
/**
 * @brief APU and RPU power control interrupt handler
 *
 * @param PwrCtrlStatus	Interrupt status to request core power down.
 * @param PwrCtrlMask	Interrupt mask
 *
 * @return XST_SUCCESS on success, XST_FAILURE or error code during failure
 *****************************************************************************/
static XStatus XPm_DispatchPwrCtrlHandler(u32 PwrCtrlStatus, u32 PwrCtrlMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;
	XPm_Core *Core;

	PmInfo("PwrCtrlStatus = 0x%x, PwrCtrlMask = 0x%x\r\n", PwrCtrlStatus, PwrCtrlMask);

	for (Index = 0U; Index < ARRAY_SIZE(ApuSleepHandlerTable); Index++) {
		/* Check for PwrCtrlStatus 1 and PwrCtrlMask is 0.
		 * PwrCtrlStatus indicates that there is an interrupt request to power down the core.
		 * PwrCtrlMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 */
		if ((CHECK_BIT(PwrCtrlStatus, ApuSleepHandlerTable[Index].Mask)) &&
		    !(CHECK_BIT(PwrCtrlMask, ApuSleepHandlerTable[Index].Mask))) {
			/* Get core data for setting the power state */
			Core = (XPm_Core *)XPmDevice_GetById(ApuSleepHandlerTable[Index].DeviceId);
			if (NULL == Core) {
				PmErr("Missing core data for device ID 0x%x\r\n", ApuSleepHandlerTable[Index].DeviceId);
				Status = XST_INVALID_PARAM;
				goto done;
			}

			if ((u32)XPM_DEVSTATE_PENDING_PWR_DWN == Core->Device.Node.State) {
				Status = XPmCore_ReleaseFromSubsys(Core);
				if (XST_SUCCESS != Status) {
					PmErr("Failed to release core 0x%x from subsystem!\r\n", Core->Device.Node.Id);
				}
				/* Defer the failure exit until the interrupt has been cleared and masked. */
				/* Ack the service */
				XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS, ApuSleepHandlerTable[Index].Mask);
				XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_DIS, ApuSleepHandlerTable[Index].Mask);

				if (XST_SUCCESS != Status) {
					PmErr("Failed to release core 0x%x: 0x%x\n", Core->Device.Node.Id, Status);
					goto done;
				}
			} else {
				PmInfo("Core 0x%x is not in pending power down state (state: 0x%x)\r\n",
					Core->Device.Node.Id, Core->Device.Node.State);

				/* Call power down handler */
				Status = XPmCore_PowerDown(Core);
				if (XST_SUCCESS != Status) {
					PmErr("Failed to power down core 0x%x!\r\n", Core->Device.Node.Id);
					goto done;
				}

				/* Ack the service */
				XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS, ApuSleepHandlerTable[Index].Mask);
				XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_DIS, ApuSleepHandlerTable[Index].Mask);

#if defined(XILPM_NG_FPD_OFF)
				/*
				 * The core has executed WFI, so nothing in the subsystem runs
				 * any more and the scheduled requirements can be applied.
				 */
				Status = ApplyScheduledReqmOnSuspend(Core);
				if (XST_SUCCESS != Status) {
					PmErr("Failed to apply scheduled reqm for core 0x%x: 0x%x\r\n",
						Core->Device.Node.Id, Status);
					goto done;
				}
#endif
			}
		}
	}

	for (Index = 0U; Index < ARRAY_SIZE(RpuSleepHandlerTable); Index++) {
		/* Check for PwrCtrlStatus 1 and PwrCtrlMask is 0.
		 * PwrCtrlStatus indicates that there is an interrupt request to power down the core.
		 * PwrCtrlMask indicates if the interrupt is masked or not. If it is 0 it is unmasked.
		 */
		if ((CHECK_BIT(PwrCtrlStatus, RpuSleepHandlerTable[Index].Mask)) &&
		    !(CHECK_BIT(PwrCtrlMask, RpuSleepHandlerTable[Index].Mask))) {
			/* Call power down handler */
			Status = XPmPower_RpuDirectPwrDwn(RpuSleepHandlerTable[Index].Args);
			if (XST_SUCCESS != Status) {
				PmErr("Failed to power down RPU core 0x%x: 0x%x\r\n",
				      RpuSleepHandlerTable[Index].DeviceId, Status);
			}

			/* Ack the service */
			XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS, RpuSleepHandlerTable[Index].Mask);
			XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_DIS, RpuSleepHandlerTable[Index].Mask);

			/* Get core data for setting the power state */
			Core = (XPm_Core *)XPmDevice_GetById(RpuSleepHandlerTable[Index].DeviceId);
			if (NULL == Core) {
				PmErr("Missing core data for device ID 0x%x\r\n",
				      RpuSleepHandlerTable[Index].DeviceId);
				Status = XST_INVALID_PARAM;
				goto done;
			}
			DISABLE_WFI(Core->SleepMask);

			/* Call XPmCore_AfterDirectPwrDwn to set the core power state */
			Status = XPmCore_AfterDirectPwrDwn(Core);
		}
	}
done:
	return Status;
}
