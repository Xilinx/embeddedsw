/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_class_storage.c
 *
 * This file contains the implementation of the Mass Storage specific class
 * code for the example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   sg  06/06/16  First release
 *       ms  04/10/17  Modified filename tag to include the file in doxygen
 *                     examples.
 * 1.4   BK  12/01/18  Renamed the file and added changes to have a common
 *		       example for all USB IPs.
 * 1.5	  vak 13/02/19  Added support for versal
 * 1.5    vak 03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 * 1.8   pm   15/09/20 Fixed C++ Compilation error.
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xusb_class_storage.h"
#include "xparameters.h"
#include "xusb_ch9_storage.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern u8 Phase;
extern u8 VirtFlash[];

/*
 * Pre-manufactured response to the SCSI Inquiry command.
 */
#if __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
#else
#pragma data_alignment = 32
#endif
const static SCSI_INQUIRY scsiInquiry[] = {
#else
const static SCSI_INQUIRY scsiInquiry[] ALIGNMENT_CACHELINE = {
#endif
	{
		0x00,
		0x80,
		0x00,
		0x01,
		0x1f,
		0x00,
		0x00,
		0x00,
		{"Xilinx "},			/* Vendor ID:  must be  8 characters long. */
		{"PS USB VirtDisk"},	/* Product ID: must be 16 characters long. */
		{"1.0"}				/* Revision:   must be  4 characters long. */
	},
	{
		0x00,
		0x80,
		0x02,
		0x02,
		0x1F,
		0x00,
		0x00,
		0x00,
		{"Linux  "},			/* Vendor ID:  must be  8 characters long. */
		{"File-StorGadget"},	/* Product ID: must be 16 characters long. */
		{"040"}				/* Revision:   must be  4 characters long. */
	}
};

#ifdef __ICCARM__
static u8 MaxLUN = 0;
#else
static u8 MaxLUN ALIGNMENT_CACHELINE = 0;
#endif

extern USB_CBW CBW;
extern USB_CSW CSW;

extern u32	rxBytesLeft;
extern u8	*VirtFlashWritePointer;

/* Local transmit buffer for simple replies. */
#ifdef __ICCARM__
static u8 txBuffer[128];
#else
static u8 txBuffer[128] ALIGNMENT_CACHELINE;
#endif

/*****************************************************************************/
/**
* This function is class handler for Mass storage and is called when
* Setup packet received is for Class request(not Standard Device request)
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	SetupData is pointer to SetupPacket received.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData   != NULL);

	switch (SetupData->bRequest) {
		case USB_CLASSREQ_MASS_STORAGE_RESET:
			/* For Control transfers, Status Phase is handled by driver */

			EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
			break;

		case USB_CLASSREQ_GET_MAX_LUN:
			EpBufferSend(InstancePtr->PrivateData, 0, &MaxLUN, 1);
			break;

		default:
			/*
			 * Unsupported command. Stall the end point.
			 */
			EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
			break;
	}
}

/*****************************************************************************/
/**
* This function handles Reduced Block Command (RBC) requests from the host.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void ParseCBW(struct Usb_DevData *InstancePtr)
{
	u32	Offset;
	u8 Array[50];
	u8 Index;
	s32 Status;

	switch (CBW.CBWCB[0]) {
		case USB_RBC_INQUIRY: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: INQUIRY\r\n");
#endif
				Phase = USB_EP_STATE_DATA_IN;

				Status = IsSuperSpeed(InstancePtr);
				if (Status != XST_SUCCESS) {
					/* USB 2.0 */
					Index = 0;
				} else {
					/* USB 3.0 */
					Index = 1;
				}

				EpBufferSend(InstancePtr->PrivateData, 1,
					     (u8 *) &scsiInquiry[Index],
					     sizeof(scsiInquiry[Index]));
				break;
			}

		case USB_UFI_GET_CAP_LIST: {
				SCSI_CAP_LIST	*CapList;

				CapList = (SCSI_CAP_LIST *) txBuffer;
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: CAPLIST\r\n");
#endif
				CapList->listLength	= 8;
				CapList->descCode	= 3;
				CapList->numBlocks	= htonl(VFLASH_NUM_BLOCKS);
				CapList->blockLength = htons(VFLASH_BLOCK_SIZE);

				Phase = USB_EP_STATE_DATA_IN;
				EpBufferSend(InstancePtr->PrivateData, 1, txBuffer,
					     sizeof(SCSI_CAP_LIST));

				break;
			}

		case USB_RBC_READ_CAP: {
				SCSI_READ_CAPACITY	*Cap;

				Cap = (SCSI_READ_CAPACITY *) txBuffer;
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: READCAP\r\n");
#endif
				Cap->numBlocks = htonl(VFLASH_NUM_BLOCKS - 1);
				Cap->blockSize = htonl(VFLASH_BLOCK_SIZE);
				Phase = USB_EP_STATE_DATA_IN;
				EpBufferSend(InstancePtr->PrivateData, 1, txBuffer,
					     sizeof(SCSI_READ_CAPACITY));

				break;
			}

		case USB_RBC_READ: {
				Offset = htonl(((SCSI_READ_WRITE *) &CBW.CBWCB)->block) *
					 VFLASH_BLOCK_SIZE;
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: READ Offset 0x%08x\r\n", Offset);
#endif

				Phase = USB_EP_STATE_DATA_IN;
				u32 RetVal = EpBufferSend(InstancePtr->PrivateData, 1,
							  &VirtFlash[Offset],
							  htons(((SCSI_READ_WRITE *) &CBW.CBWCB)->
								length) * VFLASH_BLOCK_SIZE);
				if (RetVal != XST_SUCCESS) {
					xil_printf("Failed: READ Offset 0x%08x\n",
						   Offset);
					return;
				}
				break;
			}
		case USB_RBC_MODE_SENSE: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: MODE SENSE\r\n");
#endif
				Phase = USB_EP_STATE_DATA_IN;
				EpBufferSend(InstancePtr->PrivateData, 1,
					     (u8 *) "\003\000\000\000", 4);
				break;
			}
		case USB_RBC_MODE_SELECT: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: MODE_SELECT\r\n");
#endif
				Phase = USB_EP_STATE_DATA_OUT;
				EpBufferRecv(InstancePtr->PrivateData,
					     1, (u8 *)Array, 24);
				break;
			}
		case USB_RBC_TEST_UNIT_READY: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: TEST UNIT READY\r\n");
#endif
				SendCSW(InstancePtr, 0);
				break;
			}
		case USB_RBC_MEDIUM_REMOVAL: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: MEDIUM REMOVAL\r\n");
#endif
				SendCSW(InstancePtr, 0);
				break;
			}
		case USB_RBC_VERIFY: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: VERIFY\n");
#endif
				SendCSW(InstancePtr, 0);
				break;
			}
		case USB_RBC_WRITE: {
				Offset = htonl(((SCSI_READ_WRITE *) &CBW.CBWCB)->
					       block) * VFLASH_BLOCK_SIZE;
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: WRITE Offset 0x%08x\r\n", Offset);
#endif
				VirtFlashWritePointer = &VirtFlash[Offset];

				rxBytesLeft = htons(((SCSI_READ_WRITE *) &CBW.CBWCB)->length)
					      * VFLASH_BLOCK_SIZE;

				Phase = USB_EP_STATE_DATA_OUT;
				EpBufferRecv(InstancePtr->PrivateData, 1,
					     &VirtFlash[Offset], rxBytesLeft);
				break;
			}
		case USB_RBC_STARTSTOP_UNIT: {
				u8 immed;

				immed = ((SCSI_START_STOP *) &CBW.CBWCB)->immed;
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: START/STOP unit: immed %02x\r\n", immed);
#endif
				/* If the immediate bit is 0 we are supposed to send
				 * a success status.
				 */
				if (0 == (immed & 0x01)) {
					SendCSW(InstancePtr, 0);
				}
				break;
			}

		case USB_RBC_REQUEST_SENSE: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: REQUEST_SENSE\r\n");
#endif
				break;
			}
		case USB_SYNC_SCSI: {
#ifdef CLASS_STORAGE_DEBUG
				printf("SCSI: SYNCHRONISE_SCSI\r\n");
#endif
				SendCSW(InstancePtr, 0);
				break;
			}
	}
}

/****************************************************************************/
/**
* This function is used to send SCSI Command Status Wrapper to Host.
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	Length is the data residue.
*
* @return	None
*
* @note
*
*****************************************************************************/
void SendCSW(struct Usb_DevData *InstancePtr, u32 Length)
{
	CSW.dCSWSignature = 0x53425355;
	CSW.dCSWTag = CBW.dCBWTag;
	CSW.dCSWDataResidue = Length;
	CSW.bCSWStatus = 0;
	Phase = USB_EP_STATE_STATUS;
	EpBufferSend(InstancePtr->PrivateData, 1, (u8 *) &CSW, 13);
}
