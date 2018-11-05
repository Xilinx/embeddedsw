/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xfsbl_usb.c
*
* This file contains definitions of the generic handler functions to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   bvikram  02/01/17 First release
*
* </pre>
*
*****************************************************************************/
/***************************** Include Files ********************************/
#include "xfsbl_hw.h"

#ifdef XFSBL_USB
#include "xusbpsu.h"
#include "xusb_wrapper.h"
#include "xfsbl_dfu_util.h"
#include "xfsbl_misc.h"
#include "xfsbl_usb.h"
#include "sleep.h"
#include "xcsudma.h"
#include "xfsbl_csu_dma.h"
#include "xfsbl_dfu_util.h"

/************************** Constant Definitions ****************************/

#define XFSBL_USB_DEVICE_ID		XPAR_XUSBPSU_0_DEVICE_ID
#define XFSBL_REQ_REPLY_LEN		1024U	/**< Max size of reply buffer. */
#define XFSBL_DOWNLOAD_COMPLETE		2U


/************************** Function Prototypes ******************************/
static void XFsbl_StdDevReq(SetupPacket *SetupData);
static void XFsbl_Ch9Handler(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

/************************** Variable Definitions *****************************/
struct XUsbPsu UsbInstance;
u8* DfuVirtFlash = (u8*)XFSBL_DDR_TEMP_BUFFER_ADDRESS;
u32 DownloadDone = 0U;
extern struct XFsblPs_DfuIf DfuObj;
extern XCsuDma CsuDma;

/*****************************************************************************
* This function initializes the USB interface.
*
* @param	Device Flags
*
* @return
*		- XFSBL_SUCCESS if successful,
*		- XFSBL_FAILURE if unsuccessful.
*
* @note		None.
*
*
*****************************************************************************/
u32 XFsbl_UsbInit(u32 DeviceFlags)
{
	u32 Status;
	s32 SStatus;
	XUsbPsu_Config *UsbConfigPtr;

	Status = XFsbl_CheckTempDfuMemory(0);
	if(Status == XFSBL_FAILURE){
		goto END;
	}

	(void)memset(&UsbInstance,0,sizeof(UsbInstance));
	(void)memset(&DfuObj, 0, sizeof(DfuObj));

	UsbConfigPtr = XUsbPsu_LookupConfig(XFSBL_USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	SStatus = XUsbPsu_CfgInitialize(&UsbInstance, UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != SStatus) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	XUsbPsu_SetSpeed(&UsbInstance, XUSBPSU_DCFG_HIGHSPEED);
	/* hook up chapter9 handler */
	UsbInstance.Chapter9 = XFsbl_Ch9Handler;

	/* Set the reset event handler */
	UsbInstance.ResetIntrHandler = XFsbl_DfuReset;

	/*Enable events for Reset, Disconnect, ConnectionDone, Link State
	* Wakeup and Overflow events.*/
	XUsbPsu_EnableIntr(&UsbInstance, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
			XUSBPSU_DEVTEN_WKUPEVTEN | XUSBPSU_DEVTEN_ULSTCNGEN |
	        XUSBPSU_DEVTEN_CONNECTDONEEN | XUSBPSU_DEVTEN_USBRSTEN |
							XUSBPSU_DEVTEN_DISCONNEVTEN);

	/* Start the controller so that Host can see our device */
	SStatus = XUsbPsu_Start(&UsbInstance);
	if (SStatus != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	while((DownloadDone < XFSBL_DOWNLOAD_COMPLETE) && (DfuObj.CurrStatus != STATE_DFU_ERROR)) {
		XUsbPsu_IntrHandler(&UsbInstance);
	}

	if(DownloadDone == XFSBL_DOWNLOAD_COMPLETE) {
		Status = XFSBL_SUCCESS;
	}
	else
	{
		Status = XFSBL_FAILURE;
	}
	(void)XUsbPsu_Stop(&UsbInstance);
END:
	return Status;
}

/*****************************************************************************
* This function copies from DFU temporary address in DDR to destination.
*
* @param	Source Address
* @param	Destination Address
* @param	Number of Bytes to be copied
*
* @return
*		- XFSBL_SUCCESS if successful,
*		- XFSBL_FAILURE if unsuccessful.
*
* @note		None.
*
*
*****************************************************************************/

u32 XFsbl_UsbCopy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length)
{
	u32 Status;
	Status = XFsbl_CheckTempDfuMemory(SrcAddress + Length);
	if(Status == XFSBL_FAILURE){
		goto END;
	}

	/* Setup the  SSS for DMA */
	u32 RegVal = XFsbl_In32(CSU_CSU_SSS_CFG) & XFSBL_CSU_SSS_DMA_MASK;
	u32	RegVal1 = RegVal | XFSBL_CSU_SSS_SRC_DEST_DMA;
	XFsbl_Out32(CSU_CSU_SSS_CFG, RegVal1);

	/* Set up the Destination DMA Channel*/
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, DestAddress, Length/4U, 0);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, (PTRSIZE) DfuVirtFlash + SrcAddress, Length/4U, 0);

	/* wait for the DMA transfer to finish */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_DST_CHANNEL){}

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	Status = XFSBL_SUCCESS;
END:
	return Status;
}

/*****************************************************************************
* This function is only for compatibility with other device ops structures.
*
* @param	None
*
* @return
*		- XFSBL_SUCCESS
*
* @note		None.
*
*****************************************************************************/
u32 XFsbl_UsbRelease(void)
{
	return XFSBL_SUCCESS;
}

/*********************************************************************************
* This function handles a Setup Data packet from the host.
*
* @param	InstancePtr is a pointer to XUsbPsu instance of the controller.
* @param	SetupData is the structure containing the setup request.
*
* @return
* 			None
*
* @note		None.
*
******************************************************************************/
static void XFsbl_Ch9Handler(struct Usb_DevData *InstancePtr,
			SetupPacket *SetupData)
{
	switch (SetupData->bRequestType & XFSBL_REQ_TYPE_MASK) {
		case XFSBL_CMD_STDREQ:
		{
			XFsbl_StdDevReq(SetupData);
		}
			break;

		case XFSBL_CMD_CLASSREQ:
		{
			XFsbl_DfuClassReq(SetupData);
		}
			break;

		default:
		{
			/* Stall on Endpoint 0 */
			XFsbl_Printf(DEBUG_INFO, "\nUnknown class req, stalling at %s\n\r", __func__);
			XUsbPsu_EpSetStall(InstancePtr->PrivateData, 0U, XUSBPSU_EP_DIR_OUT);
		}
			break;
	}
}

/*****************************************************************************
* This function handles a standard device request.
*
* @param	SetupData is a pointer to the data structure containing the
*		setup request.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XFsbl_StdDevReq(SetupPacket *SetupData)
{
	s32 SStatus;
	u32 ReplyLen;
	static u8 Reply[XFSBL_REQ_REPLY_LEN]={0};
	static u8 TmpBuffer[DFU_STATUS_SIZE]={0};
	u8 EpNum = SetupData->wIndex & USB_ENDPOINT_NUMBER_MASK;
	/*
	 * Direction - 1 -- XUSBPSU_EP_DIR_IN
	 * Direction - 0 -- XUSBPSU_EP_DIR_OUT
	 */
	u8 Direction = !!(SetupData->wIndex & USB_ENDPOINT_DIR_MASK);
	u16 ShortVar;

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen.*/
	if (SetupData->wLength > XFSBL_REQ_REPLY_LEN) {
		SStatus = XST_SUCCESS;
		goto END;
	}

	switch (SetupData->bRequest) {

		case XFSBL_REQ_GET_STATUS:
		{

			switch(SetupData->bRequestType & XFSBL_STATUS_MASK) {
				case XFSBL_STATUS_DEVICE:
				{
					ShortVar = 0x0100;
					(void)XFsbl_MemCpy(&Reply[0], &ShortVar, sizeof(u16));/* Self powered */
				}
					break;
				case XFSBL_STATUS_INTERFACE:
				{
					/* Need to send all zeroes as reply*/
				}
					break;
				case XFSBL_STATUS_ENDPOINT:
				{
					ShortVar = XUsbPsu_IsEpStalled(&UsbInstance, EpNum, Direction);
					(void)XFsbl_MemCpy(&Reply[0],&ShortVar, sizeof(u16));
				}
					break;
				default:
				{
					/* default case */
				}
					break;
			}

			SStatus = XUsbPsu_EpBufferSend(&UsbInstance, 0U, Reply, SetupData->wLength);
		}
			break;

		case XFSBL_REQ_SET_ADDRESS:
		{
			SStatus = XUsbPsu_SetDeviceAddress(&UsbInstance, SetupData->wValue);
		}
			break;

		case XFSBL_REQ_GET_DESCRIPTOR:
		{
			/* Get descriptor type. */
			switch ((SetupData->wValue >> 8U) & 0xFFU) {

				case XFSBL_TYPE_DEVICE_DESC:
				case XFSBL_TYPE_DEVICE_QUALIFIER:
				{
					ReplyLen = XFsbl_Ch9SetupDevDescReply(Reply, XFSBL_REQ_REPLY_LEN);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}

					if(((SetupData->wValue >> 8U) & 0xFFU) ==
							XFSBL_TYPE_DEVICE_QUALIFIER) {
						Reply[0] = ReplyLen;
						Reply[1] = 0x6U;
						Reply[2] = 0x0U;
						Reply[3] = 0x2U;
						Reply[4] = 0xFFU;
						Reply[5] = 0x00U;
						Reply[6] = 0x0U;
						Reply[7] = 0x10U;
						Reply[8] = 0x0U;
						Reply[9] = 0x0U;
					}
					SStatus = XUsbPsu_EpBufferSend(&UsbInstance, 0,
							Reply, ReplyLen);
				}
					break;

				case XFSBL_TYPE_CONFIG_DESC:
				{

					ReplyLen = XFsbl_Ch9SetupCfgDescReply(Reply, XFSBL_REQ_REPLY_LEN);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}
					SStatus = XUsbPsu_EpBufferSend(&UsbInstance, 0U,
								Reply, ReplyLen);
				}
					break;

				case XFSBL_TYPE_STRING_DESC:
				{
					ReplyLen = XFsbl_Ch9SetupStrDescReply(Reply, STRING_SIZE,
								SetupData->wValue & 0xFFU);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}
					SStatus = XUsbPsu_EpBufferSend(&UsbInstance, 0U,
								Reply, ReplyLen);
				}
					break;

				case XFSBL_TYPE_BOS_DESC:
				{
					ReplyLen = XFsbl_Ch9SetupBosDescReply(
							Reply, XFSBL_REQ_REPLY_LEN);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}
					SStatus = XUsbPsu_EpBufferSend(&UsbInstance, 0U,
								Reply, ReplyLen);
				}
				break;

				default:
				{
					SStatus = XST_FAILURE;
				}
					break;
			}
		}
			break;

		case XFSBL_REQ_SET_CONFIGURATION:
		{
			if ((SetupData->wValue & 0xFFU) != 1U) {
				SStatus = XST_FAILURE;
				break;
			}

			SStatus = XFsbl_SetConfiguration(SetupData);
		}
			break;

		case XFSBL_REQ_GET_CONFIGURATION:
		{
			SStatus = XST_SUCCESS;
		}
			break;

		case XFSBL_REQ_SET_FEATURE:
		{
			switch(SetupData->bRequestType & XFSBL_STATUS_MASK) {
				case XFSBL_STATUS_ENDPOINT:
				{
					if(SetupData->wValue == XFSBL_ENDPOINT_HALT) {
							XUsbPsu_EpSetStall(&UsbInstance, EpNum, Direction);

					}
					SStatus = XST_SUCCESS;
				}
					break;

				case XFSBL_STATUS_DEVICE:
				{
					SStatus = XST_SUCCESS;
				}
					break;
				default:
				{
					SStatus = XST_FAILURE;
				}
					break;
			}
		}
			break;

		case XFSBL_REQ_SET_INTERFACE:
		{
			XFsbl_DfuSetIntf(SetupData);
			SStatus = XST_SUCCESS;
		}
			break;

		case XFSBL_REQ_SET_SEL:
		{

			SStatus = XUsbPsu_EpBufferRecv(&UsbInstance, 0U, TmpBuffer, DFU_STATUS_SIZE);
		}
			break;

		default:
		{
			SStatus = XST_FAILURE;
		}
			break;
	}
END:
		/* Set the send stall bit if there was an error */
		if (SStatus == XST_FAILURE) {
			XFsbl_Printf(DEBUG_INFO,"\nStd dev req %d/%d error, stall 0 in out\n",
					SetupData->bRequest, (SetupData->wValue >> 8U) & 0xFFU);

			XUsbPsu_EpSetStall(&UsbInstance, 0U, XUSBPSU_EP_DIR_OUT);
		}

}

/*****************************************************************************
* This function verifies whether the address range is valid or not.
*
* @param	Range from the Starting address of DDR memory.
*
* @return	XFSBL_SUCCESS or XFSBL_FAILURE
*
* @note		None.
*
******************************************************************************/

u32 XFsbl_CheckTempDfuMemory(u32 Offset)
{
	u32 Status;
	if((DfuVirtFlash != NULL) && (((PTRSIZE)DfuVirtFlash + Offset) <= XFSBL_PS_DDR_END_ADDRESS)){
		Status = XFSBL_SUCCESS;
	}
	else
	{
		Status = XFSBL_FAILURE;
	}
	return Status;
}
#endif
