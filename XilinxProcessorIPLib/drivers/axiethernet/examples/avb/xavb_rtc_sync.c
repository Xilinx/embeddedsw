/******************************************************************************
*
* Copyright (C) 2008 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/******************************************************************************/
/***
*
* @file xavb_rtc_sync.c
*
* The XAvb driver. Functions in this file all contain calculations which are
* essential for the AVB (1588 based) Real Time Clock (RTC) Sychronisation.  In
* here are functions to measure the Link Delay (Master and Slave); to measure
* and correct the current RTC error (Slave); to measure and correct the current
* RTC increment rate error.
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

/****************************** Include Files *********************************/

#include "xil_types.h"
#include "xavb_hw.h"
#include "xavb.h"

/*************************** Constant Definitions *****************************/

/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Variable Definitions *****************************/

/*************************** Function Prototypes ******************************/

/******************************************************************************/

/*****************************************************************************/
/***
*
* A function to capture the nanosecond timestamp field from a received PTP frame
*
* @param  BaseAddress is the base address of the device
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return The Nanoseconds Timestamp field, captured from an Rx PTP frame
*
* @note   None.
*
*****************************************************************************/
u32 XAvb_CaptureNanoSec(u32 BaseAddress, u32 PtpFrameBaseAddr) {
  u32 Timestamp   = 0;
  u32 BufferWordA = 0;
  u32 BufferWordB = 0;

  /** The timestamp is located over several 32-bit Words of the PTP frame buffer
   *  Read the relevant Words containing the ns timestamp: */
  BufferWordA = XAvb_ReadPtpBuffer(BaseAddress,
                                    PtpFrameBaseAddr,
                                    XAVB_PTP_RX_PKT_TIMESTAMP_MID_OFFSET);

  BufferWordB = XAvb_ReadPtpBuffer(BaseAddress,
                                    PtpFrameBaseAddr,
                                    XAVB_PTP_RX_PKT_TIMESTAMP_LOWER_OFFSET);

  /** Now re-arrange the data from the Words to obtain the required ns Timestamp
   *  in binrary format */
  Timestamp = (XAvb_ReorderWord(BufferWordA)<<16) |
              (XAvb_ReorderWord(BufferWordB)>>16);

  return Timestamp;

}


/*****************************************************************************/
/***
*
* A function to Measure the Link Delay of the local full-duplex Ethernet link
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
void XAvb_CalcDelay(XAvb * InstancePtr) {
  u32 T4MinusT1 = 0;
  u32 T3MinusT2 = 0;
  u32 Delay     = 0;

  /** Since we are only using the nanoseconds field here we need to account for
   *  wrap.  So we add one second to the T4 and T3 terms to ensure that the
   *  T4MinusT1 and T3MinusT2 results cannot be negative.  These two additional
   *  seconds then cancel each other out in the T4MinusT1 - T3MinusT2 equation.
   */
#ifdef DEBUG_XAVB_LEVEL2
    xil_printf("\r\nXAvb_CalcDelay()");
    xil_printf("\r\nt1        %x ", InstancePtr->PtpRecords.PDelayTimestampT1);
    xil_printf("\r\nt2        %x ", InstancePtr->PtpRecords.PDelayTimestampT2);
    xil_printf("\r\nt3        %x ", InstancePtr->PtpRecords.PDelayTimestampT3);
    xil_printf("\r\nt4        %x ", InstancePtr->PtpRecords.PDelayTimestampT4);
#endif

  /** If the nanoseconds count has wrapped, add on 1 second to ensure we get the right answer*/
  if (InstancePtr->PtpRecords.PDelayTimestampT4 < InstancePtr->PtpRecords.PDelayTimestampT1) {
     T4MinusT1 = (InstancePtr->PtpRecords.PDelayTimestampT4 + XAVB_ONE_SECOND)
                - InstancePtr->PtpRecords.PDelayTimestampT1;
  } else {
     T4MinusT1 = InstancePtr->PtpRecords.PDelayTimestampT4
                - InstancePtr->PtpRecords.PDelayTimestampT1;
  }
  /** If the nanoseconds count has wrapped, add on 1 second to ensure we get the right answer*/
  if (InstancePtr->PtpRecords.PDelayTimestampT3 < InstancePtr->PtpRecords.PDelayTimestampT2) {
    T3MinusT2 = (InstancePtr->PtpRecords.PDelayTimestampT3 + XAVB_ONE_SECOND)
                - InstancePtr->PtpRecords.PDelayTimestampT2;
  } else {
     T3MinusT2 = InstancePtr->PtpRecords.PDelayTimestampT3
                - InstancePtr->PtpRecords.PDelayTimestampT2;
  }

  Delay       = (T4MinusT1 - T3MinusT2) >> 1;

  /** For now we are simply going to throw out any absurdly large link delays.*/
  if (Delay < XAVB_NEIGHBOR_PROP_DELAY_THRESH ) {

    InstancePtr->PtpRecords.LinkDelay  = Delay;

    /** The peer has responded to the pDelay_Req and the measured delay is
    * within tolerance: the peer is deemed to be AS capable */
    XAvb_ChangePeerASCapability(InstancePtr, 1);

  } else {
    xil_printf("\r\n Bad Link Delay %d ", Delay);
#ifdef DEBUG_XAVB_LEVEL2
    xil_printf("\r\nXAvb_CalcDelay()");
    xil_printf("\r\nt1        %x ", InstancePtr->PtpRecords.PDelayTimestampT1);
    xil_printf("\r\nt2        %x ", InstancePtr->PtpRecords.PDelayTimestampT2);
    xil_printf("\r\nt3        %x ", InstancePtr->PtpRecords.PDelayTimestampT3);
    xil_printf("\r\nt4        %x ", InstancePtr->PtpRecords.PDelayTimestampT4);
    xil_printf("\r\nLinkDelay %x ", InstancePtr->PtpRecords.LinkDelay);
#endif
  }
}


/*****************************************************************************/
/***
*
* A function to calculate the Slave Offset from the GrandMaster time
*
* @param  InstancePtr is a pointer to the XAvb instance to be worked on
* @param  PtpFrameBaseAddr is the base address of the received Announce Packet
*         in the Rx PTP Packet Buffer
*
* @return The PtpRecords data structure is updated with the calculated RTC
*         Offset value.
*
* @note   None.
*
*****************************************************************************/
void XAvb_CalcRtcOffset (XAvb * InstancePtr,
                         u32    PtpFrameBaseAddr) {

  u32 MasterNanosec        = 0;
  u32 MasterSeconds        = 0;
  u32 MasterEpoch          = 0;

  u32 SyncRouteDelay       = 0;
  u32 MasterNsCorrected    = 0;
  u32 MasterNsHasWrapped   = 0;

  u32 SlaveNsTimestamp     = 0;
  XAvb_RtcFormat RtcError;

  u32 BufferWordA          = 0;
  u32 BufferWordB          = 0;

  XAvb_RtcFormat SlaveRtc;


  /** Capture the Slave Time
   * ----------------------------
   * We do this immediately to get the slave time ASAP (since processing
   * time is uncertain and the RTC does not stand still). */
  XAvb_ReadRtc(InstancePtr->Config.BaseAddress, &SlaveRtc);


  /** Capture the Master Origin Timestamp (from received FollowUp Frame)
   * ---------------------------- */
  MasterNanosec = XAvb_CaptureNanoSec(InstancePtr->Config.BaseAddress,
                                      PtpFrameBaseAddr);

  /** read the Words from the PTP frame buffer containing the RTC seconds field
   */
  BufferWordA = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                               PtpFrameBaseAddr,
                               XAVB_PTP_RX_PKT_TIMESTAMP_UPPER_OFFSET);

  BufferWordB = XAvb_ReadPtpBuffer(InstancePtr->Config.BaseAddress,
                               PtpFrameBaseAddr,
                               XAVB_PTP_RX_PKT_TIMESTAMP_MID_OFFSET);

  /** Now re-arrange the required data from the Words to obtain the required
   *  seconds field timestamp in binary format */
  MasterSeconds = (XAvb_ReorderWord(BufferWordA) << 16) |
                  (XAvb_ReorderWord(BufferWordB) >> 16);

  MasterEpoch   =  XAvb_ReorderWord(BufferWordA) >> 16;



  /** Correct the Nanoseconds
   * ----------------------------
   * NOTE: we are trying to compare the value of the slave RTC nano-
   * seconds field timestamp with the nano-seconds value of the Masters
   * RTC nanosecond field at exactly that time.
   *
   *
   * Sync Frame routing delay is equal to the value of the correction
   * field (sum of correction fields in Sync and FollowUp frames) plus
   * the link delay measurement made by this slave.
   */
   SyncRouteDelay = InstancePtr->PtpRecords.MasterCorrectionField
                    + InstancePtr->PtpRecords.LinkDelay;

  /** MasterNsCorrected time here is the calculated time that the
   * master will be at the point in time when the sync frame is received
   * (and timestamped) at the slave.  This is calculated from the
   * originTimeStamp (from the FollowUpframe), plus the Sync Frame
   * routing delay.  A direct comparison can then be made between master
   * and slave.
   */
  MasterNsCorrected = MasterNanosec + SyncRouteDelay;

  /** Check for ns wrap-around condition */
  if (MasterNsCorrected >= XAVB_ONE_SECOND) {
    MasterNsCorrected   = MasterNsCorrected - XAVB_ONE_SECOND;
    MasterNsHasWrapped  = 1;
  }


  /** Make the Master and Slave comparison and discover the difference! */
  RtcError.NanoSeconds = MasterNsCorrected
                         - InstancePtr->PtpRecords.SlaveSyncTimestamp;

  /** Check for ns wrap-around condition */
  if (RtcError.NanoSeconds >= XAVB_ONE_SECOND) {
    RtcError.NanoSeconds = RtcError.NanoSeconds + XAVB_ONE_SECOND;
  }


  /** Return these comparison figures in the form of a pointer (RTC
   * increment rate adjust function also needs to know this information) */
  InstancePtr->PtpRecords.NewSlaveTime
      = InstancePtr->PtpRecords.SlaveSyncTimestamp;

  InstancePtr->PtpRecords.NewMasterTime = MasterNsCorrected;


  /** Adjust the 8k clock logic (if necessary) */
  XAvb_Adjust8kClock(InstancePtr->Config.BaseAddress, RtcError.NanoSeconds);



  /** Correct the Seconds and Epoch
   * -----------------------------
   * NOTE: we are trying to compare the value of the slave RTC seconds
   * field at the exact time when the timestamp was taken with the
   * RTC seconds value of the Master at that time.
   *
   *
   * We need to know the value of the slaves synchronised nano-seconds
   * field at the time when the timestamp was taken (since timestamps
   * use the syntonised time).  So we add the current nanosecond field
   * offset value:
   */
  SlaveNsTimestamp = InstancePtr->PtpRecords.SlaveSyncTimestamp
                     + XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                                     XAVB_RTC_NANOSEC_OFFSET);

  /** Check for ns wrap-around condition */
  if (SlaveNsTimestamp >= XAVB_ONE_SECOND) {
    SlaveNsTimestamp = SlaveNsTimestamp - XAVB_ONE_SECOND;
  }

  /** Even though we read the slave RTC value at the beginning of this
   * function, there would have been processing delay between the
   * actual reception (and timestamping) of the FollowUp frame and the
   * start of this function.  During this time, the slave RTC seconds
   * field could have wrapped around.  We need to detect this and if it
   * has done, the slave seconds field would also have incremented (so
   * it needs to be set back).
   */
  if (SlaveRtc.NanoSeconds < SlaveNsTimestamp) {
    /** slave_nanosec has wrapped since timestamp so decrement the
     * seconds field */
    if (SlaveRtc.SecondsLower == 0x00000000) {
      SlaveRtc.SecondsUpper = SlaveRtc.SecondsUpper   - 0x1;
    }
    SlaveRtc.SecondsLower = SlaveRtc.SecondsLower - 0x1;
  }


  /** If the Master nano seconds field wrapped during the Sync frame
   * routing delay, then we need to increment the seconds field.
   */
  if (MasterNsHasWrapped == 1) {
    if (MasterSeconds == 0xFFFFFFFF) {
      MasterEpoch = MasterEpoch + 0x1;
    }
    MasterSeconds = MasterSeconds + 0x1;
  }


  /** Calculate the slave RTC error: the master time minus the timestamp
   * taken by this slave for Sync Frame reception. */
  RtcError.SecondsLower =  MasterSeconds - SlaveRtc.SecondsLower;
  RtcError.SecondsUpper =  MasterEpoch   - SlaveRtc.SecondsUpper;


#ifdef DEBUG_XAVB_LEVEL2
  if (RtcError.SecondsLower != 0) {
    xil_printf("\r\nXAvb_CalcRtcOffset()");
    xil_printf("\r\n-- Seconds Field Correction");
    xil_printf("\r\nSlaveNsTimestamp : %x"  , SlaveNsTimestamp);
    xil_printf("\r\nslave_ns           : %x", SlaveRtc.NanoSeconds);
    xil_printf("\r\n--");
    xil_printf("\r\nread slave seconds : %x",
                 XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                               XAVB_RTC_SEC_LOWER_VALUE_OFFSET));

    xil_printf("\r\ncalc slave secs    : %x", SlaveRtc.SecondsLower);
    xil_printf("\r\n--");
    xil_printf("\r\nmaster sec wrap  : %x"  , MasterNsHasWrapped);
    xil_printf("\r\ncalc master_secs : %x"  , MasterSeconds);
    xil_printf("\r\nrtc_sec_error : %x"     , RtcError.SecondsLower);
    xil_printf("\r\n--");
  }
#endif


  /** Write the results to the RTC Offset registers
   * --------------------------------------------- */
  XAvb_WriteRtcOffset(InstancePtr->Config.BaseAddress, &RtcError);

}


/*****************************************************************************/
/***
*
* A function to Adjust the phase offset of the 8k clock
*
* @param  InstancePtr->BaseAddress is the base address of the device
* @param  NewOffset is the newly calculated RTC Offset value
*
* @return None. But the devices RTC Phase Adjustment Register is updated
*
* @note   None.
*
*****************************************************************************/
void XAvb_Adjust8kClock (u32 BaseAddress, u32 NewOffset) {

  u32 PreviousOffset      = 0;
  u32 OffsetChange        = 0;
  u32 ChangeIn8kPeriods   = 0;
#ifdef DEBUG_XAVB_LEVEL2
  u32 Clock8kOffset       = 0;
#endif

  /** Read the previous offset */
  PreviousOffset = XAvb_ReadReg(BaseAddress, XAVB_RTC_NANOSEC_OFFSET);

  /** Calculate the change in the previous and current RTC ns offset */
  if (PreviousOffset > NewOffset) {
    OffsetChange =  PreviousOffset - NewOffset;
  } else {
    OffsetChange =  NewOffset - PreviousOffset;
  }


  /** Is the adjustment "large"? "large" is chosen here to be one 8k
   * clock period which is a somewhat arbitrary figure */
  if (OffsetChange > XAVB_PERIOD_8KHZ) {

#ifdef DEBUG_XAVB_LEVEL2
    Clock8kOffset = XAvb_ReadReg(BaseAddress, XAVB_RTC_8K_OFFSET_OFFSET);
    xil_printf("\r\nXAvb_Adjust8kClock()");
    xil_printf("\r\nold ns offset: %x"  , PreviousOffset);
    xil_printf("\r\nold Clk8kOffset: %x", Clock8kOffset);
#endif


    /** The value XAVB_PERIOD_8KHZ is one 8k clock period in ns.  We divide the
     * RTC ns offset change by this to get the offset change in a
     * multiple of 8k clock periods, the add 1 so that we always round
     * up.  Then multiply this by XAVB_PERIOD_8KHZ again so that we are always
     * phased aligned to the RTC master (only evey adjust in a multiple
     * of 8k periods.
     */
    ChangeIn8kPeriods = NewOffset / XAVB_PERIOD_8KHZ;
    OffsetChange = (ChangeIn8kPeriods + 1) * XAVB_PERIOD_8KHZ;

    /** Write the results to the 8K clock logic Offset register */
    XAvb_WriteReg(BaseAddress, XAVB_RTC_8K_OFFSET_OFFSET, OffsetChange);

#ifdef DEBUG_XAVB_LEVEL2
    xil_printf("\r\nXAvb_Adjust8kClock()");
    xil_printf("\r\nnew ns offset: %x"  , NewOffset);
    xil_printf("\r\nnew Clk8kOffset: %x", OffsetChange);
#endif

  }
}


/*****************************************************************************/
/***
*
* A function to calculate the RTC increment value based on the Slave Error
*
* @return None. But the devices RTC Increment Value Control Register is updated
*
* @note   None.
*
*****************************************************************************/
void XAvb_UpdateRtcIncrement(XAvb * InstancePtr) {

  u32 LoopCount           = 31;
  u8  SlaveIsFast         = 0;
  u32 SlaveTimeDuration   = 0;
  u32 MasterTimeDuration  = 0;
  u32 SlaveError          = 0;
  u32 ScaledError         = 0;
  u32 NormalisedError     = 0;
  u32 IncrementAdjust     = 0;
  u32 OldIncrement        = 0;
  u32 NewIncrement        = 0;


  /** Sanity Check that Sync Frames were n apart.  This safeguards the
   * calculation against the ethernet cable being pulled out and then
   * replaced, etc. */
  if ( ((InstancePtr->SequenceIdRecords.OldSyncSequenceId +
         XAVB_NUM_SYNC_FU_PAIR_CALC_RTC_INCREMENT) & 0xFFFF) ==
         InstancePtr->SequenceIdRecords.NewSyncSequenceId ) {

#ifdef DEBUG_XAVB_LEVEL2
    xil_printf("\r\nXAvb_UpdateRtcIncrement(): Debug...(a)");
    xil_printf("\r\nNewMasterTime : %x, %d" , InstancePtr->PtpRecords.NewMasterTime, InstancePtr->PtpRecords.NewMasterTime);
    xil_printf("\r\nOldMasterTime : %x, %d" , InstancePtr->PtpRecords.OldMasterTime, InstancePtr->PtpRecords.OldMasterTime);
    xil_printf("\r\nNewSlaveTime : %x, %d"  , InstancePtr->PtpRecords.NewSlaveTime,  InstancePtr->PtpRecords.NewSlaveTime);
    xil_printf("\r\nOldSlaveTime : %x, %d\r\n"  , InstancePtr->PtpRecords.OldSlaveTime,  InstancePtr->PtpRecords.OldSlaveTime);
#endif

    /** Measure the time duration, as measured by the RTC master of the
     * M sync delay measurment period. */
    MasterTimeDuration = (InstancePtr->PtpRecords.NewMasterTime
                         - InstancePtr->PtpRecords.OldMasterTime);

    if (MasterTimeDuration >= XAVB_ONE_SECOND) {
      MasterTimeDuration = MasterTimeDuration + XAVB_ONE_SECOND;
    }

    /** Measure the time duration, as measured by the RTC slave of the
     * M sync delay measurment period. */
    SlaveTimeDuration  = (InstancePtr->PtpRecords.NewSlaveTime
                         - InstancePtr->PtpRecords.OldSlaveTime);

    if (SlaveTimeDuration >= XAVB_ONE_SECOND) {
      SlaveTimeDuration = SlaveTimeDuration + XAVB_ONE_SECOND;
    }

    /** Therefore calculate the slave error (in ns) */
    SlaveError = MasterTimeDuration - SlaveTimeDuration;

    /** If the slave error is zero, skip the remainder of function.
     * (Note : a zero error would otherwise get stuck in the while loop
     *         further down this function). */
    if (SlaveError != 0) {


       /** Analyse msb of error signal to see which clock is running fastest */
       if (SlaveError & 0x80000000) {
         SlaveIsFast = 1;
         SlaveError   = SlaveTimeDuration - MasterTimeDuration;
       } else {
         SlaveIsFast = 0;
       }

       /** This check is in addition to the checks described in IEEE802.1as.
        *  If the SlaveError is unexpectedly large, then set asCapable to 0.
        */
       if (SlaveError < XAVB_CLOCK_LOCK_THRESHOLD) {
         XAvb_ChangePTPLockStatus(InstancePtr, 1);
       } else {
         XAvb_ChangePTPLockStatus(InstancePtr, 0);
       }

       /** SlaveError signal is 32-bits (ns).  This can indicate > 4 sec of
        * error: this is too large for 100 ms measurement period.  So we
        * expect upper bits to be zero.
        *
        * This function will shift the 1st none zero bit of SlaveError up
        * to bit 31, so that forthcoming calculation uses maximum accuracy.
        *
        * This shift is equivalent to a multiply (of the error signal). A
        * shift the opposite way (equivalent to a divide) will follow at
        * end of full calculation. */

       while ( !(SlaveError & (0x1 << LoopCount)) ) {
         LoopCount = LoopCount - 1;
       }
       LoopCount = 31 - LoopCount;
       ScaledError  = (SlaveError << LoopCount);


       /** Calculate the relative error: can be thought of as a scaled ratio
        * of error per time unit */
       NormalisedError = ScaledError / MasterTimeDuration;


       /** Obtain the current increment value */
       OldIncrement = (XAvb_ReadReg(InstancePtr->Config.BaseAddress,
                                     XAVB_RTC_INCREMENT_OFFSET)
                       & XAVB_RTC_INCREMENT_VALUE_MASK);


       /** Calculate the increment adjustment: multiply NormalisedError by
        * the increment time unit.  Then shift back the other way to
        * correct the calculation (restore to ns). */
       IncrementAdjust = (NormalisedError * OldIncrement) >> LoopCount;


       /** Now calculate the new increment value */
       if (SlaveIsFast) {
         NewIncrement = OldIncrement - IncrementAdjust;
       } else {
         NewIncrement = OldIncrement + IncrementAdjust;
       }

       /** Add some rails so that recovery is possible after a
        *  string of bad pDelay values.  The RTC should be able to lock
        *  to within 100ppm of the slowest allowable clock (25 MHz).
        *  This equates to +/-4ps.  Let's arbitrarily set the rails to
        *  400ppm (+/-16ps) just in case someone decides to use a
        *  particularly bad oscillator.  The lowest 20 bits of
        *  NewIncrement are fractions of a nanosecond, which equates
        *  to +/- 0x04189
        */
       if( NewIncrement > (XAVB_RTC_INCREMENT_NOMINAL_RATE + XAVB_RTC_400PPM_OFFSET) ) {
         xil_printf("\r\nRTC Exceeded 400ppm offset: Railing to 400ppm\r\n");
         NewIncrement = XAVB_RTC_INCREMENT_NOMINAL_RATE + XAVB_RTC_400PPM_OFFSET;
       }
       if( NewIncrement < (XAVB_RTC_INCREMENT_NOMINAL_RATE - XAVB_RTC_400PPM_OFFSET) ) {
         xil_printf("\r\nRTC Exceeded 400ppm offset: Railing to 400ppm\r\n");
         NewIncrement = XAVB_RTC_INCREMENT_NOMINAL_RATE - XAVB_RTC_400PPM_OFFSET;
       }

       /** And write the new increment value! */
       XAvb_WriteReg(InstancePtr->Config.BaseAddress,
                      XAVB_RTC_INCREMENT_OFFSET,
                      NewIncrement);

#ifdef DEBUG_XAVB_LEVEL2
    xil_printf("\r\nXAvb_UpdateRtcIncrement(): Debug...");
    xil_printf("\r\nM Time : %x" , MasterTimeDuration);
    xil_printf("\r\nS Time : %x" , SlaveTimeDuration);
    xil_printf("\r\nErr : %x %x" , SlaveIsFast, SlaveError);
    xil_printf("\r\nScaled : %x" , ScaledError);
    xil_printf("\r\nNorm   : %x" , NormalisedError);
    xil_printf("\r\nAdjust : %x" , IncrementAdjust);
    xil_printf("\r\nNew Inc: %x" , NewIncrement);
#endif

    }
  } else {

    xil_printf("\r\nXAvb_UpdateRtcIncrement()");
    xil_printf("\r\nERROR: Syncs not %d apart - %d\r\n",
               XAVB_NUM_SYNC_FU_PAIR_CALC_RTC_INCREMENT,
               InstancePtr->SequenceIdRecords.NewSyncSequenceId -
               InstancePtr->SequenceIdRecords.OldSyncSequenceId);
  }

  if (SlaveError > 0x2700) {
    xil_printf("\r\nXAvb_UpdateRtcIncrement(): Large Error over 100ms");
    xil_printf("\r\nM Time : %x" , MasterTimeDuration);
    xil_printf("\r\nS Time : %x" , SlaveTimeDuration);
    xil_printf("\r\nErr : %x %x" , SlaveIsFast, SlaveError);
    xil_printf("\r\nScaled : %x" , ScaledError);
    xil_printf("\r\nNorm   : %x" , NormalisedError);
    xil_printf("\r\nAdjust : %x" , IncrementAdjust);
    xil_printf("\r\nNew Inc: %x" , NewIncrement);
}

}
