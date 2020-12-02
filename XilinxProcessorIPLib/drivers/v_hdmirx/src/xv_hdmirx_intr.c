/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx_intr.c
*
* This file contains interrupt related functions for Xilinx HDMI RX core.
* Please see xv_hdmirx.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  gm, mg 11/03/15 Initial release.
* 1.01  yh     14/01/16 Set AxisEnable PIO to high when RX stream locked
* 1.03  MG     18/02/16 Added Link Check callback
* 1.04  MG     08/03/16 Added pixel clock calculation to HdmiRx_TmrIntrHandler
* 1.05  MH     08/03/16 Added support for read not complete DDC event
* 1.06  MG     27/05/16 Updated HdmiRx_VtdIntrHandler
* 1.07  MG     27/05/16 Updated HdmiRx_TmrIntrHandler
* 1.08  MG     30/05/16 Fixed issue with pixel clock adjustment for YUV422 colorspace
* 1.09  MH     26/07/16 Added DDC HDCP protocol event.
* 1.10  YH     18/08/16 squash unused variable compiler warning
* 1.11  MG     03/03/17 Updated function HdmiRx_TmrIntrHandler with
*                           GetVideoPropertiesTries
* 1.12  YH     22/08/17 Update AudFormat when servicing Aud Interrupt
* 1.13  MH     31/08/17 Update Reset sequence for Video_Bridge
* 2.00  EB     16/01/18 Added clearing of XV_HDMIRX_AUX_STA_GCP_CD_EVT flag
*                           after servicing it
*       YH     16/11/17 Added bridge overflow interrupt
*              16/11/17 Update Reset sequence with dedicated reset for
*                           each clock domain
*       MMO    08/02/18 Adding proper handling for Sync Loss/Sync Recover
* 2.10  YH     13/04/18 Fixed a bug in PioIntrHandler
* 2.20  EB     16/08/18 Replaced TIME_10MS, TIME_16MS, TIME_200MS with
*                           XV_HdmiRx_GetTime10Ms, XV_HdmiRx_GetTime16Ms
*                           XV_HdmiRx_GetTime200Ms
*                       Added TMDS Clock Ratio callback support
*       YB     15/08/18 Added new cases for HDCP 1.4 & 2.2 protocol events in
*                           XV_HdmiRx_SetCallback function.
*                       Updated the HdmiRx_DdcIntrHandler() function.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void HdmiRx_PioIntrHandler(XV_HdmiRx *InstancePtr);
static void HdmiRx_TmrIntrHandler(XV_HdmiRx *InstancePtr);
static void HdmiRx_VtdIntrHandler(XV_HdmiRx *InstancePtr);
static void HdmiRx_DdcIntrHandler(XV_HdmiRx *InstancePtr);
static void HdmiRx_AuxIntrHandler(XV_HdmiRx *InstancePtr);
static void HdmiRx_AudIntrHandler(XV_HdmiRx *InstancePtr);
static void HdmiRx_LinkStatusIntrHandler(XV_HdmiRx *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX driver.
*
* This handler reads the pending interrupt from PIO, DDC, TIMDET, AUX, AUD
* and LNKSTA peripherals, determines the source of the interrupts, clears the
* interrupts and calls callbacks accordingly.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XV_HdmiRx_SetCallback() during initialization phase. An example delivered
* with this driver demonstrates how this could be done.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx instance that just
*       interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx_IntrHandler(void *InstancePtr)
{
    u32 Data;
    XV_HdmiRx *HdmiRxPtr = (XV_HdmiRx *)InstancePtr;

    /* Verify arguments */
    Xil_AssertVoid(HdmiRxPtr != NULL);
    Xil_AssertVoid(HdmiRxPtr->IsReady == XIL_COMPONENT_IS_READY);

    /* PIO */
    Data = XV_HdmiRx_ReadReg(HdmiRxPtr->Config.BaseAddress, (XV_HDMIRX_PIO_STA_OFFSET)) & (XV_HDMIRX_PIO_STA_IRQ_MASK);

    /* Check for IRQ flag set */
    if (Data) {
        /* Jump to PIO handler */
        HdmiRx_PioIntrHandler(HdmiRxPtr);
    }

    /* Timer */
    Data = XV_HdmiRx_ReadReg(HdmiRxPtr->Config.BaseAddress, (XV_HDMIRX_TMR_STA_OFFSET)) & (XV_HDMIRX_TMR_STA_IRQ_MASK);

    /* Check for IRQ flag set */
    if (Data) {
        /* Jump to PIO handler */
        HdmiRx_TmrIntrHandler(HdmiRxPtr);
    }

    /* Video Timing detector */
    Data = XV_HdmiRx_ReadReg(HdmiRxPtr->Config.BaseAddress, (XV_HDMIRX_VTD_STA_OFFSET)) & (XV_HDMIRX_VTD_STA_IRQ_MASK);

    /* Check for IRQ flag set */
    if (Data) {
        /* Jump to video timing detector handler */
        HdmiRx_VtdIntrHandler(HdmiRxPtr);
    }

    /* DDC */
    Data = XV_HdmiRx_ReadReg(HdmiRxPtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET)) & (XV_HDMIRX_DDC_STA_IRQ_MASK);

    /* Is the IRQ flag set */
    if (Data) {
        /* Jump to DDC handler */
        HdmiRx_DdcIntrHandler(HdmiRxPtr);
    }

    /* AUX */
    Data = XV_HdmiRx_ReadReg(HdmiRxPtr->Config.BaseAddress, (XV_HDMIRX_AUX_STA_OFFSET)) & (XV_HDMIRX_AUX_STA_IRQ_MASK);

    /* Check for IRQ flag set */
    if (Data) {
        /* Jump to AUX handler */
        HdmiRx_AuxIntrHandler(HdmiRxPtr);
    }

    /* Audio */
    Data = XV_HdmiRx_ReadReg(HdmiRxPtr->Config.BaseAddress, (XV_HDMIRX_AUD_STA_OFFSET)) & (XV_HDMIRX_AUD_STA_IRQ_MASK);

    /* Check for IRQ flag set */
    if (Data) {
        /* Jump to Audio handler */
        HdmiRx_AudIntrHandler(HdmiRxPtr);
    }

    /* Link status */
    Data = XV_HdmiRx_ReadReg(HdmiRxPtr->Config.BaseAddress, (XV_HDMIRX_LNKSTA_STA_OFFSET)) & (XV_HDMIRX_LNKSTA_STA_IRQ_MASK);

    /* Check for IRQ flag set */
    if (Data) {
        /* Jump to Link Status handler */
        HdmiRx_LinkStatusIntrHandler(HdmiRxPtr);
    }
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                 Callback Function Type
* -------------------------   -----------------------------------------------
* (XV_HDMIRX_HANDLER_VTD)         VtdCallback
* (XV_HDMIRX_HANDLER_AUX)      AuxCallback
* (XV_HDMIRX_HANDLER_AUD)      AudCallback
* (XV_HDMIRX_HANDLER_LNKSTA)   LnkStaCallback
* (XV_HDMIRX_HANDLER_PIO)      PioCallback
* </pre>
*
* @param    InstancePtr is a pointer to the HDMI RX core instance.
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
int XV_HdmiRx_SetCallback(XV_HdmiRx *InstancePtr,
		XV_HdmiRx_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef)
{
    u32 Status;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(HandlerType >= (XV_HDMIRX_HANDLER_CONNECT));
    Xil_AssertNonvoid(CallbackFunc != NULL);
    Xil_AssertNonvoid(CallbackRef != NULL);

    /* Check for handler type */
    switch (HandlerType) {

        case (XV_HDMIRX_HANDLER_CONNECT):
            InstancePtr->ConnectCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->ConnectRef = CallbackRef;
            InstancePtr->IsConnectCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRX_HANDLER_AUX):
            InstancePtr->AuxCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->AuxRef = CallbackRef;
            InstancePtr->IsAuxCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRX_HANDLER_AUD):
            InstancePtr->AudCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->AudRef = CallbackRef;
            InstancePtr->IsAudCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRX_HANDLER_LNKSTA):
            InstancePtr->LnkStaCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->LnkStaRef = CallbackRef;
            InstancePtr->IsLnkStaCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Ddc
        case (XV_HDMIRX_HANDLER_DDC):
            InstancePtr->DdcCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->DdcRef = CallbackRef;
            InstancePtr->IsDdcCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Stream down
        case (XV_HDMIRX_HANDLER_STREAM_DOWN):
            InstancePtr->StreamDownCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->StreamDownRef = CallbackRef;
            InstancePtr->IsStreamDownCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Stream Init
        case (XV_HDMIRX_HANDLER_STREAM_INIT):
            InstancePtr->StreamInitCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->StreamInitRef = CallbackRef;
            InstancePtr->IsStreamInitCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Stream up
        case (XV_HDMIRX_HANDLER_STREAM_UP):
            InstancePtr->StreamUpCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->StreamUpRef = CallbackRef;
            InstancePtr->IsStreamUpCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // HDCP
        case (XV_HDMIRX_HANDLER_HDCP):
            InstancePtr->HdcpCallback = (XV_HdmiRx_HdcpCallback)CallbackFunc;
            InstancePtr->HdcpRef = CallbackRef;
            InstancePtr->IsHdcpCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // HDCP 1.4 Event
        case (XV_HDMIRX_HANDLER_DDC_HDCP_14_PROT):
            InstancePtr->Hdcp14ProtEvtCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->Hdcp14ProtEvtRef = CallbackRef;
            InstancePtr->IsHdcp14ProtEvtCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // HDCP 2.2 Event
        case (XV_HDMIRX_HANDLER_DDC_HDCP_22_PROT):
            InstancePtr->Hdcp22ProtEvtCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->Hdcp22ProtEvtRef = CallbackRef;
            InstancePtr->IsHdcp22ProtEvtCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRX_HANDLER_LINK_ERROR):
            InstancePtr->LinkErrorCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->LinkErrorRef = CallbackRef;
            InstancePtr->IsLinkErrorCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Bridge FIFO Overflow
        case (XV_HDMIRX_HANDLER_BRDG_OVERFLOW):
            InstancePtr->BrdgOverflowCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->BrdgOverflowRef = CallbackRef;
            InstancePtr->IsBrdgOverflowCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Sync Loss
        case (XV_HDMIRX_HANDLER_SYNC_LOSS):
            InstancePtr->SyncLossCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->SyncLossRef = CallbackRef;
            InstancePtr->IsSyncLossCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Mode
        case (XV_HDMIRX_HANDLER_MODE):
            InstancePtr->ModeCallback = (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->ModeRef = CallbackRef;
            InstancePtr->IsModeCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // TMDS clock ratio
        case (XV_HDMIRX_HANDLER_TMDS_CLK_RATIO):
            InstancePtr->TmdsClkRatioCallback =
                                  (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->TmdsClkRatioRef = CallbackRef;
            InstancePtr->IsTmdsClkRatioCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        // Vic Error
        case (XV_HDMIRX_HANDLER_VIC_ERROR):
            InstancePtr->VicErrorCallback =
                                  (XV_HdmiRx_Callback)CallbackFunc;
            InstancePtr->VicErrorRef = CallbackRef;
            InstancePtr->IsVicErrorCallbackSet = (TRUE);
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
* This function is the interrupt handler for the HDMI RX Timing Detector
* (TIMDET) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx_VtdIntrHandler(XV_HdmiRx *InstancePtr)
{
    u32 Status;
    XVidC_VideoMode DecodedVmId = 0;

    /* Read Video timing detector Status register */
    Status = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress,
		    (XV_HDMIRX_VTD_STA_OFFSET));

    /* Check for time base event */
    if ((Status) & (XV_HDMIRX_VTD_STA_TIMEBASE_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX_VTD_STA_OFFSET),
			(XV_HDMIRX_VTD_STA_TIMEBASE_EVT_MASK));

        // Check if we are in lock state
        if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_LOCK) {

            // Read video timing
            Status = XV_HdmiRx_GetVideoTiming(InstancePtr);

            if (Status == XST_SUCCESS) {
                if (InstancePtr->Stream.Vic != 0) {
                    DecodedVmId = XV_HdmiRx_LookupVmId(InstancePtr->Stream.Vic);

                    if (DecodedVmId != InstancePtr->Stream.Video.VmId &&
				    !(DecodedVmId == XVIDC_VM_NOT_SUPPORTED &&
						    InstancePtr->Stream.Video.VmId ==
								    XVIDC_VM_CUSTOM)) {
                        /* Call VIC error callback */
                        if (InstancePtr->VicErrorCallback) {
				InstancePtr->VicErrorCallback(
						InstancePtr->VicErrorRef);
                        }
                    }
                }

                // Enable AXI Stream output
                XV_HdmiRx_AxisEnable(InstancePtr, (TRUE));

                // Set stream status to up
                InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_UP;

                // Set stream sync status to est
                InstancePtr->Stream.SyncStatus = XV_HDMIRX_SYNCSTAT_SYNC_EST;

                // Enable sync loss
                //XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress,
                //  (XV_HDMIRX_VTD_CTRL_SET_OFFSET), (XV_HDMIRX_VTD_CTRL_SYNC_LOSS_MASK));

                // Call stream up callback
                if (InstancePtr->IsStreamUpCallbackSet) {
                    InstancePtr->StreamUpCallback(InstancePtr->StreamUpRef);
                }
            }
        }

        // Check if we are in stream up state
        else if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_UP) {

            // Read video timing
            Status = XV_HdmiRx_GetVideoTiming(InstancePtr);

            if (Status != XST_SUCCESS) {
                // Disable sync loss
                //XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress,
                //  (XV_HDMIRX_VTD_CTRL_CLR_OFFSET), (XV_HDMIRX_VTD_CTRL_SYNC_LOSS_MASK));

                // Set stream status to up
                InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_LOCK;

            } else if (InstancePtr->Stream.SyncStatus == XV_HDMIRX_SYNCSTAT_SYNC_LOSS) {
            	// Sync Est/Recover Flag
            	InstancePtr->Stream.SyncStatus = XV_HDMIRX_SYNCSTAT_SYNC_EST;

				// Call sync lost callback
				if (InstancePtr->IsSyncLossCallbackSet) {
					InstancePtr->SyncLossCallback(InstancePtr->SyncLossRef);
				}
            }
        }

    }

    /* Check for sync loss event */
    else if ((Status) & (XV_HDMIRX_VTD_STA_SYNC_LOSS_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_VTD_STA_OFFSET), (XV_HDMIRX_VTD_STA_SYNC_LOSS_EVT_MASK));

        if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_UP) {
        	// Enable the Stream Up + Sync Loss Flag
        	InstancePtr->Stream.SyncStatus = XV_HDMIRX_SYNCSTAT_SYNC_LOSS;

			// Call sync lost callback
			if (InstancePtr->IsSyncLossCallbackSet) {
				InstancePtr->SyncLossCallback(InstancePtr->SyncLossRef);
			}
        }
    }
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX DDC peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx_DdcIntrHandler(XV_HdmiRx *InstancePtr)
{
    u32 Status;

    /* Read Status register */
    Status = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET));

    /* Check for HDCP write event */
    if ((Status) & (XV_HDMIRX_DDC_STA_HDCP_WMSG_NEW_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET), (XV_HDMIRX_DDC_STA_HDCP_WMSG_NEW_EVT_MASK));

        /* Callback */
        if (InstancePtr->IsHdcpCallbackSet) {
            InstancePtr->HdcpCallback(InstancePtr->HdcpRef, XV_HDMIRX_DDC_STA_HDCP_WMSG_NEW_EVT_MASK);
        }
    }

    /* Check for HDCP read event */
    if ((Status) & (XV_HDMIRX_DDC_STA_HDCP_RMSG_END_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET), (XV_HDMIRX_DDC_STA_HDCP_RMSG_END_EVT_MASK));

        /* Callback */
        if (InstancePtr->IsHdcpCallbackSet) {
            InstancePtr->HdcpCallback(InstancePtr->HdcpRef, XV_HDMIRX_DDC_STA_HDCP_RMSG_END_EVT_MASK);
        }
    }

    /* Check for HDCP read not complete event */
    if ((Status) & (XV_HDMIRX_DDC_STA_HDCP_RMSG_NC_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET), (XV_HDMIRX_DDC_STA_HDCP_RMSG_NC_EVT_MASK));

        /* Callback */
        if (InstancePtr->IsHdcpCallbackSet) {
            InstancePtr->HdcpCallback(InstancePtr->HdcpRef, XV_HDMIRX_DDC_STA_HDCP_RMSG_NC_EVT_MASK);
        }
    }

    /* Check for HDCP 1.4 Aksv event */
    if ((Status) & (XV_HDMIRX_DDC_STA_HDCP_AKSV_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET), (XV_HDMIRX_DDC_STA_HDCP_AKSV_EVT_MASK));

        /* Callback */
        if (InstancePtr->IsHdcpCallbackSet) {
            InstancePtr->HdcpCallback(InstancePtr->HdcpRef, XV_HDMIRX_DDC_STA_HDCP_AKSV_EVT_MASK);
        }
    }

    /* Check for HDCP 1.4 protocol event */
    if ((Status) & (XV_HDMIRX_DDC_STA_HDCP_1_PROT_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET), (XV_HDMIRX_DDC_STA_HDCP_1_PROT_EVT_MASK));

        /* Callback */
        if (InstancePtr->IsHdcp14ProtEvtCallbackSet) {
            InstancePtr->Hdcp14ProtEvtCallback(InstancePtr->Hdcp14ProtEvtRef);
        }
    }

    /* Check for HDCP 2.2 protocol event */
    if ((Status) & (XV_HDMIRX_DDC_STA_HDCP_2_PROT_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_DDC_STA_OFFSET), (XV_HDMIRX_DDC_STA_HDCP_2_PROT_EVT_MASK));

        /* Callback */
        if (InstancePtr->IsHdcp22ProtEvtCallbackSet) {
            InstancePtr->Hdcp22ProtEvtCallback(InstancePtr->Hdcp22ProtEvtRef);
        }
    }

}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx_PioIntrHandler(XV_HdmiRx *InstancePtr)
{
    u32 Event;
    u32 Data;

    /* Read PIO IN Event register.*/
    Event = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_PIO_IN_EVT_OFFSET));

    /* Clear event flags */
    XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_PIO_IN_EVT_OFFSET), (Event));

    /* Read data */
    Data = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_PIO_IN_OFFSET));

    // Cable detect event has occurred
    if ((Event) & (XV_HDMIRX_PIO_IN_DET_MASK)) {

        // Cable is connected
        if ((Data) & (XV_HDMIRX_PIO_IN_DET_MASK)) {
            // Set connected flag
            InstancePtr->Stream.IsConnected = (TRUE);
        }

        // Cable is disconnected
        else {
            // Clear connected flag
            InstancePtr->Stream.IsConnected = (FALSE);

            // Clear SCDC variables
            XV_HdmiRx_DdcScdcClear(InstancePtr);
        }

        // Check if user callback has been registered
        if (InstancePtr->IsConnectCallbackSet) {
            InstancePtr->ConnectCallback(InstancePtr->ConnectRef);
        }
    }

    // Link ready event has occurred
    if ((Event) & (XV_HDMIRX_PIO_IN_LNK_RDY_MASK)) {

    // Set stream status to idle
        InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_IDLE;            // The stream idle

        // Load timer
        XV_HdmiRx_TmrStart(InstancePtr,
			XV_HdmiRx_GetTime10Ms(InstancePtr)); // 10 ms
    }

    // Video ready event has occurred
    if ((Event) & (XV_HDMIRX_PIO_IN_VID_RDY_MASK)) {

        // Ready
        if ((Data) & (XV_HDMIRX_PIO_IN_VID_RDY_MASK)) {

            // Check the previous state
            // The link can only change to up when the previous state was init
            // Else there was a glitch on the video ready input
            if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_INIT) {

				/* Toggle video reset for HDMI RX core */
				XV_HdmiRx_INT_VRST(InstancePtr, TRUE);
				XV_HdmiRx_INT_VRST(InstancePtr, FALSE);

				/* Toggle bridge reset */
				XV_HdmiRx_EXT_VRST(InstancePtr, TRUE);
				XV_HdmiRx_EXT_SYSRST(InstancePtr, TRUE);
				XV_HdmiRx_EXT_VRST(InstancePtr, FALSE);
				XV_HdmiRx_EXT_SYSRST(InstancePtr, FALSE);

            // Set stream status to arm
                InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_ARM;         // The stream is armed

                // Load timer - 200 ms (one UHD frame is 40 ms, 5 frames)
                XV_HdmiRx_TmrStart(InstancePtr,
				XV_HdmiRx_GetTime200Ms(InstancePtr));
            }
        }

        // Stream down
        else {
			/* Assert HDMI RX core resets */
			XV_HdmiRx_INT_VRST(InstancePtr, TRUE);
			XV_HdmiRx_INT_LRST(InstancePtr, TRUE);

            /* Clear variables */
            XV_HdmiRx_Clear(InstancePtr);

            // Disable aux and audio peripheral
            // At this state the link clock is not stable.
            // Therefore these two peripheral are disabled to prevent any glitches.
            XV_HdmiRx_AuxDisable(InstancePtr);
            XV_HdmiRx_AudioDisable(InstancePtr);

            /* Disable VTD */
            XV_HdmiRx_VtdDisable(InstancePtr);

            // Disable link
            XV_HdmiRx_LinkEnable(InstancePtr, (FALSE));

            // Disable video
            // MH AI: Don't reset bridge when clock is not stable
            XV_HdmiRx_VideoEnable(InstancePtr, (TRUE));

            XV_HdmiRx_AxisEnable(InstancePtr, (FALSE));

            // Set stream status to down
            InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_DOWN;

            // Disable sync loss
            XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress,
                (XV_HDMIRX_VTD_CTRL_CLR_OFFSET), (XV_HDMIRX_VTD_CTRL_SYNC_LOSS_MASK));

            // Call stream down callback
            if (InstancePtr->IsStreamDownCallbackSet) {
                InstancePtr->StreamDownCallback(InstancePtr->StreamDownRef);
            }
        }
    }

    // SCDC Scrambler Enable
    if ((Event) & (XV_HDMIRX_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK)) {

        // Enable scrambler
        if ((Data) & (XV_HDMIRX_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK)) {
            XV_HdmiRx_SetScrambler(InstancePtr, (TRUE));
        }

        // Disable scrambler
        else {
            XV_HdmiRx_SetScrambler(InstancePtr, (FALSE));
        }
    }

    // Mode
    if ((Event) & (XV_HDMIRX_PIO_IN_MODE_MASK)) {

        // HDMI Mode
        if ((Data) & (XV_HDMIRX_PIO_IN_MODE_MASK)) {
            InstancePtr->Stream.IsHdmi = (TRUE);
        }

        // DVI Mode
        else {
            InstancePtr->Stream.IsHdmi = (FALSE);
        }

        if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_UP) {

            /* Clear variables */
            XV_HdmiRx_Clear(InstancePtr);

        	// Set stream status to idle
            InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_IDLE;

            // Load timer
            XV_HdmiRx_TmrStart(InstancePtr,
			XV_HdmiRx_GetTime10Ms(InstancePtr)); // 10 ms
        }

        // Call mode callback
        if (InstancePtr->IsModeCallbackSet) {
            InstancePtr->ModeCallback(InstancePtr->ModeRef);
        }
    }

    // TMDS clock ratio
    if ((Event) & (XV_HDMIRX_PIO_IN_SCDC_TMDS_CLOCK_RATIO_MASK)) {
        // Call TMDS Ratio callback
        if (InstancePtr->IsTmdsClkRatioCallbackSet) {
            InstancePtr->TmdsClkRatioCallback(InstancePtr->TmdsClkRatioRef);
        }
    }

    /* Bridge Overflow event has occurred */
    if ((Event) & (XV_HDMIRX_PIO_IN_BRDG_OVERFLOW_MASK)) {
        // Check if user callback has been registered
        if (InstancePtr->IsBrdgOverflowCallbackSet) {
            InstancePtr->BrdgOverflowCallback(InstancePtr->BrdgOverflowRef);
        }
    }

}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX TMR peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx_TmrIntrHandler(XV_HdmiRx *InstancePtr)
{
    u32 Status;

    /* Read Status register */
    Status = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_TMR_STA_OFFSET));

    /* Check for counter event */
    if ((Status) & (XV_HDMIRX_TMR_STA_CNT_EVT_MASK)) {

        // Clear counter event
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_TMR_STA_OFFSET), (XV_HDMIRX_TMR_STA_CNT_EVT_MASK));

        // Idle state
        if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_IDLE) {

            // The link is stable now
            // Then the aux and audio peripherals can be enabled
            XV_HdmiRx_AuxEnable(InstancePtr);
            XV_HdmiRx_AudioEnable(InstancePtr);

			/* Release HDMI RX core resets */
			XV_HdmiRx_INT_VRST(InstancePtr, FALSE);
			XV_HdmiRx_INT_LRST(InstancePtr, FALSE);

            // Enable link
            XV_HdmiRx_LinkEnable(InstancePtr, (TRUE));

            // Set stream status to init
            InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_INIT;    // The stream init

            // Clear GetVideoPropertiesTries
            InstancePtr->Stream.GetVideoPropertiesTries = 0;

            // Load timer - 200 ms (one UHD frame is 40 ms, 5 frames)
            XV_HdmiRx_TmrStart(InstancePtr,
			XV_HdmiRx_GetTime200Ms(InstancePtr));
        }

        // Init state
        else if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_INIT) {

            // Read video properties
            if (XV_HdmiRx_GetVideoProperties(InstancePtr) == XST_SUCCESS) {

            // Now we know the reference clock and color depth,
            // the pixel clock can be calculated
            // In case of YUV 422 the reference clock is the pixel clock
            if (InstancePtr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_422) {
                InstancePtr->Stream.PixelClk = InstancePtr->Stream.RefClk;
            }

            // For the other color spaces the pixel clock needs to be adjusted
            else {

                switch (InstancePtr->Stream.Video.ColorDepth) {
                    case XVIDC_BPC_10:
                        InstancePtr->Stream.PixelClk = (InstancePtr->Stream.RefClk * 4)/5;
                        break;

                    case XVIDC_BPC_12:
                        InstancePtr->Stream.PixelClk = (InstancePtr->Stream.RefClk * 2)/3;
                        break;

                    case XVIDC_BPC_16:
                        InstancePtr->Stream.PixelClk = InstancePtr->Stream.RefClk / 2;
                        break;

                    default:
                        InstancePtr->Stream.PixelClk = InstancePtr->Stream.RefClk;
                        break;
                }
            }

				// Call stream init callback
				if (InstancePtr->IsStreamInitCallbackSet) {
					InstancePtr->StreamInitCallback(InstancePtr->StreamInitRef);
				}
			}

            else {
		// Load timer - 200 ms (one UHD frame is 40 ms, 5 frames)
                XV_HdmiRx_TmrStart(InstancePtr,
				XV_HdmiRx_GetTime200Ms(InstancePtr));
            }
        }

        // Armed state
        else if (InstancePtr->Stream.State == XV_HDMIRX_STATE_STREAM_ARM) {

            /* Enable VTD */
            XV_HdmiRx_VtdEnable(InstancePtr);

            /* Enable interrupt */
            XV_HdmiRx_VtdIntrEnable(InstancePtr);

            // Set stream status to lock
            InstancePtr->Stream.State = XV_HDMIRX_STATE_STREAM_LOCK;
        }
    }
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX AUX peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx_AuxIntrHandler(XV_HdmiRx *InstancePtr)
{
    u32 Status;
    u8 Index;

    /* Read Status register */
    Status = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUX_STA_OFFSET));

    /* Check for GCP colordepth event */
    if ((Status) & (XV_HDMIRX_AUX_STA_GCP_CD_EVT_MASK)) {
    	/* Clear event flag */
    	XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUX_STA_OFFSET), (XV_HDMIRX_AUX_STA_GCP_CD_EVT_MASK));

    	if ((Status) & (XV_HDMIRX_AUX_STA_GCP_MASK)) {
    		InstancePtr->Stream.Video.ColorDepth = XV_HdmiRx_GetGcpColorDepth(InstancePtr);
    	}
    }

    /* Check for new packet */
    if ((Status) & (XV_HDMIRX_AUX_STA_NEW_MASK)) {
        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUX_STA_OFFSET), (XV_HDMIRX_AUX_STA_NEW_MASK));

        /* Set HDMI flag */
        InstancePtr->Stream.IsHdmi = (TRUE);

        /* Read header word and update AUX header field */
        InstancePtr->Aux.Header.Data = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUX_DAT_OFFSET));

        for (Index = 0x0; Index < 8; Index++) {
            /* Read data word and update AUX data field */
            InstancePtr->Aux.Data.Data[Index] = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUX_DAT_OFFSET));
        }

        /* Callback */
        if (InstancePtr->IsAuxCallbackSet) {
            InstancePtr->AuxCallback(InstancePtr->AuxRef);
        }
    }

    /* Link integrity check */
    if ((Status) & (XV_HDMIRX_AUX_STA_ERR_MASK)) {
		XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUX_STA_OFFSET), (XV_HDMIRX_AUX_STA_ERR_MASK));

        /* Callback */
        if (InstancePtr->IsLinkErrorCallbackSet) {
			InstancePtr->LinkErrorCallback(InstancePtr->LinkErrorRef);
		}
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX AUD peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx_AudIntrHandler(XV_HdmiRx *InstancePtr)
{
    u32 Status;

    // Read Status register
    Status = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUD_STA_OFFSET));

    // Check for active stream event
    if ((Status) & (XV_HDMIRX_AUD_STA_ACT_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUD_STA_OFFSET), (XV_HDMIRX_AUD_STA_ACT_EVT_MASK));

        InstancePtr->Stream.Audio.Active = (TRUE);
    }

    // Check for audio channel event
    if ((Status) & (XV_HDMIRX_AUD_STA_CH_EVT_MASK)) {

        // Clear event flag
        XV_HdmiRx_WriteReg(InstancePtr->Config.BaseAddress, (XV_HDMIRX_AUD_STA_OFFSET), (XV_HDMIRX_AUD_STA_CH_EVT_MASK));

        // Active channels
	switch ((Status >> XV_HDMIRX_AUD_STA_AUD_CH_SHIFT) &
			XV_HDMIRX_AUD_STA_AUD_CH_MASK) {
            // 32 channels
	case 6 :
		InstancePtr->Stream.Audio.Channels = 32;
		break;

	// 24 channels
	case 5 :
		InstancePtr->Stream.Audio.Channels = 24;
		break;

	// 12 channels
	case 4 :
		InstancePtr->Stream.Audio.Channels = 12;
		break;

	// 8 channels
	case 3 :
		InstancePtr->Stream.Audio.Channels = 8;
		break;

	// 6 channels
	case 2 :
		InstancePtr->Stream.Audio.Channels = 6;
		break;

	// 4 channels
	case 1 :
		InstancePtr->Stream.Audio.Channels = 4;
		break;

	// 2 channels
	default :
		InstancePtr->Stream.Audio.Channels = 2;
		break;
	}

        // Audio Format
        InstancePtr->AudFormat = (XV_HdmiRx_AudioFormatType)((Status >> XV_HDMIRX_AUD_STA_AUD_FMT_SHIFT) & XV_HDMIRX_AUD_STA_AUD_FMT_MASK);

        /* Callback */
        if (InstancePtr->IsAudCallbackSet) {
            InstancePtr->AudCallback(InstancePtr->AudRef);
        }
    }
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX Link Status
* (LNKSTA) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx_LinkStatusIntrHandler(XV_HdmiRx *InstancePtr)
{
    /* Callback */
    if (InstancePtr->IsLnkStaCallbackSet) {
        InstancePtr->LnkStaCallback(InstancePtr->LnkStaRef);
    }
}
