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

#include "xpfw_events.h"
#include "xpfw_interrupts.h"

static struct XPfw_Event_t EventTable[XPFW_EV_MAX] = {
	[0U] = {.Type = (u8)0U, .RegMask = MASK32_ALL_LOW, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MB_FAULT] = { .Type = XPFW_EV_TYPE_GPI0, .RegMask = MASK32_ALL_HIGH, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_APB_AIB_ERROR] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_APB_AIB_ERROR_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_AXI_AIB_ERROR] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_AXI_AIB_ERROR_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ERROR_2] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ERROR_2_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ERROR_1] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ERROR_1_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_3_DBG_PWRUP] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_3_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_2_DBG_PWRUP] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_2_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_1_DBG_PWRUP] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_1_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_0_DBG_PWRUP] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_0_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_FPD_WAKE_GIC_PROXY] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_FPD_WAKE_GIC_PROXY_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_5] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_5_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_4] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_4_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_3] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_3_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_2] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_2_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_1] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_1_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_0] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DAP_RPU_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_DAP_RPU_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DAP_FPD_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_DAP_FPD_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_USB_1_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_USB_1_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_USB_0_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_USB_0_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_1_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_R5_1_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_0_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_R5_0_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_3_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_2_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_1_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_0_WAKE] = { .Type = XPFW_EV_TYPE_GPI1, .RegMask =  PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_VCC_INT_FP_DISCONNECT] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_VCC_INT_FP_DISCONNECT_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_VCC_INT_DISCONNECT] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_VCC_INT_DISCONNECT_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_VCC_AUX_DISCONNECT] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_VCC_AUX_DISCONNECT_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU3_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU3_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU2_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU2_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU1_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU1_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU0_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU0_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU3_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU3_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU2_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU2_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU1_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU1_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU0_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU0_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_RCPU1_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_DBG_RCPU1_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_RCPU0_RST_REQ] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_DBG_RCPU0_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_1_SLEEP] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_R5_1_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_0_SLEEP] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_R5_0_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_3_SLEEP] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_ACPU_3_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_2_SLEEP] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_ACPU_2_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_1_SLEEP] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_ACPU_1_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_0_SLEEP] = { .Type = XPFW_EV_TYPE_GPI2, .RegMask =  PMU_IOMODULE_GPI2_ACPU_0_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_31] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_31_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_30] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_30_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_29] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_29_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_28] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_28_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_27] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_27_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_26] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_26_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_25] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_25_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_24] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_24_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_23] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_23_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_22] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_22_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_21] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_21_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_20] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_20_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_19] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_19_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_18] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_18_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_17] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_17_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_16] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_16_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_15] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_15_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_14] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_14_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_13] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_13_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_12] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_12_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_11] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_11_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_10] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_10_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_9] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_9_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_8] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_8_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_7] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_7_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_6] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_6_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_5] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_5_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_4] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_4_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_3] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_3_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_2] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_2_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_1] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_1_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_0] = { .Type = XPFW_EV_TYPE_GPI3, .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_0_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_RTC_SECONDS] = { .Type = XPFW_EV_TYPE_RTC, .RegMask =  PMU_IOMODULE_IRQ_PENDING_RTC_EVERY_SECOND_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_RTC_ALARM] = { .Type = XPFW_EV_TYPE_RTC, .RegMask =  PMU_IOMODULE_IRQ_PENDING_RTC_ALARM_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_REQ_PWRUP] = { .Type = XPFW_EV_TYPE_GEN, .RegMask =  PMU_IOMODULE_IRQ_PENDING_PWR_UP_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_REQ_PWRDN] = { .Type = XPFW_EV_TYPE_GEN, .RegMask =  PMU_IOMODULE_IRQ_PENDING_PWR_DN_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_REQ_ISOLATION] = { .Type = XPFW_EV_TYPE_GEN, .RegMask =  PMU_IOMODULE_IRQ_PENDING_ISO_REQ_MASK, .ModMask = MASK32_ALL_LOW },
};

u32 XPfw_EventGetModMask(u32 EventId)
{
	u32 Mask;

	if (EventId < XPFW_EV_MAX) {
		Mask = EventTable[EventId].ModMask;
	} else {
		Mask = 0U;
	}

	return Mask;
}

u32 XPfw_EventGetRegMask(u32 EventId)
{
	u32 Mask;

	if (EventId < XPFW_EV_MAX) {
		Mask = EventTable[EventId].RegMask;
	} else {
		Mask = MASK32_ALL_HIGH;
	}

	return Mask;
}

static XStatus XPfw_EventEnable(u32 EventId)
{
	XStatus Status;

	switch (EventTable[EventId].Type) {
	case XPFW_EV_TYPE_GEN:
		/* Enable REQ_PWRUP /PWR_DN bit in IRQ_ENABLE */
		XPfw_InterruptEnable(EventTable[EventId].RegMask);
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI0:
		/* Nothing to do for GPI0. These are enabled by default */
		/* Enable GPI0 bit in IRQ_ENABLE */
		XPfw_InterruptEnable(PMU_IOMODULE_IRQ_ENABLE_GPI0_MASK);
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI1:
		/* Enable the event in GPI1 ENABLE register */
		XPfw_RMW32(PMU_LOCAL_GPI1_ENABLE, EventTable[EventId].RegMask, EventTable[EventId].RegMask);
		/* Enable the GPI1 bit in IRQ ENABLE */
		XPfw_InterruptEnable(PMU_IOMODULE_IRQ_ENABLE_GPI1_MASK);
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI2:
		/* Enable the event in GPI2 ENABLE register */
		XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, EventTable[EventId].RegMask, EventTable[EventId].RegMask);
		/* Enable the GPI2 bit in IRQ ENABLE */
		XPfw_InterruptEnable(PMU_IOMODULE_IRQ_ENABLE_GPI2_MASK);
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI3:
		/* Enable the event in GPI3 ENABLE register */
		XPfw_RMW32(PMU_LOCAL_GPI3_ENABLE, EventTable[EventId].RegMask, EventTable[EventId].RegMask);
		/* Enable the GPI3 bit in IRQ ENABLE */
		XPfw_InterruptEnable(PMU_IOMODULE_IRQ_ENABLE_GPI3_MASK);
		Status = XST_SUCCESS;
		break;
	case XPFW_EV_TYPE_RTC:
		/* Enable RTC in IRQENABLE Register */
		XPfw_InterruptEnable(EventTable[EventId].RegMask);
		Status = XST_SUCCESS;
		break;

	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static XStatus XPfw_EventDisable(u32 EventId)
{
	XStatus Status;

	switch (EventTable[EventId].Type) {
	case XPFW_EV_TYPE_GEN:
		/* Enable REQ_PWRUP /PWR_DN bit in IRQ_ENABLE */
		XPfw_InterruptDisable(EventTable[EventId].RegMask);
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI0:
		/* Nothing to do for GPI0. These are enabled by default */
		/* Disable GPI0 bit in IRQ_ENABLE */
		XPfw_InterruptDisable(PMU_IOMODULE_IRQ_ENABLE_GPI0_MASK);
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI1:
		/* Disable the event in GPI1 ENABLE register */
		XPfw_RMW32(PMU_LOCAL_GPI1_ENABLE, EventTable[EventId].RegMask, 0U);
		/* Disable the GPI1 bit in IRQ ENABLE */
		if (0U == XPfw_Read32(PMU_LOCAL_GPI1_ENABLE)) {
			XPfw_InterruptDisable(PMU_IOMODULE_IRQ_ENABLE_GPI1_MASK);
		}
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI2:
		/* Disable the event in GPI2 ENABLE register */
		XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, EventTable[EventId].RegMask, 0U);
		/* Disable the GPI2 bit in IRQ ENABLE */
		if (0U == XPfw_Read32(PMU_LOCAL_GPI2_ENABLE)) {
			XPfw_InterruptDisable(PMU_IOMODULE_IRQ_ENABLE_GPI2_MASK);
		}
		Status = XST_SUCCESS;
		break;

	case XPFW_EV_TYPE_GPI3:
		/* Disable the event in GPI3 ENABLE register */
		XPfw_RMW32(PMU_LOCAL_GPI3_ENABLE, EventTable[EventId].RegMask, 0U);
		/* Enable the GPI3 bit in IRQ ENABLE */
		if (0U == XPfw_Read32(PMU_LOCAL_GPI3_ENABLE)) {
			XPfw_InterruptDisable(PMU_IOMODULE_IRQ_ENABLE_GPI3_MASK);
		}
		Status = XST_SUCCESS;
		break;
	case XPFW_EV_TYPE_RTC:
		/* Enable RTC in IRQENABLE Register */
		XPfw_InterruptDisable(EventTable[EventId].RegMask);
		Status = XST_SUCCESS;
		break;

	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

XStatus XPfw_EventAddOwner(u8 ModId, u32 EventId)
{
	XStatus Status;

	if ((EventId < XPFW_EV_MAX) && (ModId < 32U)) {
		EventTable[EventId].ModMask = EventTable[EventId].ModMask | ((u32)1U<<ModId);
		Status = XPfw_EventEnable(EventId);
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

XStatus XPfw_EventRemoveOwner(u32 ModId, u32 EventId)
{
	XStatus Status;
	if ((EventId < XPFW_EV_MAX) && (ModId < 32U)) {
		/* Update the ModMask for Event */
		EventTable[EventId].ModMask = EventTable[EventId].ModMask & (~((u32)1U<<ModId));

		/* Check if the Event has been de-registered by All Modules */
		if (0U == EventTable[EventId].ModMask) {
			/* Disable the corresponding Event from occurring */
			Status = XPfw_EventDisable(EventId);
		} else {
			/* We successfully completed de-registration */
			Status = XST_SUCCESS;
		}
	} else {
		/* Failed due to Invalid EventId or Mod Id */
		Status = XST_FAILURE;
	}

	return Status;
}

u32 XPfw_EventGetId(u32 EventType, u32 Mask)
{
	u32 Index;

	for (Index = 0U; Index<XPFW_EV_MAX;Index++) {
		if ((EventTable[Index].RegMask == Mask) &&
		    (EventTable[Index].Type == EventType)) {
			break;
		}
	}

	return Index;
}

u32 XPfw_EventGetType(u32 EventId)
{
	u32 EventType;

	if (EventId < XPFW_EV_MAX) {
		EventType = EventTable[EventId].Type;
	} else {
		EventType = XPFW_EV_TYPE_INVALID;
	}

	return EventType;
}
