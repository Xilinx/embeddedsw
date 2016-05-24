/******************************************************************************
*
* Copyright (C) 2011 - 2016 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************/
/**
*
* @file xemacps_ieee1588.c
*
* This file implements the following functionalities.
* - Contains a routine upon reception of Tx done ISR to store the time stamp of
*   the transmitted packet.
* - Does formatting and initiates a Tx for Announce frame, Sync frame, FollowUp
*   frame, PDelayReq, PDelayResp and PDelayRespFollowUp frames.
* - Decodes and processes the received PTP frames of type Sync Frame, FollowUp
*   frame, Announce Frame, PDelayReq frame, PDelayResp frame and
*   PDelayRespFollowUp frame.
* - Implements the best master clock algorithm.
* - Contains function that calculates the link delay from existing data.
* - Contains function that calculates the clock offset from existing data and
*   applies the clock offset to correct the PTP clock.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a asa  09/16/11 First release based on the AVB driver.
* 1.01a asa  03/03/12 Support for Zynq is added.
* 3.3   asa  05/19/16 Removed code specific to PEEP.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"
#include "xil_assert.h"
#include "xparameters.h"
#include "stdio.h"
#include "sleep.h"
#include "xparameters.h"
#include "xparameters_ps.h"	/* defines XPAR values */
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xpseudo_asm.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xscugic.h"
#include "xemacps.h"		/* defines XEmacPs API */
#include "xemacps_ieee1588.h"
#include "xil_mmu.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/*****************************************************************************/
/**
*
* This function is invoked from the TxDone callback. For sync frame it obtains
* the time stamp and populates the FollowUp frame Tx buffer. For PDelayReq and
* PDelayResp frames it obtains the time stamp and stores it is appropriate
* buffers.
*
* @param	InstancePntr is a pointer to the instance of the
*		XEmacPs_Ieee1588.
* @param	PacketBuf which contains the buffer just transmitted
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XEmacPs_PtpTxDoFurtherProcessing (XEmacPs_Ieee1588 *InstancePtr,
							u8 *PacketBuf)
{
	u8 MessageType;
	u32 Sec;
	u32 NanoSec;
	u8 TimeStampTemp[10];
	u32 *TempLongPntr;
	u16 SeqId;
	u16 *TempPntr;

	MessageType = XEmacPs_GetMsgType (PacketBuf);
	if (MessageType == XEMACPS_PTP_TYPE_SYNC) {
		SyncSent = TRUE;
		/*
		 * Read the seconds and nanoseconds register values
		 */
		Sec = XEmacPs_ReadReg (
		InstancePtr->EmacPsInstance->Config.BaseAddress,
		XEMACPS_PTP_TXSEC_OFFSET);

		NanoSec = XEmacPs_ReadReg (
		InstancePtr->EmacPsInstance->Config.BaseAddress,
		XEMACPS_PTP_TXNANOSEC_OFFSET);

		/*
		 * Now store the timestamps in the follow-up msg
		 */
		TimeStampTemp[0] = 0;
		TimeStampTemp[1] = 0;
		TempLongPntr = (u32 *)&(TimeStampTemp[2]);
		*TempLongPntr = Xil_Htonl (Sec);
		TempLongPntr = (u32 *)&(TimeStampTemp[6]);
		*TempLongPntr = Xil_Htonl (NanoSec);
		memcpy ((u8 *)&(InstancePtr->
		FollowUpFrmToTx[XEMACPS_PRECISE_TS_OFFSET]),
						TimeStampTemp, 10);
		/*
		 * Update the sequence id of the followup frame with
		 * the sequence id extracted from the sync frame.
		 */
		SeqId = XEmacPs_GetSequenceId (InstancePtr->SyncFrmToTx);
		TempPntr = (u16 *)&(InstancePtr->
			FollowUpFrmToTx[XEMACPS_SEQID_OFFSET]);
		*TempPntr = Xil_Htons (SeqId);

		InstancePtr->PtpRecords.Nanosec = NanoSec;
	}

	else if (MessageType == XEMACPS_PTP_TYPE_PDELAYREQ) {
		/*
		 * Read the nanoseconds register value
		 */
		NanoSec = XEmacPs_ReadReg (
		InstancePtr->EmacPsInstance->Config.BaseAddress,
		XEMACPS_PTPP_TXNANOSEC_OFFSET);
		InstancePtr->PtpRecords.PDelayTimestampT1 = NanoSec;
		InstancePtr->SequenceIdRecords.PDelayReqSequenceId =
		XEmacPs_GetSequenceId (InstancePtr->PDelayReqFrmToTx);
	}

	else if (MessageType == XEMACPS_PTP_TYPE_PDELAYRESP) {
		PDelayRespSent = TRUE;
		/*
		 * Read the seconds and nanoseconds register values
		 */
		InstancePtr->PtpRecords.PDelayRespTxedTSNs = XEmacPs_ReadReg (
			InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_PTPP_TXNANOSEC_OFFSET);
		InstancePtr->PtpRecords.PDelayRespTxedTSSec =
			XEmacPs_ReadReg (
			InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_PTPP_TXSEC_OFFSET);
	}

}

/****************************************************************************/
/**
*
* A function to format and then initiate the Tx of a PTP Announce Packet. The
* sequence Id of the announce frame is incremented before initiating the Tx.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_MasterSendAnnounce(XEmacPs_Ieee1588 *InstancePtr)
{
	u16 SeqId = 0;

	/*
	 * Increment the sequenceId
	 */
	SeqId = XEmacPs_IncSequenceId(InstancePtr->AnnounceFrmToTx);

#ifdef DEBUG_LEVEL_TWO
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\n---XEmacPs_MasterSendAnnounce()----");
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\nsequenceId is %x", SeqId);
#endif
	/* Send the Announce Frame! */
	XEmacPs_PtpTxPacket (InstancePtr, InstancePtr->AnnounceFrmToTx,
					XEMACPS_ANNOUNCEMSG_TOT_LEN);

}

/****************************************************************************/
/**
*
* A function to format and then initiate the Tx of a PTP SYNC Packet. The
* sequence Id of the announce frame is incremented before initiating the Tx.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_MasterSendSync(XEmacPs_Ieee1588 *InstancePtr)
{
	u16 SeqId = 0;

	SyncSent = FALSE;
	/*
	 * Increment the sequenceId
	 */
	SeqId = XEmacPs_IncSequenceId(InstancePtr->SyncFrmToTx);

	/*
	 * Send the SYNC Frame!
	 */
	PTPSendPacket |= SEND_SYNC;

#ifdef DEBUG_LEVEL_TWO
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\n-----XEmacPs_MasterSendSync()------");
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\nsequenceId is %x", SeqId);
#endif
}

/****************************************************************************/
/**
*
* A function to format and then initiate the Tx of a PTP FOLLOWUP Packet.
* Updates the correction field in the buffer.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return	None.
*
* @note		The correction field is hard coded to zero as of now.
*
*****************************************************************************/
void XEmacPs_MasterSendFollowUp(XEmacPs_Ieee1588 *InstancePtr)
{
	u32 CorrectionField = 0;
	u32 BufferWord = 0;
	u32 *TempLongPntr;

	/*
	 * Correction Field is 0 for the time being!
	 */
	CorrectionField = 0;

	BufferWord = Xil_Htonl (CorrectionField);

	TempLongPntr = (u32 *)&(InstancePtr->
				FollowUpFrmToTx[XEMACPS_CORRFIELD_OFFSET]);
	*TempLongPntr = BufferWord;

	XEmacPs_PtpTxPacket (InstancePtr, InstancePtr->FollowUpFrmToTx,
					XEMACPS_FOLLOWUPMSG_TOT_LEN);

#ifdef DEBUG_LEVEL_TWO
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\n---XEmacPs_MasterSendFollowUp()----");
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\nRTC nano seconds field is %x",
					InstancePtr->PtpRecords.Nanosec);
	xil_printf("\r\nCorrection Field %x", CorrectionField);
#endif
}

/****************************************************************************/
/**
*
* A function to format and then initiate the Tx of a PTP PDelayReq Packet.
* The sequence Id of the announce frame is incremented before initiating the
* Tx.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_SendPDelayReq(XEmacPs_Ieee1588 *InstancePtr)
{
	u32 SequenceId;

	/*
	 * Increment the SequenceId
	 */
	SequenceId = XEmacPs_IncSequenceId(InstancePtr->PDelayReqFrmToTx);
	InstancePtr->SequenceIdRecords.PDelayReqSequenceId = SequenceId;

#ifdef DEBUG_LEVEL_TWO
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\n------XEmacPs_SendPDelayReq()------");
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\nsequenceId is %x", SequenceId);
#endif
	Xil_DCacheFlushRange((u32)&(InstancePtr->PDelayReqFrmToTx[0]),
	XEMACPS_PDELAYREQMSG_TOT_LEN);
	/*
	 * Update the corresponding bit in PTPSendPacket so that a Tx
	 * can be initiated in function XEmacPs_RunIEEE1588Protocol.
	 */
	PTPSendPacket |= SEND_PDELAY_REQ;

}

/****************************************************************************/
/**
*
* A function to format and then initiate the Tx of a PTP PDelayResp Packet.
* This function is invoked upon receiving a PDelayReq frame. It first gets
* the time stamps of the received PDelayReq from the hardware and stores
* them at appropriate entries in the structure instance PtpRecords. It then
* formats the PDelayResp frame with these time stamp values.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_SendPDelayResp(XEmacPs_Ieee1588 *InstancePtr)
{
	u16 SequenceId     = 0;
	u32 TimestampT2Ns  = 0;
	u32 TimestampT2Sec = 0;
	u8 TimeStampTemp[10];
	u32 *TempLongPntr;
	u16 *TempPntr;
	XEmacPs_PortIdentity TempPortIdentity;

	PDelayRespSent = FALSE;
	/*
	 * Get the time stamp for the received PDelayReq frame and
	 * store them.
	 */
	InstancePtr->PtpRecords.PDelayReqRecdTSSec = XEmacPs_ReadReg (
			InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_PTPP_RXSEC_OFFSET);
	InstancePtr->PtpRecords.PDelayReqRecdTSNs = XEmacPs_ReadReg (
			InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_PTPP_RXNANOSEC_OFFSET);
	TimestampT2Sec = InstancePtr->PtpRecords.PDelayReqRecdTSSec;
	TimestampT2Ns = InstancePtr->PtpRecords.PDelayReqRecdTSNs;
	/*
	 * Populate the PDelayResp buffer with PDelayReq time stamp.
	 */
	TimeStampTemp[0] = 0;
	TimeStampTemp[1] = 0;
	TempLongPntr = (u32 *)&(TimeStampTemp[2]);
	*TempLongPntr = Xil_Htonl (TimestampT2Sec);
	TempLongPntr = (u32 *)&(TimeStampTemp[6]);
	*TempLongPntr = Xil_Htonl (TimestampT2Ns);
	memcpy ((u8 *)&(InstancePtr->
	PDelayRespFrmToTx[XEMACPS_PRECISE_TS_OFFSET]), TimeStampTemp, 10);
	/*
	 * Copy the sequence id from Pdelay request frame
	 */
	SequenceId=XEmacPs_GetSequenceId (InstancePtr->LastRecdPDelayReqFrm);
	TempPntr = (u16 *)&(InstancePtr->
				PDelayRespFrmToTx[XEMACPS_SEQID_OFFSET]);
	*TempPntr = Xil_Htons (SequenceId);

	/*
	 * Copy the source port identity
	 */
	XEmacPs_GetPortIdentity (InstancePtr->LastRecdPDelayReqFrm,
						&TempPortIdentity);
	memcpy ((u8 *)&(InstancePtr->
			PDelayRespFrmToTx[XEMACPS_REQPORTID_OFFSET]),
			(u8 *)&(TempPortIdentity.ClockIdentity[0]), 8);
	TempPntr = (u16 *)&(InstancePtr->
			PDelayRespFrmToTx[XEMACPS_REQPORTID_OFFSET + 8]);
	*TempPntr = Xil_Htons (TempPortIdentity.PortNumber);

	/*
	 * Update the corresponding bit in PTPSendPacket so that a Tx
	 * can be initiated in function XEmacPs_RunIEEE1588Protocol.
	 */
	PTPSendPacket |= SEND_PDELAY_RESP;

#ifdef DEBUG_LEVEL_TWO
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\n------XEmacPs_SendPDelayResp()-----");
	xil_printf("\r\n------------------------------------");
	xil_printf("\r\nTimestampT2 is %x", TimestampT2Ns);
#endif
}

/****************************************************************************/
/**
*
* A function to format and then initiate the Tx of a PTP PDelayRespFollowUp
* Packet. This function is invoked after a PDelayResp is successfully sent
* out (Tx done interrupt is received for PDelayResp packet).
* It populates the PDelayRespFollowUp frame with time stamps of the just
* transmitted PDelayResp packet.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_SendPDelayRespFollowUp(XEmacPs_Ieee1588 *InstancePtr)
{
	u16 SequenceId     = 0;
	u32 TimestampT3Ns  = 0;
	u32 TimestampT3Sec = 0;
	u8 TimeStampTemp[10];
	u32 *TempLongPntr;
	u16 *TempPntr;
	XEmacPs_PortIdentity TempPortIdentity;

	TimestampT3Sec = InstancePtr->PtpRecords.PDelayRespTxedTSSec;
	TimestampT3Ns = InstancePtr->PtpRecords.PDelayRespTxedTSNs;
	/*
	 * Populate the PDelayRespFollowUp buffer with PDelayResp time stamp.
	 */
	TimeStampTemp[0] = 0;
	TimeStampTemp[1] = 0;
	TempLongPntr = (u32 *)&(TimeStampTemp[2]);
	*TempLongPntr = Xil_Htonl (TimestampT3Sec);
	TempLongPntr = (u32 *)&(TimeStampTemp[6]);
	*TempLongPntr = Xil_Htonl (TimestampT3Ns);
	memcpy ((u8 *)&(InstancePtr->
			PDelayRespFollowUpFrmToTx[XEMACPS_PRECISE_TS_OFFSET]),
			TimeStampTemp, 10);

	/*
	 * Copy the sequence id from last received Pdelay request frame
	 */
	SequenceId=XEmacPs_GetSequenceId (InstancePtr->LastRecdPDelayReqFrm);
	TempPntr = (u16 *)&(InstancePtr->
			PDelayRespFollowUpFrmToTx[XEMACPS_SEQID_OFFSET]);
	*TempPntr = Xil_Htons (SequenceId);

	/*
	 * Copy the source port identity from last received PDelayReq frame.
	 */
	XEmacPs_GetPortIdentity (InstancePtr->LastRecdPDelayReqFrm,
						&TempPortIdentity);
	memcpy ((u8 *)&(InstancePtr->
			PDelayRespFollowUpFrmToTx[XEMACPS_REQPORTID_OFFSET]),
	(u8 *)&(TempPortIdentity.ClockIdentity[0]), 8);
	TempPntr = (u16 *)&(InstancePtr->
		PDelayRespFollowUpFrmToTx[XEMACPS_REQPORTID_OFFSET + 8]);
	*TempPntr = Xil_Htons (TempPortIdentity.PortNumber);

	/*
	 * Tx the packet.
	 */
	XEmacPs_PtpTxPacket(InstancePtr,
				InstancePtr->PDelayRespFollowUpFrmToTx,
				XEMACPS_PDELAYRESPFOLLOWUP_TOT_LEN);
#ifdef DEBUG_LEVEL_TWO
	xil_printf("\r\n---------------------------------------");
	xil_printf("\r\n--XEmacPs_SendPDelayRespFollowUp()----");
	xil_printf("\r\n---------------------------------------");
#endif
}

/****************************************************************************/
/**
*
* A function to decode a received PTP Sync Packet. It extracts the sync frame
* time stamp and stores it in appropriate buffer. It stores the sequence ID
* as well.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	PacketBuf is the buffer that contains the sync packet.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_DecodeRxSync(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf)
{
	XEmacPs_PortIdentity syncPortID;

	/*
	 * Read sourcePortIdentity from packet
	 */
	XEmacPs_GetPortIdentity(PacketBuf, &syncPortID);

	/*
	 * Only decode if configured for a slave and if SourcePortID is that
	 * of the RTC Clock Master
	 */
	if ((InstancePtr->CurrentBmc.IAmTheRtcMaster == 0) &&
		XEmacPs_ComparePortIdentity(
			InstancePtr->CurrentBmc.SourcePortIdentity,
			syncPortID)) {

		/*
		 * Reset Sync Interval Counter as we have received
		 * a valid Sync
		 */
		InstancePtr->PtpCounters.CounterSyncInterval = 0;

		/*
		 * Capture the local Timestamp for receipt of this frame
		 */

		InstancePtr->PtpRecords.SlaveSyncTimestampSec
		= XEmacPs_ReadReg(
			InstancePtr->EmacPsInstance->Config.BaseAddress,
						XEMACPS_PTP_RXSEC_OFFSET );
		InstancePtr->PtpRecords.SlaveSyncTimestampNSec
		= XEmacPs_ReadReg(
			InstancePtr->EmacPsInstance->Config.BaseAddress,
					XEMACPS_PTP_RXNANOSEC_OFFSET
				);
		InstancePtr->SequenceIdRecords.SyncSequenceId =
				XEmacPs_GetSequenceId (PacketBuf);
		InstancePtr->LatestMDSyncReceive.LogMessageInterval = 0;
		InstancePtr->LatestMDSyncReceive.SyncIntervalDuration = 2;

	} else {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("\r\nXEmacPs_DecodeRxSync()");
		xil_printf("\r\nSync ignored due to unmatched SourcePortID");
#endif
	}
}

/****************************************************************************/
/**
*
* A function to decode a received PTP FollowUp Packet. If the PTP node is
* master, source port identity of the received FollowUp frame matches that
* with this node's source port identity, the sequence Id of the received
* folowup frame matches that of last received Sync frame sequence Id, the
* function XEmacPs_CalcRtcOffset is invoked to calculate clock offset.
* Similalry for every 2 sync frames clock rate adjustment is done by
* calling XEmacPs_UpdateRtcIncrement. However, as of now, this function
* is not implemented and is empty.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	PacketBuf is the buffer that contains the followup packet.
*
* @return	None.
*
* @note		The clock rate adjustment is not implemented as of now.
*		Though the function XEmacPs_UpdateRtcIncrement is called
*		from here, the function does nothing!
*
*****************************************************************************/
void XEmacPs_DecodeRxFollowUp(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf)
{
	XEmacPs_PortIdentity  followUpPortID;

	/*
	 * Read sourcePortIdentity from packet
	 */
	XEmacPs_GetPortIdentity(PacketBuf, &followUpPortID);
	/*
	 * Only decode if configured for a slave and if SA is that of the RTC
	 * Clock Master
	 */
	if ( (InstancePtr->CurrentBmc.IAmTheRtcMaster == 0) &&
		XEmacPs_ComparePortIdentity(
			InstancePtr->CurrentBmc.SourcePortIdentity,
			followUpPortID) ) {

		/*
		 * Capture the Follow Up SequenceID
		 */
		InstancePtr->SequenceIdRecords.FollowUpSequenceId =
					XEmacPs_GetSequenceId (PacketBuf);

		/*
		 * SequenceID in Follow Up Frame should always match that of
		 * the Sync Frame
		 */
		if (InstancePtr->SequenceIdRecords.FollowUpSequenceId
			== InstancePtr->SequenceIdRecords.SyncSequenceId) {
			/*
			 * Capture the correction field from follow up
			 * frame
			 */

			InstancePtr->PtpRecords.MasterCorrectionField =
			*((u32 *)(PacketBuf + XEMACPS_CORRFIELD_OFFSET));

			/*
			 * Perform the Course RTC Offset correction for every
			 * Sync FollowUp pair
			 */
			XEmacPs_CalcRtcOffset(InstancePtr);

			/*
			 * Every n Sync / FollowUp pairs, we are going to
			 * calculate a corrected increment rate of RTC
			 */
			if ((InstancePtr->PtpCounters.CounterSyncEvents & 0xF)
				== (XEMACPS_NUM_SYNC_FU_PAIR_CALC_RTC_INCREMENT
									- 1)) {

				/*
				 * Reset the CounterSyncEvents Counter
				 */
				InstancePtr->PtpCounters.CounterSyncEvents =
									0x0;

				/*
				 * Capture the Sequence ID of the Follow Up
				 * frame.
				 */
				InstancePtr->
				SequenceIdRecords.NewSyncSequenceId
				= InstancePtr->
				SequenceIdRecords.FollowUpSequenceId;

				/*
				 * Perform the RTC increment rate adjustment
				 * calculation
				 */
				XEmacPs_UpdateRtcIncrement(InstancePtr);

				InstancePtr->PtpRecords.OldSlaveTime
				= InstancePtr->PtpRecords.NewSlaveTime;

				InstancePtr->PtpRecords.OldMasterTime
				= InstancePtr->PtpRecords.NewMasterTime;

				InstancePtr->
				SequenceIdRecords.OldSyncSequenceId
				= InstancePtr->
				SequenceIdRecords.NewSyncSequenceId;

			} else {
				InstancePtr->
				PtpCounters.CounterSyncEvents
				= InstancePtr->
				PtpCounters.CounterSyncEvents + 1;
			}

		} else {
#ifdef DEBUG_XEMACPS_LEVEL1
			xil_printf("SequenceIDs on RxFollowup don't match.\r\n");
			xil_printf("FollowupSeqID is : %d",
			InstancePtr->SequenceIdRecords.FollowUpSequenceId);
			xil_printf("SyncSeqID is : %d",
			InstancePtr->SequenceIdRecords.SyncSequenceId);
#endif
			;
		}
	} else {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("\r\nXEmacPs_DecodeRxFollowUp()");
		xil_printf("\r\nFollowUp ignored due to unmatched SourcePortID\r\n");
#endif
		;
	}
}

/****************************************************************************/
/**
*
* A function to decode a received PTP PDelayResp Packet.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	PacketBuf is the buffer that contains the PDelayResp packet.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_DecodeRxPDelayResp(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf)
{
	u16 *TempPntr;

	/*
	 * Have we already seen a PDelayResp since the last
	 * PDelayReq was sent? If so, ignore the packet
	 */
	if( InstancePtr->StateMachineData.RcvdPDelayResp ) {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Error: already saw a PDelayResp since the last PDelayReq was sent\r\n");
#endif
		return;
	}

	/*
	 * Find the ClockIdentity of the Sender
	 */
	XEmacPs_GetPortIdentity (PacketBuf,
			&(InstancePtr->StateMachineData.RespPortIdentity));

	/*
	 * Is the PDelayResp message from ourself?  If so, the Peer
	 * is most likely a dumb hub and should be considered not
	 * IEEE1588v2 Capable
	 */
	if (XEmacPs_ComparePortIdentity (InstancePtr->PortIdLocal,
			InstancePtr->StateMachineData.RespPortIdentity)) {
		XEmacPs_ChangePeerIeee1588v2Capability(InstancePtr, 0);
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("\r\nXEmacPs_DecodeRxPDelayResp():The peer is no longer IEEE1588v2 Capable ");
		xil_printf("\r\nXEmacPs_DecodeRxPDelayResp():Error: saw a PDelayResp from myself\r\n");
#endif
		return;
	}

	memcpy (&(InstancePtr->
	StateMachineData.RespReqPortIdentity.ClockIdentity[0]),
	(u8 *)&(PacketBuf[XEMACPS_REQPORTID_OFFSET]), 8);
	TempPntr = (u16 *)&(PacketBuf[XEMACPS_REQPORTID_OFFSET + 8]);
	InstancePtr->
	StateMachineData.RespReqPortIdentity.PortNumber =
	Xil_Htons (*TempPntr);

	/*
	 * Capture the PDelayResp SequenceID
	 */
	InstancePtr->SequenceIdRecords.PDelayRespSequenceId =
		XEmacPs_GetSequenceId (PacketBuf);

	/*
	 * Verify that the requestingPortIdentity matches our
	 * portIdentity
	 */
	if (!(XEmacPs_ComparePortIdentity (InstancePtr->PortIdLocal,
			InstancePtr->StateMachineData.RespReqPortIdentity))) {

#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Error: PDelayResp reqPortID doesn't match our portID\r\n");
#endif
		return;
	}

	/*
	 * Only process if the received frame's sequenceId matches
	 * the sequenceId sent in the last pDelay_Req packet
	 */
	if( (InstancePtr->SequenceIdRecords.PDelayReqSequenceId ==
		InstancePtr->SequenceIdRecords.PDelayRespSequenceId) ) {

		/* Mark this as a valid PDelayResp packet */
		InstancePtr->StateMachineData.RcvdPDelayResp = 1;

		InstancePtr->PtpRecords.PDelayTimestampT2 =
		Xil_Ntohl (*(u32 *)&(PacketBuf[XEMACPS_PRECISE_TS_OFFSET + 6]));
		/*
		 * Capture timestamp for receipt time of PDelayResp at Slave (t4)
		 * and adjust it for MAC receive latency
		 */
		InstancePtr->PtpRecords.PDelayTimestampT4 = XEmacPs_ReadReg (
		InstancePtr->EmacPsInstance->Config.BaseAddress,
		XEMACPS_PTPP_RXNANOSEC_OFFSET);
	} else {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Error: PDelayResp seqID's don't match\r\n");
#endif
		;
	}
}

/****************************************************************************/
/**
*
* A function to decode a received PTP PDelayRespFollowUp Packet.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	PacketBuf is the buffer that contains the PDelayRespFollowUp
*		packet.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_DecodeRxPDelayRespFollowUp(XEmacPs_Ieee1588 *InstancePtr,
							u8 *PacketBuf)
{
	XEmacPs_PortIdentity portId;
	u16 *TempPntr;

	/*
	 * Has a valid PDelayResp packet been received since the
	 * last PDelayReq packet was sent?
	 */
	if( !InstancePtr->StateMachineData.RcvdPDelayResp ) {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Error: Received a PDelayRespFollowUp before receiving a PDelayResp\r\n");
#endif
		return;
	}

	/* Capture the PDelayRespFollowUp SequenceID */
	InstancePtr->SequenceIdRecords.PDelayFollowUpSequenceId =
					XEmacPs_GetSequenceId (PacketBuf);

	/* Get the sourcePortIdentity of the sender */
	XEmacPs_GetPortIdentity (PacketBuf, &portId);

	/*
	 * The sourcePortIdentity of the PDelayRespFollowUp should
	 * match that of the last PDelayResp packet received
	 */
	if (!(XEmacPs_ComparePortIdentity (portId,
			InstancePtr->StateMachineData.RespPortIdentity))) {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Error: sourcePortIdentity of PDelayRespFollowUp doesn't match PDelayResp\r\n");
#endif
		return;
	}

	memcpy (&(portId.ClockIdentity[0]),
			(u8 *)&(PacketBuf[XEMACPS_REQPORTID_OFFSET]), 8);
	TempPntr = (u16 *)&(PacketBuf[XEMACPS_REQPORTID_OFFSET + 8]);
	portId.PortNumber = Xil_Htons (*TempPntr);

	if (!(XEmacPs_ComparePortIdentity (portId,
			InstancePtr->StateMachineData.RespReqPortIdentity))) {

#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Error: reqPortID of PDelayRespFollowUp doesn't match PDelayResp\r\n");
#endif
		return;
	}

	/*
	 * SequenceID of PDelayRespFollowUp Frame should always match that of
	 * the PDelayResp Frame and the original PDelayReq Frame.
	 */
	if (InstancePtr->SequenceIdRecords.PDelayFollowUpSequenceId ==
		InstancePtr->SequenceIdRecords.PDelayRespSequenceId) {

		/* Mark this as a valid PDelayRespFollowUp packet */
		InstancePtr->StateMachineData.RcvdPDelayRespFollowUp = 1;

		/*
		 * Capture the timestamp for transmit time of PDelayResp at Master
		 * (t3)
		 */
		InstancePtr->PtpRecords.PDelayTimestampT3 =
		Xil_Ntohl (*(u32 *)&(PacketBuf[XEMACPS_PRECISE_TS_OFFSET
								+ 6]));

		/* Now we know t1, t2, t3 and t4, calculate the link delay */
		XEmacPs_CalcDelay(InstancePtr);
	} else {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Error: seqID of PDelayRespFollowUp doesn't match PDelayResp\r\n");
#endif
		;
	}
}

/****************************************************************************/
/**
*
* A function to decode a received PTP Signalling Packet. Empty as of now.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	PacketBuf is the buffer that contains the signalling packet.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_DecodeRxSignaling(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf)
{
	/*TO DO*/
}

/****************************************************************************/
/**
* This function is invoked to compare the Current Master clock's parameter
* with that of the PTP node's (own) and run Best Master Clock Algorithm. A
* typical scenario is a new Announce frame has been received from the PTP
* Master and the PTP master record has been updated. Then this function is
* invoked to run Best Master Clock Algorithm with the PTP node's Announce
* Frame parameters and Current PTP Master's parameter.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	PacketBuf is the buffer that contains the Announce Frame.
*		For this function the buffer passed is the Tx Announce Frame
*		stored in the buffer AnnounceFrmToTx in XEmacPs_Ieee1588
*		instance.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_DecodeTxAnnounceFrame(XEmacPs_Ieee1588 *InstancePtr,
							u8 *PacketBuf)
{

	u32 NewMaster = 0;
	XEmacPs_BmcData TxAnnounceFrame;

	/*
	 * Read the attributes for the new Announce frame in the Tx PTP buffer
	 */
	XEmacPs_ReadAnnounceFrame(PacketBuf, &TxAnnounceFrame);

	/*
	 * Compare the clock attributes between then new Announce frame and the
	 * current master
	 */
	NewMaster = XEmacPs_BestMasterClockAlgorithm(&TxAnnounceFrame,
						&InstancePtr->CurrentBmc);


	if ((NewMaster == 1) | (InstancePtr->CurrentBmc.IAmTheRtcMaster == 1))
									{
		/*
		 * Update records with the NEW best master
		 */
		XEmacPs_UpdateBmcRecords(&TxAnnounceFrame,
						&InstancePtr->CurrentBmc);

#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("\r\nXEmacPs_DecodeTxAnnounceFrame()");
		xil_printf("\r\n* BMC : I am the MASTER");
		xil_printf("\r\n-----------------------");
		xil_printf("\r\nLocal Announce Frame");
		xil_printf("\r\n-----------------------");
		xil_printf("\r\nPriority1   %x",
			InstancePtr->CurrentBmc.GrandmasterPriority1);

		xil_printf("\r\nclockClass  %x",
			InstancePtr->CurrentBmc.ClockQuality.clockClass);

		xil_printf("\r\nPriority2   %x",
			InstancePtr->CurrentBmc.GrandmasterPriority2);

		/*
		 * Our new Tx Announce Packet has won - so this device must be the
		 * master
		 */
		xil_printf("\r\n*** XEmacPs_DecodeTxAnnounceFrame() : Call XEmacPs_BecomeRtcMaster() *** \r\n");
#endif
		XEmacPs_BecomeRtcMaster(InstancePtr, 1);
	}
}

/****************************************************************************/
/**
* This function is invoked from various places to extract clock information
* from a buffer and populate the XEmacPs_BmcData instance passed.
*
* @param	PacketBuf from which the clock information is extracted
* @param	AnnounceFrame which is populated with clock information
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_ReadAnnounceFrame(u8 *PacketBuf, XEmacPs_BmcData *AnnounceFrame)
{
	u32 ReadWord;

	memset(AnnounceFrame->SourcePortIdentity.ClockIdentity, 0, 8);

	/*
	 * Get the Source Port Identity of the port sending the Announce Packet
	 */
	XEmacPs_GetPortIdentity (PacketBuf, &AnnounceFrame->SourcePortIdentity);


	/*
	 * Read priority1 and top half of ClockQuality
	 */
	AnnounceFrame->GrandmasterPriority1 =
					PacketBuf[XEMACPS_GMPRI_ONE_OFFSET];

	ReadWord =
	Xil_Ntohl(*((u32 *)&(PacketBuf[XEMACPS_GM_CLK_QUALITY_OFFSET])));
	memcpy ((u8 *)&(AnnounceFrame->ClockQuality), (u8 *)&ReadWord, 4);

	AnnounceFrame->GrandmasterPriority2 =
	PacketBuf[XEMACPS_GMPRI_TWO_OFFSET];

	memcpy((u8 *)&(AnnounceFrame->GrandmasterIdentity.ClockIdentity),
		(u8 *)&PacketBuf[XEMACPS_GM_IDENTITY_OFFSET], 8);

	/* AnnounceFrame->StepsRemoved =
	Xil_Ntohs (*((u16 *)&(PacketBuf[XEMACPS_STEPS_REMOVED_OFFSET])));*/
	AnnounceFrame->StepsRemoved = 0;

}

/*****************************************************************************/
/**
* This function is invoked when a new Announce frame is received. This function
* first reads the Announce frame parameters. If it is found that the announce
* frame has been received from the PTP master then the records are updated
* with the clock parameters. The Best Master Clock Algorithm is run in case
* any of the parameters in the announce frame has changed. There can be
* cases when the PTP master reduces its clock priority that may force the
* current PTP node to become master.
* If the PTP node is master and it has received this announce frame, then
* BMCA is run and if the incoming clock parameters are better tyhan that of
* the present node, the present node becomes SLAVE.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	PacketBuf is the buffer that contains the Announce Frame.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_DecodeRxAnnounceFrame(XEmacPs_Ieee1588 *InstancePtr,
								u8 *PacketBuf)
{

	u32 NewMaster = 0;
	XEmacPs_BmcData RxAnnounceFrame;

	/*
	 * Read the attributes for the new Announce frame received
	 */
	XEmacPs_ReadAnnounceFrame(PacketBuf, &RxAnnounceFrame);
	/*
	 *  If the received packet's clockIdentity matches our
	 *  clockIdentity, ignore the packet
	 */
	if( XEmacPs_ComparePortIdentity(InstancePtr->PortIdLocal,
					RxAnnounceFrame.SourcePortIdentity) ) {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Got an announce from myself.. ignoring\r\n");
#endif
		return;
	}

	/*
	 * If the received packet's StepsRemoved field is >= 255,
	 * ignore the packet
	 */
	if( RxAnnounceFrame.StepsRemoved >= 255 ) {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("Got an announce with StepsRemoved > 255.. ignoring\r\n");
#endif
		return;
	}

	/*
	 * If the Announce packet's GMID matches that of our current GM
	 * record, then update its records based on the current packet,
	 * just in case something (such as priority) has changed.
	 */
	if( XEmacPs_CompareClockIdentity(
			RxAnnounceFrame.GrandmasterIdentity,
			InstancePtr->CurrentBmc.GrandmasterIdentity) ) {
		/*
		 * Update timeout information
		 */
		InstancePtr->PtpCounters.CounterAnnounceInterval = 0;

		XEmacPs_UpdateBmcRecords(&RxAnnounceFrame,
		&InstancePtr->CurrentBmc);
		/*
		 * Compare against this device's information to see if we
		 * should be GM
		 */
		XEmacPs_DecodeTxAnnounceFrame(InstancePtr,
		InstancePtr->AnnounceFrmToTx);

	} else if( InstancePtr->CurrentBmc.IAmTheRtcMaster ) {
		/*
		 * Run BMCA on this announce to see if it is better than me
		 */
		NewMaster =  XEmacPs_BestMasterClockAlgorithm(&RxAnnounceFrame,
						&InstancePtr->CurrentBmc);

		if (NewMaster == 1) {
			/*
			 * Update records with the NEW best master
			 */
			XEmacPs_UpdateBmcRecords(&RxAnnounceFrame,
			&InstancePtr->CurrentBmc);

			/*
			 * Capture the Announce Receipt Timeout Interval.
			 * Reset the announce receipt timeout interval to
			 * use the new value.
			 */
			RxAnnounceFrame.LogMessageInterval =
				PacketBuf[XEMACPS_LOGMSG_INTERVAL_OFFSET];
			InstancePtr->CurrentBmc.AnnounceIntervalDuration = 10;
#ifdef DEBUG_XEMACPS_LEVEL1
			xil_printf("\r\r\nXEmacPs_DecodeRxAnnounceFrame()");
			xil_printf("\r\n* XEmacPs_DecodeRxAnnounceFrame()::BMC : I am a SLAVE");
			xil_printf("\r\n-----------------------");
			xil_printf("\r\nWinning Announce Frame");
			xil_printf("\r\n-----------------------");

			xil_printf("\r\nPriority1   %x",
			InstancePtr->CurrentBmc.GrandmasterPriority1);
			xil_printf("\r\nclockClass  %x",
			InstancePtr->CurrentBmc.ClockQuality.clockClass);

			xil_printf("\r\nPriority2   %x",
			InstancePtr->CurrentBmc.GrandmasterPriority2);
#endif
			/*
			 * New Rx Announce Packet has won - so this device
			 * cannot be a master
			 */
			XEmacPs_BecomeRtcSlave(InstancePtr);
		}
	}
}

/****************************************************************************/
/**
* This function will accept the data pointer to the current BMCA records,
* accept a pointer to an equivalent data structure for the new Announce
* Packet. TheBest Master Clock Algorithm (BMCA) is then performed on these
* two data structures by comparing the data fields
*
* @param	AnnounceFrame of the received new frame
* @param	CurrentBmc is the existing BMC records
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
u32 XEmacPs_BestMasterClockAlgorithm(XEmacPs_BmcData *AnnounceFrame,
					XEmacPs_BmcData *CurrentBmc)
{

	u32 NewMaster = 0;
#ifdef DEBUG_XEMACPS_LEVEL1
	xil_printf("*** Performing BMCA ***\r\n");
#endif
	/*
	 * Priority1 takes precedence over all over priorites
	 */
	if (AnnounceFrame->GrandmasterPriority1 <
					CurrentBmc->GrandmasterPriority1) {
		/*
		 * we have found a better master!
		 */
		NewMaster = 1;
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("BMCA: Found new GM on priority1: AnnPri1 (%d) < BmcPri1 (%d)\r\n",
			AnnounceFrame->GrandmasterPriority1,
			CurrentBmc->GrandmasterPriority1);
#endif

	} else if (AnnounceFrame->GrandmasterPriority1 ==
					CurrentBmc->GrandmasterPriority1) {

		u32 AnnClockQualityInteger;
		u32 BmcClockQualityInteger;
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("BMCA: priority1 equal  moving on: (%d)\r\n",
				AnnounceFrame->GrandmasterPriority1);
#endif
		/*
		 * Convert structs to u32 values for easy comparison
		 */

		AnnClockQualityInteger = (AnnounceFrame->
		ClockQuality.clockClass << 24) |
		(AnnounceFrame->ClockQuality.clockAccuracy << 16) |
		(AnnounceFrame->ClockQuality.offsetScaledLogVariance);
		BmcClockQualityInteger = (CurrentBmc->
		ClockQuality.clockClass << 24) |
		(CurrentBmc->ClockQuality.clockAccuracy << 16) |
		(CurrentBmc->ClockQuality.offsetScaledLogVariance);

		/*
		 * ClockQuality has the next priority
		 */
		if (AnnClockQualityInteger < BmcClockQualityInteger ) {
#ifdef DEBUG_XEMACPS_LEVEL1
			xil_printf("BMCA: Found new GM on clockQuality: Ann (0x%08x) < Bmc (0x%08x)\r\n",
						AnnClockQualityInteger,
						BmcClockQualityInteger);
#endif
			/*
			 * We have found a better master!
			 */
			NewMaster = 1;

		} else if ( AnnClockQualityInteger ==
						BmcClockQualityInteger ) {
#ifdef DEBUG_XEMACPS_LEVEL1
			xil_printf("BMCA: clockQuality equal moving on: (0x%08x)\r\n",
				AnnClockQualityInteger);
#endif
			/*
			 * Priority2 provides fine grained ordering amongst otherwise equal
			 * clocks
			 */
			if (AnnounceFrame->GrandmasterPriority2 <
					CurrentBmc->GrandmasterPriority2) {
#ifdef DEBUG_XEMACPS_LEVEL1
				xil_printf("BMCA: Found new GM on priority2: AnnPri1 (%d) < BmcPri1 (%d)\r\n",
					AnnounceFrame->GrandmasterPriority2,
					CurrentBmc->GrandmasterPriority2);
#endif
				/*
				 * We have found a better master!
				 */
				NewMaster = 1;

			/*
			 * Next compare the Clock Identities
			 */
			} else if (AnnounceFrame->GrandmasterPriority2
				== CurrentBmc->GrandmasterPriority2) {

				signed int CompareResult;
#ifdef DEBUG_XEMACPS_LEVEL1
				xil_printf("BMCA: priority2 equal moving on: (%d)\r\n",
					AnnounceFrame->GrandmasterPriority2);
#endif

				CompareResult =
				memcmp (AnnounceFrame->
				GrandmasterIdentity.ClockIdentity,
				CurrentBmc->GrandmasterIdentity.ClockIdentity,
				8);
				if (CompareResult < 0) {
#ifdef DEBUG_XEMACPS_LEVEL1
					xil_printf("BMCA: Found new GM on GMIDClockID\r\n");
#endif
					/*
					 * We have found a better master!
					 */
					NewMaster = 1;

				} else if (CompareResult == 0) {
#ifdef DEBUG_XEMACPS_LEVEL1
					xil_printf("BMCA: GMIDclockID equal moving on\r\n");
#endif
					if( AnnounceFrame->StepsRemoved <
					CurrentBmc->StepsRemoved ) {
#ifdef DEBUG_XEMACPS_LEVEL1
						xil_printf("BMCA: Found new GM on StepsRemoved: Ann (%d) < Bmc (%d)\r\n",
							AnnounceFrame->
							StepsRemoved,
							CurrentBmc->
							StepsRemoved);
#endif
						/*
						 * We have found a better master!
						 */
						NewMaster = 1;

						/*
						 * Next compare SourcePortIdentity
						 */
					} else if( AnnounceFrame->StepsRemoved
						== CurrentBmc->StepsRemoved ) {
						signed int CompareResult;
#ifdef DEBUG_XEMACPS_LEVEL1
						xil_printf("BMCA: StepsRemoved equal moving on: (%d)\r\n",
						AnnounceFrame->StepsRemoved);
#endif
						CompareResult =
						memcmp (AnnounceFrame->
						SourcePortIdentity.
						ClockIdentity,
						CurrentBmc->SourcePortIdentity.
						ClockIdentity,
						8);
						if( CompareResult < 0) {
#ifdef DEBUG_XEMACPS_LEVEL1
							xil_printf("BMCA: Found new GM on sourceIDClockID\r\n");
#endif
							/*
							 * We have found a better master!
							 */
							NewMaster = 1;

						} else if( CompareResult
								== 0 ) {
#ifdef DEBUG_XEMACPS_LEVEL1
							xil_printf("BMCA: sourceIDport equal moving on\r\n");
#endif
							/*
							 * If all else fails, the SourcePortIdentity Port Number must
							 * act as the tie-breaker
							 */
							if( AnnounceFrame->
							SourcePortIdentity.
							PortNumber <
							CurrentBmc->
							SourcePortIdentity.
							PortNumber ) {
#ifdef DEBUG_XEMACPS_LEVEL1
								xil_printf("BMCA: Found new GM on sourceIDportNum: AnnPort (0x%08x) < BmcPort (0x%08x)\r\n",
								AnnounceFrame->
								SourcePortIdentity.
								PortNumber,
								CurrentBmc->
								SourcePortIdentity.
								PortNumber);
#endif
								/* A new master has won on the tie-break! */
								NewMaster = 1;
							}
						}
					}
				}
			}
		}
	}
#ifdef DEBUG_XEMACPS_LEVEL1
	xil_printf("*** END BMCA ***\r\n");
#endif
	return NewMaster;

}

/****************************************************************************/
/**
* This function updates the PTP master records (BMC records) with incoming
* BMC data.
*
* @param	NewMaster is the new data to be updated
* @param	CurrentBmc is the existing BMC records that needs to be
*		updated.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_UpdateBmcRecords(XEmacPs_BmcData *NewMaster,
						XEmacPs_BmcData *CurrentBmc)
{
	memcpy (CurrentBmc->SourcePortIdentity.ClockIdentity,
			NewMaster->SourcePortIdentity.ClockIdentity,
			8);
	CurrentBmc->SourcePortIdentity.PortNumber =
	NewMaster->SourcePortIdentity.PortNumber;

	memcpy (CurrentBmc->GrandmasterIdentity.ClockIdentity,
			NewMaster->GrandmasterIdentity.ClockIdentity,
			8);
	CurrentBmc->StepsRemoved = NewMaster->StepsRemoved;
	CurrentBmc->ClockQuality = NewMaster->ClockQuality;
	CurrentBmc->GrandmasterPriority1 = NewMaster->GrandmasterPriority1;
	CurrentBmc->GrandmasterPriority2 = NewMaster->GrandmasterPriority2;
}

/****************************************************************************/
/**
* This function is called when the PTP node becomes RTC or PTP Master. This
* will make any adjustments needed when the node becomes the Grand Master,
* including resetting the RTC to its nominal value
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	txAnnounceHasWon if 1, indicates that this function has been
*		called from the function XEmacPs_DecodeTxAnnounceFrame(). This
*		way it can avoid performing things that are already done.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_BecomeRtcMaster(XEmacPs_Ieee1588 *InstancePtr,
							u8 txAnnounceHasWon)
{

	XEmacPs_BmcData deviceData;
	unsigned int NSIncrementVal;

	if (txAnnounceHasWon == 0) {
		/*
		 * Update the BMCA records to this device's information
		 */

		/*
		 * Read the attributes in the Tx PTP buffer
		 */
		XEmacPs_ReadAnnounceFrame(InstancePtr->AnnounceFrmToTx,
								&deviceData);

		/*
		 * Update records
		 */
		XEmacPs_UpdateBmcRecords(&deviceData,&InstancePtr->CurrentBmc);
	}

	NSIncrementVal = XEmacPs_TsuCalcClk(XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 6);
	/*
	 * Set the 1588 Timer register for 50 MHz timer, i.e. 20 ns increment
	 * for every clock cycle.
	 */
	XEmacPs_WriteReg(InstancePtr->EmacPsInstance->Config.BaseAddress,
				XEMACPS_1588_INC_OFFSET,
				NSIncrementVal);
	/*
	 * Set timestamp uncertainty if new status
	 */
	if( !InstancePtr->CurrentBmc.IAmTheRtcMaster ) {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("\r\n*** I am now the Grand Master ***");
		xil_printf("\r\nNOTICE: timestamps are now certain\r\n");
#endif
		;
	}

	/*
	 * Inform the rest of the system
	 */
	InstancePtr->CurrentBmc.IAmTheRtcMaster = 1;
}

/****************************************************************************/
/**
* This function is called when the PTP node becomes PTP Slave.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_BecomeRtcSlave(XEmacPs_Ieee1588 *InstancePtr)
{
	/*
	 * Set timestamp uncertainty if new status
	 */
	if( InstancePtr->CurrentBmc.IAmTheRtcMaster ) {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("\r\n*** I am now a PTP slave ***");
		xil_printf("\r\nNOTICE: timestamps are now uncertain\r\n");
#endif
		;
	}

	/*
	 * Reset the syncReceiptTimeoutTimeInterval counter as this has now
	 * changed purpose.
	 */
	InstancePtr->PtpCounters.CounterSyncInterval = 0;

	/*
	 * Inform the rest of the system
	 */
	InstancePtr->CurrentBmc.IAmTheRtcMaster = 0;
}

/****************************************************************************/
/**
* This function is called to change the Peer capability of processing
* Iee1588v2 specific frames, e.g. PDelayReq, PDelayResp etc. A peer is
* Ieee1588v2 capable when it is able to send PDelayReq frames or able to
* respond to PDelayReq frames.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
* @param	capable is Peer's capability
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_ChangePeerIeee1588v2Capability(XEmacPs_Ieee1588 *InstancePtr,
								u8 Capable)
{
	u32 CapableOld;

	CapableOld = InstancePtr->PeerIeee1588v2Capable;

	/* set status variable */
	InstancePtr->PeerIeee1588v2Capable = Capable;

	if( Capable != CapableOld ) {
		if( Capable ) {
#ifdef DEBUG_XEMACPS_LEVEL1
			xil_printf("\r\nThe Peer is now IEEE1588 v2 Capable\r\n");
#endif
			;
		} else {
#ifdef DEBUG_XEMACPS_LEVEL1
			xil_printf("\r\nThe Peer is no longer IEEE1588 v2 Capable\r\n");
#endif
			;
		}
	}

}

/****************************************************************************/
/**
* This function is called to calculate the link delay. This is called after
* a complete sequence of PDelay packets. The PTP node sends a PDelayReq
* packet to start. Afterwards it receives the PDelayResp and
* PDelayRespFollowUp frames. Upon receiving the PDelayRespFollowUp, this
* function is invoked to calculate the link delay.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_CalcDelay(XEmacPs_Ieee1588 *InstancePtr)
{
	u32 T4MinusT1 = 0;
	u32 T3MinusT2 = 0;
	u32 Delay     = 0;
	/*
	 * Since we are only using the nanoseconds field here we need to
	 * account for wrap.  So we add one second to the T4 and T3 times to
	 * ensure that the T4MinusT1 and T3MinusT2 results cannot be negative.
	 * These two additional seconds then cancel each other out in the
	 * T4MinusT1 - T3MinusT2 equation.
	 */
#ifdef DEBUG_LEVEL_TWO
	xil_printf("\r\nXEmacPs_CalcDelay()");
	xil_printf("\r\nt1        %x ",
			InstancePtr->PtpRecords.PDelayTimestampT1);
	xil_printf("\r\nt2        %x ",
			InstancePtr->PtpRecords.PDelayTimestampT2);
	xil_printf("\r\nt3        %x ",
			InstancePtr->PtpRecords.PDelayTimestampT3);
	xil_printf("\r\nt4        %x ",
			InstancePtr->PtpRecords.PDelayTimestampT4);
#endif

	/*
	 * If the nanoseconds count has wrapped, add on 1 second to ensure
	 * we get the right answer
	 */
	if (InstancePtr->PtpRecords.PDelayTimestampT4 <
				InstancePtr->PtpRecords.PDelayTimestampT1) {
		T4MinusT1 = (InstancePtr->PtpRecords.PDelayTimestampT4
				+ XEMACPS_ONE_SECOND)
				- InstancePtr->PtpRecords.PDelayTimestampT1;
	} else {
		T4MinusT1 = InstancePtr->PtpRecords.PDelayTimestampT4
			- InstancePtr->PtpRecords.PDelayTimestampT1;
	}
	/*
	 * If the nanoseconds count has wrapped, add on 1 second to ensure
	 * we get the right answer
	 */
	if (InstancePtr->PtpRecords.PDelayTimestampT3 <
				InstancePtr->PtpRecords.PDelayTimestampT2) {
		T3MinusT2 = (InstancePtr->PtpRecords.PDelayTimestampT3
			+ XEMACPS_ONE_SECOND)
			- InstancePtr->PtpRecords.PDelayTimestampT2;
	} else {
		T3MinusT2 = InstancePtr->PtpRecords.PDelayTimestampT3
			- InstancePtr->PtpRecords.PDelayTimestampT2;
	}

	Delay = (T4MinusT1 - T3MinusT2) >> 1;

	/*
	 * For now we are simply going to throw out any absurdly large
	 * link delays.
	 */
	if (Delay < XEMACPS_NEIGHBOR_PROP_DELAY_THRESH ) {
		InstancePtr->PtpRecords.LinkDelay  = Delay;
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("THE FINAL LINK DELAY IS %x \r\n",
		InstancePtr->PtpRecords.LinkDelay);
#endif
		/*
		 * The peer has responded to the pDelay_Req and the measured
		 * delay is within tolerance: the peer is deemed to be
		 * Ieee1588v2 capable
		 */
		XEmacPs_ChangePeerIeee1588v2Capability(InstancePtr, 1);

	} else {
#ifdef DEBUG_XEMACPS_LEVEL1
		xil_printf("\r\nXEmacPs_CalcDelay()");
		xil_printf("\r\n Bad Link Delay %d ", Delay);
		xil_printf("\r\nt1        %x ",
				InstancePtr->PtpRecords.PDelayTimestampT1);
		xil_printf("\r\nt2        %x ",
				InstancePtr->PtpRecords.PDelayTimestampT2);
		xil_printf("\r\nt3        %x ",
				InstancePtr->PtpRecords.PDelayTimestampT3);
		xil_printf("\r\nt4        %x ",
				InstancePtr->PtpRecords.PDelayTimestampT4);
		xil_printf("\r\nLinkDelay %x ",
				InstancePtr->PtpRecords.LinkDelay);
#endif
		;

	}
}

/****************************************************************************/
/**
* This function calculates the Slave Offset from the GrandMaster time. It is
* called after receiving a Sync and FollowUp frame pair.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_CalcRtcOffset (XEmacPs_Ieee1588 *InstancePtr)
{
	u32 MasterNanosec = 0;
	u32 MasterSeconds = 0;
	u32 SlaveSecs = 0;

	u32 SyncRouteDelay = 0;
	u32 MasterNsCorrected = 0;
	u32 MasterNsHasWrapped = 0;

	XEmacPs_RtcFormat RtcError;
	u32 MCorrection;
	u32 ToSubtract = 0;
	u32 NSecsRemaining = 0;

#ifdef DEBUG_XEMACPS_LEVEL2
	xil_printf("*** In  XEmacPs_CalcRtcOffset***\r\n");
#endif
	/*
	 * Capture the Master Origin Timestamp (from received FollowUp Frame).
	 */
	MasterNanosec = Xil_Ntohl (*((u32 *)&(InstancePtr->
	LastRecdFollowUpFrm[XEMACPS_PRECISE_TS_OFFSET + 6])));
	MasterSeconds = Xil_Ntohl (*((u32 *)&(InstancePtr->
	LastRecdFollowUpFrm[XEMACPS_PRECISE_TS_OFFSET + 2])));

	/*
	 * Correct the Nanoseconds
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
	MCorrection = (u32) (InstancePtr->PtpRecords.MasterCorrectionField);
	SyncRouteDelay = MCorrection + InstancePtr->PtpRecords.LinkDelay;

	/*
	 * MasterNsCorrected time here is the calculated time that the
	 * master will be at the point in time when the sync frame is received
	 * (and timestamped) at the slave.  This is calculated from the
	 * originTimeStamp (from the FollowUpframe), plus the Sync Frame
	 * routing delay.  A direct comparison can then be made between master
	 * and slave.
	 */
	MasterNsCorrected = MasterNanosec + SyncRouteDelay;

	/* Check for ns wrap-around condition */
	if (MasterNsCorrected >= XEMACPS_ONE_SECOND) {
		MasterNsCorrected   = MasterNsCorrected - XEMACPS_ONE_SECOND;
		MasterNsHasWrapped  = 1;
	}

	/* Make the Master and Slave comparison and discover the difference! */
	if (MasterNsCorrected > InstancePtr->PtpRecords.SlaveSyncTimestampNSec)
									{
		RtcError.NanoSeconds = MasterNsCorrected
			- InstancePtr->PtpRecords.SlaveSyncTimestampNSec;
		ToSubtract = 0;
	} else {
		RtcError.NanoSeconds =
				InstancePtr->PtpRecords.SlaveSyncTimestampNSec
				- MasterNsCorrected;
		ToSubtract = 0x80000000;
	}

	/*
	 * Return these comparison figures in the form of a pointer (RTC
	 * increment rate adjust function also needs to know this information)
	 */
	InstancePtr->PtpRecords.NewSlaveTime =
	InstancePtr->PtpRecords.SlaveSyncTimestampNSec;
	InstancePtr->PtpRecords.NewMasterTime = MasterNsCorrected;

	/*
	 * If the Master nano seconds field wrapped during the Sync frame
	 * routing delay, then we need to increment the seconds field.
	 */

	if (MasterNsHasWrapped == 1) {
		MasterSeconds = MasterSeconds + 0x1;
	}

	/*
	 * Calculate the slave RTC error: the master time minus the timestamp
	 * taken by this slave for Sync Frame reception.
	 */
	if (MasterSeconds > InstancePtr->PtpRecords.SlaveSyncTimestampSec) {
		RtcError.Seconds =  MasterSeconds -
		InstancePtr->PtpRecords.SlaveSyncTimestampSec;
		SlaveSecs = XEmacPs_ReadReg
		(InstancePtr->EmacPsInstance->Config.BaseAddress,
		XEMACPS_1588_SEC_OFFSET);
		SlaveSecs += RtcError.Seconds;

	} else {
		RtcError.Seconds =
			InstancePtr->PtpRecords.SlaveSyncTimestampSec
					-  MasterSeconds;
		SlaveSecs = XEmacPs_ReadReg
			(InstancePtr->EmacPsInstance->Config.BaseAddress,
				XEMACPS_1588_SEC_OFFSET);
		SlaveSecs = SlaveSecs - RtcError.Seconds;
	}

	XEmacPs_WriteReg (InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_1588_SEC_OFFSET, SlaveSecs);

#ifdef DEBUG_XEMACPS_LEVEL1
	xil_printf("*** RtcError.NanoSeconds = %d***\r\n",
						RtcError.NanoSeconds);
	xil_printf("*** RtcError.Seconds = %d***\r\n",RtcError.Seconds);
#endif
	/*
	 * Write the results to the RTC registers
	 * ---------------------------------------------
	 */
	if(RtcError.NanoSeconds < 0x40000000UL) {
		RtcError.NanoSeconds = RtcError.NanoSeconds | ToSubtract;
		XEmacPs_WriteReg (
			InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_1588_ADJ_OFFSET, RtcError.NanoSeconds);
	} else {
		NSecsRemaining = RtcError.NanoSeconds - 0x40000000UL;
		XEmacPs_WriteReg (
			InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_1588_ADJ_OFFSET, 0xA0000000);

		NSecsRemaining = NSecsRemaining | ToSubtract;
		XEmacPs_WriteReg (
			InstancePtr->EmacPsInstance->Config.BaseAddress,
			XEMACPS_1588_ADJ_OFFSET, NSecsRemaining);
	}
}

/****************************************************************************/
/**
* This function clock rate adjustment. Not implemented as of now.
*
* @param	InstancePtr is a pointer to the XEmacPs_Ieee1588 instance.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPs_UpdateRtcIncrement(XEmacPs_Ieee1588 *InstancePtr)
{
	/* TO DO */
}
