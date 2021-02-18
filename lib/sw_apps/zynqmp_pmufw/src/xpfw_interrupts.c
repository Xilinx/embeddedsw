/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_interrupts.h"
#include "xpfw_events.h"
#include "xpfw_core.h"
#include "xil_exception.h"
#include "xpfw_error_manager.h"
#include "pmu_lmb_bram.h"
/**
 * InterruptRegister holds the state of the IRQ Enable Register
 *
 * @note IRQ_ENABLE register is write-only, So its state is stored here
 */
static u32 InterruptRegister;

static void XPfw_NullHandler(void)
{
	/**
	 *  This should never be called.
	 */
	XPfw_Printf(DEBUG_ERROR,"Error: NullHandler Triggered!\r\n");
}

static void XPfw_PmuRamCEHandler(void)
{
	XPfw_Printf(DEBUG_DETAILED,"PMU RAM Correctable ECC occurred!\r\n");

	/* Clear the interrupt */
	XPfw_Write32(PMU_LMB_BRAM_ECC_STATUS_REG, PMU_LMB_BRAM_CE_MASK);
}

static void XPfw_InterruptPwrUpHandler(void)
{
	XStatus Status = XPfw_CoreDispatchEvent(XPFW_EV_REQ_PWRUP);

	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch Event ID:"
				" %d\r\n",XPFW_EV_REQ_PWRUP);
	}
}

static void XPfw_InterruptPwrDnHandler(void)
{
	XStatus Status = XPfw_CoreDispatchEvent(XPFW_EV_REQ_PWRDN);

	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch Event ID:"
				" %d\r\n",XPFW_EV_REQ_PWRDN);
	}
}

static void XPfw_InterruptIsolationHandler(void)
{
	XStatus Status = XPfw_CoreDispatchEvent(XPFW_EV_REQ_ISOLATION);

	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch Event ID:"
				" %d\r\n",XPFW_EV_REQ_ISOLATION);
	}
}

static void XPfw_InterruptGpi0Handler(void)
{
	XStatus Status = XPfw_CoreDispatchEvent(XPFW_EV_MB_FAULT);

	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch Event ID:"
				" %d\r\n",XPFW_EV_MB_FAULT);
	}
}

static void XPfw_InterruptGpi1Handler(void)
{
	u32 EventId;

	for (EventId = XPFW_EV_APB_AIB_ERROR; EventId <= XPFW_EV_ACPU_0_WAKE;
			++EventId) {
		u32 RegMask = XPfw_EventGetRegMask(EventId);
		u32 GpiRegVal = XPfw_Read32(PMU_IOMODULE_GPI1);

		if ((GpiRegVal & RegMask) == RegMask) {
			/* Dispatch the event to Registered Modules */
			XStatus Status = XPfw_CoreDispatchEvent(EventId);

			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: "
						"Failed to dispatch Event ID: %lu\r\n", EventId);
			}
		}
	}
}

static void XPfw_InterruptGpi2Handler(void)
{
	u32 EventId;

	for (EventId = XPFW_EV_VCC_INT_FP_DISCONNECT;
			EventId <= XPFW_EV_ACPU_0_SLEEP; ++EventId) {
		u32 RegMask = XPfw_EventGetRegMask(EventId);
		u32 GpiRegVal = XPfw_Read32(PMU_IOMODULE_GPI2);

		if ((GpiRegVal & RegMask) == RegMask) {
			/* Dispatch the event to Registered Modules */
			XStatus Status = XPfw_CoreDispatchEvent(EventId);

			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: "
						"Failed to dispatch Event ID: %lu\r\n", EventId);
			}
		}
	}
}

static void XPfw_InterruptGpi3Handler(void)
{
	u32 EventId;

	for (EventId = XPFW_EV_PL_GPI_31;
			EventId <= XPFW_EV_PL_GPI_0; ++EventId) {
		u32 RegMask = XPfw_EventGetRegMask(EventId);
		u32 GpiRegVal = XPfw_Read32(PMU_IOMODULE_GPI3);

		if ((GpiRegVal & RegMask) == RegMask) {
			/* Dispatch the event to Registered Modules */
			XStatus Status = XPfw_CoreDispatchEvent(EventId);

			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: "
						"Failed to dispatch Event ID: %lu\r\n", EventId);
			}
		}
	}
}

static void XPfw_InterruptRtcAlaramHandler(void)
{
	if (XST_SUCCESS != XPfw_CoreDispatchEvent(XPFW_EV_RTC_ALARM)) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch "
				"Event ID: %d\r\n", XPFW_EV_RTC_ALARM);
	}
}

static void XPfw_InterruptRtcSecondsmHandler(void)
{
	if (XST_SUCCESS != XPfw_CoreDispatchEvent(XPFW_EV_RTC_SECONDS)) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch "
				"Event ID: %d\r\n", XPFW_EV_RTC_SECONDS);
	}
}

static void XPfw_Pit1Handler(void)
{
	XPfw_CoreTickHandler();
}

static void XPfw_Ipi0Handler(void)
{
	u32 Mask;
	XStatus Status;

	Mask = XPfw_Read32(IPI_PMU_0_ISR);
	Status = XPfw_CoreDispatchIpi(0U, Mask);
	XPfw_Write32(IPI_PMU_0_ISR, Mask);

	/* If no Mod has registered for IPI, Ack it to prevent re-triggering */
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_ERROR,"Error: Unhandled IPI received\r\n");
	}
}

static void XPfw_Ipi1Handler(void)
{
	u32 Mask;
	XStatus Status;

	Mask = XPfw_Read32(IPI_PMU_1_ISR);
	Status = XPfw_CoreDispatchIpi(1U, Mask);
	XPfw_Write32(IPI_PMU_1_ISR, Mask);

	/* If no Mod has registered for IPI, Ack it to prevent re-triggering */
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_ERROR,"Error: Unhandled IPI received\r\n");
	}
}

static void XPfw_Ipi2Handler(void)
{
	u32 Mask;
	XStatus Status;

	Mask = XPfw_Read32(IPI_PMU_2_ISR);
	Status = XPfw_CoreDispatchIpi(2U, Mask);
	XPfw_Write32(IPI_PMU_2_ISR, Mask);

	/* If no Mod has registered for IPI, Ack it to prevent re-triggering */
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_ERROR,"Error: Unhandled IPI received\r\n");
	}
}

static void XPfw_Ipi3Handler(void)
{
	u32 Mask;
	XStatus Status;

	Mask = XPfw_Read32(IPI_PMU_3_ISR);
	Status = XPfw_CoreDispatchIpi(3U, Mask);
	XPfw_Write32(IPI_PMU_3_ISR, Mask);

	/* If no Mod has registered for IPI, Ack it to prevent re-triggering */
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_ERROR,"Error: Unhandled IPI received\r\n");
	}
}

static struct HandlerTable g_TopLevelInterruptTable[] = {
	{PMU_IOMODULE_IRQ_PENDING_GPI1_MASK, XPfw_InterruptGpi1Handler},
	{PMU_IOMODULE_IRQ_PENDING_IPI0_MASK, XPfw_Ipi0Handler},
	{PMU_IOMODULE_IRQ_PENDING_RTC_ALARM_MASK, XPfw_InterruptRtcAlaramHandler},
	{PMU_IOMODULE_IRQ_PENDING_RTC_EVERY_SECOND_MASK, XPfw_InterruptRtcSecondsmHandler},
	{PMU_IOMODULE_IRQ_PENDING_CORRECTABLE_ECC_MASK, XPfw_PmuRamCEHandler},
	{PMU_IOMODULE_IRQ_PENDING_INV_ADDR_MASK, XPfw_NullHandler},
	{PMU_IOMODULE_IRQ_PENDING_IPI3_MASK, XPfw_Ipi3Handler},
	{PMU_IOMODULE_IRQ_PENDING_IPI2_MASK, XPfw_Ipi2Handler},
	{PMU_IOMODULE_IRQ_PENDING_IPI1_MASK, XPfw_Ipi1Handler},
	{PMU_IOMODULE_IRQ_PENDING_PWR_UP_REQ_MASK, XPfw_InterruptPwrUpHandler},
	{PMU_IOMODULE_IRQ_PENDING_PWR_DN_REQ_MASK, XPfw_InterruptPwrDnHandler},
	{PMU_IOMODULE_IRQ_PENDING_ISO_REQ_MASK, XPfw_InterruptIsolationHandler},
	{PMU_IOMODULE_IRQ_PENDING_SW_RST_REQ_MASK, XPfw_NullHandler},
	{PMU_IOMODULE_IRQ_PENDING_HW_RST_REQ_MASK, XPfw_NullHandler},
	{PMU_IOMODULE_IRQ_PENDING_GPI3_MASK, XPfw_InterruptGpi3Handler},
	{PMU_IOMODULE_IRQ_PENDING_GPI2_MASK, XPfw_InterruptGpi2Handler},
	{PMU_IOMODULE_IRQ_PENDING_GPI0_MASK, XPfw_InterruptGpi0Handler},
	{PMU_IOMODULE_IRQ_PENDING_PIT3_MASK, XPfw_NullHandler},
	{PMU_IOMODULE_IRQ_PENDING_PIT2_MASK, XPfw_NullHandler},
	{PMU_IOMODULE_IRQ_PENDING_PIT1_MASK, XPfw_Pit1Handler},
	{PMU_IOMODULE_IRQ_PENDING_PIT0_MASK, XPfw_NullHandler},
	{PMU_IOMODULE_IRQ_PENDING_CSU_PMU_SEC_LOCK_MASK, XPfw_NullHandler}
};

void XPfw_InterruptInit(void)
{
	XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, 0U);
	Xil_ExceptionDisable();
	XPfw_Write32(PMU_IOMODULE_IRQ_ACK, 0xffffffffU);
	InterruptRegister = PMU_IOMODULE_IRQ_ENABLE_CSU_PMU_SEC_LOCK_MASK;
}

void XPfw_InterruptStart(void)
{
	XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, InterruptRegister);
	Xil_ExceptionEnable();
}

void XPfw_InterruptHandler(void)
{
	u32 l_IrqReg;
	u32 l_index;

	if (XST_SUCCESS == XPfw_CoreIsReady()) {
		/* Latch the IRQ_PENDING register into a local variable */
		l_IrqReg = XPfw_Read32(PMU_IOMODULE_IRQ_PENDING);

		/* Loop through the Handler Table and handle the trigger interrupts */
		for (l_index = 0U; l_index < ARRAYSIZE(g_TopLevelInterruptTable);
				l_index++) {
			if ((l_IrqReg & g_TopLevelInterruptTable[l_index].Mask)
					== g_TopLevelInterruptTable[l_index].Mask) {
				/* Call the Handler */
				g_TopLevelInterruptTable[l_index].Handler();
				/* ACK the Interrupt */
				XPfw_Write32(PMU_IOMODULE_IRQ_ACK,
						g_TopLevelInterruptTable[l_index].Mask);
			}
		}

		/* Disable and Enable PMU interrupts in PMU Global register.
		 * This will re-generated any interrupt which is generated while
		 * serving the other interrupt
		 */
		XPfw_PulseErrorInt();
	} else {
		/* We shouldn't be here before Init, but we are.. So disable the Interrupts */
		/* Init will enable only the required interrupts */
		XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, 0U);
		XPfw_Write32(PMU_IOMODULE_IRQ_ACK, 0xffffffffU);
	}
}

void XPfw_InterruptDisable(u32 Mask)
{
	InterruptRegister = InterruptRegister & (~Mask);
	XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, InterruptRegister);
}

void XPfw_InterruptEnable(u32 Mask)
{
	InterruptRegister = InterruptRegister | Mask;
	XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, InterruptRegister);
}
