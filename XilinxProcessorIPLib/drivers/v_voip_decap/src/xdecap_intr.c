/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdecap_intr.c
*
* This file contains interrupt related functions for Xilinx VoIP Decapsulator
* core.
* Please see xdecap.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00   mmo   02/12/16 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xdecap.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if XDECAP_MAX_CHANNEL > 16
#define XDCECAP_INTERRUPT_CHANNEL 16
#else
#define XDCECAP_INTERRUPT_CHANNEL XDECAP_MAX_CHANNEL
#endif

/**************************** Type Definitions *******************************/

/**
* This typedef contains Current Interrupt Information
*/
typedef struct ChArray{
    u32 Size;
    u8  IntrHWStatus [XDCECAP_INTERRUPT_CHANNEL];
    u32 IntrChannels [XDCECAP_INTERRUPT_CHANNEL];
    u32 IntrStatus   [XDCECAP_INTERRUPT_CHANNEL];
} XDecapChIntr;
/************************** Function Prototypes ******************************/
static void XDecap_PacketLockHandler(XDecap *InstancePtr);
static void XDecap_PacketUnLockHandler(XDecap *InstancePtr);
static void XDecap_PacketStopHandler(XDecap *InstancePtr);
XDecapChIntr XDecap_ChIntr (XDecap *InstancePtr);
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the VoIP Decapsulator driver.
*
* This handler reads the pending interrupt from VoIP Decapsulator, determines
* the source of the interrupts, clears the interrupts and calls callbacks
* accordingly.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDecap_SetCallback() during initialization phase. An example delivered
* with this driver demonstrates how this could be done.
*
* @param    InstancePtr is a pointer to the XDecap instance that just
*       interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_IntrHandler(void *InstancePtr)
{
    u32 Data=0;
    u32 Channel=0;
    u32 IntrStat_RegValue=0;
    u16 Index=0;
    u8  Curr_Lock_Status=0;

    u32 Lock = 0;
    u32 UnLock = 0;
    u32 Stop = 0;
    u8  Final_UnLock = 0;

    XDecap *XDecapPtr = (XDecap *)InstancePtr;
    XDecapChIntr  XDecapChIntrVal;

    /* Verify arguments */
    Xil_AssertVoid(XDecapPtr != NULL);
    Xil_AssertVoid(XDecapPtr->IsReady == XIL_COMPONENT_IS_READY);

    XDecapChIntrVal.Size = 0;

    for (Index = 0x00; Index < XDCECAP_INTERRUPT_CHANNEL; Index++) {
        XDecapChIntrVal.IntrHWStatus[Index] = 0;
        XDecapChIntrVal.IntrStatus[Index] = 0;
        XDecapChIntrVal.IntrChannels[Index]= 0;
    }

    XDecapChIntrVal = XDecap_ChIntr(XDecapPtr);

    for (Index = 0x00; Index < XDecapChIntrVal.Size; Index++) {
        /* Assigning Interrupt Channel to structure */
        XDecapPtr->IntrCh = XDecapChIntrVal.IntrChannels[Index];

        /* Assigning the Interrupt Status Register Value */
        IntrStat_RegValue = XDecapChIntrVal.IntrStatus[Index];

        /* Access the Interrupt Channel */
        XDecap_ChannelAccess(XDecapPtr, XDecapPtr->IntrCh);

        Stop  = 0;
        Stop  = ((IntrStat_RegValue &
            (XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_MASK)) >>
                 (XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_SHIFT));

        if (Stop) {
            /* To Differentiate, Stream Change / Link Distruption */
            XDecapPtr->PacketStopInterrupt = (TRUE);
        }

        UnLock = 0;
        UnLock = ((IntrStat_RegValue &
            (XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_MASK)) >>
                   (XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_SHIFT));

        if (UnLock) {
            /* To Differentiate, Stream Change / Link Distruption */
            XDecapPtr->PacketUnLockInterrupt = (TRUE);
        }

        Lock  = 0;
        Lock = (IntrStat_RegValue &
                      (XDECAP_INTERRUPT_STATUS_PACKET_LOCKED_MASK));

        Final_UnLock = 0;
        if (UnLock || Stop) {
           Final_UnLock = 1;
        }

        Curr_Lock_Status = XDecapChIntrVal.IntrHWStatus[Index];

        switch (Curr_Lock_Status) {
           case 0 : /* UnLock */
                if (Final_UnLock) {
                  XDecapPtr->IsPacketUnLockCallbackSet = (TRUE);
                  /* Jump to Video Un-Lock handler */
                  XDecapPtr->FinalUnLockInterrupt = (TRUE);
                  XDecap_PacketUnLockHandler(XDecapPtr);
                } else {
                  xil_printf("Error RTL Bug\n\r");
                }
                break;
           case 1 : /* Lock */
                if (Lock == 1) {
                   if (Final_UnLock) {
                      XDecapPtr->IsPacketUnLockCallbackSet = (TRUE);
                      /* Jump to Video Un-Lock handler */
                      XDecapPtr->FinalUnLockInterrupt = (TRUE);
                      XDecap_PacketUnLockHandler(XDecapPtr);
                   }
                   XDecapPtr->IsPacketLockCallbackSet = (TRUE);
                   XDecapPtr->PacketLockInterrupt = (TRUE);
                   /* Jump to Video Lock handler */
                   XDecap_PacketLockHandler(XDecapPtr);
                }
                else {
                   xil_printf("Error RTL Bug\n\r");
                }
                break;
           default :
                xil_printf("Error RTL Bug\n\r");
           break;
        }

        Data=0;
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
* </pre>
*
* @param    InstancePtr is a pointer to the VoIP Decapsulator core instance.
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
int XDecap_SetCallback(XDecap *InstancePtr, XDecap_HandlerType HandlerType,
            void *CallbackFunc, void *CallbackRef)
{
    u32 Status;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(HandlerType >= (XDECAP_PACKET_HANDLER_CONNECT));
    Xil_AssertNonvoid(CallbackFunc != NULL);
    Xil_AssertNonvoid(CallbackRef != NULL);

    /* Check for handler type */
    switch (HandlerType) {
      case (XDECAP_PACKET_HANDLER_LOCKED):
           InstancePtr->PacketLockCallback = (XDecap_Callback)CallbackFunc;
           InstancePtr->PacketLockRef = CallbackRef;
           InstancePtr->IsPacketLockCallbackSet = (TRUE);
           Status = (XST_SUCCESS);
      break;
      case (XDECAP_PACKET_HANDLER_UNLOCKED):
           InstancePtr->PacketUnLockCallback = (XDecap_Callback)CallbackFunc;
           InstancePtr->PacketUnLockRef = CallbackRef;
           InstancePtr->IsPacketUnLockCallbackSet = (TRUE);
           Status = (XST_SUCCESS);
      break;
      case (XDECAP_PACKET_HANDLER_STOP):
           InstancePtr->PacketStopCallback = (XDecap_Callback)CallbackFunc;
           InstancePtr->PacketStopRef = CallbackRef;
           InstancePtr->IsPacketStopCallbackSet = (TRUE);
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
* This function is the VoIP Decapsulator Packet Lock Interrupt Handler
*
* This handler reads corresponding event interrupt from the Lock Interrupt
* Status register. It determines the source of the interrupts and calls
* according callbacks.
*
* @param    InstancePtr is a pointer to the VoIP Decapsulator core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void XDecap_PacketLockHandler(XDecap *InstancePtr)
{
    u32 RegValue;
    u16 Index;

    XDecap_ChannelAccess(InstancePtr, InstancePtr->IntrCh);

    /* Reads the Video Format and Update the Data Structure */
    RegValue = XDecap_ReadReg((InstancePtr->Config.BaseAddress),
                      (XDECAP_MEDIA_HEADER_OFFSET));

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.video_ts_include =
              ((RegValue) & (XDECAP_MEDIA_HEADER_VIDEO_TS_INCLUDE_MASK));

    RegValue = XDecap_ReadReg((InstancePtr->Config.BaseAddress),
            (XDECAP_VIDEO_FORMAT_OFFSET));

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.rx_bitrate =
        ((RegValue) &
            (XDECAP_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK)) >>
                    (XDECAP_VIDEO_FORMAT_BITRATE_DIV_1_001_SHIFT);

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.level_b_3g =
        ((RegValue) &
            (XDECAP_VIDEO_FORMAT_3G_LEVEL_B_MASK)) >>
                    (XDECAP_VIDEO_FORMAT_3G_LEVEL_B_SHIFT);

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.sdi_mode =
        ((RegValue) &
            (XDECAP_VIDEO_FORMAT_VIDEO_MODE_MASK));

    /* Reads the MAP,FRAME,FRATE & SAMPLE Update the Data Structure */
    RegValue = XDecap_ReadReg((InstancePtr->Config.BaseAddress),
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_OFFSET));

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.MAP =
        ((RegValue) &
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK)) >>
                    (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT);

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.FRAME =
        ((RegValue) &
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK)) >>
                    (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT);

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.FRATE =
        ((RegValue) &
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK)) >>
                    (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT);

    InstancePtr->ChannelCfg[InstancePtr->IntrCh].StreamInfo.SAMPLE =
        ((RegValue) &
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK)) >>
                    (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT);

    /* Callback */
    if (InstancePtr->IsPacketLockCallbackSet) {
        InstancePtr->PacketLockCallback (InstancePtr->PacketLockRef);
    }

    InstancePtr->PacketLockInterrupt = (FALSE);
}

/*****************************************************************************/
/**
*
* This function is the VoIP Decapsulator Packet Un-Lock Interrupt Handler
*
* This handler reads corresponding event interrupt from the Un-Lock Interrupt
* Status register. It determines the source of the interrupts and calls
* according callbacks.
*
* @param    InstancePtr is a pointer to the VoIP Decapsulator core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void XDecap_PacketUnLockHandler(XDecap *InstancePtr)
{

    XDecap_ChannelAccess(InstancePtr, InstancePtr->IntrCh);

    /* Callback */
    if (InstancePtr->IsPacketUnLockCallbackSet) {
        InstancePtr->PacketUnLockCallback (InstancePtr->PacketUnLockRef);
    }

    InstancePtr->FinalUnLockInterrupt  = (FALSE);
    InstancePtr->PacketUnLockInterrupt = (FALSE);
    InstancePtr->PacketStopInterrupt   = (FALSE);

}

/*****************************************************************************/
/**
*
* This function is the VoIP Decapsulator Channel Time-out Interrupt Handler
*
* This handler reads corresponding event interrupt from the Time-out Interrupt
* Status register. It determines the source of the interrupts and calls
* according callbacks.
*
* @param    InstancePtr is a pointer to the VoIP Decapsulator core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void XDecap_PacketStopHandler(XDecap *InstancePtr)
{

    XDecap_ChannelAccess(InstancePtr, InstancePtr->IntrCh);

    /* Callback */
    if (InstancePtr->IsPacketStopCallbackSet) {
        InstancePtr->PacketStopCallback (InstancePtr->PacketStopRef);
    }

    InstancePtr->PacketStopInterrupt = (FALSE);
}


/*****************************************************************************/
/**
*
* This function scans through the Interrupt General Register to detect & store
* the information of the interrupted channel.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XDecapChIntr XDecap_ChIntr (XDecap *InstancePtr)
{
    u32 IntrGroup = 0x0;
    u32 ChGroup = 0x0;
    XDecapChIntr XDecapChIntr_Check;

    u32 Index  = 0x0;
    u32 Result = 0x0;
    u32 zeroes = 0x0;

    /* Setting the Initial value to zero */
    XDecapChIntr_Check.Size = 0;

    /* Setting all the variable in the structure to zero */
    for (Index = 0x00; Index < XDCECAP_INTERRUPT_CHANNEL; Index++) {
        XDecapChIntr_Check.IntrHWStatus[Index] = 0;
        XDecapChIntr_Check.IntrStatus[Index] = 0;
        XDecapChIntr_Check.IntrChannels[Index]= 0;
    }

    /* Interrupt Group */
    IntrGroup = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
                       (XDECAP_CH_INTR_GROUP_SUMMARY_OFFSET));

    /* Channel Group for Channel 0-31 */
    if (((IntrGroup) & (XDECAP_CH_INTR_GROUP_SUMMARY_G0_MASK))) {
        ChGroup = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
                                              (XDECAP_CH_INTR_GROUP_0_OFFSET));

        /* Skip if not this Interrupt Group */
        if (ChGroup != 0) {
            for (Index = 0x00; Index < 32; Index++){
               if ((ChGroup) & (0x00000001)) {
                  XDecapChIntr_Check.IntrChannels[XDecapChIntr_Check.Size] =
                                                                        Index;

                  /* Access to the Channel Interrupt */
                  XDecap_ChannelAccess(InstancePtr, Index);

                  /* Get the current HW Status */
                  XDecapChIntr_Check.IntrHWStatus[XDecapChIntr_Check.Size] =
                              XDecap_SDIPacketLockStatus (InstancePtr, Index);

                  /* Read the Interrupt Status */
                  XDecapChIntr_Check.IntrStatus[XDecapChIntr_Check.Size] =
                    (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
                        (XDECAP_INTERRUPT_STATUS_OFFSET)) &
                            (XDECAP_INTERRUPT_STATUS_MASK));

                  /* Clear the Interrupt Register */
                  XDecap_WriteReg((InstancePtr->Config.BaseAddress),
                   (XDECAP_INTERRUPT_CLEAR_OFFSET),
                     (XDecapChIntr_Check.IntrStatus[XDecapChIntr_Check.Size]));

                  /* Update the Channel */
                  XDecap_ChannelUpdate(InstancePtr);

                  /* Scan the Next Channel */
                  XDecapChIntr_Check.Size++;
               }

               /* If No more Interrupt End the searching */
               if (ChGroup == 1) {
                  break;
               }

               ChGroup = ChGroup>>1;
            }
        }
    }

    return(XDecapChIntr_Check);
}
