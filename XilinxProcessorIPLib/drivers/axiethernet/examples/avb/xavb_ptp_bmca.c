/******************************************************************************
*
* Copyright (C) 2008 - 2018 Xilinx, Inc.  All rights reserved.
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
* @file xavb_ptp_bmca.c
*
* The XAvb driver. Functions in this file all relate to the Best Master Clock
* Algorithm (BMCA) which is performed on the AVB network to select a network
* Grand Master Clock.
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
#include "xil_assert.h"
#include "xavb_hw.h"
#include "xavb.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/****************************************************************************/
/**
*
* A New Announce Packet has been written to this device to transmit.  We need to
* decode it and rerun the Best Master Clock Algorithm (BMCA)
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.  But an updated True/False decision as to whether this device
*         should operate as a clock master or a slave is written into the
*         CurrentBmc data structure.
*
* @note   None.
*
*****************************************************************************/
void XAvb_DecodeTxAnnounceFrame(XAvb * InstancePtr) {

  u32 NewMaster = 0;
  XAvb_BmcData TxAnnounceFrame;


  /** Read the attributes for the new Announce frame in the Tx PTP buffer */
  XAvb_ReadAnnounceFrame(InstancePtr->Config.BaseAddress,
                         (XAVB_PTP_TX_ANNOUNCE_OFFSET + 8),
                         &TxAnnounceFrame);

  /** Compare the clock attributes between then new Announce frame and the
   * current master */
  NewMaster = XAvb_BestMasterClockAlgorithm(&TxAnnounceFrame,
                                            &InstancePtr->CurrentBmc);


  if ((NewMaster == 1) | (InstancePtr->CurrentBmc.IAmTheRtcMaster == 1)) {
    /** Update records with the NEW best master */
    XAvb_UpdateBmcRecords(&TxAnnounceFrame,
                          &InstancePtr->CurrentBmc);

#ifdef DEBUG_XAVB_LEVEL1
    xil_printf("\r\nXAvb_DecodeTxAnnounceFrame()");
    xil_printf("\r\n* BMC : I am the MASTER");
    xil_printf("\r\n-----------------------");
    xil_printf("\r\nLocal Announce Frame");
    xil_printf("\r\n-----------------------");
    xil_printf("\r\nGM ID upper %x",
               InstancePtr->CurrentBmc.GrandmasterIdentity.ClockIdentityUpper);

    xil_printf("\r\nGM ID lower %x",
               InstancePtr->CurrentBmc.GrandmasterIdentity.ClockIdentityLower);

    xil_printf("\r\nPriority1   %x",
               InstancePtr->CurrentBmc.GrandmasterPriority1);

    xil_printf("\r\nclockClass  %x",
              InstancePtr->CurrentBmc.ClockQuality.clockClass);

    xil_printf("\r\nPriority2   %x",
              InstancePtr->CurrentBmc.GrandmasterPriority2);
#endif

    /** Our new Tx Announce Packet has won - so this device must be the
     * master */
    xil_printf("\r\n*** XAvb_DecodeTxAnnounceFrame() : Call XAvb_BecomeRtcMaster() *** \r\n");
    XAvb_BecomeRtcMaster(InstancePtr,1);

  }

}


/****************************************************************************/
/**
*
* A New Announce Packet has been received.  We need to decode it and rerun the
* Best Master Clock Algorithm (BMCA)
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return None.  But an updated True/False decision as to whether this device
*         should operate as a clock master or a slave is written into the
*         CurrentBmc data structure.
*
* @note   None.
*
*****************************************************************************/
void XAvb_DecodeRxAnnounceFrame(XAvb * InstancePtr,
                                u32    PtpFrameBaseAddr) {

  u32 NewMaster = 0;
  XAvb_BmcData RxAnnounceFrame;

  /** Read the attributes for the new Announce frame received */
  XAvb_ReadAnnounceFrame(InstancePtr->Config.BaseAddress,
                         PtpFrameBaseAddr,
                         &RxAnnounceFrame);

  /** If the received packet's clockIdentity matches our
   *  clockIdentity, ignore the packet */
  if( XAvb_ComparePortIdentity(InstancePtr->Config.BaseAddress,
                               InstancePtr->portIdLocal,
                               RxAnnounceFrame.SourcePortIdentity) ) {
    xil_printf("Got an announce from myself.. ignoring\r\n");
    return;
  }

  /** If the received packet's stepsRemoved field is >= 255,
   *  ignore the packet */
  if( RxAnnounceFrame.stepsRemoved >= 255 ) {
    xil_printf("Got an announce with stepsRemoved > 255.. ignoring\r\n");
    return;
  }

  /** If the Announce packet's GMID matches that of our current GM
   *  record, then update its records based on the current packet,
   *  just in case something (such as priority) has changed. */
  if( XAvb_CompareClockIdentity(InstancePtr->Config.BaseAddress,
                                RxAnnounceFrame.GrandmasterIdentity,
                                InstancePtr->CurrentBmc.GrandmasterIdentity) ) {

    /** update timeout information */
    InstancePtr->PtpCounters.CounterAnnounceInterval = 0;

    XAvb_UpdateBmcRecords(&RxAnnounceFrame,
                          &InstancePtr->CurrentBmc);
    /** Compare against this device's information to see if we should be GM */
    XAvb_DecodeTxAnnounceFrame(InstancePtr);

  } else if( InstancePtr->CurrentBmc.IAmTheRtcMaster ) {

    /** run BMCA on this announce to see if it is better than me */
    NewMaster =  XAvb_BestMasterClockAlgorithm(&RxAnnounceFrame,
                                               &InstancePtr->CurrentBmc);

    if (NewMaster == 1) {
      /** Update records with the NEW best master */
      XAvb_UpdateBmcRecords(&RxAnnounceFrame,
                            &InstancePtr->CurrentBmc);

      /** Capture the Announce Receipt Timeout Interval.
       *  Reset the announce receipt timeout interval to use the new value.
       */
      XAvb_ReadAnnounceReceiptTimeout(InstancePtr->Config.BaseAddress,
                                      PtpFrameBaseAddr,
                                      &RxAnnounceFrame);

      InstancePtr->CurrentBmc.AnnounceIntervalDuration =
            XAvb_ConvertLogMeanToDuration(RxAnnounceFrame.logMessageInterval);


#ifdef DEBUG_XAVB_LEVEL1
      xil_printf("\r\r\nXAvb_DecodeRxAnnounceFrame()");
      xil_printf("\r\n-----------------------");
      xil_printf("\r\nWinning Announce Frame");
      xil_printf("\r\n-----------------------");

      xil_printf("\r\nGM ID upper %x",
                 InstancePtr->CurrentBmc.GrandmasterIdentity.ClockIdentityUpper);

      xil_printf("\r\nGM ID lower %x",
                 InstancePtr->CurrentBmc.GrandmasterIdentity.ClockIdentityLower);

      xil_printf("\r\nPriority1   %x",
                 InstancePtr->CurrentBmc.GrandmasterPriority1);

      xil_printf("\r\nclockClass  %x",
                 InstancePtr->CurrentBmc.ClockQuality.clockClass);

      xil_printf("\r\nPriority2   %x",
                 InstancePtr->CurrentBmc.GrandmasterPriority2);
#endif

      /** New Rx Announce Packet has won - so this device cannot be a master */
      xil_printf("\r\n* XAvb_DecodeRxAnnounceFrame()::BMC : I am a SLAVE");
      XAvb_BecomeRtcSlave(InstancePtr);
    }
  }
}


/****************************************************************************/
/**
*
* A New Announce Packet is to be analyzed.  This function will read in the
* packet, decode it, and extract the relevant information fields to the
* "AnnounceFrame" data pointer.
*
* @param  BaseAddress is the base address of the device
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
* @param  AnnounceFrame is a pointer to a suitable data structure, designed to
*         record the useful fields from the received Announce Packet
*
* @return The AnnounceFrame data structure is updated.
*
* @note   None.
*
*****************************************************************************/
void XAvb_ReadAnnounceFrame(u32 BaseAddress,
                            u32 PtpFrameBaseAddr,
                            XAvb_BmcData * AnnounceFrame) {

  u32 ReadWord;

  AnnounceFrame->SourcePortIdentity.ClockIdentityLower = 0;
  AnnounceFrame->SourcePortIdentity.ClockIdentityUpper = 0;

  /** Get the Source Port Identity of the port sending the Announce Packet */
  XAvb_GetPortIdentity(BaseAddress, PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_PORTID_UPPER_OFFSET, &AnnounceFrame->SourcePortIdentity);


  /** Read priority1 and top half of ClockQuality */
  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr,
                                                  XAVB_PTP_RX_PKT_ANNOUNCE_PRI1_QUAL_HI_OFFSET));
  AnnounceFrame->GrandmasterPriority1       = (ReadWord >> 16);
  AnnounceFrame->ClockQuality.clockClass    = (ReadWord >> 8);
  AnnounceFrame->ClockQuality.clockAccuracy =  ReadWord;


  /** Read bottom half of ClockQuality, priority2, and top byte of GMID */
  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr,
                                                  XAVB_PTP_RX_PKT_ANNOUNCE_QUAL_LOW_PRI2_GMID_HI_OFFSET));
  AnnounceFrame->ClockQuality.offsetScaledLogVariance   = (ReadWord >> 16);
  AnnounceFrame->GrandmasterPriority2                   = (ReadWord >> 8);
  AnnounceFrame->GrandmasterIdentity.ClockIdentityUpper = (ReadWord << 24);

  /** Read bytes 4-7 of GMID */
  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr,
                                                  XAVB_PTP_RX_PKT_ANNOUNCE_GMID_MID_OFFSET));
  AnnounceFrame->GrandmasterIdentity.ClockIdentityUpper |= (ReadWord >> 8);
  AnnounceFrame->GrandmasterIdentity.ClockIdentityLower  = (ReadWord << 24);

  /** Read bytes 1-3 of GMID and high byte of stepsRemoved */
  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr,
                                                  XAVB_PTP_RX_PKT_ANNOUNCE_GMID_LOW_STEPSREMOVED_HI_OFFSET));
  AnnounceFrame->GrandmasterIdentity.ClockIdentityLower |= (ReadWord >> 8);
  AnnounceFrame->stepsRemoved                            = (ReadWord << 8);

  /** Read low byte of stepsRemoved */
  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr,
                                                  XAVB_PTP_RX_PKT_ANNOUNCE_STEPSREMOVED_LOW_TIMESRC_OFFSET));
  AnnounceFrame->stepsRemoved                           |= (ReadWord >> 24);

}

/****************************************************************************/
/**
*
* This function reads the logMessageinteval from an RX PTP Buffer and updates
* the AnnounceFrame struct with the value read.
*
* @param  BaseAddress is the base address of the device
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
* @param  AnnounceFrame is a pointer to a suitable data structure, designed to
*         record the useful fields from the received Announce Packet
*
* @return The AnnounceFrame data structure is updated.
*
* @note   None.
*
*****************************************************************************/
void XAvb_ReadAnnounceReceiptTimeout(u32 BaseAddress,
                                     u32 PtpFrameBaseAddr,
                                     XAvb_BmcData * AnnounceFrame) {

  u32 ReadWord;
  u8  logMessageInterval;

  ReadWord = XAvb_ReorderWord((XAvb_ReadPtpBuffer(BaseAddress,
                                                   PtpFrameBaseAddr,
                                                   XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET)) &
                               0xFF000000);

  logMessageInterval = ReadWord;

  /* Implicit convert from unsigned (u8) to signed (char) */
  AnnounceFrame->logMessageInterval = logMessageInterval;

}

/****************************************************************************/
/**
*
* This function will accept the data pointer to the current BMCA records, accept
* a pointer to an equivalent data structure for the new Announce Packet.  The
* Best Master Clock Algorithm (BMCA) is then performed on these two data
* structures by comparing the data fields
*
* @param  CurrentBmc is a pointer to a suitable data structure, designed to
*         record the current fields from the current Grand Master's Announce
*         Packet.
* @param  AnnounceFrame is a pointer to a suitable data structure, designed to
*         record the useful fields from the received Announce Packet
*
* @return An updated True/False decision as to whether there is to be a change
*         of Grand Master in the network.
*
* @note   None.
*
*****************************************************************************/
u32 XAvb_BestMasterClockAlgorithm(XAvb_BmcData * AnnounceFrame,
                                  XAvb_BmcData * CurrentBmc) {

  u32 NewMaster = 0;

#ifdef XAVB_DEBUG_LEVEL2
  xil_printf("*** Performing BMCA ***\r\n");
#endif

  /** Priority1 takes precedence over all over priorites */
  if (AnnounceFrame->GrandmasterPriority1 < CurrentBmc->GrandmasterPriority1) {
    /** we have found a better master! */
    NewMaster = 1;

#ifdef XAVB_DEBUG_LEVEL2
    xil_printf("BMCA: Found new GM on priority1: AnnPri1 (%d) < BmcPri1 (%d)\r\n",
               AnnounceFrame->GrandmasterPriority1,
               CurrentBmc->GrandmasterPriority1);
#endif

  } else if (AnnounceFrame->GrandmasterPriority1 ==
             CurrentBmc->GrandmasterPriority1) {

#ifdef XAVB_DEBUG_LEVEL2
    xil_printf("BMCA: priority1 equal  moving on: (%d)\r\n",
               AnnounceFrame->GrandmasterPriority1);
#endif

    /** convert structs to u32 values for easy comparison */
    u32 AnnClockQualityInteger;
    u32 BmcClockQualityInteger;
    AnnClockQualityInteger = (AnnounceFrame->ClockQuality.clockClass    << 24) |
                             (AnnounceFrame->ClockQuality.clockAccuracy << 16) |
                             (AnnounceFrame->ClockQuality.offsetScaledLogVariance);
    BmcClockQualityInteger = (CurrentBmc->ClockQuality.clockClass    << 24) |
                             (CurrentBmc->ClockQuality.clockAccuracy << 16) |
                             (CurrentBmc->ClockQuality.offsetScaledLogVariance);

    /** ClockQuality has the next priority */
    if (AnnClockQualityInteger < BmcClockQualityInteger ) {

#ifdef XAVB_DEBUG_LEVEL2
      xil_printf("BMCA: Found new GM on clockQuality: Ann (0x%08x) < Bmc (0x%08x)\r\n",
                 AnnClockQualityInteger,
                 BmcClockQualityInteger);
#endif

      /** we have found a better master! */
      NewMaster = 1;

    } else if ( AnnClockQualityInteger == BmcClockQualityInteger ) {

#ifdef XAVB_DEBUG_LEVEL2
      xil_printf("BMCA: clockQuality equal moving on: (0x%08x)\r\n",
                 AnnClockQualityInteger);
#endif

      /** Priority2 provides fine grained ordering amongst otherwise equal
       * clocks */
      if (AnnounceFrame->GrandmasterPriority2 <
          CurrentBmc->GrandmasterPriority2) {

#ifdef XAVB_DEBUG_LEVEL2
        xil_printf("BMCA: Found new GM on priority2: AnnPri1 (%d) < BmcPri1 (%d)\r\n",
                   AnnounceFrame->GrandmasterPriority2,
                   CurrentBmc->GrandmasterPriority2);
#endif

        /** we have found a better master! */
        NewMaster = 1;

      /** Next compare the Clock Identities */
      } else if (AnnounceFrame->GrandmasterPriority2
              == CurrentBmc->GrandmasterPriority2) {

#ifdef XAVB_DEBUG_LEVEL2
        xil_printf("BMCA: priority2 equal moving on: (%d)\r\n",
                   AnnounceFrame->GrandmasterPriority2);
#endif

        if (AnnounceFrame->GrandmasterIdentity.ClockIdentityUpper <
            CurrentBmc->GrandmasterIdentity.ClockIdentityUpper) {

#ifdef XAVB_DEBUG_LEVEL2
          xil_printf("BMCA: Found new GM on GMIDClockIDUp: Ann (0x%08x) < Bmc (0x%08x)\r\n",
                     AnnounceFrame->GrandmasterIdentity.ClockIdentityUpper,
                     CurrentBmc->GrandmasterIdentity.ClockIdentityUpper);
#endif

          /** we have found a better master! */
          NewMaster = 1;

        } else if (AnnounceFrame->GrandmasterIdentity.ClockIdentityUpper
                == CurrentBmc->GrandmasterIdentity.ClockIdentityUpper) {

#ifdef XAVB_DEBUG_LEVEL2
          xil_printf("BMCA: GMIDclockIDUp equal moving on: (0x%08x)\r\n",
                     AnnounceFrame->GrandmasterIdentity.ClockIdentityUpper);
#endif

          if (AnnounceFrame->GrandmasterIdentity.ClockIdentityLower <
              CurrentBmc->GrandmasterIdentity.ClockIdentityLower) {

#ifdef XAVB_DEBUG_LEVEL2
            xil_printf("BMCA: Found new GM on GMIDClockIDLow: Ann (0x%08x) < Bmc (0x%08x)\r\n",
                       AnnounceFrame->GrandmasterIdentity.ClockIdentityLower,
                       CurrentBmc->GrandmasterIdentity.ClockIdentityLower);
#endif

            /** we have found a better master! */
            NewMaster = 1;

          /** Next compare stepsRemoved */
          } else if( AnnounceFrame->GrandmasterIdentity.ClockIdentityLower
                  == CurrentBmc->GrandmasterIdentity.ClockIdentityLower ) {

#ifdef XAVB_DEBUG_LEVEL2
            xil_printf("BMCA: GMIDclockIDLo equal moving on: (0x%08x)\r\n",
                       AnnounceFrame->GrandmasterIdentity.ClockIdentityLower);
#endif

            if( AnnounceFrame->stepsRemoved < CurrentBmc->stepsRemoved ) {

#ifdef XAVB_DEBUG_LEVEL2
              xil_printf("BMCA: Found new GM on stepsRemoved: Ann (%d) < Bmc (%d)\r\n",
                         AnnounceFrame->stepsRemoved,
                         CurrentBmc->stepsRemoved);
#endif

              /** we have found a better master! */
              NewMaster = 1;

            /** Next compare SourcePortIdentity */
            } else if( AnnounceFrame->stepsRemoved == CurrentBmc->stepsRemoved ) {

#ifdef XAVB_DEBUG_LEVEL2
              xil_printf("BMCA: stepsRemoved equal moving on: (%d)\r\n",
                         AnnounceFrame->stepsRemoved);
#endif

              if( AnnounceFrame->SourcePortIdentity.ClockIdentityUpper <
                  CurrentBmc->SourcePortIdentity.ClockIdentityUpper) {

#ifdef XAVB_DEBUG_LEVEL2
                xil_printf("BMCA: Found new GM on sourceIDClockIDupper: Ann (0x%08x) < Bmc (0x%08x)\r\n",
                           AnnounceFrame->SourcePortIdentity.ClockIdentityUpper,
                           CurrentBmc->SourcePortIdentity.ClockIdentityUpper);
#endif

                /** we have found a better master! */
                NewMaster = 1;

              } else if( AnnounceFrame->SourcePortIdentity.ClockIdentityUpper
                      == CurrentBmc->SourcePortIdentity.ClockIdentityUpper ) {

#ifdef XAVB_DEBUG_LEVEL2
                xil_printf("BMCA: sourceIDportUp equal moving on: (0x%08x)\r\n",
                           CurrentBmc->SourcePortIdentity.ClockIdentityUpper);
#endif

                if( AnnounceFrame->SourcePortIdentity.ClockIdentityLower <
                    CurrentBmc->SourcePortIdentity.ClockIdentityLower ) {

#ifdef XAVB_DEBUG_LEVEL2
                  xil_printf("BMCA: Found new GM on sourceIDClockIDlow: Ann (0x%08x) < Bmc (0x%08x)\r\n",
                             AnnounceFrame->SourcePortIdentity.ClockIdentityLower,
                             CurrentBmc->SourcePortIdentity.ClockIdentityLower);
#endif

                  /** we have found a better master! */
                  NewMaster = 1;

                /** If all else fails, the SourcePortIdentity Port Number must
                 * act as the tie-breaker */
                } else if( AnnounceFrame->SourcePortIdentity.PortNumber <
                           CurrentBmc->SourcePortIdentity.PortNumber ) {

#ifdef XAVB_DEBUG_LEVEL2
                  xil_printf("BMCA: Found new GM on sourceIDportNum: AnnPort (0x%08x) < BmcPort (0x%08x)\r\n",
                             AnnounceFrame->SourcePortIdentity.PortNumber,
                             CurrentBmc->SourcePortIdentity.PortNumber);
#endif

                  /** A new master has won on the tie-break! */
                  NewMaster = 1;
                }
              }
            }
          }
        }
      }
    }
  }

#ifdef XAVB_DEBUG_LEVEL2
  xil_printf("*** END BMCA ***\r\n");
#endif

  return NewMaster;

}


/****************************************************************************/
/**
*
* This function will accept the data pointer to the current BMCA records, accept
* an equivalent pointer to a new (winning) Grand Masters Announce Packet
* information.  The CurrentBmc data structure is then updated with the
* information from the NewMaster.
*
* @param  NewMaster is a pointer to a suitable data structure which has recorded
*         the relevant Announce Packet fields of the new (winning) Grand Master.
* @param  CurrentBmc is a pointer to a suitable data structure which has
*         recorded the current fields of the current Grand Master's Announce
*         Packet.
*
* @return The CurrentBmc data structure is updated.
*
* @note   None.
*
*****************************************************************************/
void XAvb_UpdateBmcRecords(XAvb_BmcData* NewMaster,
                           XAvb_BmcData* CurrentBmc) {

    CurrentBmc->SourcePortIdentity.ClockIdentityUpper =
        NewMaster->SourcePortIdentity.ClockIdentityUpper;

    CurrentBmc->SourcePortIdentity.ClockIdentityLower =
        NewMaster->SourcePortIdentity.ClockIdentityLower;

    CurrentBmc->SourcePortIdentity.PortNumber =
        NewMaster->SourcePortIdentity.PortNumber;

    CurrentBmc->GrandmasterIdentity.ClockIdentityUpper =
        NewMaster->GrandmasterIdentity.ClockIdentityUpper;

    CurrentBmc->GrandmasterIdentity.ClockIdentityLower =
        NewMaster->GrandmasterIdentity.ClockIdentityLower;


    CurrentBmc->stepsRemoved         = NewMaster->stepsRemoved;
    CurrentBmc->ClockQuality         = NewMaster->ClockQuality;
    CurrentBmc->GrandmasterPriority1 = NewMaster->GrandmasterPriority1;
    CurrentBmc->GrandmasterPriority2 = NewMaster->GrandmasterPriority2;

}


/****************************************************************************/
/**
*
* This function will make any adjustments needed when the node becomes the Grand
* Master, including resetting the RTC to its nominal value
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @param  txAnnounceHasWon indicates that this function has been called from
*         the function XAvb_DecodeTxAnnounceFrame(). Is this is set then this
*         function has no need to repeat actions that have been already performed.
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_BecomeRtcMaster(XAvb *InstancePtr,
                          u8    txAnnounceHasWon) {

  XAvb_BmcData deviceData;

  if (txAnnounceHasWon == 0) {
    /**
     * Update the BMCA records to this device's information
     */

    /** Read the attributes in the Tx PTP buffer */
    XAvb_ReadAnnounceFrame(InstancePtr->Config.BaseAddress,
                           (XAVB_PTP_TX_ANNOUNCE_OFFSET + 8),
                           &deviceData);

    /** Update records  */
    XAvb_UpdateBmcRecords(&deviceData,
                          &InstancePtr->CurrentBmc);
  }

  /** reset the RTC to a nominal value */
  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_RTC_INCREMENT_OFFSET,
                 XAVB_RTC_INCREMENT_NOMINAL_RATE);

  /** set timestamp uncertainty if new status */
  if( !InstancePtr->CurrentBmc.IAmTheRtcMaster ) {
    xil_printf("\r\n*** I am now the Grand Master ***");
    xil_printf("\r\nNOTICE: timestamps are now certain\r\n");
    InstancePtr->GMDiscHandler(InstancePtr->GMDiscCallBackRef,
                               0);
  }

  /** inform the rest of the system */
  InstancePtr->CurrentBmc.IAmTheRtcMaster = 1;

}

/****************************************************************************/
/**
*
* This function will make any adjustments needed when the node becomes a PTP slave
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_BecomeRtcSlave(XAvb *InstancePtr) {

  /** set timestamp uncertainty if new status */
  if( InstancePtr->CurrentBmc.IAmTheRtcMaster ) {
    xil_printf("\r\n*** I am now a PTP slave ***");
    xil_printf("\r\nNOTICE: timestamps are now uncertain\r\n");
    InstancePtr->GMDiscHandler(InstancePtr->GMDiscCallBackRef,
                               1);
  }

  /* Reset the syncReceiptTimeoutTimeInterval counter as this has now changed purpose
   */
  InstancePtr->PtpCounters.CounterSyncInterval = 0;

  /* inform the rest of the system */
  InstancePtr->CurrentBmc.IAmTheRtcMaster = 0;


}

/****************************************************************************/
/**
*
* Operations needed when PTP locks or unlocks
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  locked is 1 if changing to locked status, zero if unlocked
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_ChangePTPLockStatus(XAvb *InstancePtr, u8 locked) {
  u32 lockedOld;
  u32 tu = 0;

  lockedOld = InstancePtr->PTPLocked;

  /** set status variable */
  InstancePtr->PTPLocked = locked;

  /** set timestamp uncertainty if necessary */
  if( InstancePtr->PTPLocked != lockedOld ) {
    XAvb_ChangePeerASCapability(InstancePtr, locked);

#ifdef XAVB_DEBUG_LEVEL2
    if (locked == 0) {
      xil_printf("\r\nXAvb_ChangePTPLockStatus():The peer is no longer ASCapable ");
      xil_printf("\r\nXAvb_ChangePTPLockStatus():locked = %d\r\n",locked);
    }
#endif

    tu = InstancePtr->PTPLocked ? 0 : 1;
    xil_printf("\r\nXAvb_ChangePTPLockStatus()::");
    xil_printf("\r\nNOTICE: timestamps are now %s\r\n", tu ? "uncertain" : "certain");
    InstancePtr->GMDiscHandler(InstancePtr->GMDiscCallBackRef,
                               tu);
  }

}

/****************************************************************************/
/**
*
* Operations needed when the peer's AS capability changes
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  capable is 1 if the peer is ASCapable, 0 otherwise
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_ChangePeerASCapability(XAvb *InstancePtr, u8 capable) {
  u32 capableOld;

  capableOld = InstancePtr->PeerASCapable;

  /* set status variable */
  InstancePtr->PeerASCapable = capable;

  if( capable != capableOld ) {
    if( capable ) {
      xil_printf("\r\nThe Peer is now AS Capable\r\n");
    } else {
      xil_printf("\r\nThe Peer is no longer AS Capable\r\n");
    }
  }

}

/****************************************************************************/
/**
*
* This function sets the handler that will be called when a GM discontinuity
* event is identified by the driver. The purpose of the handler is to allow
* application specific processing to be performed.
*
* @param        InstancePtr is a pointer to the XAvb instance.
* @param        FuncPtr is the pointer to the callback function.
* @param        CallBackRef is the upper layer callback reference passed back
*               when the callback function is invoked.
*
* @return       None.
*
* @note         There is no assert on the CallBackRef since the driver doesn't
*               know what it is (nor should it)
*
*****************************************************************************/
void XAvb_SetGMDiscontinuityHandler(XAvb *InstancePtr,
                                    XAvb_Handler FuncPtr, void *CallBackRef)
{
  /*
   * Assert validates the input arguments
   * CallBackRef not checked, no way to know what is valid
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(FuncPtr != NULL);
  Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

  InstancePtr->GMDiscHandler     = FuncPtr;
  InstancePtr->GMDiscCallBackRef = CallBackRef;
}
