/******************************************************************************
* Copyright (C) 2008 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xavb_ptp_packets.c
*
* The XAvb driver. Functions in this file all contain functions which decode the
* received Precise Timing Protocol (PTP) frames, or to format and transmit PTP
* frames.
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
#include "xavb_hw.h"
#include "xavb.h"
#include "stdlib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/****************************************************************************/
/**
*
* A function to compare two ClockIdentity values.
*
* @param  BaseAddress is the base address of the device
* @param  Identity1 is the first ClockIdentity to be compared
* @param  Identity2 is the second ClockIdentity to be compared
*
* @return 1 if the two values are equal, 0 if not equal
*
* @note
*
*****************************************************************************/
u32 XAvb_CompareClockIdentity(u32 BaseAddress,
                              XAvb_ClockIdentity Identity1,
                              XAvb_ClockIdentity Identity2) {

  if( Identity1.ClockIdentityUpper != Identity2.ClockIdentityUpper ) {
    /*xil_printf("ID1Upper (0x%08x) != ID2Upper (0x%08x)\r\n",
               Identity1.ClockIdentityUpper,
               Identity2.ClockIdentityUpper);*/
    return 0;
  }
  if( Identity1.ClockIdentityLower != Identity2.ClockIdentityLower ) {
    /*xil_printf("ID1Lower (0x%08x) != ID2Lower (0x%08x)\r\n",
               Identity1.ClockIdentityLower,
               Identity2.ClockIdentityLower);*/
    return 0;
  }

  /** values are equal */
  return 1;
}

/****************************************************************************/
/**
*
* A function to compare two PortIdentity values.
*
* @param  BaseAddress is the base address of the device
* @param  Identity1 is the first sourcePortIdentity to be compared
* @param  Identity2 is the second sourcePortIdentity to be compared
*
* @return 1 if the two values are equal, 0 if not equal
*
* @note   None.
*
*****************************************************************************/
u32 XAvb_ComparePortIdentity(u32 BaseAddress,
                             XAvb_PortIdentity Identity1,
                             XAvb_PortIdentity Identity2) {

  if( Identity1.ClockIdentityUpper != Identity2.ClockIdentityUpper ) {
    /*xil_printf("ID1Upper (0x%08x) != ID2Upper (0x%08x)\r\n",
               Identity1.ClockIdentityUpper,
               Identity2.ClockIdentityUpper);*/
    return 0;
  }
  if( Identity1.ClockIdentityLower != Identity2.ClockIdentityLower ) {
    /*xil_printf("ID1Lower (0x%08x) != ID2Lower (0x%08x)\r\n",
               Identity1.ClockIdentityLower,
               Identity2.ClockIdentityLower);*/
    return 0;
  }
  if( Identity1.PortNumber != Identity2.PortNumber ) {
    /*xil_printf("ID1Port (0x%08x) != ID2Port (0x%08x)\r\n",
               Identity1.PortNumber,
               Identity2.PortNumber);*/
    return 0;
  }

  /** values are equal */
  return 1;
}

/****************************************************************************/
/**
*
* A function to extract portIdentity information from a received PTP frame.
* This can be any portIdentity field (header portIdentity, requestingPortIdentity,
* etc.)
*
* @param  BaseAddress is the base address of the device
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
* @param  PortIdOffset is the packet offset of the first byte of the portIdentity
*         field to be parsed
* @param  portID is the XAvb_PortIdentity struct that the data will be written to.
*
* @return None, but portID will be updated with the portIdentity information
*
* @note   None.
*
*****************************************************************************/
void XAvb_GetPortIdentity(u32 BaseAddress, u32 PtpFrameBaseAddr,
                          u32 PortIdOffset, XAvb_PortIdentity *portID) {

  u32 ReadWord;

  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr, PortIdOffset));

  portID->ClockIdentityUpper = (ReadWord << 16);

  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr, PortIdOffset + 4));

  portID->ClockIdentityUpper |= (ReadWord >> 16);
  portID->ClockIdentityLower  = (ReadWord << 16);

  ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(BaseAddress, PtpFrameBaseAddr, PortIdOffset + 8));

  portID->ClockIdentityLower |= (ReadWord >> 16);
  portID->PortNumber          = ReadWord;

}

/****************************************************************************/
/**
*
* A function to write common data (eg the Source Address) to all PTP frames
* stored in the Tx PTP Packet buffer
*
* @param  BaseAddress is the base address of the device
* @param  PtpFieldAddress is the offset address of the relevant field in PTP
*         frames.
* @param  Data is the common data to be written to all Tx PTP frame templates
* @param  DataBitEnable allows only selected bits of the 32-bit Data word to
*         be modified.
* @param  BufferEnable allows the selected buffer to be selected: there are 8
*         PTP buffers - these are encoded as one-hot.  For example, 0x3F will
*         write the selected data to the first 6 buffers only.
*
* @return None.  But the Tx PTP Packet Buffer is written to as requested
*
* @note   None.
*
*****************************************************************************/
void XAvb_WriteToMultipleTxPtpFrames(u32 BaseAddress,
                                     u32 PtpFieldAddress,
                                     u32 Data,
                                     u32 DataBitEnable,
                                     u8  BufferEnable) {
  u32 PtpBufferPointer = 0;
  u32 LocalData        = 0;
  u32 LocalAddr        = 0;

  /** Write to all 8 PTP frame templates */
  for (PtpBufferPointer= 0; PtpBufferPointer < 8; PtpBufferPointer++) {

    /** Only write to selected buffers */
    if ( ((BufferEnable >> PtpBufferPointer) & 0x1) == 0x1) {

      LocalAddr = (PtpBufferPointer<<8) + XAVB_PTP_TX_SYNC_OFFSET;

      /** Read the current value */
      LocalData = XAvb_ReadPtpBuffer(BaseAddress,
                                      LocalAddr,
                                      PtpFieldAddress);

      /** Only change the selected data bits */
      LocalData = LocalData | (Data & DataBitEnable);

      /** Write the updated value */
      XAvb_WritePtpBuffer(BaseAddress,
                           LocalAddr,
                           PtpFieldAddress,
                           LocalData);
#ifdef DEBUG_XAVB_LEVEL3
      xil_printf("\r\nWriteToMultipleTxPtpFrames read/mod(PtpBufferPointer = %x) ", PtpBufferPointer);
      xil_printf("\r\n PTPAddress = %x ", LocalAddr);
      xil_printf("\r\n PtpFieldAddress = %x ", PtpFieldAddress);
      xil_printf("\r\n LocalData = %x ", LocalData);
#endif
    }
  }
}


/****************************************************************************/
/**
*
* This function switches the bytes in a 4-byte word, swapping the MSB for the
* LSB, and vice-versa.
*
* @param  Data is the 4-byte input data word
*
* @return The input data word with the bytes swapped (most significant down to
*         least significant
*
* @note   None.
*

*****************************************************************************/
u32 XAvb_ReorderWord(u32 Data) {
  u32 ReOrder = 0;

  ReOrder =            (Data & 0x000000FF) << 24;
  ReOrder = ReOrder | ((Data & 0x0000FF00) << 8);
  ReOrder = ReOrder | ((Data & 0x00FF0000) >> 8);
  ReOrder = ReOrder | ((Data & 0xFF000000) >> 24);
  return ReOrder;
}


/****************************************************************************/
/**
*
* A function to increment the sequenceId in a PTP frame template
*
* @param  BaseAddress is the base address of the device
* @param  PtpFrameBaseAddress is the base address of the TX PTP Buffer whose
*         SequenceID is to be incremented
*
* @return None.  But the relevant TX PTP Packet Buffer is written to with the
*                updated SequenceID
*
* @note   None.
*
*****************************************************************************/
u32 XAvb_IncSequenceId(u32 BaseAddress, u32 PtpFrameBaseAddress) {
  u32 BufferWord = 0;
  u32 SequenceId = 0;

  /** Read the 32-bit BufferWord containing the SequenceId from the PTP buffer */
  BufferWord =  XAvb_ReadPtpBuffer(BaseAddress,
                                    PtpFrameBaseAddress,
                                    XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET);

  /** Swap byte order into correct binary and increment the SequenceId */
  SequenceId = XAvb_ReorderWord(BufferWord) + 0x10000;

  /** Swap back the byte order into frame storage order */
  SequenceId = XAvb_ReorderWord(SequenceId);

  /** Write the 32-bit BufferWord variable containing the updated SequenceId */
  XAvb_WritePtpBuffer(BaseAddress,
                      PtpFrameBaseAddress,
                      XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET,
                      SequenceId);

  return SequenceId;

}


/****************************************************************************/
/**
*
* The software drivers are kept simple by only requesting a single PTP frame to
* be transmitted at a time.  This function checks (and if necessary waits)
* until the previously request PTP frame has been transmitted.
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.
**
* @note   None.
*
*****************************************************************************/
void XAvb_WaitOnTxPtpQueue(XAvb * InstancePtr) {
  u32 TxPtpType = 0;

  do {

  /** Wait until any queued PTP frame has been transmitted.  This is a
   * software safety feature, not a hardware restriction */
  TxPtpType = (XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                             XAVB_PTP_TX_CONTROL_OFFSET)
               & XAVB_PTP_TX_WAIT_ALL_FRAMES_MASK);

  } while(TxPtpType != 0);

}


/****************************************************************************/
/**
*
* A function to format then request the transmission of a PTP Announce Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.  But the relevant Tx PTP Packet Buffer is written to with the
*                updated PTP fields, and then the Tx PTP Packet Buffer Control
*                Register is written to request the frame transmission.
*
* @note   None.
*
*****************************************************************************/
void XAvb_MasterSendAnnounce(XAvb * InstancePtr) {
  u32 Unused = 0;


  /** Wait until there are no PTP frames to be transmitted */
  XAvb_WaitOnTxPtpQueue(InstancePtr);

  /** Increment the sequenceId */
  Unused = XAvb_IncSequenceId(InstancePtr->Config.BaseAddress,
                              XAVB_PTP_TX_ANNOUNCE_OFFSET);

#ifdef DEBUG_XAVB_LEVEL3
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\n---XAvb_MasterSendAnnounce()----");
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\nsequenceId is %x", Unused);
  Unused  = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                          XAVB_RTC_NANOSEC_VALUE_OFFSET);
  Unused  = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                          XAVB_RTC_SEC_LOWER_VALUE_OFFSET);
  xil_printf("\r\nSeconds %d", Unused);
#endif

  /** Send the Announce Frame! */
  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_PTP_TX_CONTROL_OFFSET,
                 XAVB_PTP_TX_SEND_ANNOUNCE_FRAME_MASK);
}


/****************************************************************************/
/**
*
* A function to format then request the transmission of a PTP Sync Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.  But the relevant Tx PTP Packet Buffer is written to with the
*                updated PTP fields, and then the Tx PTP Packet Buffer Control
*                Register is written to request the frame transmission.
*
* @note   None.
*
*****************************************************************************/
void XAvb_MasterSendSync(XAvb * InstancePtr) {
  u32 Epoch       = 0;
  u32 Sec         = 0;
  u32 SequenceId  = 0;
  u32 BufferWord  = 0;

  /** Wait until there are no PTP frames to be transmitted */
  XAvb_WaitOnTxPtpQueue(InstancePtr);

  /** Increment the sequenceId in the Sync frame */
  SequenceId = XAvb_IncSequenceId(InstancePtr->Config.BaseAddress,
                                  XAVB_PTP_TX_SYNC_OFFSET);

  /** Read the current RTC Offset values */
  InstancePtr->PtpRecords.Nanosec
      = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                      XAVB_RTC_NANOSEC_VALUE_OFFSET);

  Sec    = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                         XAVB_RTC_SEC_LOWER_VALUE_OFFSET);

  Epoch  = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                         XAVB_RTC_SEC_UPPER_VALUE_OFFSET);

  /** Send the Sync Frame! */
  XAvb_WriteReg(InstancePtr->Config.BaseAddress, XAVB_PTP_TX_CONTROL_OFFSET,
                 XAVB_PTP_TX_SEND_SYNC_FRAME_MASK);


  /** Now some pre-work on the Follow-Up Frame
   *-----------------------------------------
   *
   * Write the same sequenceId to the Follow-up frame */
  BufferWord =  XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                    XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                                    XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET);

  BufferWord = (BufferWord & 0xFFFF0000) | (SequenceId & 0x0000FFFF);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET,
                       BufferWord);


  /** Format the Timestamp (RTC) into correct byte positioning.
   * Note: this is how the Timestamp is stored in
   * the PTP frame itself (transmitted MSB of Epoch first):
   *
   * |----------------|----------------|----------------|----------------|
   * | seconds[23:16] | seconds[31:24] |   epoch[7:0]   |   epoch[15:8]  |
   * |----------------|----------------|----------------|----------------|
   * | nanosec[23:16] | nanosec[31:24] |  seconds[7:0]  |  seconds[15:8] |
   * |----------------|----------------|----------------|----------------|
   * |      0's       |      0's       |  nanosec[7:0]  |  nanosec[15:8] |
   * |----------------|----------------|----------------|----------------|
   */

  BufferWord = XAvb_ReorderWord((Epoch<<16) | (Sec>>16));

  /** Write the Timestamp (RTC) to the Follow-up frame */
  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_UPPER_OFFSET,
                       BufferWord);

  BufferWord = XAvb_ReorderWord((Sec<<16) |
                                (InstancePtr->PtpRecords.Nanosec>>16));

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_MID_OFFSET,
                       BufferWord);

  BufferWord = XAvb_ReorderWord(InstancePtr->PtpRecords.Nanosec<<16);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_LOWER_OFFSET,
                       BufferWord);

#ifdef DEBUG_XAVB_LEVEL3
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\n-----XAvb_MasterSendSync()------");
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\n--------------RTC---------------");
  xil_printf("\r\nRTC nanosec field is %x", InstancePtr->PtpRecords.Nanosec);
  xil_printf("\r\nRTC seconds field is %x", Sec);
  xil_printf("\r\nRTC epoch   field is %x", Epoch);
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\nsequenceId is %x", SequenceId);
#endif

}



/****************************************************************************/
/**
*
* A function to format then request the transmission of a PTP Follow-Up Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.  But the relevant Tx PTP Packet Buffer is written to with the
*                updated PTP fields, and then the Tx PTP Packet Buffer Control
*                Register is written to request the frame transmission.
*
* @note   None.
*
*****************************************************************************/
void XAvb_MasterSendFollowUp(XAvb * InstancePtr) {
  u32 Timestamp        = 0;
  u32 NsOffset         = 0;
  u32 CorrectionField  = 0;
  u32 BufferWord       = 0;

  /** Wait until there are no PTP frames to be transmitted */
  XAvb_WaitOnTxPtpQueue(InstancePtr);

  /** Read the current RTC offset */
  NsOffset = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                           XAVB_RTC_NANOSEC_OFFSET);

  /** Read the Timestamp and adjust it for the MAC transmit latency */
  Timestamp = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                  XAVB_PTP_TX_SYNC_OFFSET,
                                  XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET)
              + XAVB_TX_MAC_LATENCY_IN_NS;

  /** Adjust the Timestamp with current RTC ns offset */
  Timestamp = Timestamp + NsOffset;
  if (Timestamp >= XAVB_ONE_SECOND) {
    Timestamp = Timestamp - XAVB_ONE_SECOND;
  }

  /** Calculate the Correction Field */
  CorrectionField = Timestamp - InstancePtr->PtpRecords.Nanosec;
  if (CorrectionField >= XAVB_ONE_SECOND) {
    CorrectionField = CorrectionField + XAVB_ONE_SECOND;
  }

  /** Format the Correction Field into correct byte positioning for PTP frame
   *  storage in the buffer
   */
  BufferWord = XAvb_ReorderWord(CorrectionField);

  /** Write the Correction Field to the Follow Up frame */
  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_CORRECTION_FIELD_OFFSET,
                       BufferWord);

  /** Send the Follow Up Frame! */
  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_PTP_TX_CONTROL_OFFSET,
                 XAVB_PTP_TX_SEND_FOLLOWUP_FRAME_MASK);

#ifdef DEBUG_XAVB_LEVEL3
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\n---XAvb_MasterSendFollowUp()----");
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\nTimestamp is %x", Timestamp);
  xil_printf("\r\nRTC nano seconds field is %x",
             InstancePtr->PtpRecords.Nanosec);
  xil_printf("\r\nCorrection Field %x", CorrectionField);
#endif

}



/****************************************************************************/
/**
*
* A function to format then request the transmission of a PTP PDelay Request
* Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.  But the relevant Tx PTP Packet Buffer is written to with the
*                updated PTP fields, and then the Tx PTP Packet Buffer Control
*                Register is written to request the frame transmission.
*
* @note   None.
*
*****************************************************************************/
void XAvb_SendPDelayReq(XAvb * InstancePtr) {
  u32 SequenceId;

  /** Wait until there are no PTP frames to be transmitted */
  XAvb_WaitOnTxPtpQueue(InstancePtr);

  /** Increment the SequenceId */
  SequenceId = XAvb_IncSequenceId(InstancePtr->Config.BaseAddress,
                                  XAVB_PTP_TX_PDELAYREQ_OFFSET)
               & 0x0000FFFF;

#ifdef DEBUG_XAVB_LEVEL3
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\n------XAvb_SendPDelayReq()------");
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\nsequenceId is %x", SequenceId);
#endif

  /** Send the PDelayReq Frame! */
  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_PTP_TX_CONTROL_OFFSET,
                 XAVB_PTP_TX_SEND_PDELAYREQ_FRAME_MASK);

  /** Wait for the frame to be transmitted */
  XAvb_WaitOnTxPtpQueue(InstancePtr);

  /** Capture the Timestamp for Tx of PDelayReq (t1) and adjust it for MAC
   *  transmit latency */
  InstancePtr->PtpRecords.PDelayTimestampT1
      = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                            XAVB_PTP_TX_PDELAYREQ_OFFSET,
                            XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET)
        + XAVB_TX_MAC_LATENCY_IN_NS;

  /** Capture the SequenceID of the the PDelayReq */
  InstancePtr->SequenceIdRecords.PDelayReqSequenceId =
     XAvb_ReorderWord(XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                          XAVB_PTP_TX_PDELAYREQ_OFFSET,
                          XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET)) >> 16;
}


/****************************************************************************/
/**
*
* A function to format then request the transmission of a PTP PDelay Response
* Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return None.  But the relevant Tx PTP Packet Buffer is written to with the
*                updated PTP fields, and then the Tx PTP Packet Buffer Control
*                Register is written to request the frame transmission.
*
* @note   None.
*
*****************************************************************************/
void XAvb_SendPDelayResp(XAvb * InstancePtr,
                         u32    PtpFrameBaseAddr) {
  u32 SequenceId     = 0;
  u32 TimestampT2    = 0;
  u32 BufferWord     = 0;
  u32 NanoSec        = 0;
  u32 Seconds        = 0;
  u32 Epoch          = 0;
  u32 CopyPortId     = 0;
  u32 CopyPortId1    = 0;

  /** Wait until there are no PTP frames to be transmitted */
  XAvb_WaitOnTxPtpQueue(InstancePtr);

  /** Format the Timestamp
   *---------------------
   *
   * Capture the current Synchronised time */
  NanoSec  = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                           XAVB_RTC_NANOSEC_VALUE_OFFSET);

  Seconds  = XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                           XAVB_RTC_SEC_LOWER_VALUE_OFFSET);
  Epoch    = XAvb_ReadReg(InstancePtr->Config.BaseAddress,

                           XAVB_RTC_SEC_UPPER_VALUE_OFFSET);

  /** Read the current RTC offset */
  InstancePtr->PtpRecords.NsOffsetForPDelayResp =
      XAvb_ReadReg(InstancePtr->Config.BaseAddress, XAVB_RTC_NANOSEC_OFFSET);

  /** Read the TimestampT2 for PDelayReq reception and adjust it for MAC
   *  receive latency */
  TimestampT2 = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                    PtpFrameBaseAddr,
                                    XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET)
                - XAVB_RX_MAC_LATENCY_IN_NS;

  /** The TimestampT2 was captured using syntonised ns time.  We need to
   * convert this into synchronised time by adding on the current offset */
  TimestampT2 = TimestampT2 + InstancePtr->PtpRecords.NsOffsetForPDelayResp;

  /** Check for ns wrap-around condition */
  if (TimestampT2 >= XAVB_ONE_SECOND) {
    TimestampT2   = TimestampT2 - XAVB_ONE_SECOND;
  }

  /** Even though we read the RTC value at the beginning of this
   * function, there would have been processing delay between the
   * actual reception (and timestamping) of the PDelayReq frame and the
   * start of this function.  During this time, the RTC Seconds
   * field could have wrapped around.  We need to detect this and if it
   * has done, the slave Seconds field would also have incremented (so
   * it needs to be set back).
   */
  if (NanoSec < TimestampT2) {
    /** NanoSec has wrapped since timestamp was taken so decrement the
     * Seconds field */
    if (Seconds == 0x00000000) {
      Epoch = Epoch   - 0x1;
    }
    Seconds = Seconds - 0x1;
  }

  /** Format the Timestamp (t2) into correct byte positioning for PTP frame
   *  storage, then write the Timestamp (t2) to the PDelayResp frame
   */
  BufferWord = XAvb_ReorderWord((Epoch<<16) | (Seconds>>16));


  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_UPPER_OFFSET,
                       BufferWord);

  BufferWord = XAvb_ReorderWord((Seconds<<16) | (TimestampT2>>16));

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_MID_OFFSET,
                       BufferWord);

  BufferWord = XAvb_ReorderWord(TimestampT2<<16);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_LOWER_OFFSET,
                       BufferWord);


  /** Format the SequenceId
   *----------------------
   *
   * Set the SequenceId in the PDelayResp and PDelayRespFollowUp frame to be
   * that of the received PDelayReq frame */
  SequenceId = (XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                    PtpFrameBaseAddr,
                                    XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET))
                & 0x0000FFFF;

  BufferWord = XAVB_PDELAY_LOG_MEAN_MESSAGE_INT | SequenceId;

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_OFFSET,
                       XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET,
                       BufferWord);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                      XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET,
                       BufferWord);


  /** Format the sourcePortIdentity
   *------------------------------
   *
   * Copy the sourcePortIdentity field from the PDelayReq into the PDelayResp
   * and PDelayRespFollowUp frame
   */
  CopyPortId = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                   PtpFrameBaseAddr,
                                   XAVB_PTP_RX_PKT_PORTID_UPPER_OFFSET);

  CopyPortId &= 0xffff0000;
  CopyPortId1 = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                    XAVB_PTP_TX_PDELAYRESP_OFFSET,
                    XAVB_PTP_TX_PKT_REQ_PORTID_UPPER_OFFSET);

  CopyPortId |= (CopyPortId1 & 0x0000ffff);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_OFFSET,
                       XAVB_PTP_TX_PKT_REQ_PORTID_UPPER_OFFSET,
                       CopyPortId);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_REQ_PORTID_UPPER_OFFSET,
                       CopyPortId);

  CopyPortId = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                   PtpFrameBaseAddr,
                                   XAVB_PTP_RX_PKT_PORTID_MID_OFFSET);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_OFFSET,
                       XAVB_PTP_TX_PKT_REQ_PORTID_MID_OFFSET,
                       CopyPortId);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_REQ_PORTID_MID_OFFSET,
                       CopyPortId);

  CopyPortId = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                   PtpFrameBaseAddr,
                                   XAVB_PTP_RX_PKT_PORTID_LOWER_OFFSET);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_OFFSET,
                       XAVB_PTP_TX_PKT_REQ_PORTID_LOWER_OFFSET,
                       CopyPortId);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_REQ_PORTID_LOWER_OFFSET,
                       CopyPortId);


  /** Send the PDelayResp Frame!
   *---------------------------*/

  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_PTP_TX_CONTROL_OFFSET,
                 XAVB_PTP_TX_SEND_PDELAYRESP_FRAME_MASK);

#ifdef DEBUG_XAVB_LEVEL3
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\n------XAvb_SendPDelayResp()-----");
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\nTimestampT2 is %x", TimestampT2);
#endif

}


/****************************************************************************/
/**
*
* A function to format then request the transmission of a PTP PDelay Response
* Follow-Up Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.  But the relevant Tx PTP Packet Buffer is written to with the
*                updated PTP fields, and then the Tx PTP Packet Buffer Control
*                Register is written to request the frame transmission.
*
* @note   None.
*
*****************************************************************************/
void XAvb_SendPDelayRespFollowUp(XAvb * InstancePtr) {
  u32 TimestampT3 = 0;
  u32 BufferWordA = 0;
  u32 BufferWordB = 0;
  XAvb_RtcFormat Rtc[1];

  /** Wait until there are no PTP frames to be transmitted */
  XAvb_WaitOnTxPtpQueue(InstancePtr);

  /** Format the Timestamp
   *---------------------*/

  /** Capture the current Synchronised time */
  XAvb_ReadRtc(InstancePtr->Config.BaseAddress, Rtc);


  /** Read the TimestampT3 for PDelayResp transmission and adjust it for MAC
   *  transmit latency */
  TimestampT3 = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                    XAVB_PTP_TX_PDELAYRESP_OFFSET,
                                    XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET)
                + XAVB_TX_MAC_LATENCY_IN_NS;

  /** The TimestampT3 was captured using syntonised ns time.  We need to
   * convert this into synchronised time by adding on the ns offset.  We
   * use the same offset here as for the PDelayResp frame since if a
   * ns offset change had been made between PDelayResp and ,
   * PDelayRespFollowUp, this would result in an error in the link delay
   * measurement. */
  TimestampT3 = TimestampT3 + InstancePtr->PtpRecords.NsOffsetForPDelayResp;

  /** Check for ns wrap-around condition */
  if (TimestampT3 >= XAVB_ONE_SECOND) {
    TimestampT3   = TimestampT3 - XAVB_ONE_SECOND;
  }

  /** Even though we read the RTC value at the beginning of this
   * function, there would have been processing delay between the
   * actual reception (and timestamping) of the PDelayReq frame and the
   * start of this function.  During this time, the RTC seconds
   * field could have wrapped around.  We need to detect this and if it
   * has done, the slave seconds field would also have incremented (so
   * it needs to be set back).
   */
  if (Rtc->NanoSeconds < TimestampT3) {
    /** nanosec has wrapped since timestamp was taken so decrement the
     * seconds field */
    if (Rtc->SecondsLower == 0x00000000) {
      Rtc->SecondsUpper = Rtc->SecondsUpper - 0x1;
    }
    Rtc->SecondsLower = Rtc->SecondsLower - 0x1;
  }

  /** Format the Timestamp (t3) into correct byte positioning for PTP frame
   *  storage, the write the Timestamp (t3) to the PDelayRespFollowUp frame
   */
  BufferWordA = XAvb_ReorderWord((Rtc->SecondsUpper<<16) |
                                 (Rtc->SecondsLower>>16));

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_UPPER_OFFSET,
                       BufferWordA);

  BufferWordA = XAvb_ReorderWord((Rtc->SecondsLower<<16) | (TimestampT3>>16));

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_MID_OFFSET,
                       BufferWordA);

  BufferWordA = XAvb_ReorderWord(TimestampT3<<16);
  BufferWordB = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                     XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                                     XAVB_PTP_TX_PKT_TIMESTAMP_LOWER_OFFSET);
  BufferWordA &= 0x0000ffff;
  BufferWordA |= (BufferWordB & 0xffff0000);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET,
                       XAVB_PTP_TX_PKT_TIMESTAMP_LOWER_OFFSET,
                       BufferWordA);


  /** Send the PDelayRespFollowUp Frame!
   *----------------------------------- */

  XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                 XAVB_PTP_TX_CONTROL_OFFSET,
                 XAVB_PTP_TX_SEND_PDELAYRESPFOLLOWUP_FRAME_MASK);

#ifdef DEBUG_XAVB_LEVEL3
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\n--XAvb_SendPDelayRespFollowUp()-");
  xil_printf("\r\n--------------------------------");
  xil_printf("\r\nReading TimestampT3 from address %x",
            (InstancePtr->Config.BaseAddress + XAVB_PTP_TX_PDELAYRESP_OFFSET
             + XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET));
  xil_printf("\r\nTimestampT3 is %x", TimestampT3);
#endif

}


/****************************************************************************/
/**
*
* A function to check that various fields in the received frame contain the
* expected values which define it as a valid AVB PTP frame.  If this check does
* not pass then the frame should not be decoded and used.
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return An updated True/False decision as to whether this received frame
*         really is a valid PTP type.
*
* @note   None.
*
*****************************************************************************/
u32 XAvb_IsRxFramePTP(XAvb * InstancePtr,
                      u32    PtpFrameBaseAddr) {
  u32 FrameIsPTP;
  u32 FrameField;

  /** Start by assuming that it is a valid PTP frame */
  FrameIsPTP = 1;

  /** Perform a 32-bit read from the relevant position in the frame */
  FrameField = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                   PtpFrameBaseAddr,
                                   XAVB_PTP_RX_PKT_TYPE_OFFSET);

  FrameField = XAvb_ReorderWord(FrameField);

  /** Check the Length/Type field for a valid Ethertype */
  if ((FrameField >>16) != XAVB_PTP_ETHERTYPE) {
    FrameIsPTP = 0;
#ifdef DEBUG_XAVB_LEVEL1
    xil_printf("\r\nXAvb_IsRxFramePTP(): Bad Ethertype: %x", (FrameField >>16));
#endif
  }

  /** Check the versionPTP */
  if ((FrameField & 0xF) != XAVB_PTP_VERSION_PTP) {
    FrameIsPTP = 0;
#ifdef DEBUG_XAVB_LEVEL1
    xil_printf("\r\nXAvb_IsRxFramePTP():Bad versionPTP:%x", (FrameField & 0xF));
#endif
  }

  return FrameIsPTP;

}


/****************************************************************************/
/**
*
* A function to decode a received PTP Sync Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_DecodeRxSync(XAvb * InstancePtr,
                       u32    PtpFrameBaseAddr) {

  u32 ReadWord = 0;
  XAvb_PortIdentity syncPortID;

  /** Read sourcePortIdentity from packet */
  XAvb_GetPortIdentity(InstancePtr->Config.BaseAddress, PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_PORTID_UPPER_OFFSET, &syncPortID);

  /** Only decode if configured for a slave and if SourcePortID is that of the
   * RTC Clock Master */
  if ( (InstancePtr->CurrentBmc.IAmTheRtcMaster == 0) &&
        XAvb_ComparePortIdentity(InstancePtr->Config.BaseAddress,
                                 InstancePtr->CurrentBmc.SourcePortIdentity,
                                 syncPortID) ) {

    /** Reset Sync Interval Counter as we have received a valid Sync */
    InstancePtr->PtpCounters.CounterSyncInterval = 0;

    /** Capture the local Timestamp for receipt of this frame and adjust it for
     *  MAC receive latency */
    InstancePtr->PtpRecords.SlaveSyncTimestamp
        = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                              PtpFrameBaseAddr,
                              XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET)
          - XAVB_RX_MAC_LATENCY_IN_NS;

    /**  Capture the Sync SequenceID */
    ReadWord = XAvb_ReorderWord(XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                PtpFrameBaseAddr,
                                XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET));
    InstancePtr->SequenceIdRecords.SyncSequenceId = (ReadWord >> 16);

    /** Capture the logMeanMessageInterval and convert into a useful duration
     * (NOTE: there is an implicit conversion from u32 to char here)
     */
    InstancePtr->latestMDSyncReceive.logMessageInterval = ReadWord & 0x000000ff;
    InstancePtr->latestMDSyncReceive.SyncIntervalDuration =
                 XAvb_ConvertLogMeanToDuration(InstancePtr->latestMDSyncReceive.logMessageInterval);

    /** We don't need to capture the correction field - unless we want to check that it is 0.*/

  } else {
    xil_printf("\r\nXAvb_DecodeRxSync()");
    xil_printf("\r\nSync ignored due to unmatched SourcePortID");
#ifdef DEBUG_XAVB_LEVEL1
    xil_printf("\r\nInstancePtr->CurrentBmc.SourcePortIdentity.ClockIdentityUpper = %x",InstancePtr->CurrentBmc.SourcePortIdentity.ClockIdentityUpper);
    xil_printf("\r\nInstancePtr->CurrentBmc.SourcePortIdentity.ClockIdentityLower = %x",InstancePtr->CurrentBmc.SourcePortIdentity.ClockIdentityLower);
    xil_printf("\r\nInstancePtr->CurrentBmc.SourcePortIdentity.PortNumber = %x\r\n",InstancePtr->CurrentBmc.SourcePortIdentity.PortNumber);
    xil_printf("\r\nsyncPortID.ClockIdentityUpper = %x",syncPortID.ClockIdentityUpper);
    xil_printf("\r\nsyncPortID.ClockIdentityLower = %x",syncPortID.ClockIdentityLower);
    xil_printf("\r\nsyncPortID.PortNumber = %x\r\n",syncPortID.PortNumber);
#endif
  }
}


/****************************************************************************/
/**
*
* A function to decode a received PTP Follow-up Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_DecodeRxFollowUp(XAvb * InstancePtr,
                           u32    PtpFrameBaseAddr) {

  XAvb_PortIdentity followUpPortID;

  /** Read sourcePortIdentity from packet */
  XAvb_GetPortIdentity(InstancePtr->Config.BaseAddress, PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_PORTID_UPPER_OFFSET, &followUpPortID);

  /** Only decode if configured for a slave and if SA is that of the RTC
   * Clock Master */
  if ( (InstancePtr->CurrentBmc.IAmTheRtcMaster == 0) &&
        XAvb_ComparePortIdentity(InstancePtr->Config.BaseAddress,
                                 InstancePtr->CurrentBmc.SourcePortIdentity,
                                 followUpPortID) ) {

    /** Capture the Follow Up SequenceID */
    InstancePtr->SequenceIdRecords.FollowUpSequenceId
       = XAvb_ReorderWord(XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                              PtpFrameBaseAddr,
                                              XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET))
         >> 16;

    /** SequenceID in Follow Up Frame should always match that of the
     * Sync Frame */
    if (InstancePtr->SequenceIdRecords.FollowUpSequenceId
         == InstancePtr->SequenceIdRecords.SyncSequenceId) {

      /** Capture the correction field from follow up frame */
      InstancePtr->PtpRecords.MasterCorrectionField
         = XAvb_ReorderWord(XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                                PtpFrameBaseAddr,
                                                XAVB_PTP_RX_PKT_CORRECTION_FIELD_OFFSET));

      /** Perform the Course RTC Offset correction for every Sync /
       * FollowUp pair */
      XAvb_CalcRtcOffset(InstancePtr,
                         PtpFrameBaseAddr);

      /** Every n Sync / FollowUp pairs, we are going to calculate a
       * corrected increment rate of RTC */
      if ((InstancePtr->PtpCounters.CounterSyncEvents & 0xF)
          == (XAVB_NUM_SYNC_FU_PAIR_CALC_RTC_INCREMENT - 1)) {

        /** Reset the CounterSyncEvents Counter */
        InstancePtr->PtpCounters.CounterSyncEvents = 0x0;

        /** Capture the Sequence ID of the Follow Up frame */
        InstancePtr->SequenceIdRecords.NewSyncSequenceId
            = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                  PtpFrameBaseAddr,
                                  XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET);

        InstancePtr->SequenceIdRecords.NewSyncSequenceId
            = (XAvb_ReorderWord
                   (InstancePtr->SequenceIdRecords.NewSyncSequenceId)
               ) >> 16;

        /** Perform the RTC increment rate adjustment calculation */
        XAvb_UpdateRtcIncrement(InstancePtr);

        /** Sample Sync Frame Time sent (as estimated by the slave) for
         * comparison in ten more repetition's time */
        InstancePtr->PtpRecords.OldSlaveTime
            = InstancePtr->PtpRecords.NewSlaveTime;

        /** Sample Sync Frame Time sent (as calculated by the master)
         * for comparison in ten more repetition's time */
        InstancePtr->PtpRecords.OldMasterTime
            = InstancePtr->PtpRecords.NewMasterTime;

        /** Sample the current Follow Up Sequence ID for comparison in
         * ten more repetition's time */
        InstancePtr->SequenceIdRecords.OldSyncSequenceId
            = InstancePtr->SequenceIdRecords.NewSyncSequenceId;

      } else {
        InstancePtr->PtpCounters.CounterSyncEvents
             = InstancePtr->PtpCounters.CounterSyncEvents + 1;
      }
    } else {
      xil_printf("SequenceIDs on RxFollowup don't match.\r\n");
    }
  } else {
    xil_printf("\r\nXAvb_DecodeRxFollowUp()");
    xil_printf("\r\nFollowUp ignored due to unmatched SourcePortID\r\n");
  }
}


/****************************************************************************/
/**
*
* A function to decode a received PDelayResp Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_DecodeRxPDelayResp(XAvb * InstancePtr,
                             u32    PtpFrameBaseAddr) {

  /** Have we already seen a PDelayResp since the last
   *  PDelayReq was sent? If so, ignore the packet */
  if( InstancePtr->StateMachineData.rcvdPDelayResp ) {
    xil_printf("Error: already saw a PDelayResp since the last PDelayReq was sent\r\n");
    return;
  }

  /** Find the ClockIdentity of the Sender */
  XAvb_GetPortIdentity(InstancePtr->Config.BaseAddress, PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_PORTID_UPPER_OFFSET,
                       &InstancePtr->StateMachineData.respPortIdentity);

  /** Is the PDelayResp message from ourself?  If so, the Peer
   *  is most likely a dumb hub and should be considered not
   *  ASCapable */
  if( XAvb_ComparePortIdentity(InstancePtr->Config.BaseAddress,
                               InstancePtr->portIdLocal,
                               InstancePtr->StateMachineData.respPortIdentity) ) {

    XAvb_ChangePeerASCapability(InstancePtr, 0);

    xil_printf("\r\nXAvb_DecodeRxPDelayResp():The peer is no longer ASCapable ");
    xil_printf("\r\nXAvb_DecodeRxPDelayResp():Saw a PDelayResp from myself\r\n");
    return;

  }

  /** Capture the requestingPortIdentity */
  XAvb_GetPortIdentity(InstancePtr->Config.BaseAddress, PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_REQ_PORTID_UPPER_OFFSET,
                       &InstancePtr->StateMachineData.respReqPortIdentity);

  /** Capture the PDelayResp SequenceID */
  InstancePtr->SequenceIdRecords.PDelayRespSequenceId =
      XAvb_ReorderWord(XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                       PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET)) >> 16;

  /** Verify that the requestingPortIdentity matches our
   *  portIdentity */
  if( !XAvb_ComparePortIdentity(InstancePtr->Config.BaseAddress,
                           InstancePtr->portIdLocal,
                           InstancePtr->StateMachineData.respReqPortIdentity) ) {
    xil_printf("Error: PDelayResp reqPortID doesn't match our portID\r\n");
    return;
  }

  /** Only process if the received frame's sequenceId matches
   *  the sequenceId sent in the last pDelay_Req packet */
  if( (InstancePtr->SequenceIdRecords.PDelayReqSequenceId ==
       InstancePtr->SequenceIdRecords.PDelayRespSequenceId) ) {

    /** Mark this as a valid PDelayResp packet */
    InstancePtr->StateMachineData.rcvdPDelayResp = 1;

    /** Capture timestamp for receipt time of PDelayReq at Master (t2) */
    InstancePtr->PtpRecords.PDelayTimestampT2 =
        XAvb_CaptureNanoSec(InstancePtr->Config.BaseAddress,
                            PtpFrameBaseAddr);

    /** Capture timestamp for receipt time of PDelayResp at Slave (t4) and adjust
     *  it for MAC receive latency */
    InstancePtr->PtpRecords.PDelayTimestampT4 =
        XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                            PtpFrameBaseAddr,
                            XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET)
        - XAVB_RX_MAC_LATENCY_IN_NS;
  } else {
    xil_printf("Error: PDelayResp seqID's don't match\r\n");
  }
}


/****************************************************************************/
/**
*
* A function to decode a received PDelayRespFollowUp Packet
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_DecodeRxPDelayRespFollowUp(XAvb * InstancePtr,
                                     u32    PtpFrameBaseAddr) {

   XAvb_PortIdentity portId;

  /** Has a valid PDelayResp packet been received since the
   *  last PDelayReq packet was sent? */
  if( !InstancePtr->StateMachineData.rcvdPDelayResp ) {
    /*xil_printf("Error: Received a PDelayRespFollowUp before receiving a PDelayResp\r\n");*/
    return;
  }

  /** Capture the PDelayRespFollowUp SequenceID */
  InstancePtr->SequenceIdRecords.PDelayFollowUpSequenceId =
      XAvb_ReorderWord(XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                           PtpFrameBaseAddr,
                                           XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET)) >> 16;

  /** Get the sourcePortIdentity of the sender */
  XAvb_GetPortIdentity(InstancePtr->Config.BaseAddress, PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_PORTID_UPPER_OFFSET,
                       &portId);

  /** The sourcePortIdentity of the PDelayRespFollowUp should
   *  match that of the last PDelayResp packet received */
  if( !XAvb_ComparePortIdentity(InstancePtr->Config.BaseAddress,
                                portId,
                                InstancePtr->StateMachineData.respPortIdentity) ) {
    xil_printf("Error: sourcePortIdentity of PDelayRespFollowUp doesn't match PDelayResp\r\n");
    return;
  }

  /** Get the requestingPortIdentity of the sender */
  XAvb_GetPortIdentity(InstancePtr->Config.BaseAddress, PtpFrameBaseAddr,
                       XAVB_PTP_RX_PKT_REQ_PORTID_UPPER_OFFSET,
                       &portId);

  /** The requestingPortIdentity of the PDelayRespFollowUp should
   *  match that of the last PDelayResp packet received */
  if( !XAvb_ComparePortIdentity(InstancePtr->Config.BaseAddress,
                                portId,
                                InstancePtr->StateMachineData.respReqPortIdentity) ) {
    xil_printf("Error: reqPortID of PDelayRespFollowUp doesn't match PDelayResp\r\n");
    return;
  }

  /** SequenceID of PDelayRespFollowUp Frame should always match that of
   * the PDelayResp Frame and the original PDelayReq Frame. */
  if (InstancePtr->SequenceIdRecords.PDelayFollowUpSequenceId ==
      InstancePtr->SequenceIdRecords.PDelayRespSequenceId) {

    /** Mark this as a valid PDelayRespFollowUp packet */
    InstancePtr->StateMachineData.rcvdPDelayRespFollowUp = 1;

    /** Capture the timestamp for transmit time of PDelayResp at Master
     * (t3) */
    InstancePtr->PtpRecords.PDelayTimestampT3 =
        XAvb_CaptureNanoSec(InstancePtr->Config.BaseAddress, PtpFrameBaseAddr);

    /** Now we know t1, t2, t3 and t4, calculate the link delay */
    XAvb_CalcDelay(InstancePtr);

  } else {
    xil_printf("Error: seqID of PDelayRespFollowUp doesn't match PDelayResp\r\n");
  }
}

/****************************************************************************/
/**
*
* A function to decode a received Signalling Packet and modify the TX PTP
* Buffers based on the requested values.
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Signaling Packet
*         in the Rx PTP Packet Buffer
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_DecodeRxSignaling(XAvb * InstancePtr, u32 PtpFrameBaseAddr) {

  u32 ReadData;
  char currentInterval;

  /** Read the requested logMeanMessage durations from the Signalling frame */
  ReadData = XAvb_ReorderWord(XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                                  PtpFrameBaseAddr,
                                                  XAVB_PTP_RX_PKT_SIGNALING_DELAY_INTERVAL_OFFSET));

  /** update linkDelayInterval */
  currentInterval = (ReadData >> 24);
  switch( currentInterval ) {
  case (-128):
    /** don't change the interval */
    break;
  /** currently only support the default value */
  case XAVB_DEFAULT_LOG_MEAN_PDELAY_REQ_INTERVAL:
  case 126:
    /** set the interval to initial value */
    InstancePtr->SignallingFrameData.LinkDelayIntervalDuration =
      XAvb_UpdateIntervalDuration(InstancePtr->SignallingFrameData.LinkDelayIntervalDuration,
                                  XAVB_DEFAULT_LOG_MEAN_PDELAY_REQ_INTERVAL);

    /** Update logMeanMessageInterval in the pre-loaded TX PDELAYREQ message buffer */
    XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                      XAVB_PTP_TX_PDELAYREQ_OFFSET,
                                      InstancePtr->SignallingFrameData.LinkDelayIntervalDuration);
    break;
  case 127:
    /** stop sending pDelay messages */
    InstancePtr->SignallingFrameData.LinkDelayIntervalDuration = XAVB_PKT_TYPE_DISABLED;
    break;
  default:
      xil_printf( "Got a signalling message with an interval (%d) I don't support!\r\n", currentInterval);
  }

  /** update timeSyncInterval */
  currentInterval = (ReadData >> 16);
  switch( currentInterval ) {
  case (-128):
    /** don't change the interval */
    break;
  case XAVB_DEFAULT_LOG_MEAN_SYNC_INTERVAL:
  case 126:
    /** set the interval to initial value */
    InstancePtr->SignallingFrameData.SyncIntervalDuration =
      XAvb_UpdateIntervalDuration(InstancePtr->SignallingFrameData.SyncIntervalDuration,
                                  XAVB_DEFAULT_LOG_MEAN_SYNC_INTERVAL);

    /** Update logMeanMessageInterval in the pre-loaded TX SYNC message buffer */
    XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                      XAVB_PTP_TX_SYNC_OFFSET,
                                      InstancePtr->SignallingFrameData.SyncIntervalDuration);
    /** Update logMeanMessageInterval in the pre-loaded TX FOLLOW_UP message buffer */
    XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                    XAVB_PTP_TX_FOLLOW_UP_OFFSET,
                                    InstancePtr->SignallingFrameData.SyncIntervalDuration);
    break;
  case 127:
    /** stop sending sync messages */
    InstancePtr->SignallingFrameData.SyncIntervalDuration = XAVB_PKT_TYPE_DISABLED;
    break;
  default:
      xil_printf( "Got a signalling message with an interval (%d) I don't support!\r\n", currentInterval);
  }

  /** update announceInterval */
  currentInterval = (ReadData >> 8);
  switch( currentInterval ) {
  case (-128):
    /** don't change the interval */
    break;
  case XAVB_DEFAULT_LOG_MEAN_ANNOUNCE_INTERVAL:
  case 126:
    /** set the interval to initial value */
    InstancePtr->SignallingFrameData.AnnounceIntervalDuration =
      XAvb_UpdateIntervalDuration(InstancePtr->SignallingFrameData.AnnounceIntervalDuration,
                                  XAVB_DEFAULT_LOG_MEAN_ANNOUNCE_INTERVAL);

    /** Update logMeanMessageInterval in the pre-loaded TX ANNOUNCE message buffer  */
    XAvb_UpdateLogMeanMessageInterval(InstancePtr->Config.BaseAddress,
                                      XAVB_PTP_TX_ANNOUNCE_OFFSET,
                                      InstancePtr->SignallingFrameData.AnnounceIntervalDuration);
    break;
  case 127:
    /** stop sending Announce messages */
    InstancePtr->SignallingFrameData.AnnounceIntervalDuration = XAVB_PKT_TYPE_DISABLED;
    break;
  default:
      xil_printf( "Got a signalling message with an interval (%d) I don't support!\r\n", currentInterval);
  }
}


/****************************************************************************/
/**
*
* A function to update a PTP message Interval Duration (defined as a
* fraction of 128 seconds). If the endpoint cannot support a requested
* logMeanVal then do not perform the conversion - return the current value.
*
* @param  currentIntervalDuration is the Interval Duration to be updated
* @param  logMeanVal is the base2 value that is to be converted
*
* @return logMeanVal represented as a fraction of 128
*
* @note   This endpoint only supports logMeanValues >=-7 and <=8.
*
*****************************************************************************/
u16 XAvb_UpdateIntervalDuration(u16   currentIntervalDuration,
                                char logMeanVal) {

  if((logMeanVal >=  XAVB_MIN_SUPPORTED_LOG_MEAN_INTERVAL) &&
     (logMeanVal <=  XAVB_MAX_SUPPORTED_LOG_MEAN_INTERVAL)) {

    return XAvb_ConvertLogMeanToDuration(logMeanVal);

  } else {

    return currentIntervalDuration;

  }
}


/****************************************************************************/
/**
*
* A function to convert a logMean (power of 2) value into a fraction of 128
* that is compatible with Signalling data.
*
* @param  logMeanVal is the base2 value that is to be converted

* @return logMeanVal represented as a fraction of 128
*
* @note   None.
*
*****************************************************************************/
u16 XAvb_ConvertLogMeanToDuration(char logMeanVal) {

  u8  logMeanAbs;

  logMeanAbs = (u8)(abs(logMeanVal));

  return (logMeanVal < 0) ?
         (128 >> logMeanAbs) :
         (logMeanVal > 0) ?
         (128 << logMeanAbs) :
         128;
}

/****************************************************************************/
/**
*
* A function to convert a fraction of 128 value that is compatible with
* Signalling data into a logMean (power of 2) value;
*
* @param  fractionalVal is the Signalling data value that is to be converted

* @return fractionalVal represented as logMean (power of 2) value
*
* @note   None.
*
*****************************************************************************/
char XAvb_ConvertDurationToLogMean(u16 fractionalVal) {

  char logMeanVal = 0;
  u8    numShifts = 0;

  /** just in case fractionalVal is not a power of 2, we'll
   *  only look at the most significant bit
   *  Count how many shifts it takes for most significant set bit
   *  to be in the highest (16th) bit location
   */
  while( !(fractionalVal & 0x8000) ) {
    fractionalVal <<= 1;
    numShifts++;
  }

  /** logMeanVal = 0 = 2^0 = 128/128 would give us a numShifts
   *  result of 8, so 8 will be our base
   */
  logMeanVal = 8 - numShifts;

  return logMeanVal;
}

/****************************************************************************/
/**
*
* A function to update the logMeanMessageInterval field in a PTP packet
*
* @param  BaseAddress is the base address of the device
* @param  PtpFrameBaseAddr is the base address of the TX PTP Buffer to be updated
* @param  intervalDuration is the "fraction of 128" value of the data to be written
*
* @return None.  But the relevant Tx PTP Packet Buffer is written to with the
*                updated logMeanMessageInterval
*
* @note   None.
*
*****************************************************************************/
void XAvb_UpdateLogMeanMessageInterval(u32 BaseAddress,
                                       u32 PtpFrameBaseAddr,
                                       u16 intervalDuration) {

  u32 ReadVal;
  u8 logMean;

  /** Convert intervalDuration to a logMean value */
  logMean = (u8)XAvb_ConvertDurationToLogMean(intervalDuration);

  /** Read the current fields */
  ReadVal = XAvb_ReadPtpBuffer(BaseAddress,
                                PtpFrameBaseAddr,
                                XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET);

  /** Update just the logMeanMessageInterval field */
  ReadVal = (ReadVal & 0x00ffffff) | (logMean << 24);

  /** Write back */
  XAvb_WritePtpBuffer(BaseAddress,
                       PtpFrameBaseAddr,
                       XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET,
                       ReadVal);
}

/****************************************************************************/
/**
*
* This function updates the portIdLocal local copy of the sourcePortIdentity
* and writes this value into the TX PTP frame buffer templates. The fields
* that are written are:
* o sourcePortIdentity for all default PTP frames
* o Announce:: grandmasterIdentity
* o Announce:: TLV clockIdentity
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  systemIdentity is the clockIdentity and portNumber for this endpoint
*
* @return None.
*
* @note   Announce::TLV. By default the tlvType and length field are set
*         up in the BRAM, assuming that N = 1.
*
****************************************************************************/
void XAvb_SetupSourcePortIdentity(XAvb * InstancePtr,
                                  XAvb_PortIdentity systemIdentity)
{
  u32 BufferWord;
  u8  BufferEnable;
  u32 DataBitEnable;
  u32 ReadWord;

  /* Set up our local copy of the sourcePortIdentity */
  InstancePtr->portIdLocal = systemIdentity;

  /** Write the sourcePortIdentity into the header for all TX PTP buffers
   *  except the empty default buffer AND write the GMID for TX announce AND
   *  Write the ClockIdentity into the TX Announce TLV */
  BufferEnable =  0x7F;

  /** (a) Write the upper 2 bytes of the ClockIdentityUpper
   *   - REM: Swap back the byte order into frame storage order */
  DataBitEnable = 0xFFFF0000;
  BufferWord = (systemIdentity.ClockIdentityUpper >> 16);
  BufferWord = XAvb_ReorderWord(BufferWord);

  XAvb_WriteToMultipleTxPtpFrames(InstancePtr->Config.BaseAddress,
                                  XAVB_PTP_TX_PKT_PORTID_UPPER_OFFSET,
                                  BufferWord,
                                  DataBitEnable,
                                  BufferEnable);

  ReadWord = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                   XAVB_PTP_TX_ANNOUNCE_OFFSET,
                                   XAVB_PTP_TX_PKT_ANNOUNCE_TLVLEN_PATHSEQ_START_OFFSET);

  BufferWord =  (ReadWord & 0x0000FFFF) | (BufferWord & 0xFFFF0000);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_ANNOUNCE_OFFSET,
                       XAVB_PTP_TX_PKT_ANNOUNCE_TLVLEN_PATHSEQ_START_OFFSET,
                       BufferWord);

  /** (b) Write the lower 2 bytes of the ClockIdentityUpper and upper
   *   2 bytes of the ClockIdentityLower.
   *   - REM: Swap back the byte order into frame storage order*/
  BufferWord = ((systemIdentity.ClockIdentityUpper << 16) & 0xFFFF0000) | ((systemIdentity.ClockIdentityLower >> 16) & 0x0000FFFF);
  BufferWord = XAvb_ReorderWord(BufferWord);
  DataBitEnable = 0xFFFFFFFF;

  XAvb_WriteToMultipleTxPtpFrames(InstancePtr->Config.BaseAddress,
                                  XAVB_PTP_TX_PKT_PORTID_MID_OFFSET,
                                  BufferWord,
                                  DataBitEnable,
                                  BufferEnable);


  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_ANNOUNCE_OFFSET,
                       (XAVB_PTP_TX_PKT_ANNOUNCE_TLVLEN_PATHSEQ_START_OFFSET + 4),
                       BufferWord);

  /** (c) Write the lower 2 bytes of the ClockIdentityLower and the portNumber
   *   - REM: Swap back the byte order into frame storage order*/
  BufferWord = ((systemIdentity.ClockIdentityLower << 16) & 0xFFFF0000) | (systemIdentity.PortNumber & 0x0000FFFF);
  BufferWord = XAvb_ReorderWord(BufferWord);

  XAvb_WriteToMultipleTxPtpFrames(InstancePtr->Config.BaseAddress,
                                  XAVB_PTP_TX_PKT_PORTID_LOWER_OFFSET,
                                  BufferWord,
                                  DataBitEnable,
                                  BufferEnable);

  BufferWord = (systemIdentity.ClockIdentityLower << 16) & 0xFFFF0000;
  BufferWord = XAvb_ReorderWord(BufferWord);
  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_ANNOUNCE_OFFSET,
                       (XAVB_PTP_TX_PKT_ANNOUNCE_TLVLEN_PATHSEQ_START_OFFSET + 8),
                       BufferWord);



  /** Write the grandmasterIdentity into the header for the TX Announce PTP buffer */
  /** (a) Write 1 byte of GMID (Upper)*/
  BufferWord = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                   XAVB_PTP_TX_ANNOUNCE_OFFSET,
                                   XAVB_PTP_TX_PKT_ANNOUNCE_QUAL_LOW_PRI2_GMID_HI_OFFSET);

  BufferWord =  (BufferWord & 0x00FFFFFF) | (systemIdentity.ClockIdentityUpper & 0xFF000000);

  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_ANNOUNCE_OFFSET,
                       XAVB_PTP_TX_PKT_ANNOUNCE_QUAL_LOW_PRI2_GMID_HI_OFFSET,
                       BufferWord);

  /** (b) Write 3 bytes of GMID (Upper) and 1 byte of GMID (Lower)**/
  BufferWord =  (systemIdentity.ClockIdentityUpper << 8) | ((systemIdentity.ClockIdentityLower >> 24) & 0x000000FF);
  BufferWord = XAvb_ReorderWord(BufferWord);
  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_ANNOUNCE_OFFSET,
                       XAVB_PTP_TX_PKT_ANNOUNCE_GMID_MID_OFFSET,
                       BufferWord);

  /** (c) Write 3 bytes of GMID (Lower) */
  BufferWord = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                                   XAVB_PTP_TX_ANNOUNCE_OFFSET,
                                   XAVB_PTP_TX_PKT_ANNOUNCE_GMID_LOW_STEPSREMOVED_HI_OFFSET);

  BufferWord = ((BufferWord >> 24) & 0x000000FF) | ((systemIdentity.ClockIdentityLower << 8) & 0xFFFFFF00);
  BufferWord = XAvb_ReorderWord(BufferWord);
  XAvb_WritePtpBuffer(InstancePtr->Config.BaseAddress,
                       XAVB_PTP_TX_ANNOUNCE_OFFSET,
                       XAVB_PTP_TX_PKT_ANNOUNCE_GMID_LOW_STEPSREMOVED_HI_OFFSET,
                       BufferWord);


#ifdef DEBUG_XAVB_LEVEL2
    xil_printf("\r\n setupSourcePortIdentity: Writing systemIdentity to buffers %08x", BufferEnable);
    xil_printf("\r\n setupSourcePortIdentity: ClockIdentityUpper --> 0x%08x, ClockIdentityLower --> 0x%08x, PortNumber --> 0x%08x",
                                 InstancePtr->portIdLocal.ClockIdentityUpper,
                                 InstancePtr->portIdLocal.ClockIdentityLower,
                                 InstancePtr->portIdLocal.PortNumber);
#endif
}
