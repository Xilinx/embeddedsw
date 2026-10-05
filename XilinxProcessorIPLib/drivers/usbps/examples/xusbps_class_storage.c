/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xusbps_class_storage.c
 *
 * This file contains the implementation of the storage class code for the
 * example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.00a wgr  10/10/10 First release
 * 2.1   kpc  4/28/14  Align DMA buffers to cache line boundary
 * 2.4   vak  4/01/19  Fixed IAR data_alignment warnings
 * 2.10  ka   8/21/25  Fixed GCC warnings
 * 2.11  ka   01/29/26 Fix codespell errors.
 * 2.12  sm   10/01/26 Added SendCSW() to send pass/fail command status.
 *                     Added ValidateLbaRange() to reject out-of-range and
 *                     handle zero-block SCSI READ(10)/WRITE(10), and
 *                     to reject a CBW transfer length or direction that
 *                     does not match the command.
 *                     Added ValidateDataLen() to reject WRITE(10) data
 *                     exceeding the command length.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>

#include "xusbps.h"		/* USB controller driver */

#include "xusbps_ch9_storage.h"
#include "xusbps_ch9.h"
#include "xusbps_class_storage.h"
#include "xil_printf.h"

/* #define CLASS_STORAGE_DEBUG */

#ifdef CLASS_STORAGE_DEBUG
#define printf xil_printf
#endif

/************************** Constant Definitions *****************************/


#define USB_CSW_SIGNATURE	0x53425355U
#define USB_CSW_CMD_PASSED	0U
#define USB_CSW_CMD_FAILED	1U
#define USB_CBW_FLAGS_DIR_IN	0x80U

/************************** Function Prototypes ******************************/

static void SendCSW(XUsbPs *InstancePtr, USB_CBW *CBW, u32 Residue,
		    u8 Status);
static s32 ValidateLbaRange(XUsbPs *InstancePtr, USB_CBW *CBW, u32 Lba,
			    u16 BlockCount, u8 Direction);
static s32 ValidateDataLen(XUsbPs *InstancePtr, USB_CBW *CBW,
			   u32 BufferLen, u32 BytesLeft);

/************************** Variable Definitions *****************************/

/* Pre-manufactured response to the SCSI Inquirey command.
 */
#ifdef __ICCARM__
#pragma data_alignment = 32
const static SCSI_INQUIRY scsiInquiry = {
	0x00,
	0x80,
	0x00,
	0x01,
	0x1f,
	0x00,
	0x00,
	0x00,
	{"Xilinx  "},		/* Vendor ID:  must be  8 characters long. */
	{"PS USB VirtDisk"},	/* Product ID: must be 16 characters long. */
	{"1.00"}		/* Revision:   must be  4 characters long. */
};

#pragma data_alignment = 32
static u8 MaxLUN = 0;

#pragma data_alignment = 32
/* Buffer for virtual flash disk space. */
static u8 VirtFlash[VFLASH_SIZE];

#pragma data_alignment = 32
static USB_CBW lastCBW;

#pragma data_alignment = 32
/* Local transmit buffer for simple replies. */
static u8 txBuffer[128];

#pragma data_alignment = 32
/* Command Status Wrapper buffer. */
static USB_CSW CSW;
#else
static const SCSI_INQUIRY scsiInquiry ALIGNMENT_CACHELINE = {
	0x00,
	0x80,
	0x00,
	0x01,
	0x1f,
	0x00,
	0x00,
	0x00,
	{"Xilinx  "},		/* Vendor ID:  must be  8 characters long. */
	{"PS USB VirtDisk"},	/* Product ID: must be 16 characters long. */
	{"1.00"}		/* Revision:   must be  4 characters long. */
};
static u8 MaxLUN ALIGNMENT_CACHELINE = 0;
/** @cond NODOC */
/* Buffer for virtual flash disk space. */
static u8 VirtFlash[VFLASH_SIZE] ALIGNMENT_CACHELINE;

static USB_CBW lastCBW ALIGNMENT_CACHELINE;

/**< Local transmit buffer for simple replies. */
static u8 txBuffer[128] ALIGNMENT_CACHELINE;

/**< Command Status Wrapper buffer. */
static USB_CSW CSW ALIGNMENT_CACHELINE;
/** @endcond */
#endif

/*****************************************************************************/
/**
* This function handles Reduced Block Command (RBC) requests from the host.
*
* @param[in] InstancePtr is a pointer to XUsbPs instance of the controller.
* @param[in] EpNum is the number of the endpoint on which the RBC was received.
* @param[in] BufferPtr is the data buffer containing the RBC or data.
* @param[in] BufferLen is the length of the data buffer.
*
******************************************************************************/
void XUsbPs_HandleStorageReq(XUsbPs *InstancePtr, u8 EpNum,
			     u8 *BufferPtr, u32 BufferLen)
{
	USB_CBW	*CBW;
	u32	Offset;
	u32	Lba;
	u16	BlockCount;
	s32	Status;
	static u8 *VirtFlashWritePointer = VirtFlash;
	/* Static variables used for data transfers.*/
	static int	rxBytesLeft;

	/**< Current SCSI machine state. */
	static int	phase = USB_EP_STATE_COMMAND;

	(void)EpNum;

	/* COMMAND phase. */
	if (USB_EP_STATE_COMMAND == phase) {
		CBW = (USB_CBW *) BufferPtr;

		switch (CBW->CBWCB[0]) {
			case USB_RBC_INQUIRY:
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: INQUIRY\n");
#endif
				XUsbPs_EpBufferSend(InstancePtr, 1,
						    (void *) &scsiInquiry,
						    sizeof(scsiInquiry));
				/* Send Success Status 	 */
				CBW->dCBWSignature = 0x55534253;
				CBW->dCBWDataTransferLength = 0;
				CBW->bmCBWFlags = 0;

				XUsbPs_EpBufferSend(InstancePtr, 1, (void *) CBW, 13);
				break;


			case USB_UFI_GET_CAP_LIST: {
					SCSI_CAP_LIST	*CapList;

					CapList = (SCSI_CAP_LIST *) txBuffer;
#ifdef CLASS_STORAGE_DEBUG
					printf("SCSI: CAPLIST\n");
#endif
					CapList->listLength	= 8;
					CapList->descCode	= 3;
					CapList->numBlocks	= htonl(VFLASH_NUM_BLOCKS);
					CapList->blockLength	= htons(VFLASH_BLOCK_SIZE);
					XUsbPs_EpBufferSend(InstancePtr, 1, txBuffer,
							    sizeof(SCSI_CAP_LIST));
					/* Send Success Status
					 */
					CBW->dCBWSignature = 0x55534253;
					CBW->dCBWDataTransferLength =
						be2le(be2le(CBW->dCBWDataTransferLength) -
						      sizeof(SCSI_CAP_LIST));
					CBW->bmCBWFlags = 0;

					XUsbPs_EpBufferSend(InstancePtr, 1, (u8 *) CBW, 13);
					break;
				}

			case USB_RBC_READ_CAP: {
					SCSI_READ_CAPACITY	*Cap;

					Cap = (SCSI_READ_CAPACITY *) txBuffer;
#ifdef CLASS_STORAGE_DEBUG
					printf("SCSI: READCAP\n");
#endif
					Cap->numBlocks = htonl(VFLASH_NUM_BLOCKS - 1);
					Cap->blockSize = htonl(VFLASH_BLOCK_SIZE);
					XUsbPs_EpBufferSend(InstancePtr, 1, txBuffer,
							    sizeof(SCSI_READ_CAPACITY));
					/* Send Success Status  */
					CBW->dCBWSignature = 0x55534253;
					CBW->dCBWDataTransferLength = 0;
					CBW->bmCBWFlags = 0;

					XUsbPs_EpBufferSend(InstancePtr, 1, (u8 *) CBW, 13);
					break;
				}

			case USB_RBC_READ:
				Lba = htonl(((SCSI_READ_WRITE *) CBW->CBWCB)->block);
				BlockCount = htons(((SCSI_READ_WRITE *) CBW->CBWCB)->length);
				Status = ValidateLbaRange(InstancePtr, CBW, Lba, BlockCount,
							  XUSBPS_EP_DIRECTION_IN);
				if (Status != XST_SUCCESS) {
					break;
				}
				Offset = Lba * VFLASH_BLOCK_SIZE;
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: READ Offset 0x%08x\n", (int) Offset);
#endif
				XUsbPs_EpBufferSend(InstancePtr, 1, &VirtFlash[Offset],
						    BlockCount * VFLASH_BLOCK_SIZE);
				/* Send Success Status */
				CBW->dCBWSignature = 0x55534253;
				CBW->dCBWDataTransferLength = 0;
				CBW->bmCBWFlags = 0;

				XUsbPs_EpBufferSend(InstancePtr, 1, (u8 *) CBW, 13);
				break;

			case USB_RBC_MODE_SENSE:
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: MODE SENSE\n");
#endif
				XUsbPs_EpBufferSend(InstancePtr, 1,
						    (u8 *) "\003\000\000\000", 4);

				/* Send Success Status */
				CBW->dCBWSignature = 0x55534253;
				CBW->dCBWDataTransferLength =
					be2le(be2le(CBW->dCBWDataTransferLength) - 4);
				CBW->bmCBWFlags = 0;

				XUsbPs_EpBufferSend(InstancePtr, 1, (u8 *) CBW, 13);
				break;


			case USB_RBC_TEST_UNIT_READY:
			case USB_RBC_MEDIUM_REMOVAL:
			case USB_RBC_VERIFY:
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: TEST UNIT READY\n");
#endif
				/* Send Success Status */
				CBW->dCBWSignature = 0x55534253;
				CBW->dCBWDataTransferLength = 0;
				CBW->bmCBWFlags = 0;

				XUsbPs_EpBufferSend(InstancePtr, 1, (u8 *) CBW, 13);
				break;


			case USB_RBC_WRITE:
				Lba = htonl(((SCSI_READ_WRITE *) CBW->CBWCB)->block);
				BlockCount = htons(((SCSI_READ_WRITE *) CBW->CBWCB)->length);
				Status = ValidateLbaRange(InstancePtr, CBW, Lba, BlockCount,
							  XUSBPS_EP_DIRECTION_OUT);
				if (Status != XST_SUCCESS) {
					break;
				}
				Offset = Lba * VFLASH_BLOCK_SIZE;
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: WRITE Offset 0x%08x\n", (int) Offset);
#endif
				VirtFlashWritePointer = &VirtFlash[Offset];
				/* Save the CBW for the DATA and STATUS phases. */
				lastCBW = *CBW;
				rxBytesLeft = BlockCount * VFLASH_BLOCK_SIZE;

				phase = USB_EP_STATE_DATA;
				break;


			case USB_RBC_STARTSTOP_UNIT: {
					u8 immed;

					immed = ((SCSI_START_STOP *) CBW->CBWCB)->immed;
#ifdef CLASS_STORAGE_DEBUG
					printf("SCSI: START/STOP unit: immed %02x\n", immed);
#endif
					/* If the immediate bit is 0 we are supposed to send
					 * a success status.
					 */
					if (0 == (immed & 0x01)) {
						/* Send Success Status */
						CBW->dCBWSignature = 0x55534253;
						CBW->dCBWDataTransferLength = 0;
						CBW->bmCBWFlags = 0;

						XUsbPs_EpBufferSend(InstancePtr, 1,
								    (u8 *) CBW, 13);
					}
					break;
				}


			/* Commands that we do not support for this example. */
			case 0x04:	/* Format Unit */
			case 0x15:	/* Mode Select */
			case 0x5e:	/* Persistent Reserve In */
			case 0x5f:	/* Persistent Reserve Out */
			case 0x17:	/* Release */
			case 0x03:	/* Request Sense */
			case 0x16:	/* Reserve */
			case 0x35:	/* Sync Cache */
			case 0x3b:	/* Write Buffer */
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: Got unhandled command %02x\n", CBW->CBWCB[0]);
#endif
			default:
				break;
		}
	}
	/* DATA phase.
	 */
	else if (USB_EP_STATE_DATA == phase) {
		switch (lastCBW.CBWCB[0]) {
			case USB_RBC_WRITE:
				Status = ValidateDataLen(InstancePtr, &lastCBW, BufferLen,
							 (u32)rxBytesLeft);
				if (Status != XST_SUCCESS) {
					phase = USB_EP_STATE_COMMAND;
					break;
				}
				/* Copy the data we just read into the VirtFlash buffer. */
				memcpy(VirtFlashWritePointer, BufferPtr, BufferLen);
				VirtFlashWritePointer += BufferLen;

				rxBytesLeft -= BufferLen;

				if (rxBytesLeft <= 0) {
					/* Send Success Status */
					lastCBW.dCBWSignature = 0x55534253;
					lastCBW.dCBWDataTransferLength = 0;
					lastCBW.bmCBWFlags = 0;

					XUsbPs_EpBufferSend(InstancePtr, 1,
							    (void *) &lastCBW, 13);

					phase = USB_EP_STATE_COMMAND;
				}
				break;
		}
	}
}


/*****************************************************************************/
/*
* This function handles a Storage Class Setup request from the host.
*
* @param[in] InstancePtr is a pointer to XUsbPs instance of the controller.
* @param[in] SetupData is the setup data structure containing the setup
*		request.
*
******************************************************************************/
void XUsbPs_ClassReq(XUsbPs *InstancePtr, XUsbPs_SetupData *SetupData)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData   != NULL);


	switch (SetupData->bRequest) {

		case XUSBPS_CLASSREQ_MASS_STORAGE_RESET:
			XUsbPs_EpBufferSend(InstancePtr, 0, NULL, 0);
			break;

		case XUSBPS_CLASSREQ_GET_MAX_LUN:
			XUsbPs_EpBufferSend(InstancePtr, 0, &MaxLUN, 1);
			break;

		default:
			XUsbPs_EpStall(InstancePtr, 0, XUSBPS_EP_DIRECTION_IN);
			break;
	}
}

/*****************************************************************************/
/**
* This function sends the Command Status Wrapper of a mass storage command
* on bulk IN. The CSW is built in a dedicated buffer so the received CBW is
* left intact.
*
* @param[in] InstancePtr is a pointer to XUsbPs instance of the controller.
* @param[in] CBW is the Command Block Wrapper of the completed command.
* @param[in] Residue is the number of requested bytes not transferred.
* @param[in] Status is the command status, USB_CSW_CMD_PASSED or
*		USB_CSW_CMD_FAILED.
*
******************************************************************************/
static void SendCSW(XUsbPs *InstancePtr, USB_CBW *CBW, u32 Residue,
		    u8 Status)
{
	CSW.dCSWSignature = USB_CSW_SIGNATURE;
	CSW.dCSWTag = CBW->dCBWTag;
	CSW.dCSWDataResidue = Residue;
	CSW.bCSWStatus = Status;

	XUsbPs_EpBufferSend(InstancePtr, 1, (u8 *) &CSW, sizeof(USB_CSW));
}

/*****************************************************************************/
/**
* This function validates the LBA range of a SCSI READ(10) or WRITE(10)
* command against the virtual flash. A command whose range exceeds
* VFLASH_NUM_BLOCKS is rejected. A command with a zero block count transfers
* no data, so it is completed here with a good status, or rejected when the
* host expects a data phase. A command whose CBW data transfer length or
* direction does not match the block count and the command direction is
* also rejected, so the data phase never runs out of step with the host.
*
* A rejected command fails with the whole requested data length as residue.
* When the host expects a data phase, the endpoint of the direction given in
* the CBW is halted first. The status is then queued on bulk IN, where the
* controller holds it until the host clears the halt when bulk IN is the
* stalled endpoint.
*
* @param[in] InstancePtr is a pointer to XUsbPs instance of the controller.
* @param[in] CBW is the Command Block Wrapper of the command.
* @param[in] Lba is the starting logical block address.
* @param[in] BlockCount is the number of blocks to transfer.
* @param[in] Direction is the data phase direction, XUSBPS_EP_DIRECTION_IN
*		or XUSBPS_EP_DIRECTION_OUT.
*
* @return
*		- XST_SUCCESS if the data transfer can proceed.
*		- XST_FAILURE if the command has already been completed.
*
******************************************************************************/
static s32 ValidateLbaRange(XUsbPs *InstancePtr, USB_CBW *CBW, u32 Lba,
			    u16 BlockCount, u8 Direction)
{
	u8 HostDirection;

	HostDirection = (CBW->bmCBWFlags & USB_CBW_FLAGS_DIR_IN) ?
			XUSBPS_EP_DIRECTION_IN : XUSBPS_EP_DIRECTION_OUT;

	if (BlockCount == 0) {
		if (CBW->dCBWDataTransferLength != 0) {
			XUsbPs_EpStall(InstancePtr, 1, HostDirection);
			SendCSW(InstancePtr, CBW, CBW->dCBWDataTransferLength,
				USB_CSW_CMD_FAILED);
		} else {
			SendCSW(InstancePtr, CBW, 0, USB_CSW_CMD_PASSED);
		}
		return XST_FAILURE;
	} else if (Lba >= VFLASH_NUM_BLOCKS ||
		   BlockCount > (VFLASH_NUM_BLOCKS - Lba)) {
#ifdef CLASS_STORAGE_DEBUG
		printf("LBA EXCEEDS DEVICE CAPACITY\r\n");
#endif
		if (CBW->dCBWDataTransferLength != 0) {
			XUsbPs_EpStall(InstancePtr, 1, HostDirection);
		}
		SendCSW(InstancePtr, CBW, CBW->dCBWDataTransferLength,
			USB_CSW_CMD_FAILED);
		return XST_FAILURE;
	} else if (CBW->dCBWDataTransferLength !=
		   (u32)BlockCount * VFLASH_BLOCK_SIZE ||
		   HostDirection != Direction) {
#ifdef CLASS_STORAGE_DEBUG
		printf("CBW TRANSFER LENGTH MISMATCH\r\n");
#endif
		if (CBW->dCBWDataTransferLength != 0) {
			XUsbPs_EpStall(InstancePtr, 1, HostDirection);
		}
		SendCSW(InstancePtr, CBW, CBW->dCBWDataTransferLength,
			USB_CSW_CMD_FAILED);
		return XST_FAILURE;
	} else {
		return XST_SUCCESS;
	}
}

/*****************************************************************************/
/**
* This function validates the length of a bulk OUT buffer received in the
* data phase of a SCSI WRITE(10) command against the bytes still expected
* for the command. A buffer exceeding the remaining length is rejected before
* it is copied to the virtual flash.
*
* A rejected command fails with the CBW data transfer length less the bytes
* already copied as residue. Bulk OUT is halted first, and the status is then
* queued on bulk IN.
*
* @param[in] InstancePtr is a pointer to XUsbPs instance of the controller.
* @param[in] CBW is the Command Block Wrapper of the command.
* @param[in] BufferLen is the length of the received data buffer.
* @param[in] BytesLeft is the number of bytes still expected.
*
* @return
*		- XST_SUCCESS if the buffer can be copied.
*		- XST_FAILURE if the command has already been completed.
*
******************************************************************************/
static s32 ValidateDataLen(XUsbPs *InstancePtr, USB_CBW *CBW,
			   u32 BufferLen, u32 BytesLeft)
{
	u32 Copied;
	u32 Residue;

	if (BufferLen > BytesLeft) {
#ifdef CLASS_STORAGE_DEBUG
		printf("WRITE DATA EXCEEDS COMMAND LENGTH\r\n");
#endif
		Copied = (u32)htons(((SCSI_READ_WRITE *) CBW->CBWCB)->length) *
			 VFLASH_BLOCK_SIZE - BytesLeft;
		Residue = (CBW->dCBWDataTransferLength > Copied) ?
			  (CBW->dCBWDataTransferLength - Copied) : 0U;

		XUsbPs_EpStall(InstancePtr, 1, XUSBPS_EP_DIRECTION_OUT);
		SendCSW(InstancePtr, CBW, Residue, USB_CSW_CMD_FAILED);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
