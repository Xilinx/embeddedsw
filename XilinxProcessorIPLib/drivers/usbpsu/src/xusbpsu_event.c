/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_event.c
* @addtogroup usbpsu_api USBPSU APIs
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pm  03/03/20 First release
* 1.8	pm  01/07/20 Add versal hibernation support
*	pm  24/07/20 Fixed MISRA-C and Coverity warnings
* 1.12	pm  10/08/22 Update doxygen tag and addtogroup version
* 1.15  np  26/03/24 Add doxygen and editorial fixes
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xusbpsu_local.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
* Endpoint event handler.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
* @param	Event Endpoint Event occurred in the core.
*
* @return	None.
*
*
*****************************************************************************/
void XUsbPsu_EpEvent(struct XUsbPsu *InstancePtr,
		     const struct XUsbPsu_Event_Epevt *Event)
{
	struct XUsbPsu_Ep *Ept;
	u32 Epnum;

	Epnum = Event->Epnumber;
	Ept = &InstancePtr->eps[Epnum];

	if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
		return;
	}

	if ((Epnum == (u32)0U) || (Epnum == (u32)1U)) {
		XUsbPsu_Ep0Intr(InstancePtr, Event);
		return;
	}

	/* Handle other end point events */
	switch (Event->Endpoint_Event) {
		case XUSBPSU_DEPEVT_XFERCOMPLETE:
		case XUSBPSU_DEPEVT_XFERINPROGRESS:
			XUsbPsu_EpXferComplete(InstancePtr, Event);
			break;

		case XUSBPSU_DEPEVT_XFERNOTREADY:
			XUsbPsu_EpXferNotReady(InstancePtr, Event);
			break;

		default:
			/* Made for Misra-C Compliance. */
			break;
	}
}

/****************************************************************************/
/**
* Device event handler for device specific events.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
* @param	Event Device Event occurred in core.
*
* @return	None.
*
*
*****************************************************************************/
void XUsbPsu_DeviceEvent(struct XUsbPsu *InstancePtr,
			 const struct XUsbPsu_Event_Devt *Event)
{

	switch (Event->Type) {
		case XUSBPSU_DEVICE_EVENT_DISCONNECT:
			XUsbPsu_DisconnectIntr(InstancePtr);
			break;

		case XUSBPSU_DEVICE_EVENT_RESET:
			XUsbPsu_ResetIntr(InstancePtr);
			break;

		case XUSBPSU_DEVICE_EVENT_CONNECT_DONE:
			XUsbPsu_ConnDoneIntr(InstancePtr);
			break;

		case XUSBPSU_DEVICE_EVENT_WAKEUP:
			break;

		case XUSBPSU_DEVICE_EVENT_HIBER_REQ:
#ifdef XUSBPSU_HIBERNATION_ENABLE
			if (InstancePtr->HasHibernation == (u8)TRUE) {
				if (XUsbPsu_HibernationIntr(InstancePtr)
				    == XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
					xil_printf("Hibernation event failure\r\n");
#endif
				}
			}
#endif
			break;

		case XUSBPSU_DEVICE_EVENT_LINK_STATUS_CHANGE:
			XUsbPsu_LinkStsChangeIntr(InstancePtr,
						  Event->Event_Info);
			break;

		case XUSBPSU_DEVICE_EVENT_EOPF:
			break;

		case XUSBPSU_DEVICE_EVENT_SOF:
			break;

		case XUSBPSU_DEVICE_EVENT_ERRATIC_ERROR:
			break;

		case XUSBPSU_DEVICE_EVENT_CMD_CMPL:
			break;

		case XUSBPSU_DEVICE_EVENT_OVERFLOW:
			break;

		default:
			/* Made for Misra-C Compliance. */
			break;
	}
}

/****************************************************************************/
/**
* Processes events in an event buffer.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
*
* @return	None.
*
*
*****************************************************************************/
void XUsbPsu_EventBufferHandler(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_EvtBuffer *Evt;
	union XUsbPsu_Event Event = {0U};
	u32 RegVal;

	Evt = &InstancePtr->Evt;

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheInvalidateRange((INTPTR)Evt->BuffAddr,
					  XUSBPSU_EVENT_BUFFERS_SIZE);
	}

	while (Evt->Count > 0U) {
		Event.Raw = *(UINTPTR *)((UINTPTR)Evt->BuffAddr + Evt->Offset);

		/*
		 * Process the event received
		 */
		XUsbPsu_EventHandler(InstancePtr, &Event);

		/* don't process anymore events if core is hibernated */
		if (InstancePtr->IsHibernated == (u8)TRUE) {
			return;
		}

		Evt->Offset = (Evt->Offset + 4U) % XUSBPSU_EVENT_BUFFERS_SIZE;
		Evt->Count -= 4U;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0), 4U);
	}

	Evt->Count = 0U;
	Evt->Flags &= ~XUSBPSU_EVENT_PENDING;

	/* Unmask event interrupt */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GEVNTSIZ(0U));
	RegVal &= ~XUSBPSU_GEVNTSIZ_INTMASK;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTSIZ(0U), RegVal);
}

#ifdef XUSBPSU_HIBERNATION_ENABLE

/****************************************************************************/
/**
* Processes link state events for hibernation.
*
* @param	InstancePtr Pointer to the XUsbPsu instance to be worked
* 			on.
*
* @return	XST_SUCCESS else XST_FAILURE.
*
*
*****************************************************************************/
s32 XUsbPsu_HibernationStateIntr(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;
	u8 EnterHiber = 0U;
	XusbPsuLinkState LinkState;

	LinkState = (XusbPsuLinkState)XUsbPsu_GetLinkState(InstancePtr);

	switch (LinkState) {
		case XUSBPSU_LINK_STATE_RESET:
			RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCFG);
			RegVal &= ~XUSBPSU_DCFG_DEVADDR_MASK;
			XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCFG, RegVal);

			if (XUsbPsu_SetLinkState(InstancePtr,
						 XUSBPSU_LINK_STATE_CHANGE_RECOV)
			    == (s32)XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
				xil_printf("Failed to put link in Recovery\r\n");
#endif
				return (s32)XST_FAILURE;
			}
			break;
		case XUSBPSU_LINK_STATE_SS_DIS:
			RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
			RegVal &= ~XUSBPSU_DCTL_KEEP_CONNECT;
			XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);
			EnterHiber = 1U;
			break;
		case XUSBPSU_LINK_STATE_U3:
			/* enter hibernation again */
			EnterHiber = 1U;
			break;
#if defined (versal)
		case XUSBPSU_LINK_STATE_RESUME:
			/* In USB 2.0, to avoid hibernation interrupt at the time of connection
			 * clear KEEP_CONNECT bit.
			 */
			RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
			RegVal &= ~XUSBPSU_DCTL_KEEP_CONNECT;
			XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

			if (XUsbPsu_SetLinkState(InstancePtr,
						 XUSBPSU_LINK_STATE_CHANGE_RECOV)
			    == (s32)XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
				xil_printf("Failed to put link in Recovery\r\n");
#endif
				return (s32)XST_FAILURE;
			}
			break;
#endif
		default:
			if (XUsbPsu_SetLinkState(InstancePtr,
						 XUSBPSU_LINK_STATE_CHANGE_RECOV)
			    == (s32)XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
				xil_printf("Failed to put link in Recovery\r\n");
#endif
				return (s32)XST_FAILURE;
			}
			break;
	};

	if (XUsbPsu_RestoreEps(InstancePtr) == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	InstancePtr->IsHibernated = 0U;

	if (EnterHiber == 1U)  {
		if (XUsbPsu_HibernationIntr(InstancePtr)
		    == XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
			xil_printf("Handle hibernation event fail\r\n");
#endif
			return (s32)XST_FAILURE;
		}
		return (s32)XST_SUCCESS;
	}

	xil_printf("We are back from hibernation!\r\n");
	return (s32)XST_SUCCESS;
}

#endif /* XUSBPSU_HIBERNATION_ENABLE */

/** @} */
