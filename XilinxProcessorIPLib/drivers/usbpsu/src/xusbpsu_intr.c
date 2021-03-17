/******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_intr.c
* @addtogroup usbpsu_v1_9
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg  06/06/16 First release
* 1.3   vak 04/03/17 Added CCI support for USB
* 1.4	bk  12/01/18 Modify USBPSU driver code to fit USB common example code
*		       for all USB IPs
*	myk 12/01/18 Added hibernation support
*	vak 22/01/18 Added changes for supporting microblaze platform
*	vak 13/03/18 Moved the setup interrupt system calls from driver to
*		     example.
* 1.4	vak 30/05/18 Removed xusb_wrapper files
* 1.7	pm  23/03/20 Restructured the code for more readability and modularity
* 	pm  25/03/20 Add clocking support
* 1.8	pm  01/07/20 Add versal hibernation support
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
* Disconnect Interrupt handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_DisconnectIntr(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal &= ~XUSBPSU_DCTL_INITU1ENA;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	RegVal &= ~XUSBPSU_DCTL_INITU2ENA;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	InstancePtr->IsConfigDone = 0U;
	InstancePtr->AppData->Speed = XUSBPSU_SPEED_UNKNOWN;

#ifdef XUSBPSU_HIBERNATION_ENABLE
	/* In USB 2.0, to avoid hibernation interrupt at the time of connection
	 * clear KEEP_CONNECT bit.
	 */
	if (InstancePtr->HasHibernation == (u8)TRUE) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
		if ((RegVal & XUSBPSU_DCTL_KEEP_CONNECT) != (u32)0U) {
			RegVal &= ~XUSBPSU_DCTL_KEEP_CONNECT;
			XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);
		}
	}
#endif

	/* Call the handler if necessary */
	if (InstancePtr->DisconnectIntrHandler != NULL) {
		InstancePtr->DisconnectIntrHandler(InstancePtr->AppData);
	}
}

/****************************************************************************/
/**
* Reset Interrupt handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_ResetIntr(struct XUsbPsu *InstancePtr)
{
	u32	RegVal;
	u32	Index;

	InstancePtr->AppData->State = XUSBPSU_STATE_DEFAULT;

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal &= ~XUSBPSU_DCTL_TSTCTRL_MASK;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);
	InstancePtr->TestMode = 0U;

	XUsbPsu_StopActiveTransfers(InstancePtr);
	XUsbPsu_ClearStallAllEp(InstancePtr);

	for (Index = 0U; Index <
		((u32)InstancePtr->NumInEps + (u32)InstancePtr->NumOutEps);
			Index++)
	{
		InstancePtr->eps[Index].EpStatus = 0U;
	}

	InstancePtr->IsConfigDone = 0U;

	/* Reset device address to zero */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCFG);
	RegVal &= ~((u32)XUSBPSU_DCFG_DEVADDR_MASK);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCFG, RegVal);

	/* Call the handler if necessary */
	if (InstancePtr->ResetIntrHandler != NULL) {
		InstancePtr->ResetIntrHandler(InstancePtr->AppData);
	}
}

/****************************************************************************/
/**
* Handles Interrupts of Control Endpoints EP0 OUT and EP0 IN.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Event is a pointer to the Endpoint event occurred in core.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_Ep0Intr(struct XUsbPsu *InstancePtr,
		const struct XUsbPsu_Event_Epevt *Event)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Event != NULL);

	switch (Event->Endpoint_Event) {
	case XUSBPSU_DEPEVT_XFERCOMPLETE:
		XUsbPsu_Ep0XferComplete(InstancePtr, Event);
		break;

	case XUSBPSU_DEPEVT_XFERNOTREADY:
		XUsbPsu_Ep0XferNotReady(InstancePtr, Event);
		break;

	case XUSBPSU_DEPEVT_XFERINPROGRESS:
	case XUSBPSU_DEPEVT_STREAMEVT:
	case XUSBPSU_DEPEVT_EPCMDCMPLT:
		break;

	default:
		/* Default case is a required MIRSA-C guideline. */
		break;
	}
}

/****************************************************************************/
/**
* Connection Done Interrupt handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_ConnDoneIntr(struct XUsbPsu *InstancePtr)
{
	u32			RegVal;
	u16			Size;
	u8			Speed;

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DSTS);
	Speed = (u8)(RegVal & XUSBPSU_DSTS_CONNECTSPD);
	InstancePtr->AppData->Speed = Speed;

	switch (Speed) {
	case XUSBPSU_DCFG_SUPERSPEED:
#ifdef XUSBPSU_DEBUG
		xil_printf("Super Speed\r\n");
#endif
		Size = 512U;
		InstancePtr->AppData->Speed = XUSBPSU_SPEED_SUPER;
		break;

	case XUSBPSU_DCFG_HIGHSPEED:
#ifdef XUSBPSU_DEBUG
		xil_printf("High Speed\r\n");
#endif
		Size = 64U;
		InstancePtr->AppData->Speed = XUSBPSU_SPEED_HIGH;
		break;

	case XUSBPSU_DCFG_FULLSPEED2:
	case XUSBPSU_DCFG_FULLSPEED1:
#ifdef XUSBPSU_DEBUG
		xil_printf("Full Speed\r\n");
#endif
		Size = 64U;
		InstancePtr->AppData->Speed = XUSBPSU_SPEED_FULL;
		break;

	case XUSBPSU_DCFG_LOWSPEED:
#ifdef XUSBPSU_DEBUG
		xil_printf("Low Speed\r\n");
#endif
		Size = 64U;
		InstancePtr->AppData->Speed = XUSBPSU_SPEED_LOW;
		break;
	default :
		Size = 64U;
		break;
	}

	if (InstancePtr->AppData->Speed == XUSBPSU_SPEED_SUPER) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
		RegVal &= ~XUSBPSU_DCTL_HIRD_THRES_MASK;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);
	}

	(void)XUsbPsu_EnableControlEp(InstancePtr, Size);
	(void)XUsbPsu_RecvSetup(InstancePtr);

#ifdef XUSBPSU_HIBERNATION_ENABLE
	/* In USB 2.0, to avoid hibernation interrupt at the time of connection
	 * clear KEEP_CONNECT bit.
	 */
	if (InstancePtr->HasHibernation == (u8)TRUE) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
		if ((RegVal & XUSBPSU_DCTL_KEEP_CONNECT) == (u32)0U) {
			RegVal |= XUSBPSU_DCTL_KEEP_CONNECT;
			XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);
		}
	}
#endif
}

/****************************************************************************/
/**
* Link Status Change Interrupt handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	EvtInfo is Event information.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_LinkStsChangeIntr(struct XUsbPsu *InstancePtr, u32 EvtInfo)
{
	u32	State = EvtInfo & (u32)XUSBPSU_LINK_STATE_MASK;
	InstancePtr->LinkState = (u8)State;
}

/****************************************************************************/
/**
* Processes an Event entry in Event Buffer.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Event is the Event entry.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_EventHandler(struct XUsbPsu *InstancePtr,
			const union XUsbPsu_Event *Event)
{

	if (Event->Type.Is_DevEvt == 0U) {
		/* End point Specific Event */
		XUsbPsu_EpEvent(InstancePtr, &Event->Epevt);
		return;
	}

	switch (Event->Type.Type) {
	case XUSBPSU_EVENT_TYPE_DEV:
		/* Device Specific Event */
		XUsbPsu_DeviceEvent(InstancePtr, &Event->Devt);
		break;
	/* Carkit and I2C events not supported now */
	default:
		/* Made for Misra-C Compliance. */
		break;
	}
}

/*****************************************************************************/
/**
* @brief
* Enables an interrupt in Event Enable RegValister.
*
* @param  InstancePtr is a pointer to the XUsbPsu instance to be worked on
* @param  Mask is the OR of any Interrupt Enable Masks:
*		- XUSBPSU_DEVTEN_VNDRDEVTSTRCVEDEN
*		- XUSBPSU_DEVTEN_EVNTOVERFLOWEN
*		- XUSBPSU_DEVTEN_CMDCMPLTEN
*		- XUSBPSU_DEVTEN_ERRTICERREN
*		- XUSBPSU_DEVTEN_SOFEN
*		- XUSBPSU_DEVTEN_EOPFEN
*		- XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN
*		- XUSBPSU_DEVTEN_WKUPEVTEN
*		- XUSBPSU_DEVTEN_ULSTCNGEN
*		- XUSBPSU_DEVTEN_CONNECTDONEEN
*		- XUSBPSU_DEVTEN_USBRSTEN
*		- XUSBPSU_DEVTEN_DISCONNEVTEN
*
* @return  None
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_EnableIntr(struct XUsbPsu *InstancePtr, u32 Mask)
{
	u32	RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DEVTEN);
	RegVal |= Mask;

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEVTEN, RegVal);
}

/*****************************************************************************/
/**
* @brief
* Disables an interrupt in Event Enable RegValister.
*
* @param  InstancePtr is a pointer to the XUsbPsu instance to be worked on.
* @param  Mask is the OR of Interrupt Enable Masks
*		- XUSBPSU_DEVTEN_VNDRDEVTSTRCVEDEN
*		- XUSBPSU_DEVTEN_EVNTOVERFLOWEN
*		- XUSBPSU_DEVTEN_CMDCMPLTEN
*		- XUSBPSU_DEVTEN_ERRTICERREN
*		- XUSBPSU_DEVTEN_SOFEN
*		- XUSBPSU_DEVTEN_EOPFEN
*		- XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN
*		- XUSBPSU_DEVTEN_WKUPEVTEN
*		- XUSBPSU_DEVTEN_ULSTCNGEN
*		- XUSBPSU_DEVTEN_CONNECTDONEEN
*		- XUSBPSU_DEVTEN_USBRSTEN
*		- XUSBPSU_DEVTEN_DISCONNEVTEN
*
* @return  None
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_DisableIntr(struct XUsbPsu *InstancePtr, u32 Mask)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DEVTEN);
	RegVal &= ~Mask;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEVTEN, RegVal);
}



/****************************************************************************/
/**
* @brief
* Main Interrupt Handler.
*
* @param	XUsbPsuInstancePtr is a void pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_IntrHandler(void *XUsbPsuInstancePtr)
{
	struct XUsbPsu	*InstancePtr;
	struct XUsbPsu_EvtBuffer *Evt;
	u32 Count;
	u32 RegVal;

	InstancePtr = (struct XUsbPsu  *)XUsbPsuInstancePtr;
	Xil_AssertVoid(InstancePtr != NULL);

	Evt = &InstancePtr->Evt;

	Count = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0U));
	Count &= XUSBPSU_GEVNTCOUNT_MASK;
	/*
	 * As per data book software should only process Events if Event count
	 * is greater than zero.
	 */
	if (Count == 0U) {
		return;
	}

	Evt->Count = Count;
	Evt->Flags |= XUSBPSU_EVENT_PENDING;

	/* Mask event interrupt */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GEVNTSIZ(0U));
	RegVal |= XUSBPSU_GEVNTSIZ_INTMASK;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTSIZ(0U), RegVal);

	/* Processes events in an Event Buffer */
	XUsbPsu_EventBufferHandler(InstancePtr);
}

#ifdef XUSBPSU_HIBERNATION_ENABLE

/****************************************************************************/
/**
* @brief
* Wakeup Interrupt Event Handler.
*
* @param	XUsbPsuInstancePtr is a pointer of driver Instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_WakeUpIntrHandler(void *XUsbPsuInstancePtr)
{
	u32 RegVal;
	u32 Retries;

	struct XUsbPsu  *InstancePtr = (struct XUsbPsu *)XUsbPsuInstancePtr;

#if defined  (XCLOCKING)
	/* enable ref clocks */
	if (InstancePtr->IsHibernated) {
		Xil_ClockEnable(InstancePtr->ConfigPtr->RefClk);
	}
#endif

	RegVal = XUsbPsu_ReadLpdReg(RST_LPD_TOP);
	if (InstancePtr->ConfigPtr->DeviceId == (u16)XPAR_XUSBPSU_0_DEVICE_ID) {
		XUsbPsu_WriteLpdReg(RST_LPD_TOP,
				(u32)(RegVal & ~USB0_CORE_RST));
	}

#if defined (PLATFORM_ZYNQMP)
	/* change power state to D0 */
	XUsbPsu_WriteVendorReg(XIL_REQ_PWR_STATE, XIL_REQ_PWR_STATE_D0);
#else
	/* change power state to D0 */
	XUsbPsu_WriteVslPwrStateReg(XIL_REQ_PWR_STATE, XIL_REQ_PWR_STATE_D0);
#endif

	/* wait till current state is changed to D0 */
	Retries = (u32)XUSBPSU_PWR_STATE_RETRIES;

	while (Retries > 0U) {
#if defined (PLATFORM_ZYNQMP)
		RegVal = XUsbPsu_ReadVendorReg(XIL_CUR_PWR_STATE);
#else
		RegVal = XUsbPsu_ReadVslPwrStateReg(XIL_CUR_PWR_STATE);
#endif

		if ((RegVal & XIL_CUR_PWR_STATE_BITMASK) ==
					XIL_CUR_PWR_STATE_D0) {
			break;
		}

		XUsbPsu_Sleep(XUSBPSU_TIMEOUT);
		Retries = Retries - 1U;
	}

	if (Retries == 0U) {
		xil_printf("Failed to change power state to D0\r\n");
		return;
	}

	/* ask core to restore non-sticky registers */
	if (XUsbPsu_CoreRegRestore(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to Core Restore\r\n");
		return;
	}

	/* start controller */
	if (XUsbPsu_Start(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to start core on wakeup\r\n");
		return;
	}

	/* Wait until device controller is ready */
	if (XUsbPsu_WaitClearTimeout(InstancePtr, XUSBPSU_DSTS,
		XUSBPSU_DSTS_DCNRD, XUSBPSU_CTRL_RDY_RETRIES) == XST_FAILURE) {
		xil_printf("Failed to ready device controller\r\n");
		return;
	}

	/*
	 * there can be spurious wakeup events , so wait for some time and check
	 * the link state
	 */
	XUsbPsu_Sleep(XUSBPSU_TIMEOUT * 10U);

	/* Processes link state events for hibernation */
	if (XUsbPsu_HibernationStateIntr(InstancePtr) == XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
		xil_printf("link state event failure\r\n");
#endif

	}

}

#endif
/** @} */
