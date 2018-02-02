/******************************************************************************
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
static u32 InterruptRegsiter;

/**
 * This list of IDs enables re-ordering of Events for GPI1 as per user's priority
 */
static u32 Gpi1EventIdList[] = {
	XPFW_EV_APB_AIB_ERROR,
	XPFW_EV_AXI_AIB_ERROR,
	XPFW_EV_ERROR_2,
	XPFW_EV_ERROR_1,
	XPFW_EV_ACPU_3_DBG_PWRUP,
	XPFW_EV_ACPU_2_DBG_PWRUP,
	XPFW_EV_ACPU_1_DBG_PWRUP,
	XPFW_EV_ACPU_0_DBG_PWRUP,
	XPFW_EV_FPD_WAKE_GIC_PROXY,
	XPFW_EV_MIO_WAKE_5,
	XPFW_EV_MIO_WAKE_4,
	XPFW_EV_MIO_WAKE_3,
	XPFW_EV_MIO_WAKE_2,
	XPFW_EV_MIO_WAKE_1,
	XPFW_EV_MIO_WAKE_0,
	XPFW_EV_DAP_RPU_WAKE,
	XPFW_EV_DAP_FPD_WAKE,
	XPFW_EV_USB_1_WAKE,
	XPFW_EV_USB_0_WAKE,
	XPFW_EV_R5_1_WAKE,
	XPFW_EV_R5_0_WAKE,
	XPFW_EV_ACPU_3_WAKE,
	XPFW_EV_ACPU_2_WAKE,
	XPFW_EV_ACPU_1_WAKE,
	XPFW_EV_ACPU_0_WAKE
};

static u32 Gpi2EventIdList[] = {
	XPFW_EV_VCC_INT_FP_DISCONNECT,
	XPFW_EV_VCC_INT_DISCONNECT,
	XPFW_EV_VCC_AUX_DISCONNECT,
	XPFW_EV_DBG_ACPU3_RST_REQ,
	XPFW_EV_DBG_ACPU2_RST_REQ,
	XPFW_EV_DBG_ACPU1_RST_REQ,
	XPFW_EV_DBG_ACPU0_RST_REQ,
	XPFW_EV_CP_ACPU3_RST_REQ,
	XPFW_EV_CP_ACPU2_RST_REQ,
	XPFW_EV_CP_ACPU1_RST_REQ,
	XPFW_EV_CP_ACPU0_RST_REQ,
	XPFW_EV_DBG_RCPU1_RST_REQ,
	XPFW_EV_DBG_RCPU0_RST_REQ,
	XPFW_EV_R5_1_SLEEP,
	XPFW_EV_R5_0_SLEEP,
	XPFW_EV_ACPU_3_SLEEP,
	XPFW_EV_ACPU_2_SLEEP,
	XPFW_EV_ACPU_1_SLEEP,
	XPFW_EV_ACPU_0_SLEEP
};

static u32 Gpi3EventIdList[] = {
	XPFW_EV_PL_GPI_31,
	XPFW_EV_PL_GPI_30,
	XPFW_EV_PL_GPI_29,
	XPFW_EV_PL_GPI_28,
	XPFW_EV_PL_GPI_27,
	XPFW_EV_PL_GPI_26,
	XPFW_EV_PL_GPI_25,
	XPFW_EV_PL_GPI_24,
	XPFW_EV_PL_GPI_23,
	XPFW_EV_PL_GPI_22,
	XPFW_EV_PL_GPI_21,
	XPFW_EV_PL_GPI_20,
	XPFW_EV_PL_GPI_19,
	XPFW_EV_PL_GPI_18,
	XPFW_EV_PL_GPI_17,
	XPFW_EV_PL_GPI_16,
	XPFW_EV_PL_GPI_15,
	XPFW_EV_PL_GPI_14,
	XPFW_EV_PL_GPI_13,
	XPFW_EV_PL_GPI_12,
	XPFW_EV_PL_GPI_11,
	XPFW_EV_PL_GPI_10,
	XPFW_EV_PL_GPI_9,
	XPFW_EV_PL_GPI_8,
	XPFW_EV_PL_GPI_7,
	XPFW_EV_PL_GPI_6,
	XPFW_EV_PL_GPI_5,
	XPFW_EV_PL_GPI_4,
	XPFW_EV_PL_GPI_3,
	XPFW_EV_PL_GPI_2,
	XPFW_EV_PL_GPI_1,
	XPFW_EV_PL_GPI_0,
};

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
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(Gpi1EventIdList); Index++) {
		u32 RegMask = XPfw_EventGetRegMask(Gpi1EventIdList[Index]);
		u32 GpiRegVal = XPfw_Read32(PMU_IOMODULE_GPI1);

		if ((GpiRegVal & RegMask) == RegMask) {
			/* Dispatch the event to Registered Modules */
			XStatus Status = XPfw_CoreDispatchEvent(Gpi1EventIdList[Index]);

			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: "
						"Failed to dispatch Event ID: %lu\r\n",
						Gpi1EventIdList[Index]);
			}
		}
	}
}

static void XPfw_InterruptGpi2Handler(void)
{
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(Gpi2EventIdList); Index++) {
		u32 RegMask = XPfw_EventGetRegMask(Gpi2EventIdList[Index]);
		u32 GpiRegVal = XPfw_Read32(PMU_IOMODULE_GPI2);

		if ((GpiRegVal & RegMask) == RegMask) {
			/* Dispatch the event to Registered Modules */
			XStatus Status = XPfw_CoreDispatchEvent(Gpi2EventIdList[Index]);

			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: "
						"Failed to dispatch Event ID: %lu\r\n",
						Gpi2EventIdList[Index]);
			}
		}
	}
}

static void XPfw_InterruptGpi3Handler(void)
{
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(Gpi3EventIdList); Index++) {
		u32 RegMask = XPfw_EventGetRegMask(Gpi3EventIdList[Index]);
		u32 GpiRegVal = XPfw_Read32(PMU_IOMODULE_GPI3);

		if ((GpiRegVal & RegMask) == RegMask) {
			/* Dispatch the event to Registered Modules */
			XStatus Status = XPfw_CoreDispatchEvent(Gpi3EventIdList[Index]);

			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: "
						"Failed to dispatch Event ID: %lu\r\n",
						Gpi3EventIdList[Index]);
			}
		}
	}
}

static void XPfw_InterruptRtcAlaramHandler(void)
{
	(void)XPfw_CoreDispatchEvent(XPFW_EV_RTC_ALARM);
}

static void XPfw_InterruptRtcSecondsmHandler(void)
{
	(void)XPfw_CoreDispatchEvent(XPFW_EV_RTC_SECONDS);
}

static void XPfw_Pit1Handler(void)
{
	(void)XPfw_CoreTickHandler();
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
	InterruptRegsiter = PMU_IOMODULE_IRQ_ENABLE_CSU_PMU_SEC_LOCK_MASK;
}

void XPfw_InterruptStart(void)
{
	XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, InterruptRegsiter);
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
		/* We shouldnt be here before Init, but we are.. So disable the Interrupts */
		/* Init will enable only the required interrupts */
		XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, 0U);
		XPfw_Write32(PMU_IOMODULE_IRQ_ACK, 0xffffffffU);
	}
}

void XPfw_InterruptDisable(u32 Mask)
{
	InterruptRegsiter = InterruptRegsiter & (~Mask);
	XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, InterruptRegsiter);
}

void XPfw_InterruptEnable(u32 Mask)
{
	InterruptRegsiter = InterruptRegsiter | Mask;
	XPfw_Write32(PMU_IOMODULE_IRQ_ENABLE, InterruptRegsiter);
}
