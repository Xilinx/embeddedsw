/******************************************************************************
* Copyright (C) 2008 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xavb.c
*
* The top level c file for the AVB driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a mbr  09/19/08 First release
* 1.01a mbr  06/24/09 PTP frame format updates for IEEE802.1 AS draft 5-0
* 2_02a mbr  09/16/09 Updates for programmable PTP timers
* 2_04a kag  07/23/10 PTP frame format updates for IEEE802.1 AS draft 6-7
* 3_01a kag  08/29/11 Added new APIs to update the RX Filter Control Reg.
*		      Fix for CR:572539. Updated bit map for Rx Filter
*		      control reg.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"
#include "xil_assert.h"
#include "xenv.h"
#include "xavb_hw.h"
#include "xavb.h"
#include "xparameters.h"
#include "stdio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern XAvb_Config XAvb_ConfigTable[];
extern volatile u8 EchoPTPFramesReceived;

/************************** Function Prototypes ******************************/
static void XAvb_StubHandler(void *CallBackRef, u32 TimestampsUncertain);

/*****************************************************************************/

/*
 * Mandatory Device Driver Functions
 */

/****************************************************************************/
/**
*
* A function to initialise variables in the data structure records
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  ConfigPtr is the Configuration Pointer
* @param  EffectiveAddress is the base address of the Configuration Pointer
*
* @return
*   - XST_SUCCESS if initialization was successful
*   - XST_DEVICE_NOT_FOUND if device configuration information was not found
*   for a device with the supplied device ID.*
* @note   None.
*
*****************************************************************************/
XStatus XAvb_CfgInitialize(XAvb *InstancePtr,
                           XAvb_Config *ConfigPtr,
                           u32 EffectiveAddress) {

  Xil_AssertNonvoid(InstancePtr != NULL);

  /** The component is not yet ready to use */
  InstancePtr->IsReady = 0;

  /** Clear instance memory */
  memset(InstancePtr, 0, sizeof(XAvb));
  memcpy(&InstancePtr->Config, ConfigPtr, sizeof(XAvb_Config));

  /**
   * Lookup the device configuration in the temporary CROM table. Use this
   * configuration info down below when initializing this component.
   */
  if (ConfigPtr == NULL) {
    return XST_DEVICE_NOT_FOUND;
  }

  /**
   * Populate Base Address field using the base address value in the
   * configuration structure.
   */
  InstancePtr->Config.BaseAddress = EffectiveAddress;

  /**
   * Indicate the component is now ready to use.
   */
  InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

   /**
   * Set the callback handler to a stub
   */
  InstancePtr->GMDiscHandler = XAvb_StubHandler;

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function resets all of the AVB device driver functions to the start-up
* (reset) defaults.
*
* @param  InstancePtr is a pointer to the Xavb instance to be worked on.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XAvb_Reset(XAvb * InstancePtr)
{
  /** Assert bad arguments and conditions */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

  /**
   * Perform a Software Reset of the AVB Core.
   * This will reset both the transmitter and receiver paths.
   * The RTC counter is not reset here.
   */
  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_SW_RESET_OFFSET,
                 XAVB_SW_RESET_TX_AND_RX_PATHS);

  /**
   * Set IEEE specification default values in the device's data structure
   */
  xil_printf("\r\n*** XAvb_Reset() : Call XAvb_BecomeRtcMaster() *** \r\n");
  XAvb_BecomeRtcMaster(InstancePtr,0);
  XAvb_ChangePeerASCapability(InstancePtr, 0);

  InstancePtr->PtpIsRunning                                  = 0;
  InstancePtr->PtpRecords.LinkDelay                          = 0;
  InstancePtr->SignallingFrameData.SyncIntervalDuration      =
    XAvb_ConvertLogMeanToDuration(XAVB_DEFAULT_LOG_MEAN_SYNC_INTERVAL);
  InstancePtr->SignallingFrameData.LinkDelayIntervalDuration =
    XAvb_ConvertLogMeanToDuration(XAVB_DEFAULT_LOG_MEAN_PDELAY_REQ_INTERVAL);
  InstancePtr->SignallingFrameData.AnnounceIntervalDuration  =
    XAvb_ConvertLogMeanToDuration(XAVB_DEFAULT_LOG_MEAN_ANNOUNCE_INTERVAL);

  InstancePtr->latestMDSyncReceive.SyncIntervalDuration     =
     XAvb_ConvertLogMeanToDuration(XAVB_DEFAULT_LOG_MEAN_SYNC_INTERVAL);

  /** Update logMeanMessageInterval in the pre-loaded TX SYNC message buffer */
  XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                    XAVB_PTP_TX_SYNC_OFFSET,
                                    InstancePtr->SignallingFrameData.SyncIntervalDuration);
  /** Update logMeanMessageInterval in the pre-loaded TX FOLLOW_UP message buffer */
  XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                    XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                                    InstancePtr->SignallingFrameData.SyncIntervalDuration);

  /** Update logMeanMessageInterval in the pre-loaded TX PDELAYREQ message buffer */
  XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                    XAVB_PTP_TX_PDELAYREQ_OFFSET,
                                    InstancePtr->SignallingFrameData.LinkDelayIntervalDuration);

  /** Update logMeanMessageInterval in the pre-loaded TX ANNOUNCE message buffer  */
  XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                    XAVB_PTP_TX_ANNOUNCE_OFFSET,
                                    InstancePtr->SignallingFrameData.AnnounceIntervalDuration);

  /**
   * Initialise other driver variables in the device's data structure
   */
  InstancePtr->PtpCounters.RxPtpHardPointer               = 0;
  InstancePtr->PtpCounters.RxPtpSoftPointer               = 0xFF;
  InstancePtr->PtpCounters.CounterSyncInterval            = 0;
  InstancePtr->PtpCounters.CounterLinkDelayInterval       = 0;
  InstancePtr->PtpCounters.CounterAnnounceInterval        = 0;
  InstancePtr->PtpCounters.CounterSyncEvents              = 0;
  InstancePtr->StateMachineData.lostResponses             = 0;
  InstancePtr->StateMachineData.rcvdPDelayResp            = 0;
  InstancePtr->StateMachineData.rcvdPDelayRespFollowUp    = 0;
#ifdef DEBUG_XAVB_LEVEL1
  xil_printf("\r\n** XAvb_Reset(): PTP Driver Reset **");
#endif
}


/*
 * Device Specific Driver Functions
 */


/*****************************************************************************/
/**
*
* This function will start the PTP drivers running.
*
* @param  InstancePtr is a pointer to the Xavb instance to be worked on.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XAvb_Start(XAvb * InstancePtr)
{
  /** Assert bad arguments and conditions */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

  /** Re-run the BMCA algorithm with the current PTP buffer Announce Packet */
  XAvb_DecodeTxAnnounceFrame(InstancePtr);

  /** Set to PTP running in the PTP data structure */
  InstancePtr->PtpIsRunning   = 1;

  /** Assume that the Peer is not AS capable until it replies to a pDelay_Req
    * frame */
  XAvb_ChangePeerASCapability(InstancePtr, 0);

#ifdef DEBUG_XAVB_LEVEL1
  xil_printf("\r\n** XAvb_Start(): Starting PTP **");
#endif
}


/*****************************************************************************/
/**
*
* This function will stop the PTP drivers from running.
*
* @param  InstancePtr is a pointer to the Xavb instance to be worked on.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XAvb_Stop(XAvb * InstancePtr)
{
  /** Assert bad arguments and conditions */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#ifdef DEBUG_XAVB_LEVEL1
  xil_printf("\r\n** XAvb_Stop(): PTP STOPPED **");
#endif
  XAvb_Reset(InstancePtr);
}


/****************************************************************************/
/**
*
* The Interrupt subroutine for the "interruptPtpTimer" signal.  This interrupt
* fires regularly on a 1/128 second period (based on the RTC).
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_PtpTimerInterruptHandler(XAvb * InstancePtr) {

  /** Clear Interrupt */
  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_RTC_CLEAR_INT_OFFSET,
                 XAVB_RTC_CLEAR_INT_MASK);

  /** If PTP functions are marked as not running, then take no further action */
  if (InstancePtr->PtpIsRunning == 1) {

    /** If the Link Partner is not AS capable, then take no further action */
    if (InstancePtr->PeerASCapable == 1) {


      /** If a Master, then initiate Sync Frames and Announce frames at the
       *  correct intervals
       */
      if (InstancePtr->CurrentBmc.IAmTheRtcMaster == 1) {

         /** Master will initiate a Sync Frame when the SyncIntervalDuration
          *  expires (SyncIntervalDuration is used to count/time the duration)
          * - unless a Signalling frame has told us not to send Sync Frames
          */
         if ((InstancePtr->SignallingFrameData.SyncIntervalDuration != XAVB_PKT_TYPE_DISABLED) &&
             (InstancePtr->PtpCounters.CounterSyncInterval >=
            (InstancePtr->SignallingFrameData.SyncIntervalDuration-1))) {

           XAvb_MasterSendSync(InstancePtr);
           InstancePtr->PtpCounters.CounterSyncInterval = 0;

           /** Following a Sync Frame, a Follow Up frame should always be sent
            */
           XAvb_MasterSendFollowUp(InstancePtr);


         } else {
           InstancePtr->PtpCounters.CounterSyncInterval
               = InstancePtr->PtpCounters.CounterSyncInterval + 1;
         }

         /** Master will initiate an Announce Frame when the
          *  AnnounceIntervalDuration expires (CounterAnnounceInterval is used
          *  to count/time the duration)
          * - unless a Signalling frame has told us not to send Announce Frames
          */
         if ((InstancePtr->SignallingFrameData.AnnounceIntervalDuration != XAVB_PKT_TYPE_DISABLED) &&
             (InstancePtr->PtpCounters.CounterAnnounceInterval >=
              (InstancePtr->SignallingFrameData.AnnounceIntervalDuration-1))) {

           XAvb_MasterSendAnnounce(InstancePtr);
           InstancePtr->PtpCounters.CounterAnnounceInterval = 0;

         } else {
           InstancePtr->PtpCounters.CounterAnnounceInterval
               = InstancePtr->PtpCounters.CounterAnnounceInterval + 1;
         }

      /** If a Slave, monitor Announce/Sync Packet reception from the Master */
      } else {

        /** Timeout for Announce Packet reception: XAVB_ANNOUNCE_RECEIPT_TIMEOUT
         *  The AnnounceIntervalDuration is stored with the GrandMaster BMCA data
         *  as it is captured from the last Announce frame that was received.
         */
        if (InstancePtr->PtpCounters.CounterAnnounceInterval >=
            ((InstancePtr->CurrentBmc.AnnounceIntervalDuration-1) *
             XAVB_ANNOUNCE_RECEIPT_TIMEOUT) ) {

#ifdef XAVB_DEBUG_LEVEL1
          xil_printf("XAVB_ANNOUNCE_RECEIPT_TIMEOUT: Becoming GM! CounterAnnounceInterval = %d\r\n", InstancePtr->PtpCounters.CounterAnnounceInterval);
#endif

          InstancePtr->PtpCounters.CounterAnnounceInterval = 0;

          /** No Announce received from GM for timeout interval: we become the master */
          xil_printf("\r\n*** Announce timeout : Call XAvb_BecomeRtcMaster() *** \r\n");
          XAvb_BecomeRtcMaster(InstancePtr,0);

        } else {
          InstancePtr->PtpCounters.CounterAnnounceInterval
              = InstancePtr->PtpCounters.CounterAnnounceInterval + 1;
        }

        /** Timeout for Sync Packet reception: XAVB_SYNC_RECEIPT_TIMEOUT *
         *  The SyncIntervalDuration is stored with the Received Sync data
         *  as it is captured from the last Sync frame that was received.
         */
        if( InstancePtr->PtpCounters.CounterSyncInterval >=
            ((InstancePtr->latestMDSyncReceive.SyncIntervalDuration-1) *
             XAVB_SYNC_RECEIPT_TIMEOUT) ) {

#ifdef XAVB_DEBUG_LEVEL1
          xil_printf("\r\nXAVB_SYNC_RECEIPT_TIMEOUT: Becoming GM! CounterSyncInterval = %d\r\n", InstancePtr->PtpCounters.CounterSyncInterval);
          xil_printf("\r\nXAVB_SYNC_RECEIPT_TIMEOUT: SyncIntervalDuration = %d\r\n", InstancePtr->SignallingFrameData.SyncIntervalDuration);
#endif

          InstancePtr->PtpCounters.CounterSyncInterval = 0;


          /** No Syncs received from GM for timeout interval: we become
           *  the master */
          xil_printf("\r\n*** Sync Timeout : Call XAvb_BecomeRtcMaster() *** \r\n");
          XAvb_BecomeRtcMaster(InstancePtr,0);

        } else {
          InstancePtr->PtpCounters.CounterSyncInterval
              = InstancePtr->PtpCounters.CounterSyncInterval + 1;
        }
      }
    }

    /** Both Master and Slave will initiate a link delay measurement when the
     *  LinkDelayIntervalDuration expires (LinkDelayIntervalDuration is used to
     *  count/time the duration)
     * - unless a Signalling frame has told us not to send PdelayReq Frames
     */
    if ((InstancePtr->SignallingFrameData.LinkDelayIntervalDuration != XAVB_PKT_TYPE_DISABLED) &&
        (InstancePtr->PtpCounters.CounterLinkDelayInterval >=
         (InstancePtr->SignallingFrameData.LinkDelayIntervalDuration-1))) {

      /** Check to see if we've received PDelayResp and
       *  PDelayRespFollowUp messages since the last PDelayReq was
       *  sent */
      if( InstancePtr->StateMachineData.rcvdPDelayResp &&
          InstancePtr->StateMachineData.rcvdPDelayRespFollowUp ) {

        InstancePtr->StateMachineData.lostResponses = 0;
      } else {
        InstancePtr->StateMachineData.lostResponses++;
      }

      if( InstancePtr->StateMachineData.lostResponses >= XAVB_ALLOWED_LOST_RESPONSES ) {
        /** the peer is no longer ASCapable */
        XAvb_ChangePeerASCapability(InstancePtr, 0);

        xil_printf("\r\n** XAvb_PtpTimerInterruptHandler(): The peer is no longer ASCapable **");
        xil_printf("\r\n** XAvb_PtpTimerInterruptHandler(): StateMachineData.lostResponses >= %d **",
                   XAVB_ALLOWED_LOST_RESPONSES);

        /** avoid potential overflow */
        InstancePtr->StateMachineData.lostResponses = XAVB_ALLOWED_LOST_RESPONSES;
      }

      XAvb_SendPDelayReq(InstancePtr);

      InstancePtr->StateMachineData.rcvdPDelayResp         = 0;
      InstancePtr->StateMachineData.rcvdPDelayRespFollowUp = 0;
      InstancePtr->PtpCounters.CounterLinkDelayInterval    = 0;

    } else {
      InstancePtr->PtpCounters.CounterLinkDelayInterval
          = InstancePtr->PtpCounters.CounterLinkDelayInterval + 1;
    }

  } /** end of 'if (InstancePtr->PtpIsRunning == 1)' */

}


/****************************************************************************/
/**
*
* The Interrupt subroutine for the "interrupt_ptp_rx" signal.  This interrupt
* fires whenever a PTP frame has been received.  The main function is to
* identify, decode, and act on the type of PTP frame received.
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_PtpRxInterruptHandler(XAvb * InstancePtr) {
#ifdef DEBUG_XAVB_LEVEL1
  u32 x                = 0;
#endif
  u32 MessageType      = 0;
  u32 PtpFrameBaseAddr = 0;

  /** RxPtpHardPointer indicates the bin location of the last frame to be
   * received and written into the Rx PTP buffer in hardware.  This read will
   * also clear the interrupt.*/
  InstancePtr->PtpCounters.RxPtpHardPointer
      = (XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_RX_CONTROL_OFFSET)
        & XAVB_PTP_RX_PACKET_FIELD_MASK) >> 8;

  /** If PTP functions are marked as not running, then take no further action */
  if (InstancePtr->PtpIsRunning == 1) {


    /** RxPtpSoftPointer indicates the bin location of the last frame to be
     * processed in software. */
    while (   (InstancePtr->PtpCounters.RxPtpSoftPointer & 0xF)
           != (InstancePtr->PtpCounters.RxPtpHardPointer & 0xF) ) {

      /** decode the rx'd frames until RxPtpHardPointer = RxPtpSoftPointer */
      InstancePtr->PtpCounters.RxPtpSoftPointer
          = InstancePtr->PtpCounters.RxPtpSoftPointer + 1;

      /** Set the base address of the current PTP frame in the Buffer */
      PtpFrameBaseAddr = XAVB_PTP_RX_BASE_OFFSET
                         + ((InstancePtr->PtpCounters.RxPtpSoftPointer & 0xF)
                         << 8);


      /** Perform very basic sanity checking of the frame : is it a PTP? */
      if (XAvb_IsRxFramePTP(InstancePtr, PtpFrameBaseAddr) == 1) {


        /** Determine which PTP frame was received. */
        MessageType = (XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                           PtpFrameBaseAddr,
                                           XAVB_PTP_RX_PKT_TYPE_OFFSET)
                       & 0x000F0000 ) >> 16;


        /** Now act on the received frame */
        switch (MessageType) {

          /** Sync Frame
           * ---------- */
          case XAVB_PTP_TYPE_SYNC:
            XAvb_DecodeRxSync(InstancePtr, PtpFrameBaseAddr);
          break;

          /** Follow Up Frame
           * --------------- */
          case XAVB_PTP_TYPE_FOLLOW_UP:
            XAvb_DecodeRxFollowUp(InstancePtr, PtpFrameBaseAddr);
          break;

          /** PDelayReq Frame
           * --------------- */
          case XAVB_PTP_TYPE_PDELAYREQ:
            /** Send a send PDelayResp frame after receiving a PDelayReq Frame */
            XAvb_SendPDelayResp(InstancePtr, PtpFrameBaseAddr);

            /** Send a send PDelayRespFollowUp frame after a PDelayResp Frame */
            XAvb_SendPDelayRespFollowUp(InstancePtr);

          break;

          /** PDelayResp Frame
           * ---------------- */
          case XAVB_PTP_TYPE_PDELAYRESP:
            XAvb_DecodeRxPDelayResp(InstancePtr, PtpFrameBaseAddr);
          break;

          /** PDelayRespFollowUp Frame
           * ------------------------ */
          case XAVB_PTP_TYPE_PDELAYRESP_FOLLOW_UP:
            EchoPTPFramesReceived = 1;
            XAvb_DecodeRxPDelayRespFollowUp(InstancePtr, PtpFrameBaseAddr);
          break;

          /** Announce Frame
           * -------------- */
          case XAVB_PTP_TYPE_ANNOUNCE:
            XAvb_DecodeRxAnnounceFrame(InstancePtr,
                                       PtpFrameBaseAddr);
          break;

          /** Signaling Frame
           * -------------- */
          case XAVB_PTP_TYPE_SIGNALING:
            XAvb_DecodeRxSignaling(InstancePtr, PtpFrameBaseAddr);
          break;

          /** Unknown Frame
           * -------------- */
          default:
#ifdef DEBUG_XAVB_LEVEL1
            xil_printf("\r\nXAvb_PtpRxInterruptHandler()");
            xil_printf("\r\n** Unknown PTP Frame Rx'd **");
            xil_printf("\r\nMessage Type is      %x", MessageType);
            xil_printf("\r\n-------Unknown Frame -------");
            for (x = 0; x < 0x100; x = x + 4) {
              xil_printf("\r\n %x %x", x,
                  (XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                       PtpFrameBaseAddr,
                                       x)));
            }
#endif
          break;
        }
      }
    }

  } else {  /* InstancePtr->PtpIsRunning != 1 */

    /** PTP is not running so just consume the packets so they are not left in the
     * queue and cause problems when we actually start */

    /** RxPtpSoftPointer indicates the bin location of the last frame to be
     * processed in software. */
    while (   (InstancePtr->PtpCounters.RxPtpSoftPointer & 0xF)
           != (InstancePtr->PtpCounters.RxPtpHardPointer & 0xF) ) {

      /** decode the rx'd frames until RxPtpHardPointer = RxPtpSoftPointer */
      InstancePtr->PtpCounters.RxPtpSoftPointer
          = InstancePtr->PtpCounters.RxPtpSoftPointer + 1;
    }
  }

}

/****************************************************************************
*
* This function provides a stub handler such that if the application does not
* define a handler this function will be called.
*
* @param        CallBackRef has no purpose but is necessary to match the
*               interface for a handler.
* @param        TimestampsUncertain has no purpose but is necessary to match the
*               interface for a handler.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
static void XAvb_StubHandler(void *CallBackRef, u32 TimestampsUncertain)
{
  /*
   * Assert occurs always since this is a stub and should never be called
   */
  Xil_AssertVoidAlways();
}

/****************************************************************************/
/**
*
* A function to set the VLAN PCP field for either SR A or B traffic in the
* RX Filtering Control Register - such that AV traffic is correctly filtered
* by the RX Splitter.
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @param  VlanPriority contains the 3-bit value to be written to the register
*         in the correct bit positions as defined in the address map
*
* @param  SrClass is '0' if VLAN Priority (PCP) A is to be updated and is
*                    '1' if VLAN Priority (PCP) B is to be updated
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_SetupRxFilterControlPcp(XAvb * InstancePtr,
                                  u32    VlanPriority,
                                  u8     SrClass) {
  u32 LocalData = 0;

  LocalData = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                           XAVB_RX_FILTER_CONTROL);

  if (SrClass == 0x0) {
     LocalData = LocalData | (VlanPriority & XAVB_RX_AV_VLAN_PRIORITY_A_MASK);
  } else {
     LocalData = LocalData | (VlanPriority & XAVB_RX_AV_VLAN_PRIORITY_B_MASK);
  }

  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                XAVB_RX_FILTER_CONTROL,
                LocalData);
}

/****************************************************************************/
/**
*
* A function to set the VLAN VID field for either SR A or B traffic in the
* RX Filtering Control Register - such that AV traffic is correctly filtered
* by the RX Splitter.
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @param  VlanVid contains the 12-bit value to be written to the register
*         in the correct bit positions as defined in the address map
*
* @param  SrClass is '0' if VLAN VID A is to be updated and is
*                    '1' if VLAN VID B is to be updated
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_SetupRxFilterControlVid(XAvb * InstancePtr,
                                  u32    VlanVid,
                                  u8     SrClass) {
  u32 LocalData = 0;

  LocalData = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                           XAVB_RX_FILTER_CONTROL);

  if (SrClass == 0x0) {
    LocalData = LocalData | (VlanVid & XAVB_RX_AV_VLAN_VID_A_MASK);
  } else {
    LocalData = LocalData | (VlanVid & XAVB_RX_AV_VLAN_VID_B_MASK);
  }

  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                XAVB_RX_FILTER_CONTROL,
                LocalData);
}


/****************************************************************************/
/**
*
* A function to set the VLAN Match Mode field for the RX Filtering Control
* Register - such that AV traffic is correctly filtered by the RX Splitter.
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @param  VlanMatchMode contains the 1-bit value to be written to the register
*         in the correct bit position as defined in the address map
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_SetupRxFilterControlMatchMode(XAvb * InstancePtr,
                                        u32    VlanMatchMode) {
  u32 LocalData = 0;

  LocalData = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                           XAVB_RX_FILTER_CONTROL);

  LocalData = LocalData | (VlanMatchMode & XAVB_RX_AV_VLAN_MATCH_MODE_MASK);

  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                XAVB_RX_FILTER_CONTROL,
                LocalData);
}
