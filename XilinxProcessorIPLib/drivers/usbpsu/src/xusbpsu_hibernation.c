/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_hibernation.c
*
* This patch adds hibernation support to usbpsu driver when dwc3 is operating
* as a gadget
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver    Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   Mayank 12/01/18 First release
* 1.5   VAK    14/03/19 Enable hibernation related functions only when
*                       XUSBPSU_HIBERNATION_ENABLE is defined
* 1.7	pm     23/03/20 Restructured the code for more readability and modularity
* 	pm     25/03/20 Add clocking support
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xusbpsu_hw.h"
#include "xusbpsu_endpoint.h"
#include "xusbpsu_local.h"

#ifdef XUSBPSU_HIBERNATION_ENABLE

/************************** Constant Definitions *****************************/

#define NUM_OF_NONSTICKY_REGS    		27U

#define XUSBPSU_HIBER_SCRATCHBUF_SIZE           4096U

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/
static u8 ScratchBuf[XUSBPSU_HIBER_SCRATCHBUF_SIZE];

/* Registers saved during hibernation and restored at wakeup */
static u32 SaveRegsAddr[] = {
        XUSBPSU_DCTL,
        XUSBPSU_DCFG,
        XUSBPSU_DEVTEN,
        XUSBPSU_GSBUSCFG0,
        XUSBPSU_GSBUSCFG1,
        XUSBPSU_GCTL,
        XUSBPSU_GTXTHRCFG,
        XUSBPSU_GRXTHRCFG,
        XUSBPSU_GTXFIFOSIZ(0U),
        XUSBPSU_GTXFIFOSIZ(1U),
        XUSBPSU_GTXFIFOSIZ(2U),
        XUSBPSU_GTXFIFOSIZ(3U),
        XUSBPSU_GTXFIFOSIZ(4U),
        XUSBPSU_GTXFIFOSIZ(5U),
        XUSBPSU_GTXFIFOSIZ(6U),
        XUSBPSU_GTXFIFOSIZ(7U),
        XUSBPSU_GTXFIFOSIZ(8U),
        XUSBPSU_GTXFIFOSIZ(9U),
        XUSBPSU_GTXFIFOSIZ(10U),
        XUSBPSU_GTXFIFOSIZ(11U),
        XUSBPSU_GTXFIFOSIZ(12U),
        XUSBPSU_GTXFIFOSIZ(13U),
        XUSBPSU_GTXFIFOSIZ(14U),
        XUSBPSU_GTXFIFOSIZ(15U),
        XUSBPSU_GRXFIFOSIZ(0U),
        XUSBPSU_GUSB3PIPECTL(0U),
        XUSBPSU_GUSB2PHYCFG(0U),
};
static u32 SavedRegs[NUM_OF_NONSTICKY_REGS];

/*****************************************************************************/
/**
* Save non sticky registers
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XUsbPsu_SaveRegs(struct XUsbPsu *InstancePtr)
{
	u32 i;

	for (i = 0U; i < NUM_OF_NONSTICKY_REGS; i++) {
		SavedRegs[i] = XUsbPsu_ReadReg(InstancePtr, SaveRegsAddr[i]);
	}
}

/*****************************************************************************/
/**
* Restore non sticky registers
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XUsbPsu_RestoreRegs(struct XUsbPsu *InstancePtr)
{
	u32 i;

	for (i = 0U; i < NUM_OF_NONSTICKY_REGS; i++) {
		XUsbPsu_WriteReg(InstancePtr, SaveRegsAddr[i], SavedRegs[i]);
	}
}


/*****************************************************************************/
/**
* Initialize to handle hibernation event when it comes
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	none
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_InitHibernation(struct XUsbPsu *InstancePtr)
{
	u32		RegVal;

	InstancePtr->IsHibernated = 0U;

	memset(ScratchBuf, 0U, sizeof(ScratchBuf));
	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)ScratchBuf,
					XUSBPSU_HIBER_SCRATCHBUF_SIZE);
	}

	XUsbPsu_SetupScratchpad(InstancePtr, ScratchBuf);

	/* enable PHY suspend */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U));
	RegVal |= XUSBPSU_GUSB2PHYCFG_SUSPHY;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U), RegVal);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U));
	RegVal |= XUSBPSU_GUSB3PIPECTL_SUSPHY;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U), RegVal);
}

/*****************************************************************************/
/**
* Handle hibernation event
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	none
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_HibernationIntr(struct XUsbPsu *InstancePtr)
{
	u8 EpNum;
	u32 RegVal;
	u32 retries;
	XusbPsuLinkState LinkState;

	/* sanity check */
	switch(XUsbPsu_GetLinkState(InstancePtr)) {
	case XUSBPSU_LINK_STATE_SS_DIS:
	case XUSBPSU_LINK_STATE_U3:
		break;
	default:
		/* fake hiber interrupt */
		xil_printf("got fake interrupt\r\n");
		return;
	};

	if (InstancePtr->Ep0State == XUSBPSU_EP0_SETUP_PHASE) {
		XUsbPsu_StopTransfer(InstancePtr, 0U, XUSBPSU_EP_DIR_OUT, TRUE);
		XUsbPsu_RecvSetup(InstancePtr);
	}

	/* stop active transfers for all endpoints including control
	 * endpoints force rm bit should be 0 when we do this */
	for (EpNum = 0U; EpNum < XUSBPSU_ENDPOINTS_NUM; EpNum++) {
		struct XUsbPsu_Ep *Ept;

		Ept = &InstancePtr->eps[EpNum];

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		/* save srsource index for later use */
		XUsbPsu_StopTransfer(InstancePtr, Ept->UsbEpNum,
				Ept->Direction, FALSE);

		XUsbPsu_SaveEndpointState(InstancePtr, Ept);
	}

	/*
	 * ack events, don't process them; h/w decrements the count by the value
	 * written
	 */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0U));
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0U), RegVal);
	InstancePtr->Evt.Count = 0U;
	InstancePtr->Evt.Flags &= ~XUSBPSU_EVENT_PENDING;

	if (XUsbPsu_Stop(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to stop USB core\r\n");
		return;
	}

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);

	/* Check the link state and if it is disconnected, set
	 * KEEP_CONNECT to 0
	 */
	LinkState = XUsbPsu_GetLinkState(InstancePtr);
	if (LinkState == XUSBPSU_LINK_STATE_SS_DIS) {
		RegVal &= ~XUSBPSU_DCTL_KEEP_CONNECT;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

		/* update LinkState to be used while wakeup */
		InstancePtr->LinkState = XUSBPSU_LINK_STATE_SS_DIS;
	}

	XUsbPsu_SaveRegs(InstancePtr);

	/* ask core to save state */
	RegVal |= XUSBPSU_DCTL_CSS;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	/* wait till core saves */
	if (XUsbPsu_WaitClearTimeout(InstancePtr, XUSBPSU_DSTS,
		XUSBPSU_DSTS_SSS, XUSBPSU_NON_STICKY_SAVE_RETRIES) ==
							XST_FAILURE) {
		xil_printf("Failed to save core state\r\n");
		return;
	}

	/* Enable PME to wakeup from hibernation */
	XUsbPsu_WriteVendorReg(XIL_PME_ENABLE, XIL_PME_ENABLE_SIG_GEN);

	/* change power state to D3 */
	XUsbPsu_WriteVendorReg(XIL_REQ_PWR_STATE, XIL_REQ_PWR_STATE_D3);

	/* wait till current state is changed to D3 */
        retries = (u32)XUSBPSU_PWR_STATE_RETRIES;

        while (retries > 0U) {
		RegVal = XUsbPsu_ReadVendorReg(XIL_CUR_PWR_STATE);
		if ((RegVal & XIL_CUR_PWR_STATE_BITMASK) ==
					XIL_CUR_PWR_STATE_D3) {
                        break;
                }

		XUsbPsu_Sleep(XUSBPSU_TIMEOUT);
                retries = retries - 1U;
        }

	if (retries == 0U) {
		xil_printf("Failed to change power state to D3\r\n");
		return;
	}
	XUsbPsu_Sleep(XUSBPSU_TIMEOUT);

	RegVal = XUsbPsu_ReadLpdReg(RST_LPD_TOP);
	if (InstancePtr->ConfigPtr->DeviceId == (u16)XPAR_XUSBPSU_0_DEVICE_ID) {
		XUsbPsu_WriteLpdReg(RST_LPD_TOP, RegVal | (u32)USB0_CORE_RST);
	}

#if defined (XCLOCKING)
		/* disable ref clocks */
	if (InstancePtr->IsHibernated == 0) {
		Xil_ClockDisable(InstancePtr->ConfigPtr->RefClk);
	}
#endif

	InstancePtr->IsHibernated = 1U;
	xil_printf("Hibernated!\r\n");
}

/*****************************************************************************/
/**
* Core to restore non-sticky registers
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 			on.
*
* @return	none
*
* @note		None.
*
******************************************************************************/
s32 XUsbPsu_CoreRegRestore(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;

	XUsbPsu_SetupScratchpad(InstancePtr, ScratchBuf);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal |= XUSBPSU_DCTL_CRS;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	/* wait till non-sticky registers are restored */
	if (XUsbPsu_WaitClearTimeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_RSS, XUSBPSU_NON_STICKY_SAVE_RETRIES) ==
							XST_FAILURE) {
		xil_printf("Failed to restore USB core\r\n");
		return XST_FAILURE;
	}

	XUsbPsu_RestoreRegs(InstancePtr);

	/* setup event buffers */
	XUsbPsu_EventBuffersSetup(InstancePtr);

	/* nothing to do when in OTG host mode */
	if ((XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GSTS) &
					XUSBPSU_GSTS_CUR_MODE) != (u32)0U) {
		return XST_FAILURE;
	}

	if (XUsbPsu_RestoreEp0(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to restore EP0\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#endif /* XUSBPSU_HIBERNATION_ENABLE */
/** @} */
