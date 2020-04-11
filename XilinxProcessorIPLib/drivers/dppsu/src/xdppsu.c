/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu.c
 *
 * Contains a minimal set of functions for the XDpPsu driver that allow access
 * to all of the DisplayPort TX core's functionality. See xdppsu.h for a
 * detailed description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  01/27/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "sleep.h"
#include "xdppsu.h"
#include "xdppsu_serdes.h"
#include "xenv.h"
#include "xil_assert.h"
#include "xstatus.h"
/**************************** Constant Definitions ****************************/

/* Error out if an AUX request yields a defer reply more than 50 times. */
#define XDPPSU_AUX_MAX_DEFER_COUNT 50
/* Error out if an AUX request times out more than 50 times awaiting a reply. */
#define XDPPSU_AUX_MAX_TIMEOUT_COUNT 50
/* Error out if checking for a connected device times out more than 50 times. */
#define XDPPSU_IS_CONNECTED_MAX_TIMEOUT_COUNT 50
/* 20 usec delay */
#define XDPPSU_AUX_MAX_WAIT 20000

/***************************** Type Definitions ******************************/

/**
 * This typedef enumerates the list of training states used in the state machine
 * during the link training process.
 */
typedef enum {
	XDPPSU_TS_CLOCK_RECOVERY,
	XDPPSU_TS_CHANNEL_EQUALIZATION,
	XDPPSU_TS_ADJUST_LINK_RATE,
	XDPPSU_TS_ADJUST_LANE_COUNT,
	XDPPSU_TS_FAILURE,
	XDPPSU_TS_SUCCESS
} XDpPsu_TrainingState;

/**
 * This typedef describes an AUX transaction.
 */
typedef struct {
	u16 CmdCode;		/**< The AUX command code that specifies what
					type of AUX transaction is taking
					place. */
	u8 NumBytes;		/**< The number of bytes that the AUX
					transaction will perform work on. */
	u32 Address;		/**< The AUX or I2C start address that the AUX
					transaction will perform work on. */
	u8 *Data;		/**< The data buffer that will store the data
					read from AUX read transactions or the
					data to write for AUX write
					transactions. */
} XDpPsu_AuxTransaction;

/******************************************************************************/
/**
 * This function is the delay/sleep function for the XDpPsu driver. *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	MicroSeconds is the number of microseconds to delay/sleep for.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static inline void XDpPsu_WaitUs(XDpPsu *InstancePtr, u32 MicroSeconds)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (MicroSeconds == 0)
		return;
	/* Wait the requested amount of time. */
	usleep(MicroSeconds);
}

/******************************************************************************/
/**
 * This function waits for a reply indicating that the most recent AUX request
 * has been received by the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if a reply was sent from the RX device.
 *		- XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_AuxWaitReply(XDpPsu *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Timeout = XDPPSU_AUX_MAX_WAIT;
	u32 Status;

	while (0 < Timeout) {
		Status = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
							XDPPSU_REPLY_STATUS);

		/* Check for error. */
		if (Status & XDPPSU_REPLY_STATUS_REPLY_ERROR_MASK)
			return XST_ERROR_COUNT_MAX;

		/* Check for a reply. */
		if ((Status & XDPPSU_REPLY_STATUS_REPLY_RECEIVED_MASK) &&
				!(Status &
				XDPPSU_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) &&
				!(Status &
				XDPPSU_REPLY_STATUS_REPLY_IN_PROGRESS_MASK)) {
			return XST_SUCCESS;
		}

		Timeout--;
		XDpPsu_WaitUs(InstancePtr, 20);
	}

	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This function submits the supplied AUX request to the RX device over the AUX
 * channel by writing the command, the destination address, (the write buffer
 * for write commands), and the data size to the DisplayPort TX core.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Request is a pointer to an initialized XDpPsu_AuxTransaction
 *		structure containing the required information for issuing an AUX
 *		command.
 *
 * @return
 *		- XST_SUCCESS if the request was acknowledged.
 *		- XST_ERROR_COUNT_MAX if waiting for a reply timed out.
 *		- XST_SEND_ERROR if the request was deferred.
 *		- XST_FAILURE otherwise, if the request was NACK'ed.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_AuxRequestSend(XDpPsu *InstancePtr,
						XDpPsu_AuxTransaction *Request)
{
	u32 TimeoutCount = 0;
	u32 Status;
	u8 Index;

	do {
		Status = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
							XDPPSU_REPLY_STATUS);

		XDpPsu_WaitUs(InstancePtr, 20);
		TimeoutCount++;
		if (TimeoutCount >= XDPPSU_AUX_MAX_TIMEOUT_COUNT)
			return XST_ERROR_COUNT_MAX;

	} while ((Status & XDPPSU_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) ||
			(Status & XDPPSU_REPLY_STATUS_REPLY_IN_PROGRESS_MASK));

	/* Set the address for the request. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_AUX_ADDRESS,
							Request->Address);

	if ((Request->CmdCode == XDPPSU_AUX_CMD_WRITE) ||
			(Request->CmdCode == XDPPSU_AUX_CMD_I2C_WRITE) ||
			(Request->CmdCode == XDPPSU_AUX_CMD_I2C_WRITE_MOT)) {
		/* Feed write data into the DisplayPort TX core's write FIFO. */
		for (Index = 0; Index < Request->NumBytes; Index++) {
			XDpPsu_WriteReg(InstancePtr->Config.BaseAddr,
				XDPPSU_AUX_WRITE_FIFO, Request->Data[Index]);
		}
	}

	/* Submit the command and the data size. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_AUX_CMD,
				((Request->CmdCode << XDPPSU_AUX_CMD_SHIFT) |
				((Request->NumBytes - 1) &
				XDPPSU_AUX_CMD_NBYTES_TRANSFER_MASK)));

	/* Check for a reply from the RX device to the submitted request. */
	Status = XDpPsu_AuxWaitReply(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* Waiting for a reply timed out. */
		return Status;
	}

	/* Analyze the reply. */
	Status = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
							XDPPSU_AUX_REPLY_CODE);
	if ((Status == XDPPSU_AUX_REPLY_CODE_DEFER) ||
				(Status == XDPPSU_AUX_REPLY_CODE_I2C_DEFER)) {
		/* The request was deferred. */
		return XST_SEND_ERROR;
	}
	else if ((Status == XDPPSU_AUX_REPLY_CODE_NACK) ||
				(Status == XDPPSU_AUX_REPLY_CODE_I2C_NACK)) {
		/* The request was not acknowledged. */
		return XST_FAILURE;
	}

	/* The request was acknowledged. */

	if ((Request->CmdCode == XDPPSU_AUX_CMD_READ) ||
			(Request->CmdCode == XDPPSU_AUX_CMD_I2C_READ) ||
			(Request->CmdCode == XDPPSU_AUX_CMD_I2C_READ_MOT)) {

		/* Wait until all data has been received. */
		TimeoutCount = 0;
		do {
			Status = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
						XDPPSU_REPLY_DATA_COUNT);

			XDpPsu_WaitUs(InstancePtr, 100);
			TimeoutCount++;
			if (TimeoutCount >= XDPPSU_AUX_MAX_TIMEOUT_COUNT)
				return XST_ERROR_COUNT_MAX;
		} while (Status != Request->NumBytes);

		/* Obtain the read data from the reply FIFO. */
		for (Index = 0; Index < Request->NumBytes; Index++) {
			Request->Data[Index] = XDpPsu_ReadReg(
						InstancePtr->Config.BaseAddr,
						XDPPSU_AUX_REPLY_DATA);
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function waits until another request is no longer in progress.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the the RX device is no longer busy.
 *		- XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_AuxWaitReady(XDpPsu *InstancePtr)
{
	u32 Status;
	u32 Timeout = 100;

	/* Wait until the DisplayPort TX core is ready. */
	do {
		Status = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
						XDPPSU_INTERRUPT_SIG_STATE);

		/* Protect against an infinite loop. */
		if (!Timeout--)
			return XST_ERROR_COUNT_MAX;

		XDpPsu_WaitUs(InstancePtr, 20);
	}
	while (Status & XDPPSU_REPLY_STATUS_REPLY_IN_PROGRESS_MASK);

	return XST_SUCCESS;
}
/******************************************************************************/
/**
 * This function submits the supplied AUX request to the RX device over the AUX
 * channel. If waiting for a reply times out, or if the DisplayPort TX core
 * indicates that the request was deferred, the request is sent again (up to a
 * maximum specified by XDPPSU_AUX_MAX_DEFER_COUNT|XDPPSU_AUX_MAX_TIMEOUT_COUNT).
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Request is a pointer to an initialized XDpPsu_AuxTransaction
 *		structure containing the required information for issuing an
 *		AUX command, as well as a write buffer used for write commands,
 *		and a read buffer for read commands.
 *
 * @return
 *		- XST_SUCCESS if the request was acknowledged.
 *		- XST_ERROR_COUNT_MAX if resending the request exceeded the
 *		  maximum for deferral and timeout.
 *		- XST_FAILURE otherwise (if the DisplayPort TX core sees a NACK
 *		  reply code or if the AUX transaction failed).
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_AuxRequest(XDpPsu *InstancePtr, XDpPsu_AuxTransaction *Request)
{
	u32 Status;
	u32 DeferCount = 0;
	u32 TimeoutCount = 0;

	do {
		Status = XDpPsu_AuxWaitReady(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The RX device isn't ready yet. */
			TimeoutCount++;
			continue;
		}

		/* Send the request. */
		Status = XDpPsu_AuxRequestSend(InstancePtr, Request);
		if (Status == XST_SEND_ERROR) {
			/* The request was deferred. */
			DeferCount++;
		}
		else if (Status == XST_ERROR_COUNT_MAX) {
			/* Waiting for a reply timed out. */
			TimeoutCount++;
		}
		else {
			/* XST_FAILURE indicates that the request was NACK'ed,
			 * XST_SUCCESS indicates that the request was ACK'ed. */
			return Status;
		}

		XDpPsu_WaitUs(InstancePtr, 100);
	} while ((DeferCount < XDPPSU_AUX_MAX_DEFER_COUNT) &&
				(TimeoutCount < XDPPSU_AUX_MAX_TIMEOUT_COUNT));

	/* The request was not successfully received by the RX device. */
	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This function determines what the RX device's required training delay is for
 * link training.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	The training delay specified in the RX device's DisplayPort
 *		Configuration Data (DPCD) register,
 *		XDPPSU_DPCD_TRAIN_AUX_RD_INTERVAL.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_GetTrainingDelay(XDpPsu *InstancePtr)
{
	u8 *Dpcd = InstancePtr->RxConfig.DpcdRxCapsField;

	if(Dpcd[XDPPSU_DPCD_TRAIN_AUX_RD_INTERVAL]) {
		return 400 * Dpcd[XDPPSU_DPCD_TRAIN_AUX_RD_INTERVAL] * 10;
	}
	else {
		return 400;
	}
}

/******************************************************************************/
/**
 * This function sets the training pattern to be used during link training for
 * both the DisplayPort TX core and the RX device. Scrambler isenabled when
 * pattern is turned off.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Pattern selects the pattern to be used. One of the following:
 *		- XDPPSU_TRAINING_PATTERN_SET_OFF
 *		- XDPPSU_TRAINING_PATTERN_SET_TP1
 *		- XDPPSU_TRAINING_PATTERN_SET_TP2
 *		- XDPPSU_TRAINING_PATTERN_SET_TP3
 *
 * @return
 *		- XST_SUCCESS if setting the pattern was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_SetTrainingPattern(XDpPsu *InstancePtr, u32 Pattern)
{
	u32 Status;
	u8 AuxData[5];

	/* Write to the DisplayPort TX core. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr,
					XDPPSU_TRAINING_PATTERN_SET, Pattern);

	AuxData[0] = Pattern;

	switch (Pattern) {
	case XDPPSU_TRAINING_PATTERN_SET_OFF:
		XDpPsu_WriteReg(InstancePtr->Config.BaseAddr,
						XDPPSU_SCRAMBLING_DISABLE, 0);
		InstancePtr->LinkConfig.ScramblerEn = 1;
		break;
	case XDPPSU_TRAINING_PATTERN_SET_TP1:
	case XDPPSU_TRAINING_PATTERN_SET_TP2:
	case XDPPSU_TRAINING_PATTERN_SET_TP3:
		AuxData[0] |= XDPPSU_DPCD_TP_SET_SCRAMB_DIS_MASK;
		XDpPsu_WriteReg(InstancePtr->Config.BaseAddr,
						XDPPSU_SCRAMBLING_DISABLE, 1);
		InstancePtr->LinkConfig.ScramblerEn = 0;
		break;
	default:
		return XST_FAILURE;
		break;
	}

	/* Make the adjustments to both the DisplayPort TX core and the RX
	 * device. */
	XDpPsu_SetVswingPreemp(InstancePtr, &AuxData[1]);
	/* Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device. */
	if (Pattern == XDPPSU_TRAINING_PATTERN_SET_OFF) {
		Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_TP_SET, 1,
								AuxData);
	}
	else {
		Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_TP_SET, 5,
								AuxData);
	}
	return Status;
}

/******************************************************************************/
/**
 * This function checks if the RX device's DisplayPort Configuration Data (DPCD)
 * indicates that the clock recovery sequence during link training was
 * successful - the RX device's link clock and data recovery unit has realized
 * and maintained the frequency lock for all lanes currently in use.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	LaneCount is the number of lanes to check.
 *
 * @return
 *		- XST_SUCCESS if the RX device's clock recovery PLL has
 *		  achieved frequency lock for all lanes in use.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_CheckClockRecovery(XDpPsu *InstancePtr, u8 LaneCount)
{
	u8 *LaneStatus = InstancePtr->RxConfig.LaneStatusAdjReqs;

	/* Check that all LANEx_CR_DONE bits are set. */
	switch (LaneCount) {
	case XDPPSU_LANE_COUNT_SET_2:
		if (!(LaneStatus[0] &
				XDPPSU_DPCD_STATUS_LANE_1_CR_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* Drop through and check lane 0. */
	case XDPPSU_LANE_COUNT_SET_1:
		if (!(LaneStatus[0] &
				XDPPSU_DPCD_STATUS_LANE_0_CR_DONE_MASK)) {
			return XST_FAILURE;
		}
	default:
		/* All (LaneCount) lanes have achieved clock recovery. */
		return XST_SUCCESS;
		break;
	}

}

/******************************************************************************/
/**
 * This function checks if the RX device's DisplayPort Configuration Data (DPCD)
 * indicates that the channel equalization sequence during link training was
 * successful - the RX device has achieved channel equalization, symbol lock,
 * and interlane alignment for all lanes currently in use.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	LaneCount is the number of lanes to check.
 *
 * @return
 *		- XST_SUCCESS if the RX device has achieved channel
 *		  equalization symbol lock, and interlane alignment for all
 *		  lanes in use.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_CheckChannelEqualization(XDpPsu *InstancePtr, u8 LaneCount)
{
	u8 *LaneStatus = InstancePtr->RxConfig.LaneStatusAdjReqs;

	/* Check that all LANEx_CHANNEL_EQ_DONE bits are set. */
	switch (LaneCount) {
	case XDPPSU_LANE_COUNT_SET_2:
		if (!(LaneStatus[0] &
				XDPPSU_DPCD_STATUS_LANE_1_CE_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* Drop through and check lane 0. */
	case XDPPSU_LANE_COUNT_SET_1:
		if (!(LaneStatus[0] &
				XDPPSU_DPCD_STATUS_LANE_0_CE_DONE_MASK)) {
			return XST_FAILURE;
		}
	default:
		/* All (LaneCount) lanes have achieved channel equalization. */
		break;
	}

	/* Check that all LANEx_SYMBOL_LOCKED bits are set. */
	switch (LaneCount) {
	case XDPPSU_LANE_COUNT_SET_2:
		if (!(LaneStatus[0] &
				XDPPSU_DPCD_STATUS_LANE_1_SL_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* Drop through and check lane 0. */
	case XDPPSU_LANE_COUNT_SET_1:
		if (!(LaneStatus[0] &
				XDPPSU_DPCD_STATUS_LANE_0_SL_DONE_MASK)) {
			return XST_FAILURE;
		}
	default:
		/* All (LaneCount) lanes have achieved symbol lock. */
		break;
	}

	/* Check that interlane alignment is done. */
	if (!(LaneStatus[2] &
			XDPPSU_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK)) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will do a burst AUX read from the RX device over the AUX
 * channel. The contents of the status registers will be stored for later use by
 * XDpPsu_CheckClockRecovery, XDpPsu_CheckChannelEqualization, and
 * XDpPsu_AdjVswingPreemp.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the AUX read was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_GetLaneStatusAdjReqs(XDpPsu *InstancePtr)
{
	u32 Status;
	u8 AuxData[8];

	/* Read and store the sink count, the device service IRQ vector, 4 bytes
	 * of lane status, and 2 bytes of adjustment requests. */
	Status = XDpPsu_AuxRead(InstancePtr, XDPPSU_DPCD_SINK_COUNT, 8, AuxData);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Save XDPPSU_DPCD_SINK_COUNT contents. */
	InstancePtr->RxConfig.SinkCount =
			((AuxData[0] & XDPPSU_DPCD_SINK_COUNT_HIGH_MASK) >>
			XDPPSU_DPCD_SINK_COUNT_HIGH_LOW_SHIFT) |
			(AuxData[0] & XDPPSU_DPCD_SINK_COUNT_LOW_MASK);

	/* Save XDPPSU_DPCD_DEVICE_SERVICE_IRQ contents. */
	InstancePtr->RxConfig.DevServiceIrqVec = AuxData[1];

	/* Save contents of XDPPSU_DPCD_STATUS_LANE_X_X,
	 * XDPPSU_DPCD_LANE_ALIGN_STATUS_UPDATED, XDPPSU_DPCD_SINK_STATUS. */
	memcpy(InstancePtr->RxConfig.LaneStatusAdjReqs, &AuxData[2], 6);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets new voltage swing and pre-emphasis levels using the
 * adjustment requests obtained from the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the new levels were written successfully.
 *		- XST_FAILURE otherwise (an AUX transaction failed).
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_AdjVswingPreemp(XDpPsu *InstancePtr)
{
	u32 Status;
	u8 Index;
	u8 VsLevelAdjReq[4];
	u8 PeLevelAdjReq[4];
	u8 AuxData[4];
	u8 *AdjReqs = &InstancePtr->RxConfig.LaneStatusAdjReqs[4];

	/* Analyze the adjustment requests for changes in voltage swing and
	 * pre-emphasis levels. */
	VsLevelAdjReq[0] = AdjReqs[0] & XDPPSU_DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
	VsLevelAdjReq[1] = (AdjReqs[0] & XDPPSU_DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
					XDPPSU_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
	PeLevelAdjReq[0] = (AdjReqs[0] & XDPPSU_DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
					XDPPSU_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
	PeLevelAdjReq[1] = (AdjReqs[0] & XDPPSU_DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
					XDPPSU_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;

	/* Change the drive settings to match the adjustment requests. Use the
	 * greatest level requested. */
	InstancePtr->LinkConfig.VsLevel = 0;
	InstancePtr->LinkConfig.PeLevel = 0;
	for (Index = 0; Index < InstancePtr->LinkConfig.LaneCount; Index++) {
		if (VsLevelAdjReq[Index] >InstancePtr->LinkConfig.VsLevel) {
			InstancePtr->LinkConfig.VsLevel = VsLevelAdjReq[Index];
		}
		if (PeLevelAdjReq[Index] >InstancePtr->LinkConfig.PeLevel) {
			InstancePtr->LinkConfig.PeLevel = PeLevelAdjReq[Index];
		}
	}

	if (InstancePtr->LinkConfig.PeLevel > XDPPSU_MAXIMUM_PE_LEVEL) {
		InstancePtr->LinkConfig.PeLevel = XDPPSU_MAXIMUM_PE_LEVEL;
	}
	if (InstancePtr->LinkConfig.VsLevel > XDPPSU_MAXIMUM_VS_LEVEL) {
		InstancePtr->LinkConfig.VsLevel = XDPPSU_MAXIMUM_VS_LEVEL;
	}

	/* Verify that the voltage swing and pre-emphasis combination is
	 * allowed. Some combinations will result in a differential peak-to-peak
	 * voltage that is outside the permissable range. See the VESA
	 * DisplayPort v1.2 Specification, section 3.1.5.2.
	 * The valid combinations are:
	 *      PE=0    PE=1    PE=2
	 * VS=0 Valid   Valid   Valid
	 * VS=1 Valid   Valid   Valid
	 * VS=2 Valid   Valid
	 * VS=3 Valid
	 */
	if (InstancePtr->LinkConfig.PeLevel >
					(4 - InstancePtr->LinkConfig.VsLevel)) {
		InstancePtr->LinkConfig.PeLevel =
					4 - InstancePtr->LinkConfig.VsLevel;
	}

	/* Make the adjustments to both the DisplayPort TX core and the RX
	 * device. */
	XDpPsu_SetVswingPreemp(InstancePtr, AuxData);
	/* Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device. */
	Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_TRAINING_LANE0_SET, 2,
								AuxData);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/******************************************************************************/
/**
 * This function runs the channel equalization sequence as part of link
 * training. The sequence is as follows:
 *	0) Start signaling with the same drive settings used at the end of the
 *	   clock recovery sequence.
 *	1) Transmit training pattern 2 (or 3) over the main link with symbol
 *	   scrambling disabled.
 *	2) The channel equalization loop. If channel equalization is
 *	   unsuccessful after 5 loop iterations, return.
 *	2a) Wait for at least the period of time specified in the RX device's
 *	    DisplayPort Configuration Data (DPCD) register,
 *	    TRAINING_AUX_RD_INTERVAL.
 *	2b) Check if all lanes have achieved channel equalization, symbol lock,
 *	    and interlane alignment. If so, return.
 *	2c) Check if the same voltage swing level has been used 5 consecutive
 *	    times or if the maximum level has been reached. If so, return.
 *	2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *	    requested by the RX device.
 *	2e) Loop back to 2a.
 * For a more detailed description of the channel equalization sequence, see
 * section 3.5.1.2.2 of the DisplayPort 1.2a specification document.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	The next training state:
 *		- XDPPSU_TS_SUCCESS if training succeeded.
 *		- XDPPSU_TS_FAILURE if writing the drive settings to the RX
 *		  device was unsuccessful.
 *		- XDPPSU_TS_ADJUST_LINK_RATE if, after 5 loop iterations, the
 *		  channel equalization sequence did not complete successfully.
 *
 * @note	None.
 *
*******************************************************************************/
static XDpPsu_TrainingState XDpPsu_TrainingStateChannelEqualization(
							XDpPsu *InstancePtr)
{
	u32 Status;
	u32 DelayUs;
	u32 IterationCount = 0;

	/* Obtain the required delay for channel equalization as specified by
	 * the RX device. */
	DelayUs = XDpPsu_GetTrainingDelay(InstancePtr);
	/* Start channel equalization. */

	/* Write the current drive settings. */
	/* Transmit training pattern 2/3. */
	if (InstancePtr->RxConfig.DpcdRxCapsField[XDPPSU_DPCD_MAX_LANE_COUNT] &
						XDPPSU_DPCD_TPS3_SUPPORT_MASK) {
		Status = XDpPsu_SetTrainingPattern(InstancePtr,
						XDPPSU_TRAINING_PATTERN_SET_TP3);
	}
	else {
		Status = XDpPsu_SetTrainingPattern(InstancePtr,
						XDPPSU_TRAINING_PATTERN_SET_TP2);
	}
	if (Status != XST_SUCCESS) {
		return XDPPSU_TS_FAILURE;
	}

	while (IterationCount < 5) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		XDpPsu_WaitUs(InstancePtr, DelayUs);

		/* Get lane and adjustment requests. */
		Status = XDpPsu_GetLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX read failed. */
			return XDPPSU_TS_FAILURE;
		}

		/* Adjust the drive settings as requested by the RX device. */
		Status = XDpPsu_AdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX write failed. */
			return XDPPSU_TS_FAILURE;
		}

		/* Check that all lanes still have their clocks locked. */
		Status = XDpPsu_CheckClockRecovery(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);
		if (Status != XST_SUCCESS) {
			break;
		}

		/* Check that all lanes have accomplished channel
		 * equalization, symbol lock, and interlane alignment. */
		Status = XDpPsu_CheckChannelEqualization(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);
		if (Status == XST_SUCCESS) {
			return XDPPSU_TS_SUCCESS;
		}

		IterationCount++;
	}

	/* Tried 5 times with no success. Try a reduced bitrate first, then
	 * reduce the number of lanes. */
	return XDPPSU_TS_ADJUST_LINK_RATE;
}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training, and a minimal data rate of 1.62
 * Gbps was being used. As a result, the number of lanes in use will be reduced,
 * and training will be re-attempted (starting with clock recovery) at this
 * lower lane count.
 *
 * @note	Training will be re-attempted with the maximum data rate being
 *		used with the reduced lane count to train at the main link at
 *		the maximum bandwidth possible.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	The next training state:
 *		- XDPPSU_TS_FAILURE if only one lane is already in use.
 *		- XDPPSU_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
 * @note	None.
 *
*******************************************************************************/
static XDpPsu_TrainingState XDpPsu_TrainingStateAdjustLaneCount(
							XDpPsu *InstancePtr)
{
	u32 Status;
	XDpPsu_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;

	switch (LinkConfig->LaneCount) {
	case XDPPSU_LANE_COUNT_SET_2:
		Status = XDpPsu_SetLaneCount(InstancePtr,
							XDPPSU_LANE_COUNT_SET_1);
		if (Status != XST_SUCCESS) {
			return Status;
			break;
		}

		Status = XDpPsu_SetLinkRate(InstancePtr,
						LinkConfig->MaxLinkRate);
		if (Status != XST_SUCCESS) {
			return Status;
			break;
		}
		return XDPPSU_TS_CLOCK_RECOVERY;
		break;
	default:
		/* Already at the lowest lane count. Training has failed at the
		 * lowest lane count and link rate. */
		return XDPPSU_TS_FAILURE;
		break;
	}
}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training. As a result, the data rate will
 * be downshifted, and training will be re-attempted (starting with clock
 * recovery) at the reduced data rate. If the data rate is already at 1.62 Gbps,
 * a downshift in lane count will be attempted.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	The next training state:
 *		- XDPPSU_TS_ADJUST_LANE_COUNT if the minimal data rate is already
 *		  in use. Re-attempt training at a reduced lane count.
 *		- XDPPSU_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
 * @note	None.
 *
*******************************************************************************/
static XDpPsu_TrainingState XDpPsu_TrainingStateAdjustLinkRate(XDpPsu *InstancePtr)
{
	u32 Status;

	switch (InstancePtr->LinkConfig.LinkRate) {
	case XDPPSU_LINK_BW_SET_540GBPS:
		Status = XDpPsu_SetLinkRate(InstancePtr,
						XDPPSU_LINK_BW_SET_270GBPS);
		if (Status != XST_SUCCESS) {
			Status = XDPPSU_TS_FAILURE;
			break;
		}
		Status = XDPPSU_TS_CLOCK_RECOVERY;
		break;
	case XDPPSU_LINK_BW_SET_270GBPS:
		Status = XDpPsu_SetLinkRate(InstancePtr,
						XDPPSU_LINK_BW_SET_162GBPS);
		if (Status != XST_SUCCESS) {
			Status = XDPPSU_TS_FAILURE;
			break;
		}
		Status = XDPPSU_TS_CLOCK_RECOVERY;
		break;
	default:
	/* Already at the lowest link rate. Try reducing the lane
	 * count next. */
		Status = XDPPSU_TS_ADJUST_LANE_COUNT;
		break;
	}

	return Status;
}

/******************************************************************************/
/**
 * This function runs the clock recovery sequence as part of link training. The
 * sequence is as follows:
 *	0) Start signaling at the minimum voltage swing, pre-emphasis, and post-
 *	   cursor levels.
 *	1) Transmit training pattern 1 over the main link with symbol scrambling
 *	   disabled.
 *	2) The clock recovery loop. If clock recovery is unsuccessful after 5
 *	   loop iterations, return.
 *	2a) Wait for at least the period of time specified in the RX device's
 *	    DisplayPort Configuration Data (DPCD) register,
 *	    TRAINING_AUX_RD_INTERVAL.
 *	2b) Check if all lanes have achieved clock recovery lock. If so, return.
 *	2c) Check if the same voltage swing level has been used 5 consecutive
 *	    times or if the maximum level has been reached. If so, return.
 *	2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *	    requested by the RX device.
 *	2e) Loop back to 2a.
 * For a more detailed description of the clock recovery sequence, see section
 * 3.5.1.2.1 of the DisplayPort 1.2a specification document.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	The next training state:
 *		- XDPPSU_TS_CHANNEL_EQUALIZATION if the clock recovery sequence
 *		  completed successfully.
 *		- XDPPSU_TS_FAILURE if writing the drive settings to the RX
 *		  device was unsuccesful.
 *		- XDPPSU_TS_ADJUST_LINK_RATE if the clock recovery sequence
 *		  did not complete successfully.
 *
 * @note	None.
 *
*******************************************************************************/
static XDpPsu_TrainingState XDpPsu_TrainingStateClockRecovery(XDpPsu *InstancePtr)
{
	u32 Status;
	u32 DelayUs;
	u8 PrevVsLevel = 0;
	u8 SameVsLevelCount = 0;
	XDpPsu_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;

	/* Obtain the required delay for clock recovery as specified by the
	 * RX device. */
	DelayUs = XDpPsu_GetTrainingDelay(InstancePtr);

	/* Start CRLock. */

	/* Start from minimal voltage swing and pre-emphasis levels. */
	InstancePtr->LinkConfig.VsLevel = 0;
	InstancePtr->LinkConfig.PeLevel = 0;
	/* Transmit training pattern 1. */
	Status = XDpPsu_SetTrainingPattern(InstancePtr,
						XDPPSU_TRAINING_PATTERN_SET_TP1);
	if (Status != XST_SUCCESS) {
		return XDPPSU_TS_FAILURE;
	}

	while (1) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		XDpPsu_WaitUs(InstancePtr, DelayUs);

		/* Get lane and adjustment requests. */
		Status = XDpPsu_GetLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX read failed. */
			return XDPPSU_TS_FAILURE;
		}

		/* Check if all lanes have realized and maintained the frequency
		 * lock and get adjustment requests. */
		Status = XDpPsu_CheckClockRecovery(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);
		if (Status == XST_SUCCESS) {
			return XDPPSU_TS_CHANNEL_EQUALIZATION;
		}

		/* Check if the same voltage swing for each lane has been used 5
		 * consecutive times. */
		if (PrevVsLevel == LinkConfig->VsLevel) {
			SameVsLevelCount++;
		}
		else {
			SameVsLevelCount = 0;
			PrevVsLevel = LinkConfig->VsLevel;
		}
		if (SameVsLevelCount >= 5) {
			break;
		}

		/* Only try maximum voltage swing once. */
		if (LinkConfig->VsLevel == XDPPSU_MAXIMUM_VS_LEVEL) {
			break;
		}

		/* Adjust the drive settings as requested by the RX device. */
		Status = XDpPsu_AdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX write failed. */
			return XDPPSU_TS_FAILURE;
		}
	}

	return XDPPSU_TS_ADJUST_LINK_RATE;
}

/******************************************************************************/
/**
 * This function runs the link training process. It is implemented as a state
 * machine, with each state returning the next state. First, the clock recovery
 * sequence will be run; if successful, the channel equalization sequence will
 * run. If either the clock recovery or channel equalization sequence failed,
 * the link rate or the number of lanes used will be reduced and training will
 * be re-attempted. If training fails at the minimal data rate, 1.62 Gbps with
 * a single lane, training will no longer re-attempt and fail.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the training process succeeded.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_RunTraining(XDpPsu *InstancePtr)
{
	u32 Status;
	XDpPsu_TrainingState TrainingState = XDPPSU_TS_CLOCK_RECOVERY;

	while (1) {
		switch (TrainingState) {
		case XDPPSU_TS_CLOCK_RECOVERY:
			TrainingState = XDpPsu_TrainingStateClockRecovery(
								InstancePtr);
			break;
		case XDPPSU_TS_CHANNEL_EQUALIZATION:
			TrainingState = XDpPsu_TrainingStateChannelEqualization(
								InstancePtr);
			break;
		case XDPPSU_TS_ADJUST_LINK_RATE:
			TrainingState = XDpPsu_TrainingStateAdjustLinkRate(
								InstancePtr);
			break;
		case XDPPSU_TS_ADJUST_LANE_COUNT:
			TrainingState = XDpPsu_TrainingStateAdjustLaneCount(
								InstancePtr);
			break;
		default:
			break;
		}

		if (TrainingState == XDPPSU_TS_SUCCESS) {
			break;
		}
		else if (TrainingState == XDPPSU_TS_FAILURE) {
			return XST_FAILURE;
		}

		if ((TrainingState == XDPPSU_TS_ADJUST_LANE_COUNT) ||
				(TrainingState == XDPPSU_TS_ADJUST_LINK_RATE)) {
			Status = XDpPsu_SetTrainingPattern(InstancePtr,
					XDPPSU_TRAINING_PATTERN_SET_OFF);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	/* Final status check. */
	Status = XDpPsu_CheckLinkStatus(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/******************************************************************************/
/** This function checks the lane alignment status
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the lane alignment is complete.
 *		- XST_FAILURE if the lane alignment is not complete.
*******************************************************************************/
static u32 XDpPsu_CheckLaneAlign(XDpPsu *InstancePtr)
{
	u8 *LaneStatus = InstancePtr->RxConfig.LaneStatusAdjReqs;

	if (!(LaneStatus[2] &
		XDPPSU_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK)) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function contains the common sequence of submitting an AUX command for
 * AUX read, AUX write, I2C-over-AUX read, and I2C-over-AUX write transactions.
 * If required, the reads and writes are split into multiple requests, each
 * acting on a maximum of 16 bytes.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	CmdType is the type of AUX command (one of: XDPPSU_AUX_CMD_READ,
 *		XDPPSU_AUX_CMD_WRITE, XDPPSU_AUX_CMD_I2C_READ, or
 *		XDPPSU_AUX_CMD_I2C_WRITE.
 * @param	Address is the starting address that the AUX transaction will
 *		read/write from/to the RX device.
 * @param	NumBytes is the number of bytes to read/write from/to the RX
 *		device.
 * @param	Data is a pointer to the data buffer that contains the data
 *		to be read/written from/to the RX device.
 *
 * @return
 *		- XST_SUCCESS if the AUX transaction request was acknowledged.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise (if the DisplayPort TX core sees a NACK
 *		  reply code or if the AUX transaction failed).
 *		- XST_DEVICE_NOT_FOUND if the sink is diconnected from the TX.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_AuxCommon(XDpPsu *InstancePtr, u32 CmdType, u32 Address,
							u32 NumBytes, u8 *Data)
{
	u32 Status;
	XDpPsu_AuxTransaction Request;
	u32 BytesLeft;

	if (!XDpPsu_IsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Set the start address for AUX transactions. For I2C transactions,
	 * this is the address of the I2C bus. */
	Request.Address = Address;

	BytesLeft = NumBytes;
	while (BytesLeft > 0) {
		Request.CmdCode = CmdType;

		if ((CmdType == XDPPSU_AUX_CMD_READ) ||
					(CmdType == XDPPSU_AUX_CMD_WRITE)) {
			/* Increment address for normal AUX transactions. */
			Request.Address = Address + (NumBytes - BytesLeft);
		}

		/* Increment the pointer to the supplied data buffer. */
		Request.Data = &Data[NumBytes - BytesLeft];

		if (BytesLeft > 16) {
			Request.NumBytes = 16;
		}
		else {
			Request.NumBytes = BytesLeft;
		}
		BytesLeft -= Request.NumBytes;

		if ((CmdType == XDPPSU_AUX_CMD_I2C_READ) && (BytesLeft > 0)) {
			/* Middle of a transaction I2C read request. Override
			 * the command code that was set to CmdType. */
			Request.CmdCode = XDPPSU_AUX_CMD_I2C_READ_MOT;
		}
		else if ((CmdType == XDPPSU_AUX_CMD_I2C_WRITE) &&
							(BytesLeft > 0)) {
			/* Middle of a transaction I2C write request. Override
			 * the command code that was set to CmdType. */
			Request.CmdCode = XDPPSU_AUX_CMD_I2C_WRITE_MOT;
		}

		XDpPsu_WaitUs(InstancePtr, InstancePtr->AuxDelayUs);

		Status = XDpPsu_AuxRequest(InstancePtr, &Request);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;
}


/******************************************************************************/
/**
 * This function waits for the DisplayPort PHY to come out of reset.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_ERROR_COUNT_MAX if the PHY failed to be ready.
 *		- XST_SUCCESS otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_WaitPhyReady(XDpPsu *InstancePtr)
{
	u32 Timeout = 100;
	u32 PhyStatus;

	/* Wait until the PHY is ready. */
	do {
		XDpPsu_WaitUs(InstancePtr, 20);
		PhyStatus = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
					XDPPSU_PHY_STATUS);
		PhyStatus &= XDPPSU_PHY_STATUS_ALL_LANES_READY_MASK;
		/* Protect against an infinite loop. */
		if (!Timeout--) {
			return XST_ERROR_COUNT_MAX;
		}

	}
	while (PhyStatus != XDPPSU_PHY_STATUS_ALL_LANES_READY_MASK);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the clock frequency for the DisplayPort PHY corresponding
 * to a desired data rate.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Speed determines what clock frequency will be used based on one
 *		of the following selects:
 *		- XDPPSU_PHY_CLOCK_SELECT_162GBPS = 0x01
 *		- XDPPSU_PHY_CLOCK_SELECT_270GBPS = 0x03
 *		- XDPPSU_PHY_CLOCK_SELECT_540GBPS = 0x05
 *
 * @return
 *		- XST_SUCCESS if the reset for each lane is done after the clock
 *		  frequency has been set.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDpPsu_SetPhyClkSpeed(XDpPsu *InstancePtr, u32 Speed)
{
	u32 Status;
	u32 RegVal;

	/* Disable the DisplayPort TX core first. */
	RegVal = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr, XDPPSU_ENABLE);
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_ENABLE, 0x0);

	/* Change speed of the feedback clock. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr,
						XDPPSU_PHY_CLOCK_SELECT, Speed);

	/* Re-enable the DisplayPort TX core if it was previously enabled. */
	if (RegVal != 0x0) {
		XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_ENABLE,
									RegVal);
	}

	/* Wait until the PHY is ready. */
	Status = XDpPsu_WaitPhyReady(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function prepares the DisplayPort TX core for use.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort TX core was successfully
 *		  initialized.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_InitializeTx(XDpPsu *InstancePtr)
{
	u32 Status;
	u32 PhyVal;
	u32 RegVal;
	XDpPsu_Config *Config;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Config = &InstancePtr->Config;

	PhyVal = XDpPsu_ReadReg(Config->BaseAddr, XDPPSU_PHY_CONFIG);

	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_SOFT_RESET,
						XDPPSU_SOFT_RESET_EN);

	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_ENABLE, XDPPSU_DP_DISABLE);

	RegVal = (XDpPsu_ReadReg(Config->BaseAddr, XDPPSU_AUX_CLK_DIVIDER) &
					~XDPPSU_AUX_CLK_DIVIDER_VAL_MASK) |
					(60 << 8) |
					(InstancePtr->SAxiClkHz / 1000000);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_AUX_CLK_DIVIDER, RegVal);

	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_PHY_CLOCK_SELECT,
					XDPPSU_PHY_CLOCK_SELECT_540GBPS);

	RegVal = PhyVal & ~XDPPSU_PHY_CONFIG_GT_ALL_RESET_MASK;
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_PHY_CONFIG, RegVal);

	Status = XDpPsu_WaitPhyReady(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_ENABLE, XDPPSU_DP_ENABLE);

	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_INTR_MASK,
					~XDPPSU_INTR_HPD_PULSE_DETECTED_MASK &
					~XDPPSU_INTR_HPD_EVENT_MASK &
					~XDPPSU_INTR_HPD_IRQ_MASK);

	return Status;
}

/******************************************************************************/
/**
 * This function retrieves the configuration for this DisplayPort TX instance
 * and fills in the InstancePtr->Config structure.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	ConfigPtr is a pointer to the configuration structure that will
 *		be used to copy the settings from.
 * @param	EffectiveAddr is the device base address in the virtual memory
 *		space. If the address translation is not used, then the physical
 *		address is passed.
 *
 * @return	None.
 *
 * @note	Unexpected errors may occur if the address mapping is changed
 *		after this function is invoked.
 *
*******************************************************************************/
void XDpPsu_CfgInitialize(XDpPsu *InstancePtr, XDpPsu_Config *ConfigPtr,
							u32 EffectiveAddr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigPtr != NULL);
	Xil_AssertVoid(EffectiveAddr != 0x0);

	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
	InstancePtr->Config.BaseAddr = EffectiveAddr;
	/* TODO: Read from the clock framework */
	InstancePtr->SAxiClkHz = XDPPSU_0_S_AXI_ACLK;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
}

/******************************************************************************/
/**
 * This function retrieves the RX device's capabilities from the RX device's
 * DisplayPort Configuration Data (DPCD).
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort Configuration Data was read
 *		  successfully.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_GetRxCapabilities(XDpPsu *InstancePtr)
{
	u32 Status;
	u8 RxMaxLinkRate;
	u8 RxMaxLaneCount;
	u8 *Dpcd = NULL;
	XDpPsu_LinkConfig *LinkConfig = NULL;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->RxConfig.DpcdRxCapsField != NULL);
	Xil_AssertNonvoid(&InstancePtr->LinkConfig != NULL);
	Xil_AssertNonvoid(&InstancePtr->Config != NULL);

	Dpcd = InstancePtr->RxConfig.DpcdRxCapsField;
	LinkConfig = &InstancePtr->LinkConfig;
	Status = XDpPsu_AuxRead(InstancePtr,
				XDPPSU_DPCD_RECEIVER_CAP_FIELD_START, 16, Dpcd);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	RxMaxLinkRate = Dpcd[XDPPSU_DPCD_MAX_LINK_RATE];
	RxMaxLaneCount = Dpcd[XDPPSU_DPCD_MAX_LANE_COUNT] &
						XDPPSU_DPCD_MAX_LANE_COUNT_MASK;
	LinkConfig->MaxLinkRate = (RxMaxLinkRate > XDPPSU_0_LINK_RATE) ?
					XDPPSU_0_LINK_RATE : RxMaxLinkRate;
	LinkConfig->MaxLaneCount = (RxMaxLaneCount > XDPPSU_0_LANE_COUNT) ?
					XDPPSU_0_LANE_COUNT : RxMaxLaneCount;

	LinkConfig->SupportEnhancedFramingMode =
					Dpcd[XDPPSU_DPCD_MAX_LANE_COUNT] &
					XDPPSU_DPCD_ENHANCED_FRAME_SUPPORT_MASK;
	LinkConfig->SupportDownspreadControl =
					Dpcd[XDPPSU_DPCD_MAX_DOWNSPREAD] &
					XDPPSU_DPCD_MAX_DOWNSPREAD_MASK;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function determines the common capabilities between the DisplayPort TX
 * core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if main link settings were successfully set.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_CfgMainLinkMax(XDpPsu *InstancePtr)
{
	u32 Status;
	XDpPsu_LinkConfig *LinkConfig;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	LinkConfig = &InstancePtr->LinkConfig;

	Status = XDpPsu_SetLinkRate(InstancePtr, LinkConfig->MaxLinkRate);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XDpPsu_SetLaneCount(InstancePtr, LinkConfig->MaxLaneCount);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the link needs training and runs the training
 * sequence if training is required.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS was either already trained, or has been
 *		  trained successfully.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_EstablishLink(XDpPsu *InstancePtr)
{
	u32 Status;
	u32 ReenableMainLink;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((InstancePtr->LinkConfig.LinkRate ==
					XDPPSU_LINK_BW_SET_162GBPS) ||
					(InstancePtr->LinkConfig.LinkRate ==
					XDPPSU_LINK_BW_SET_270GBPS) ||
					(InstancePtr->LinkConfig.LinkRate ==
					XDPPSU_LINK_BW_SET_540GBPS));
	Xil_AssertNonvoid((InstancePtr->LinkConfig.LaneCount ==
					XDPPSU_LANE_COUNT_SET_1) ||
					(InstancePtr->LinkConfig.LaneCount ==
					XDPPSU_LANE_COUNT_SET_2));

	XDpPsu_ResetPhy(InstancePtr,
			XDPPSU_PHY_CONFIG_TX_PHY_8B10BEN_MASK |
			XDPPSU_PHY_CONFIG_PHY_RESET_MASK);

	ReenableMainLink = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
						XDPPSU_ENABLE_MAIN_STREAM);
	if (ReenableMainLink) {
		XDpPsu_EnableMainLink(InstancePtr, 0);
	}

	Status = XDpPsu_RunTraining(InstancePtr);

	Status = XDpPsu_SetTrainingPattern(InstancePtr,
						XDPPSU_TRAINING_PATTERN_SET_OFF);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (ReenableMainLink != 0) {
		XDpPsu_EnableMainLink(InstancePtr, 1);
	}

	Status = XDpPsu_CheckLinkStatus(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the reciever's DisplayPort Configuration Data (DPCD)
 * indicates the reciever has achieved and maintained clock recovery, channel
 * equalization, symbol lock, and interlane alignment for all lanes currently in
 * use.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	LaneCount is the number of lanes to check.
 *
 * @return
 *		- XST_SUCCESS if the RX device has maintained clock recovery,
 *		  channel equalization, symbol lock, and interlane alignment.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_CheckLinkStatus(XDpPsu *InstancePtr, u8 LaneCount)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((LaneCount == XDPPSU_LANE_COUNT_SET_1) ||
					(LaneCount == XDPPSU_LANE_COUNT_SET_2));

	/* Get lane and adjustment requests. */
	Status = XDpPsu_GetLaneStatusAdjReqs(InstancePtr);
	if (Status != XST_SUCCESS) {
	/* The AUX read failed. */
		return XST_FAILURE;
	}

	/* Check if the link needs training. */
	if ((XDpPsu_CheckClockRecovery(
			InstancePtr, LaneCount) == XST_SUCCESS) &&
	    (XDpPsu_CheckChannelEqualization(
			InstancePtr, LaneCount) == XST_SUCCESS) &&
	    (XDpPsu_CheckLaneAlign(InstancePtr) == XST_SUCCESS)) {
		return XST_SUCCESS;
	}

	return XST_FAILURE;
}

/******************************************************************************/
/**
 * This function checks if there is a connected RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- TRUE if there is a connection.
 *		- FALSE if there is no connection.
 *
*******************************************************************************/
u32 XDpPsu_IsConnected(XDpPsu *InstancePtr)
{
	u32 Status;
	u8 Retries = 0;
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	do {
		Status = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
				XDPPSU_INTERRUPT_SIG_STATE) &
				XDPPSU_INTERRUPT_SIG_STATE_HPD_STATE_MASK;

		if (Retries > XDPPSU_IS_CONNECTED_MAX_TIMEOUT_COUNT) {
			return 0;
		}

		Retries++;
		XDpPsu_WaitUs(InstancePtr, 1000);
	} while (Status == 0);

	return 1;
}

/******************************************************************************/
/**
 * This function issues a read request over the AUX channel that will read from
 * the RX device's DisplayPort Configuration Data (DPCD) address space. The read
 * message will be divided into multiple transactions which read a maximum of 16
 * bytes each.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	DpcdAddress is the starting address to read from the RX device.
 * @param	BytesToRead is the number of bytes to read from the RX device.
 * @param	ReadData is a pointer to the data buffer that will be filled
 *		with read data.
 *
 * @return
 *		- XST_SUCCESS if the AUX read request was successfully
 *		  acknowledged.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_AuxRead(XDpPsu *InstancePtr, u32 DpcdAddress, u32 BytesToRead,
								void *ReadData)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToRead <= 0xFFFFF);
	Xil_AssertNonvoid(ReadData != NULL);

	/* Send AUX read transaction. */
	return XDpPsu_AuxCommon(InstancePtr, XDPPSU_AUX_CMD_READ, DpcdAddress,
						BytesToRead, (u8 *)ReadData);
}

/******************************************************************************/
/**
 * This function issues a write request over the AUX channel that will write to
 * the RX device's DisplayPort Configuration Data (DPCD) address space. The
 * write message will be divided into multiple transactions which write a
 * maximum of 16 bytes each.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	DpcdAddress is the starting address to write to the RX device.
 * @param	BytesToWrite is the number of bytes to write to the RX device.
 * @param	WriteData is a pointer to the data buffer that contains the data
 *		to be written to the RX device.
 *
 * @return
 *		- XST_SUCCESS if the AUX write request was successfully
 *		  acknowledged.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_AuxWrite(XDpPsu *InstancePtr, u32 DpcdAddress, u32 BytesToWrite,
								void *WriteData)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToWrite <= 0xFFFFF);
	Xil_AssertNonvoid(WriteData != NULL);

	/* Send AUX write transaction. */
	return XDpPsu_AuxCommon(InstancePtr, XDPPSU_AUX_CMD_WRITE, DpcdAddress,
						BytesToWrite, (u8 *)WriteData);
}

/******************************************************************************/
/**
 * This function performs an I2C read over the AUX channel. The read message
 * will be divided into multiple transactions if the requested data spans
 * multiple segments. The segment pointer is automatically incremented and the
 * offset is calibrated as needed. E.g. For an overall offset of:
 *	- 128, an I2C read is done on segptr=0; offset=128.
 *	- 256, an I2C read is done on segptr=1; offset=0.
 *	- 384, an I2C read is done on segptr=1; offset=128.
 *	- 512, an I2C read is done on segptr=2; offset=0.
 *	- etc.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	IicAddress is the address on the I2C bus of the target device.
 * @param	Offset is the offset at the specified address of the targeted
 *		I2C device that the read will start from.
 * @param	BytesToRead is the number of bytes to read.
 * @param	ReadData is a pointer to a buffer that will be filled with the
 *		I2C read data.
 *
 * @return
 *		- XST_SUCCESS if the I2C read has successfully completed with no
 *		  errors.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_IicRead(XDpPsu *InstancePtr, u8 IicAddress, u16 Offset,
						u16 BytesToRead, void *ReadData)
{
	u32 Status;
	u8 SegPtr;
	u16 NumBytesLeftInSeg;
	u16 BytesLeft;
	u8 CurrBytesToRead;
	u8 Offset8;
	u32 IntrStatus;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(ReadData != NULL);

	BytesLeft = BytesToRead;

	/* Reposition based on a segment length of 256 bytes. */
	SegPtr = 0;
	if (Offset > 255) {
		SegPtr += Offset / 256;
		Offset %= 256;
	}
	Offset8 = Offset;
	NumBytesLeftInSeg = 256 - Offset8;

	/* Set the segment pointer to 0. */
	XDpPsu_IicWrite(InstancePtr, XDPPSU_SEGPTR_ADDR, 1, &SegPtr);

	/* Send I2C read message. Multiple transactions are required if the
	 * requested data spans multiple segments. */
	while (BytesLeft > 0) {
		IntrStatus = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
							XDPPSU_INTR_STATUS);
		if (IntrStatus & XDPPSU_INTR_HPD_EVENT_MASK) {
			/* Exit EDID read early. */
			return XST_DEVICE_NOT_FOUND;
		}
		if (NumBytesLeftInSeg >= BytesLeft) {
		/* Read the remaining number of bytes as requested. */
			CurrBytesToRead = BytesLeft;
		}
		else {
		/* Read the remaining data in the current segment boundary. */
			CurrBytesToRead = NumBytesLeftInSeg;
		}

		/* Setup the I2C-over-AUX read transaction with the offset. */
		Status = XDpPsu_AuxCommon(InstancePtr,
				XDPPSU_AUX_CMD_I2C_WRITE_MOT, IicAddress, 1,
				&Offset8);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Send I2C-over-AUX read transaction. */
		Status = XDpPsu_AuxCommon(InstancePtr, XDPPSU_AUX_CMD_I2C_READ,
				IicAddress, CurrBytesToRead, (u8 *)ReadData);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		if (BytesLeft > CurrBytesToRead) {
		/* Previous I2C read was done on the remaining data in the
		 * current segment; prepare for next read. */
			BytesLeft -= CurrBytesToRead;
			Offset += CurrBytesToRead;
			ReadData += CurrBytesToRead;

			if (BytesLeft > 0) {
			/* Increment the segment pointer to access more I2C
			 * address space, if required. */
				NumBytesLeftInSeg = 256;
				Offset %= 256;
				SegPtr++;

				XDpPsu_IicWrite(InstancePtr, XDPPSU_SEGPTR_ADDR,
								1, &SegPtr);
			}
			Offset8 = Offset;
		}
		else {
		/* Last I2C read. */
			BytesLeft = 0;
		}
	}

	/* Reset the segment pointer to 0. */
	SegPtr = 0;
	XDpPsu_IicWrite(InstancePtr, XDPPSU_SEGPTR_ADDR, 1, &SegPtr);

	return Status;
}

/******************************************************************************/
/**
 * This function performs an I2C write over the AUX channel.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	IicAddress is the address on the I2C bus of the target device.
 * @param	BytesToWrite is the number of bytes to write.
 * @param	WriteData is a pointer to a buffer which will be used as the
 *		data source for the write.
 *
 * @return
 *		- XST_SUCCESS if the I2C write has successfully completed with
 *		  no errors.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_IicWrite(XDpPsu *InstancePtr, u8 IicAddress, u8 BytesToWrite,
								void *WriteData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(WriteData != NULL);

	/* Send I2C-over-AUX read transaction. */
	Status = XDpPsu_AuxCommon(InstancePtr, XDPPSU_AUX_CMD_I2C_WRITE,
				IicAddress, BytesToWrite, (u8 *)WriteData);

	return Status;
}

/******************************************************************************/
/**
 * This function enables or disables 0.5% spreading of the clock for both the
 * DisplayPort and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Enable will downspread the main link signal if set to 1 and
 *		disable downspreading if set to 0.
 *
 * @return
 *		- XST_SUCCESS if setting the downspread control enable was
 *		  successful.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_SetDownspread(XDpPsu *InstancePtr, u8 Enable)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Enable == 1) || (Enable == 0));

	InstancePtr->LinkConfig.DownspreadControl = Enable;

	/* Write downspread enable to the DisplayPort TX core. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_DOWNSPREAD_CTRL,
				InstancePtr->LinkConfig.DownspreadControl);

	/* Preserve the current RX device settings. */
	Status = XDpPsu_AuxRead(InstancePtr, XDPPSU_DPCD_DOWNSPREAD_CTRL, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (InstancePtr->LinkConfig.DownspreadControl) {
		RegVal |= XDPPSU_DPCD_SPREAD_AMP_MASK;
	}
	else {
		RegVal &= ~XDPPSU_DPCD_SPREAD_AMP_MASK;
	}

	/* Write downspread enable to the RX device. */
	Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_DOWNSPREAD_CTRL, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	return Status;
}

/******************************************************************************/
/**
 * This function enables or disables the enhanced framing symbol sequence for
 * both the DisplayPort TX core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Enable will enable enhanced frame mode if set to 1 and disable
 *		it if set to 0.
 *
 * @return
 *		- XST_SUCCESS if setting the enhanced frame mode enable was
 *		  successful.
 *		- XST_DEVICE_NOT_FOUND if no RX is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_SetEnhancedFrameMode(XDpPsu *InstancePtr, u8 Enable)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Enable == 1) || (Enable == 0));

	InstancePtr->LinkConfig.EnhancedFramingMode = Enable;

	/* Write enhanced frame mode enable to the DisplayPort TX core. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_ENHANCED_FRAME_EN,
				InstancePtr->LinkConfig.EnhancedFramingMode);

	/* Preserve the current RX device settings. */
	Status = XDpPsu_AuxRead(InstancePtr, XDPPSU_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (InstancePtr->LinkConfig.EnhancedFramingMode) {
		RegVal |= XDPPSU_DPCD_ENHANCED_FRAME_EN_MASK;
	}
	else {
		RegVal &= ~XDPPSU_DPCD_ENHANCED_FRAME_EN_MASK;
	}

	/* Write enhanced frame mode enable to the RX device. */
	Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the number of lanes to be used by the main link for both
 * the DisplayPort TX core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	LaneCount is the number of lanes to be used over the main link.
 *
 * @return
 *		- XST_SUCCESS if setting the new lane count was successful.
 *		- XST_DEVICE_NOT_FOUND if no RX is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_SetLaneCount(XDpPsu *InstancePtr, u8 LaneCount)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((LaneCount == XDPPSU_LANE_COUNT_SET_1) ||
					(LaneCount == XDPPSU_LANE_COUNT_SET_2));


	InstancePtr->LinkConfig.LaneCount = LaneCount;

	/* Write the new lane count to the DisplayPort TX core. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_LANE_COUNT_SET,
					InstancePtr->LinkConfig.LaneCount);

	/* Preserve the current RX device settings. */
	Status = XDpPsu_AuxRead(InstancePtr, XDPPSU_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	RegVal &= ~XDPPSU_DPCD_LANE_COUNT_SET_MASK;
	RegVal |= InstancePtr->LinkConfig.LaneCount;

	/* Write the new lane count to the RX device. */
	Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the data rate to be used by the main link for both the
 * DisplayPort TX core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	LinkRate is the link rate to be used over the main link based on
 *		one of the following selects:
 *		- XDPPSU_LINK_BW_SET_162GBPS = 0x06 (for a 1.62 Gbps data rate)
 *		- XDPPSU_LINK_BW_SET_270GBPS = 0x0A (for a 2.70 Gbps data rate)
 *		- XDPPSU_LINK_BW_SET_540GBPS = 0x14 (for a 5.40 Gbps data rate)
 *
 * @return
 *		- XST_SUCCESS if setting the new link rate was successful.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_SetLinkRate(XDpPsu *InstancePtr, u8 LinkRate)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((LinkRate == XDPPSU_LINK_BW_SET_162GBPS) ||
				(LinkRate == XDPPSU_LINK_BW_SET_270GBPS) ||
				(LinkRate == XDPPSU_LINK_BW_SET_540GBPS));

	/* Write a corresponding clock frequency to the DisplayPort TX core. */
	switch (LinkRate) {
	case XDPPSU_LINK_BW_SET_162GBPS:
		Status = XDpPsu_SetPhyClkSpeed(InstancePtr,
						XDPPSU_PHY_CLOCK_SELECT_162GBPS);
		break;
	case XDPPSU_LINK_BW_SET_270GBPS:
		Status = XDpPsu_SetPhyClkSpeed(InstancePtr,
						XDPPSU_PHY_CLOCK_SELECT_270GBPS);
		break;
	case XDPPSU_LINK_BW_SET_540GBPS:
		Status = XDpPsu_SetPhyClkSpeed(InstancePtr,
						XDPPSU_PHY_CLOCK_SELECT_540GBPS);
		break;
	default:
		Status = XST_FAILURE;
		break;
	}
	if (Status != XST_SUCCESS) {
		return Status;
	}

	InstancePtr->LinkConfig.LinkRate = LinkRate;

	/* Write new link rate to the DisplayPort TX core. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_LINK_BW_SET,
					InstancePtr->LinkConfig.LinkRate);

	/* Write new link rate to the RX device. */
	Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_LINK_BW_SET, 0x1,
					&InstancePtr->LinkConfig.LinkRate);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return Status;
}

/******************************************************************************/
/**
 * This function enables or disables scrambling of symbols for both the
 * DisplayPort and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Enable will enable or disable scrambling.
 *
 * @return
 *		- XST_SUCCESS if setting the scrambling enable was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_SetScrambler(XDpPsu *InstancePtr, u8 Enable)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Enable == 1) || (Enable == 0));

	InstancePtr->LinkConfig.ScramblerEn = Enable;

	/* Write scrambler disable to the DisplayPort TX core. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_SCRAMBLING_DISABLE,
							Enable ? 0x0 : 0x1);

	/* Preserve the current RX device settings. */
	Status = XDpPsu_AuxRead(InstancePtr, XDPPSU_DPCD_TP_SET, 0x1, &RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (Enable) {
		RegVal &= ~XDPPSU_DPCD_TP_SET_SCRAMB_DIS_MASK;
	}
	else {
		RegVal |= XDPPSU_DPCD_TP_SET_SCRAMB_DIS_MASK;
	}

	/* Write scrambler disable to the RX device. */
	Status = XDpPsu_AuxWrite(InstancePtr, XDPPSU_DPCD_TP_SET, 0x1, &RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables the main link.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Enable is a control flag to enable or disable main link
 *
 * @return	None.
 *
 * @note	Enable = 1, Enable main link
 *		Enable =0, Disable main link.
 *
*******************************************************************************/
void XDpPsu_EnableMainLink(XDpPsu *InstancePtr, u8 Enable)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Enable == 0) | (Enable == 1));

	/* Reset the scrambler. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr,
				XDPPSU_FORCE_SCRAMBLER_RESET, 0x1);
	/* Enable the main stream. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr,
				XDPPSU_ENABLE_MAIN_STREAM, Enable);
}

/******************************************************************************/
/**
 * This function does a PHY reset.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Reset is the type of reset to assert.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpPsu_ResetPhy(XDpPsu *InstancePtr, u32 Reset)
{
	u32 PhyVal;
	u32 RegVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_ENABLE, 0x0);

	/* Preserve the current PHY settings. */
	PhyVal = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr, XDPPSU_PHY_CONFIG);

	/* Apply reset. */
	RegVal = PhyVal | Reset;
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_PHY_CONFIG, RegVal);

	/* Remove the reset. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_PHY_CONFIG, PhyVal);

	/* Wait for the PHY to be ready. */
	XDpPsu_WaitPhyReady(InstancePtr);

	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_ENABLE, 0x1);
}
