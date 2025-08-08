/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1_intr.c
*
* This file contains interrupt related functions for Xilinx HDMI TX core.
* Please see xv_hdmitx1.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     22/05/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmitx1.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void HdmiTx1_PioIntrHandler(XV_HdmiTx1 *InstancePtr);
static void HdmiTx1_DdcIntrHandler(XV_HdmiTx1 *InstancePtr);
static void HdmiTx1_FrlIntrHandler(XV_HdmiTx1 *InstancePtr);
static void HdmiTx1_AuxIntrHandler(XV_HdmiTx1 *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI TX driver.
*
* This handler reads the pending interrupt from PIO and DDC peripheral,
* determines the source of the interrupts, clears the interrupts and calls
* callbacks accordingly.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XV_HdmiTx1_SetCallback() during initialization phase. An example delivered
* with this driver demonstrates how this could be done.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 instance that just
*       interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_IntrHandler(void *InstancePtr)
{
	u32 Data;
	XV_HdmiTx1 *HdmiTx1Ptr = (XV_HdmiTx1 *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(HdmiTx1Ptr != NULL);
	Xil_AssertVoid(HdmiTx1Ptr->IsReady == XIL_COMPONENT_IS_READY);

	/* PIO */
	Data = XV_HdmiTx1_ReadReg(HdmiTx1Ptr->Config.BaseAddress,
				  (XV_HDMITX1_PIO_STA_OFFSET)) &
				  (XV_HDMITX1_PIO_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to PIO handler */
		HdmiTx1_PioIntrHandler(HdmiTx1Ptr);
	}

	/* DDC */
	Data = XV_HdmiTx1_ReadReg(HdmiTx1Ptr->Config.BaseAddress,
				  (XV_HDMITX1_DDC_STA_OFFSET)) &
				  (XV_HDMITX1_DDC_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to DDC handler */
		HdmiTx1_DdcIntrHandler(HdmiTx1Ptr);
	}

	/* HDMI 2.1 Fixed Rate Link */
	Data = XV_HdmiTx1_ReadReg(HdmiTx1Ptr->Config.BaseAddress,
				  (XV_HDMITX1_FRL_STA_OFFSET)) &
				  (XV_HDMITX1_FRL_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to DDC handler */
		HdmiTx1_FrlIntrHandler(HdmiTx1Ptr);
	}

	/* Aux */
	Data = XV_HdmiTx1_ReadReg(HdmiTx1Ptr->Config.BaseAddress,
				  (XV_HDMITX1_AUX_STA_OFFSET)) &
				  (XV_HDMITX1_AUX_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to Aux handler */
		HdmiTx1_AuxIntrHandler(HdmiTx1Ptr);
	}

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------------------------
* (XV_HDMITX1_HANDLER_HPD)   HpdCallback
* (XV_HDMITX1_HANDLER_VS)    VsCallback
* </pre>
*
* @param    InstancePtr is a pointer to the HDMI TX core instance.
* @param    HandlerType specifies the type of handler.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*       installed replaces it with the new handler.
*
******************************************************************************/
int XV_HdmiTx1_SetCallback(XV_HdmiTx1 *InstancePtr,
			XV_HdmiTx1_HandlerType HandlerType,
			void *CallbackFunc,
			void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XV_HDMITX1_HANDLER_CONNECT));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
	case (XV_HDMITX1_HANDLER_CONNECT):
		InstancePtr->ConnectCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->ConnectRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMITX1_HANDLER_TOGGLE):
		InstancePtr->ToggleCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->ToggleRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMITX1_HANDLER_BRDGLOCK):
		InstancePtr->BrdgLockedCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->BrdgLockedRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMITX1_HANDLER_BRDGUNLOCK):
		InstancePtr->BrdgUnlockedCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->BrdgUnlockedRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMITX1_HANDLER_BRDGOVERFLOW):
		InstancePtr->BrdgOverflowCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->BrdgOverflowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMITX1_HANDLER_BRDGUNDERFLOW):
		InstancePtr->BrdgUnderflowCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->BrdgUnderflowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMITX1_HANDLER_VS):
		InstancePtr->VsCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->VsRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Stream down*/
	case (XV_HDMITX1_HANDLER_STREAM_DOWN):
		InstancePtr->StreamDownCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->StreamDownRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Stream up*/
	case (XV_HDMITX1_HANDLER_STREAM_UP):
		InstancePtr->StreamUpCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->StreamUpRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL Config*/
	case (XV_HDMITX1_HANDLER_FRL_CONFIG):
		InstancePtr->FrlConfigCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlConfigRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL FFE*/
	case (XV_HDMITX1_HANDLER_FRL_FFE):
		InstancePtr->FrlFfeCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlFfeRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL Start*/
	case (XV_HDMITX1_HANDLER_FRL_START):
		InstancePtr->FrlStartCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlStartRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL Stop*/
	case (XV_HDMITX1_HANDLER_FRL_STOP):
		InstancePtr->FrlStopCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlStopRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* TMDS Config*/
	case (XV_HDMITX1_HANDLER_TMDS_CONFIG):
		InstancePtr->TmdsConfigCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->TmdsConfigRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:L*/
	case (XV_HDMITX1_HANDLER_FRL_LTSL):
		InstancePtr->FrlLtsLCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlLtsLRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:1*/
	case (XV_HDMITX1_HANDLER_FRL_LTS1):
		InstancePtr->FrlLts1Callback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlLts1Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:2*/
	case (XV_HDMITX1_HANDLER_FRL_LTS2):
		InstancePtr->FrlLts2Callback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlLts2Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:3*/
	case (XV_HDMITX1_HANDLER_FRL_LTS3):
		InstancePtr->FrlLts3Callback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlLts3Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:4*/
	case (XV_HDMITX1_HANDLER_FRL_LTS4):
		InstancePtr->FrlLts4Callback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlLts4Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:P*/
	case (XV_HDMITX1_HANDLER_FRL_LTSP):
		InstancePtr->FrlLtsPCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->FrlLtsPRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* CED_Update*/
	case (XV_HDMITX1_HANDLER_CED_UPDATE):
		InstancePtr->CedUpdateCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->CedUpdateRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Dynamic HDR MTW_Update*/
	case (XV_HDMITX1_HANDLER_DYNHDR_MWT):
		InstancePtr->DynHdrMtwCallback = (XV_HdmiTx1_Callback)CallbackFunc;
		InstancePtr->DynHdrMtwRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* DSC Decode Fail Update*/
	case (XV_HDMITX1_HANDLER_DSCDECODE_FAIL):
			InstancePtr->DscDecodeFailCallback = (XV_HdmiTx1_Callback)CallbackFunc;
			InstancePtr->DscDecodeFailRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function is the HDMI TX PIO peripheral interrupt handler.
*
* This handler reads corresponding event interrupt from the PIO_IN_EVT
* register. It determines the source of the interrupts and calls according
* callbacks.
*
* @param    InstancePtr is a pointer to the HDMI TX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiTx1_PioIntrHandler(XV_HdmiTx1 *InstancePtr)
{
	u32 Event;
	u32 Data;

	/* Read PIO IN Event register.*/
	Event = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				   (XV_HDMITX1_PIO_IN_EVT_OFFSET));

	/* Clear event flags */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_IN_EVT_OFFSET),
			    (Event));

	/* Read data */
	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMITX1_PIO_IN_OFFSET));

	/* HPD event has occurred */
	if (((Event) & (XV_HDMITX1_PIO_IN_HPD_MASK)) ||
	    ((Event) & (XV_HDMITX1_PIO_IN_HPD_TOGGLE_MASK))) {
		/* Check the HPD status*/
		if ((Data) & (XV_HDMITX1_PIO_IN_HPD_MASK)) {
			/* Note: placement of setting the HDMI mode can
			 * be made more robust. */
			InstancePtr->Stream.IsFrl = FALSE;
			InstancePtr->Stream.IsHdmi = FALSE;
			/* Set connected flag*/
			InstancePtr->Stream.IsConnected = (TRUE);
		    	InstancePtr->Stream.Frl.TrainingState =
					XV_HDMITX1_FRLSTATE_LTS_L;
			XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);
		} else {
			if (InstancePtr->Stream.IsFrl == TRUE) {
				/* Check if user callback has been registered*/
				if (InstancePtr->StreamDownCallback) {
					InstancePtr->StreamDownCallback(
							InstancePtr->StreamDownRef);
				}

				/*
				 * Since stream has gone down, disable data mover
				 * for dynamic HDR
				 */
				XV_HdmiTx1_DynHdr_DM_Disable(InstancePtr);

			}
			/* Clear connected flag*/
			InstancePtr->Stream.IsConnected = (FALSE);

			/* On HPD disconnect data mover may hang. So disable */
			XV_HdmiTx1_DynHdr_DM_Disable(InstancePtr);
		}

		/* Check if user callback has been registered*/
		if (InstancePtr->ConnectCallback) {
			InstancePtr->ConnectCallback(InstancePtr->ConnectRef);
		}
	}

	/* Bridge Unlocked event has occurred */
	if ((Event) & (XV_HDMITX1_PIO_IN_BRDG_LOCKED_MASK)) {
		if ((Data) & (XV_HDMITX1_PIO_IN_BRDG_LOCKED_MASK)) {
			/* Check if user callback has been registered*/
			if (InstancePtr->BrdgLockedCallback) {
				InstancePtr->BrdgLockedCallback(
						InstancePtr->BrdgLockedRef);
			}
		} else {
			/* Check if user callback has been registered*/
			if (InstancePtr->BrdgUnlockedCallback) {
				InstancePtr->BrdgUnlockedCallback(
						InstancePtr->BrdgUnlockedRef);
			}
		}
	}

	/* Bridge Overflow event has occurred */
	if ((Event) & (XV_HDMITX1_PIO_IN_BRDG_OVERFLOW_MASK)) {

		/* Check if user callback has been registered*/
		if (InstancePtr->BrdgOverflowCallback) {
			InstancePtr->BrdgOverflowCallback(
					InstancePtr->BrdgOverflowRef);
		}
	}

	/* Bridge Underflow event has occurred */
	if ((Event) & (XV_HDMITX1_PIO_IN_BRDG_UNDERFLOW_MASK)) {

		/* Check if user callback has been registered*/
		if (InstancePtr->BrdgUnderflowCallback) {
			InstancePtr->BrdgUnderflowCallback(
					InstancePtr->BrdgUnderflowRef);
		}
	}

	/* Vsync event has occurred */
	if ((Event) & (XV_HDMITX1_PIO_IN_VS_MASK)) {

		/* Check if user callback has been registered*/
		if (InstancePtr->VsCallback) {
			InstancePtr->VsCallback(InstancePtr->VsRef);
		}
	}

	/* Link ready event has occurred */
	if ((Event) & (XV_HDMITX1_PIO_IN_LNK_RDY_MASK)) {
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf(ANSI_COLOR_CYAN "TX: link event");
#endif

		/* Check the link status*/
		if ((Data) & (XV_HDMITX1_PIO_IN_LNK_RDY_MASK)) {
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf(" up\n\r" ANSI_COLOR_RESET);
#endif
			/* Note : An improved methodology could use states
			 * instead of video modes to decide if aux needs
			 * to be sent here or not*/
			if (InstancePtr->Stream.IsFrl == TRUE) {
				/* Set stream status to up*/
				InstancePtr->Stream.State = XV_HDMITX1_STATE_STREAM_UP;

				if (InstancePtr->Stream.Frl.TrainingState ==
			    	    XV_HDMITX1_FRLSTATE_LTS_3_ARM) {
					XV_HdmiTx1_ExecFrlState(InstancePtr);
				}
			} else {
				/* Enable the AUX peripheral */
				XV_HdmiTx1_AuxEnable(InstancePtr);

				/* Enable the AUX peripheral interrupt */
				XV_HdmiTx1_AuxIntrEnable(InstancePtr);


				XV_HdmiTx1_DynHdr_DM_Enable(InstancePtr);

				/* Check if user callback has been registered*/
				if (InstancePtr->StreamUpCallback) {
					InstancePtr->StreamUpCallback(
							InstancePtr->StreamUpRef);
				}
			}
		} else {
		/* Link down*/
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf(" down\n\r" ANSI_COLOR_RESET);
#endif
			/* Set stream status to down*/
			InstancePtr->Stream.State = XV_HDMITX1_STATE_STREAM_DOWN;

			/* Disable Audio */
			XV_HdmiTx1_AudioDisable(InstancePtr);

			/* Disable AUX */
			XV_HdmiTx1_AuxDisable(InstancePtr);

			/* Check if user callback has been registered*/
			if (InstancePtr->StreamDownCallback) {
				InstancePtr->StreamDownCallback(
						InstancePtr->StreamDownRef);
			}
			/*
			 * Since stream has gone down, disable data mover
			 * for dynamic HDR
			 */
			XV_HdmiTx1_DynHdr_DM_Disable(InstancePtr);
		}
	}
}

/*****************************************************************************/
/**
*
* This function is the HDMI TX DDC peripheral interrupt handler.
*
* This handler reads DDC Status register and determines the timeout. It also
* determines the state and based on that performs required operation.
*
*
* @param    InstancePtr is a pointer to the HDMI TX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiTx1_DdcIntrHandler(XV_HdmiTx1 *InstancePtr)
{
	u32 Data;

	/* Read DDC Status register */
	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMITX1_DDC_STA_OFFSET));
	Data = Data; /*squash unused variable compiler warning*/
}

/*****************************************************************************/
/**
*
* This function is the HDMI TX FRL peripheral interrupt handler.
*
* This handler reads corresponding timer event interrupt from the FRL Status
* register.
*
* @param    InstancePtr is a pointer to the HDMI TX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiTx1_FrlIntrHandler(XV_HdmiTx1 *InstancePtr)
{
	u32 Data;
	/* Read FRL Status register */
	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMITX1_FRL_STA_OFFSET));

	/* FRL timer event has occurred */
	if ((Data) & (XV_HDMITX1_FRL_STA_TMR_EVT_MASK)) {
		/* Reset TMR_EVT*/
		XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMITX1_FRL_STA_OFFSET),
				    XV_HDMITX1_FRL_STA_TMR_EVT_MASK);

		/* Check if the timer event flag is clear */
		if (InstancePtr->Stream.Frl.TimerEvent) {
			xil_printf("FRL timer event flag was still set.\n\r");
			xil_printf("The FRL state machine took too much "
				   "time to process a single state.\n\r");
		}

		/* Set Timer event flag */
		InstancePtr->Stream.Frl.TimerEvent = TRUE;

		/* Execute state machine */
		XV_HdmiTx1_ExecFrlState(InstancePtr);
	}
}

/*****************************************************************************/
/**
*
* This function is the HDMI TX Aux interrupt handler.
*
* This handler reads corresponding event interrupt from the Auxiliary Status
* register. It determines the source of the interrupts and calls according
* callbacks.
*
* @param    InstancePtr is a pointer to the HDMI TX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiTx1_AuxIntrHandler(XV_HdmiTx1 *InstancePtr)
{
	u32 Event;

	/* Read Aux status register.*/
	Event = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				   XV_HDMITX1_AUX_STA_OFFSET);

	/* Clear event flags */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMITX1_AUX_STA_OFFSET,
			    Event);

	/* Dynamic HDR MTW event has occurred */
	if (Event & XV_HDMITX1_AUX_STA_DYNHDR_MTW_MASK) {
		if (InstancePtr->DynHdrMtwCallback) {
			InstancePtr->DynHdrMtwCallback(InstancePtr->DynHdrMtwRef);
		}
	}
}
