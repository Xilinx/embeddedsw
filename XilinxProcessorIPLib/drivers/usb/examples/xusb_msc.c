/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xusb_msc.c
 *
 * This file contains the mass-storage class helpers used by the mass-storage
 * examples.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 5.7   sm   10/01/26 First release. Added XUsb_MsRejectCommand() and
 *			XUsb_MsSetSense().
 *
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb_msc.h"

/************************** Constant Definitions *****************************/

/*
 * Fixed format sense data byte offsets and values.
 */
#define SENSE_RESPONSE_CODE_OFFSET	0
#define SENSE_KEY_OFFSET		2
#define SENSE_ADDITIONAL_LENGTH_OFFSET	7
#define SENSE_ASC_OFFSET		12
#define SENSE_ASCQ_OFFSET		13

#define SENSE_RESPONSE_CODE		0x70
#define SENSE_ADDITIONAL_LENGTH		(XUSB_MSC_SENSE_LEN - 8)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * This function rejects a mass storage command that cannot be serviced. The
 * command fails with the whole requested data length as residue and the data
 * phase endpoint is halted: bulk IN for READ, bulk OUT for WRITE. The status
 * is then queued on bulk IN by the caller, where the core holds it until the
 * host clears the halt when bulk IN is the stalled endpoint.
 *
 * @param[in]	InstancePtr is a pointer to the XUsb instance.
 * @param[in]	EndPoint is the endpoint carrying the data phase.
 * @param[in]	TransferLength is the data transfer length from the Command
 *		Block Wrapper.
 * @param[out]	CswStatus is a pointer to the status field of the Command
 *		Status Wrapper.
 * @param[out]	CswResidue is a pointer to the data residue field of the
 *		Command Status Wrapper.
 * @param[out]	BlockCount is a pointer to the pending transfer block count.
 *		It is cleared when non-NULL.
 * @param[out]	CmdFlag is a pointer to the command-in-progress flag. It is
 *		cleared when non-NULL.
 * @param[out]	SendStatus is a pointer to the flag that requests the status
 *		phase. It is set when non-NULL.
 *
 * @return	None.
 *
 * @note	Pass NULL for SendStatus when the example does not keep that
 *		flag. The caller queues the CSW and records it only after
 *		that send succeeds.
 *
 ******************************************************************************/
void XUsb_MsRejectCommand(XUsb *InstancePtr, u8 EndPoint,
			  u32 TransferLength, u8 *CswStatus, u32 *CswResidue,
			  volatile u32 *BlockCount, volatile u8 *CmdFlag,
			  volatile u8 *SendStatus)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CswStatus != NULL);
	Xil_AssertVoid(CswResidue != NULL);

	*CswStatus = XUSB_MSC_CSW_FAILED;
	*CswResidue = TransferLength;

	if (BlockCount != NULL) {
		*BlockCount = 0;
	}

	if (CmdFlag != NULL) {
		*CmdFlag = 0;
	}

	if (SendStatus != NULL) {
		*SendStatus = TRUE;
	}

	XUsb_EpStall(InstancePtr, EndPoint);
}

/*****************************************************************************/
/**
 * This function loads the fixed format sense data returned by the next
 * REQUEST SENSE command.
 *
 * @param[out]	SenseData is a pointer to the XUSB_MSC_SENSE_LEN byte sense
 *		data buffer.
 * @param[in]	SenseKey is the sense key.
 * @param[in]	Asc is the additional sense code.
 * @param[in]	Ascq is the additional sense code qualifier.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XUsb_MsSetSense(u8 *SenseData, u8 SenseKey, u8 Asc, u8 Ascq)
{
	u32 Index;

	Xil_AssertVoid(SenseData != NULL);

	for (Index = 0; Index < XUSB_MSC_SENSE_LEN; Index++) {
		SenseData[Index] = 0;
	}

	SenseData[SENSE_RESPONSE_CODE_OFFSET] = SENSE_RESPONSE_CODE;
	SenseData[SENSE_KEY_OFFSET] = SenseKey;
	SenseData[SENSE_ADDITIONAL_LENGTH_OFFSET] = SENSE_ADDITIONAL_LENGTH;
	SenseData[SENSE_ASC_OFFSET] = Asc;
	SenseData[SENSE_ASCQ_OFFSET] = Ascq;
}
