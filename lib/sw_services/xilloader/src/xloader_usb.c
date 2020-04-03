/******************************************************************************
*
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
******************************************************************************
****************************************************************************/
/**
*
* @file xloader_usb.c
*
* This file contains definitions of the generic handler functions to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   bvikram  02/10/19 First release
*
* </pre>
*
*****************************************************************************/
/***************************** Include Files ********************************/
#include "xloader_usb.h"
#ifdef XLOADER_USB
#include "xloader_dfu_util.h"
#include "xplmi_util.h"
#include "sleep.h"
#include "xloader.h"

/************************** Constant Definitions ****************************/
#define XLOADER_USB_DEVICE_ID		XPAR_XUSBPSU_0_DEVICE_ID
#define XLOADER_REQ_REPLY_LEN		1024U	/**< Max size of reply buffer. */
#define XLOADER_DOWNLOAD_COMPLETE		2U

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


struct Usb_DevData UsbInstance;
struct XUsbPsu UsbPrivateData;
extern struct XLoaderPs_DfuIf DfuObj;
u8* DfuVirtFlash = (u8*)XLOADER_DDR_TEMP_BUFFER_ADDRESS;
u32 DownloadDone = 0U;

/* Initialize a DFU data structure */
XLoader_UsbCh9_Data dfu_data = {
	.ch9_func = {
		.XLoaderPs_Ch9SetupDevDescReply = XLoader_Ch9SetupDevDescReply,
		.XLoaderPs_Ch9SetupCfgDescReply = XLoader_Ch9SetupCfgDescReply,
		.XLoaderPs_Ch9SetupBosDescReply = XLoader_Ch9SetupBosDescReply,
		.XLoaderPs_Ch9SetupStrDescReply = XLoader_Ch9SetupStrDescReply,
		.XLoaderPs_SetConfiguration = XLoader_SetConfiguration,
		/* hook the set interface handler */
		.XLoaderPs_SetInterfaceHandler = XLoader_DfuSetIntf,
		/* hook up storage class handler */
		.XLoaderPs_ClassReq = XLoader_DfuClassReq,
		/* Set the DFU address for call back */
	},
	.data_ptr = (void *)&DfuObj,
};

/*****************************************************************************
* This function initializes the USB interface.
*
* @param	Device Flags
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*
*****************************************************************************/
int XLoader_UsbInit(u32 DeviceFlags)
{
	(void) DeviceFlags;
	int Status = XST_FAILURE;
	Usb_Config *UsbConfigPtr;

	(void)memset(&UsbInstance, 0U, sizeof(UsbInstance));
	(void)memset(&UsbPrivateData, 0U, sizeof(struct XUsbPsu));
	(void)memset(&DfuObj, 0U, sizeof(DfuObj));

	UsbConfigPtr = XUsbPsu_LookupConfig(XLOADER_USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_USB_LOOKUP, Status);
		goto END;
	}

	UsbPrivateData.AppData = &UsbInstance;
	UsbInstance.PrivateData = (void*)&UsbPrivateData;

	Status = (int) XUsbPsu_CfgInitialize((struct XUsbPsu*)UsbInstance.PrivateData,
				UsbConfigPtr, UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_USB_CFG, Status);
		goto END;
	}
	/* hook up chapter9 handler */
	XUsbPsu_set_ch9handler((struct XUsbPsu*)UsbInstance.PrivateData,
		XLoader_Ch9Handler);

	/* Set the reset event handler */
	XUsbPsu_set_rsthandler((struct XUsbPsu*)UsbInstance.PrivateData,
		XLoader_DfuReset);

	DfuObj.InstancePtr = &UsbInstance;

	/* Set DFU state to APP_IDLE */
	XLoader_DfuSetState(&UsbInstance, STATE_APP_IDLE);

	/* Assign the data to usb driver */
	XUsbPsu_set_drvdata((struct XUsbPsu*)UsbInstance.PrivateData, &dfu_data);

	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr((struct XUsbPsu*)UsbInstance.PrivateData,
						XUSBPSU_DEVTEN_EVNTOVERFLOWEN | XUSBPSU_DEVTEN_WKUPEVTEN
						| XUSBPSU_DEVTEN_ULSTCNGEN | XUSBPSU_DEVTEN_CONNECTDONEEN
						| XUSBPSU_DEVTEN_USBRSTEN | XUSBPSU_DEVTEN_DISCONNEVTEN);
	/* Start the controller so that Host can see our device */
	Status = XUsbPsu_Start((struct XUsbPsu*)UsbInstance.PrivateData);
	if(Status != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_USB_START, Status);
		goto END;
	}

	while((DownloadDone < XLOADER_DOWNLOAD_COMPLETE) && \
			(DfuObj.CurrStatus != STATE_DFU_ERROR)) {
		XUsbPsu_IntrHandler((struct XUsbPsu*)UsbInstance.PrivateData);
	}
	(void)XUsbPsu_Stop((struct XUsbPsu*)UsbInstance.PrivateData);
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
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*
*****************************************************************************/

int XLoader_UsbCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	(void) Flags;

	SrcAddress += XLOADER_DDR_TEMP_BUFFER_ADDRESS;
	Status = XPlmi_DmaXfr(SrcAddress, DestAddress, Length/4U, XPLMI_PMCDMA_1);
	return Status;
}

/*****************************************************************************
* This function is only for compatibility with other device ops structures.
*
* @param	None
*
* @return
*		- XST_SUCCESS
*
* @note		None.
*
*****************************************************************************/
int XLoader_UsbRelease(void)
{
	return XST_SUCCESS;
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
void XLoader_Ch9Handler(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	switch (SetupData->bRequestType & XLOADER_REQ_TYPE_MASK) {
		case XLOADER_CMD_STDREQ:
		{
			XLoader_StdDevReq(InstancePtr, SetupData);
		}
			break;

		case XLOADER_CMD_CLASSREQ:
		{
			XLoader_DfuClassReq(InstancePtr, SetupData);
		}
			break;

		default:
		{
			/* Stall on Endpoint 0 */
			XLoader_Printf(DEBUG_INFO, "\nUnknown class req, \
									stalling at %s\n\r", __func__);
			XUsbPsu_Ep0StallRestart((struct XUsbPsu*)InstancePtr->PrivateData);
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
void XLoader_StdDevReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	int SStatus = XST_FAILURE;
	u32 ReplyLen;
	static u8 Reply[XLOADER_REQ_REPLY_LEN]={0U};
	static u8 TmpBuffer[DFU_STATUS_SIZE]={0U};
	u8 EpNum = SetupData->wIndex & XLOADER_USB_ENDPOINT_NUMBER_MASK;
	/*
	 * Direction - 1 -- XUSBPSU_EP_DIR_IN
	 * Direction - 0 -- XUSBPSU_EP_DIR_OUT
	 */
	u8 Direction = !!(SetupData->wIndex & XLOADER_USB_ENDPOINT_DIR_MASK);
	u16 ShortVar;

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen.*/
	if (SetupData->wLength > XLOADER_REQ_REPLY_LEN) {
		SStatus = XST_SUCCESS;
		goto END;
	}

	switch (SetupData->bRequest) {

		case XLOADER_REQ_GET_STATUS:
		{

			switch(SetupData->bRequestType & XLOADER_STATUS_MASK) {
				case XLOADER_STATUS_DEVICE:
				{
					ShortVar = 0x0100U;
					(void)XPlmi_MemCpy(&Reply[0U], &ShortVar, sizeof(u16));/* Self powered */
				}
					break;
				case XLOADER_STATUS_INTERFACE:
				{
					/* Need to send all zeroes as reply*/
				}
					break;
				case XLOADER_STATUS_ENDPOINT:
				{
					ShortVar = XUsbPsu_IsEpStalled(
								(struct XUsbPsu*)InstancePtr->PrivateData,
													EpNum, Direction);
					(void)XPlmi_MemCpy(&Reply[0U],&ShortVar, sizeof(u16));
				}
					break;
				default:
				{
					/* default case */
				}
					break;
			}
			if(SetupData->wLength != 0U)
			{
				SStatus = XUsbPsu_EpBufferSend(
						(struct XUsbPsu*)InstancePtr->PrivateData, 0U, Reply,
														SetupData->wLength);
			}
			else
			{
				SStatus = XST_SUCCESS;
			}
		}
			break;

		case XLOADER_REQ_SET_ADDRESS:
		{
			SStatus = XUsbPsu_SetDeviceAddress(
						(struct XUsbPsu*)InstancePtr->PrivateData,
							SetupData->wValue);
		}
			break;

		case XLOADER_REQ_GET_DESCRIPTOR:
		{
			/* Get descriptor type. */
			switch ((SetupData->wValue >> 8U) & 0xFFU) {

				case XLOADER_TYPE_DEVICE_DESC:
				case XLOADER_TYPE_DEVICE_QUALIFIER:
				{
					ReplyLen = XLoader_Ch9SetupDevDescReply(InstancePtr,
										Reply, XLOADER_REQ_REPLY_LEN);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}

					if(((SetupData->wValue >> 8U) & 0xFFU) ==
							XLOADER_TYPE_DEVICE_QUALIFIER) {
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
					if(ReplyLen != 0U)
					{
						SStatus = XUsbPsu_EpBufferSend(
								(struct XUsbPsu*)InstancePtr->PrivateData, 0U,
															Reply, ReplyLen);
					}
					else
					{
						SStatus = XST_SUCCESS;
					}
				}
					break;

				case XLOADER_TYPE_CONFIG_DESC:
				{

					ReplyLen = XLoader_Ch9SetupCfgDescReply(InstancePtr, Reply,
														XLOADER_REQ_REPLY_LEN);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}

					if(ReplyLen != 0U)
					{
						SStatus = XUsbPsu_EpBufferSend(
								(struct XUsbPsu*)InstancePtr->PrivateData, 0U,
															Reply, ReplyLen);
					}
					else
					{
						SStatus = XST_SUCCESS;
					}
				}
					break;

				case XLOADER_TYPE_STRING_DESC:
				{
					ReplyLen = XLoader_Ch9SetupStrDescReply(InstancePtr, Reply,
								STRING_SIZE, SetupData->wValue & 0xFFU);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}

					if(ReplyLen != 0U)
					{
						SStatus = XUsbPsu_EpBufferSend(
								(struct XUsbPsu*)InstancePtr->PrivateData, 0U,
															Reply, ReplyLen);
					}
					else
					{
						SStatus = XST_SUCCESS;
					}
				}
					break;

				case XLOADER_TYPE_BOS_DESC:
				{
					ReplyLen = XLoader_Ch9SetupBosDescReply(InstancePtr,
							Reply, XLOADER_REQ_REPLY_LEN);

					if(ReplyLen > SetupData->wLength){
						ReplyLen = SetupData->wLength;
					}

					if(ReplyLen != 0U)
					{
						SStatus = XUsbPsu_EpBufferSend(
								(struct XUsbPsu*)InstancePtr->PrivateData, 0U,
															Reply, ReplyLen);
					}
					else
					{
						SStatus = XST_SUCCESS;
					}
				}
				break;

				default:
					break;
			}
		}
			break;

		case XLOADER_REQ_SET_CONFIGURATION:
		{
			if ((SetupData->wValue & 0xFFU) != 1U) {
				SStatus = XST_FAILURE;
				break;
			}

			SStatus = XLoader_SetConfiguration(InstancePtr, SetupData);
		}
			break;

		case XLOADER_REQ_GET_CONFIGURATION:
		{
			SStatus = XST_SUCCESS;
		}
			break;

		case XLOADER_REQ_SET_FEATURE:
		{
			switch(SetupData->bRequestType & XLOADER_STATUS_MASK) {
				case XLOADER_STATUS_ENDPOINT:
				{
					if(SetupData->wValue == XLOADER_ENDPOINT_HALT) {
						if (!EpNum) {
							XUsbPsu_Ep0StallRestart((struct XUsbPsu *)InstancePtr->PrivateData);
						} else  {
							XUsbPsu_EpSetStall((struct XUsbPsu *)InstancePtr->PrivateData, EpNum, Direction);
						}
					}
					SStatus = XST_SUCCESS;
				}
					break;

				case XLOADER_STATUS_DEVICE:
				{
					SStatus = XST_SUCCESS;
				}
					break;
				default:
					break;
			}
		}
			break;

		case XLOADER_REQ_SET_INTERFACE:
		{
			XLoader_DfuSetIntf(InstancePtr, SetupData);
			SStatus = XST_SUCCESS;
		}
			break;

		case XLOADER_REQ_SET_SEL:
		{

			SStatus = XUsbPsu_EpBufferRecv(
						(struct XUsbPsu*)InstancePtr->PrivateData, 0U,
												TmpBuffer, DFU_STATUS_SIZE);
		}
			break;

		default:
			break;
	}
END:
		/* Set the send stall bit if there was an error */
		if (SStatus == XST_FAILURE) {
			XLoader_Printf(DEBUG_INFO,"\nStd dev req %d/%d error, stall 0 in out\n",
					SetupData->bRequest, (SetupData->wValue >> 8U) & 0xFFU);

			if (!EpNum)
			{
				XUsbPsu_Ep0StallRestart((struct XUsbPsu *)InstancePtr->PrivateData);
			} else
			{
				XUsbPsu_EpSetStall((struct XUsbPsu *)InstancePtr->PrivateData, EpNum, Direction);
			}
		}

}
#endif
