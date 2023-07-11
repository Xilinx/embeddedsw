/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freerots_class_storage.c
 *
 * This file contains the implementation of the Mass Storage specific class
 * code for the example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/03/18 First release
 * 1.5   vak  13/02/19 Added support for versal
 * 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "FreeRTOS.h"
#include "task.h"
#include "xusb_freertos_ch9_storage.h"
#include "xusb_freertos_class_storage.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
TaskHandle_t xMainTask;

/* Pre-manufactured response to the SCSI Inquiry command */
#ifdef __ICCARM__
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
		{"Xilinx  "},		/* Vendor ID */
		{"PS USB VirtDisk"},	/* Product ID */
		{"1.00"}		/* Revision */
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
		{"Xilinx  "},		/* Vendor ID */
		{"PS USB VirtDisk"},	/* Product ID */
		{"1.00"}		/* Revision */
	}
};

#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
static u8 MaxLUN = 0;

#pragma data_alignment = 64
/* Local transmit buffer for simple replies. */
static u8 txBuffer[128];
#else
#pragma data_alignment = 32
static u8 MaxLUN = 0;

#pragma data_alignment = 32
/* Local transmit buffer for simple replies. */
static u8 txBuffer[128];
#endif
#else
static u8 MaxLUN ALIGNMENT_CACHELINE = 0;

/* Local transmit buffer for simple replies. */
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
			EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
			break;

		case USB_CLASSREQ_GET_MAX_LUN:
			EpBufferSend(InstancePtr->PrivateData, 0, &MaxLUN, 1);
			break;

		default:
			/* Unsupported command. Stall the end point */
			EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
			break;
	}
}

/*****************************************************************************/
/**
* This function handles Reduced Block Command (RBC) requests from the host.
*
* @param	InstancePtr is a pointer to Usb_DevData instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void ParseCBW(struct Usb_DevData *InstancePtr, struct storage_dev *dev)
{
	u32	Offset;
	u8	Index;

	switch (dev->cbw.CBWCB[0]) {
		case USB_RBC_INQUIRY:
			dev->phase = USB_EP_STATE_DATA_IN;
			Index = (IsSuperSpeed(InstancePtr) != XST_SUCCESS) ? 0 : 1;
			dev->currTrans.Ptr = (u8 *) &scsiInquiry[Index];
			dev->currTrans.Length = sizeof(scsiInquiry[Index]);
			break;

		case USB_UFI_GET_CAP_LIST: {
				SCSI_CAP_LIST	*CapList;

				CapList = (SCSI_CAP_LIST *) txBuffer;
				CapList->listLength	= 8;
				CapList->descCode	= 3;
				CapList->numBlocks	= htonl(STORAGE_NUM_BLOCKS);
				CapList->blockLength = htons(STORAGE_BLOCK_SIZE);
				dev->phase = USB_EP_STATE_DATA_IN;
				dev->currTrans.Ptr = txBuffer;
				dev->currTrans.Length =  sizeof(SCSI_CAP_LIST);
			}
			break;

		case USB_RBC_READ_CAP: {
				SCSI_READ_CAPACITY	*Cap;

				Cap = (SCSI_READ_CAPACITY *) txBuffer;
				Cap->numBlocks = htonl(STORAGE_NUM_BLOCKS - 1);
				Cap->blockSize = htonl(STORAGE_BLOCK_SIZE);
				dev->phase = USB_EP_STATE_DATA_IN;
				dev->currTrans.Ptr = txBuffer;
				dev->currTrans.Length = sizeof(SCSI_READ_CAPACITY);
			}
			break;

		case USB_RBC_READ:
			Offset = htonl(((SCSI_READ_WRITE *) &dev->cbw.CBWCB)->block) *
				 STORAGE_BLOCK_SIZE;
			dev->phase = USB_EP_STATE_DATA_IN;
			dev->currTrans.Ptr = dev->disk + Offset;
			dev->currTrans.Length =
				htons(((SCSI_READ_WRITE *) &dev->cbw.CBWCB)->length) *
				STORAGE_BLOCK_SIZE;
			break;

		case USB_RBC_MODE_SENSE:
			memcpy(txBuffer, "\003\000\000\000", 4);
			dev->phase = USB_EP_STATE_DATA_IN;
			dev->currTrans.Ptr = txBuffer;
			dev->currTrans.Length = 4;
			break;

		case USB_RBC_MODE_SELECT:
			dev->phase = USB_EP_STATE_DATA_OUT;
			dev->currTrans.Ptr = txBuffer;
			dev->currTrans.Length = 24;
			break;

		case USB_RBC_TEST_UNIT_READY:
			dev->phase = USB_EP_STATE_STATUS;
			dev->currTrans.Length = 0;
			break;

		case USB_RBC_MEDIUM_REMOVAL:
			dev->phase = USB_EP_STATE_STATUS;
			dev->currTrans.Length = 0;
			break;

		case USB_RBC_VERIFY:
			dev->phase = USB_EP_STATE_STATUS;
			dev->currTrans.Length = 0;
			break;

		case USB_RBC_WRITE:
			Offset = htonl(((SCSI_READ_WRITE *) &dev->cbw.CBWCB)->block) *
				 STORAGE_BLOCK_SIZE;
			dev->phase = USB_EP_STATE_DATA_OUT;
			dev->currTrans.Ptr = dev->disk + Offset;
			dev->currTrans.Length =
				htons(((SCSI_READ_WRITE *) &dev->cbw.CBWCB)->length) *
				STORAGE_BLOCK_SIZE;
			break;

		case USB_RBC_STARTSTOP_UNIT: {
				u8 immed;

				immed = ((SCSI_START_STOP *) &dev->cbw.CBWCB)->immed;
				/* If the immediate bit is 0 we are supposed to send
				 * a success status.
				 */
				if (0 == (immed & 0x01)) {
					dev->phase = USB_EP_STATE_STATUS;
					dev->currTrans.Length = 0;
				}
				break;
			}

		case USB_SYNC_SCSI:
			dev->phase = USB_EP_STATE_STATUS;
			dev->currTrans.Length = 0;
			break;
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
void SendCSW(struct Usb_DevData *InstancePtr, struct storage_dev *dev,
	     u32 Length)
{
	dev->csw.dCSWSignature = 0x53425355;
	dev->csw.dCSWTag = dev->cbw.dCBWTag;
	dev->csw.dCSWDataResidue = Length;
	dev->csw.bCSWStatus = 0;
	dev->phase = USB_EP_STATE_STATUS;

	EpBufferSend(InstancePtr->PrivateData, STORAGE_EP,
		     (void *) &dev->csw, 13);
}

/****************************************************************************/
/**
* This task implements SCSI command functionality. task will get command from
* host and act accordingly
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
void prvSCSITask(void *pvParameters)
{
	u16 MaxPktSize;
	struct Usb_DevData *InstancePtr = pvParameters;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct storage_dev *dev = (struct storage_dev *)ch9_ptr->data_ptr;

	if (InstancePtr->Speed == USB_SPEED_SUPER) {
		MaxPktSize = 1024;
	} else {
		MaxPktSize = 512;
	}

	dev->xSemaphore = xSemaphoreCreateBinary();

	/* Endpoint enables - not needed for Control EP */
	EpEnable(InstancePtr->PrivateData, STORAGE_EP, USB_EP_DIR_IN,
		 MaxPktSize, USB_EP_TYPE_BULK);

	EpEnable(InstancePtr->PrivateData, STORAGE_EP, USB_EP_DIR_OUT,
		 MaxPktSize, USB_EP_TYPE_BULK);

	xSemaphoreGive(dev->xSemaphore);

	while (1) {

		xSemaphoreTake(dev->xSemaphore, portMAX_DELAY);
		dev->phase = USB_EP_STATE_COMMAND;
		EpBufferRecv(InstancePtr->PrivateData, STORAGE_EP,
			     (u8 *) & (dev->cbw), sizeof(dev->cbw));

		xSemaphoreTake(dev->xSemaphore, portMAX_DELAY);
		ParseCBW(InstancePtr, dev);

		if (dev->phase == USB_EP_STATE_DATA_IN)
			EpBufferSend(InstancePtr->PrivateData, STORAGE_EP,
				     dev->currTrans.Ptr,
				     dev->currTrans.Length);
		else if (dev->phase == USB_EP_STATE_DATA_OUT)
			EpBufferRecv(InstancePtr->PrivateData, STORAGE_EP,
				     dev->currTrans.Ptr,
				     dev->currTrans.Length);
		else {
			xSemaphoreGive(dev->xSemaphore);
		}

		xSemaphoreTake(dev->xSemaphore, portMAX_DELAY);
		SendCSW(InstancePtr, dev, 0);
	}
}
