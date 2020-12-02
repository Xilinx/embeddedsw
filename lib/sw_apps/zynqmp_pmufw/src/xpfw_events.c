/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_events.h"
#include "xpfw_interrupts.h"

static struct XPfw_Event_t EventTable[XPFW_EV_MAX] = {
	[0U] = { .RegMask = MASK32_ALL_LOW, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MB_FAULT] = { .RegMask = MASK32_ALL_HIGH, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_APB_AIB_ERROR] = { .RegMask =  PMU_IOMODULE_GPI1_APB_AIB_ERROR_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_AXI_AIB_ERROR] = { .RegMask =  PMU_IOMODULE_GPI1_AXI_AIB_ERROR_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ERROR_2] = { .RegMask =  PMU_IOMODULE_GPI1_ERROR_2_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ERROR_1] = { .RegMask =  PMU_IOMODULE_GPI1_ERROR_1_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_3_DBG_PWRUP] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_3_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_2_DBG_PWRUP] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_2_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_1_DBG_PWRUP] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_1_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_0_DBG_PWRUP] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_0_DBG_PWRUP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_FPD_WAKE_GIC_PROXY] = { .RegMask =  PMU_IOMODULE_GPI1_FPD_WAKE_GIC_PROXY_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_5] = { .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_5_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_4] = { .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_4_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_3] = { .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_3_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_2] = { .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_2_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_1] = { .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_1_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_MIO_WAKE_0] = { .RegMask =  PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DAP_RPU_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_DAP_RPU_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DAP_FPD_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_DAP_FPD_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_USB_1_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_USB_1_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_USB_0_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_USB_0_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_1_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_R5_1_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_0_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_R5_0_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_3_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_2_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_1_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_0_WAKE] = { .RegMask =  PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_VCC_INT_FP_DISCONNECT] = { .RegMask =  PMU_IOMODULE_GPI2_VCC_INT_FP_DISCONNECT_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_VCC_INT_DISCONNECT] = { .RegMask =  PMU_IOMODULE_GPI2_VCC_INT_DISCONNECT_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_VCC_AUX_DISCONNECT] = { .RegMask =  PMU_IOMODULE_GPI2_VCC_AUX_DISCONNECT_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU3_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU3_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU2_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU2_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU1_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU1_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_ACPU0_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_DBG_ACPU0_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU3_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU3_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU2_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU2_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU1_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU1_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_CP_ACPU0_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_CP_ACPU0_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_RCPU1_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_DBG_RCPU1_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_DBG_RCPU0_RST_REQ] = { .RegMask =  PMU_IOMODULE_GPI2_DBG_RCPU0_RST_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_1_SLEEP] = { .RegMask =  PMU_IOMODULE_GPI2_R5_1_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_R5_0_SLEEP] = { .RegMask =  PMU_IOMODULE_GPI2_R5_0_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_3_SLEEP] = { .RegMask =  PMU_IOMODULE_GPI2_ACPU_3_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_2_SLEEP] = { .RegMask =  PMU_IOMODULE_GPI2_ACPU_2_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_1_SLEEP] = { .RegMask =  PMU_IOMODULE_GPI2_ACPU_1_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_ACPU_0_SLEEP] = { .RegMask =  PMU_IOMODULE_GPI2_ACPU_0_SLEEP_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_31] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_31_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_30] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_30_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_29] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_29_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_28] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_28_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_27] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_27_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_26] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_26_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_25] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_25_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_24] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_24_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_23] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_23_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_22] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_22_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_21] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_21_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_20] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_20_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_19] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_19_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_18] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_18_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_17] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_17_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_16] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_16_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_15] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_15_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_14] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_14_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_13] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_13_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_12] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_12_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_11] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_11_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_10] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_10_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_9] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_9_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_8] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_8_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_7] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_7_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_6] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_6_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_5] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_5_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_4] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_4_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_3] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_3_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_2] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_2_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_1] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_1_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_PL_GPI_0] = { .RegMask =  PMU_IOMODULE_GPI3_PL_GPI_0_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_RTC_SECONDS] = { .RegMask =  PMU_IOMODULE_IRQ_PENDING_RTC_EVERY_SECOND_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_RTC_ALARM] = { .RegMask =  PMU_IOMODULE_IRQ_PENDING_RTC_ALARM_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_REQ_PWRUP] = { .RegMask =  PMU_IOMODULE_IRQ_PENDING_PWR_UP_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_REQ_PWRDN] = { .RegMask =  PMU_IOMODULE_IRQ_PENDING_PWR_DN_REQ_MASK, .ModMask = MASK32_ALL_LOW },
	[XPFW_EV_REQ_ISOLATION] = { .RegMask =  PMU_IOMODULE_IRQ_PENDING_ISO_REQ_MASK, .ModMask = MASK32_ALL_LOW },
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

	switch (XPfw_EventGetType(EventId)) {
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

	switch (XPfw_EventGetType(EventId)) {
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

u32 XPfw_EventGetType(u32 EventId)
{
	u32 EventType;

	if (EventId == XPFW_EV_MB_FAULT) {
		EventType = XPFW_EV_TYPE_GPI0;
	} else if ((EventId >= XPFW_EV_APB_AIB_ERROR) && (EventId <= XPFW_EV_ACPU_0_WAKE)) {
		EventType = XPFW_EV_TYPE_GPI1;
	} else if ((EventId >= XPFW_EV_VCC_INT_FP_DISCONNECT) && (EventId <= XPFW_EV_ACPU_0_SLEEP)) {
		EventType = XPFW_EV_TYPE_GPI2;
	} else if ((EventId >= XPFW_EV_PL_GPI_31) && (EventId <= XPFW_EV_PL_GPI_0)) {
		EventType = XPFW_EV_TYPE_GPI3;
	} else if((EventId >= XPFW_EV_RTC_SECONDS) && (EventId <= XPFW_EV_RTC_ALARM)) {
		EventType = XPFW_EV_TYPE_RTC;
	} else if ((EventId >= XPFW_EV_REQ_PWRUP) && (EventId <= XPFW_EV_REQ_ISOLATION)) {
		EventType = XPFW_EV_TYPE_GEN;
	} else {
		EventType = XPFW_EV_TYPE_INVALID;
	}

	return EventType;
}
