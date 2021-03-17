/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_wrapper.c
 *
 * This file contains implementation of USBPSU Driver wrappers.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  	Date     Changes
 * ----- ---- 	-------- -------------------------------------------------------
 * 1.0   BK 	12/01/18 First release
 *	 MYK	12/01/18 Added hibernation support for device mode
 *	 vak	13/03/18 Moved the setup interrupt system calls from driver to
 *			 example.
 * 1.5	 vak	02/06/19 Modified the code to add UsbPollHandler and
 *			 UsbEnableEvent API's
 *	 vak	02/07/19 Modified the code to issue XUsbPsu_Ep0StallRestart()
 *			 for Endpoint Zero
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xusb_wrapper.h"

/************************** Variable Definitions *****************************/

struct XUsbPsu PrivateData;

/************************** Function Prototypes ******************************/
Usb_Config* LookupConfig(u16 DeviceId)
{
	return XUsbPsu_LookupConfig(DeviceId);
}

void CacheInit(void)
{

}

s32 CfgInitialize(struct Usb_DevData *InstancePtr,
			Usb_Config *ConfigPtr, u32 BaseAddress)
{
	PrivateData.AppData = InstancePtr;
	InstancePtr->PrivateData = (void *)&PrivateData;

	return XUsbPsu_CfgInitialize((struct XUsbPsu *)InstancePtr->PrivateData,
			ConfigPtr, BaseAddress);
}

void Set_Ch9Handler(
		void *InstancePtr,
		void (*func)(struct Usb_DevData *, SetupPacket *))
{
	XUsbPsu_set_ch9handler((struct XUsbPsu *)InstancePtr, func);
}

void Set_RstHandler(void *InstancePtr, void (*func)(struct Usb_DevData *))
{
	XUsbPsu_set_rsthandler((struct XUsbPsu *)InstancePtr, func);
}

void Set_Disconnect(void *InstancePtr, void (*func)(struct Usb_DevData *))
{
	XUsbPsu_set_disconnect((struct XUsbPsu *)InstancePtr, func);
}

void EpConfigure(void *UsbInstance, u8 EndpointNo, u8 dir, u32 Type)
{
	(void)UsbInstance;
	(void)EndpointNo;
	(void)dir;
	(void)Type;
}

s32 ConfigureDevice(void *UsbInstance, u8 *MemPtr, u32 memSize)
{
	(void)UsbInstance;
	(void)MemPtr;
	(void)memSize;
	return XST_SUCCESS;
}

void SetEpHandler(void *InstancePtr, u8 Epnum,
			u8 Dir, void (*Handler)(void *, u32, u32))
{
	XUsbPsu_SetEpHandler((struct XUsbPsu *)InstancePtr, Epnum, Dir, Handler);
}

s32 Usb_Start(void *InstancePtr)
{
	return XUsbPsu_Start((struct XUsbPsu *)InstancePtr);
}

void *Get_DrvData(void *InstancePtr)
{
	return XUsbPsu_get_drvdata((struct XUsbPsu *)InstancePtr);
}

void Set_DrvData(void *InstancePtr, void *data)
{
	XUsbPsu_set_drvdata((struct XUsbPsu *)InstancePtr, data);
}

s32 IsEpStalled(void *InstancePtr, u8 Epnum, u8 Dir)
{
	return XUsbPsu_IsEpStalled((struct XUsbPsu *)InstancePtr, Epnum, Dir);
}

void EpClearStall(void *InstancePtr, u8 Epnum, u8 Dir)
{
	XUsbPsu_EpClearStall((struct XUsbPsu *)InstancePtr, Epnum, Dir);
}

s32 EpBufferSend(void *InstancePtr, u8 UsbEp,
			u8 *BufferPtr, u32 BufferLen)
{
	if (UsbEp == 0 && BufferLen == 0)
		return XST_SUCCESS;
	else
		return XUsbPsu_EpBufferSend((struct XUsbPsu *)InstancePtr,
						UsbEp, BufferPtr, BufferLen);

}

s32 EpBufferRecv(void *InstancePtr, u8 UsbEp,
				u8 *BufferPtr, u32 Length)
{
	return XUsbPsu_EpBufferRecv((struct XUsbPsu *)InstancePtr, UsbEp,
			BufferPtr, Length);
}

void EpSetStall(void *InstancePtr, u8 Epnum, u8 Dir)
{
	if (!Epnum) {
		XUsbPsu_Ep0StallRestart((struct XUsbPsu *)InstancePtr);
	} else  {
		XUsbPsu_EpSetStall((struct XUsbPsu *)InstancePtr, Epnum, Dir);
	}
}

void SetBits(void *InstancePtr, u32 TestSel)
{
	(void)InstancePtr;
	(void)TestSel;
}

s32 SetDeviceAddress(void *InstancePtr, u16 Addr)
{
	return XUsbPsu_SetDeviceAddress((struct XUsbPsu *)InstancePtr, Addr);
}

s32 SetU1SleepTimeout(void *InstancePtr, u8 Sleep)
{
	return XUsbPsu_SetU1SleepTimeout((struct XUsbPsu *)InstancePtr, Sleep);
}

s32 SetU2SleepTimeout(void *InstancePtr, u8 Sleep)
{
	return XUsbPsu_SetU2SleepTimeout((struct XUsbPsu *)InstancePtr, Sleep);
}

s32 AcceptU1U2Sleep(void *InstancePtr)
{
	return XUsbPsu_AcceptU1U2Sleep((struct XUsbPsu *)InstancePtr);

}

s32 U1SleepEnable(void *InstancePtr)
{
	return XUsbPsu_U1SleepEnable((struct XUsbPsu *)InstancePtr);
}

s32 U2SleepEnable(void *InstancePtr)
{
	return XUsbPsu_U2SleepEnable((struct XUsbPsu *)InstancePtr);
}

s32 U1SleepDisable(void *InstancePtr)
{
	return XUsbPsu_U1SleepDisable((struct XUsbPsu *)InstancePtr);
}

s32 U2SleepDisable(void *InstancePtr)
{
	return XUsbPsu_U2SleepDisable((struct XUsbPsu *)InstancePtr);
}

s32 EpEnable(void *InstancePtr, u8 UsbEpNum, u8 Dir, u16 Maxsize, u8 Type)
{
	return XUsbPsu_EpEnable((struct XUsbPsu *)InstancePtr, UsbEpNum, Dir,
			Maxsize, Type, FALSE);
}

s32 EpDisable(void *InstancePtr, u8 UsbEpNum, u8 Dir)
{
	return XUsbPsu_EpDisable((struct XUsbPsu *)InstancePtr, UsbEpNum, Dir);
}

void Usb_SetSpeed(void *InstancePtr, u32 Speed)
{
	XUsbPsu_SetSpeed((struct XUsbPsu *)InstancePtr, Speed);
}

/****************************************************************************/
/**
* Handler called from polled mode for handling usb events
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void UsbPollHandler(struct XUsbPsu *InstancePtr)
{
	XUsbPsu_IntrHandler(InstancePtr);
}

/****************************************************************************/
/**
* Enables the events specified in the mask
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void UsbEnableEvent(struct XUsbPsu *InstancePtr, u32 Mask)
{
	XUsbPsu_EnableIntr(InstancePtr, Mask);
}

/****************************************************************************/
/**
* Sets speed of the Core for connecting to Host
*
* @param	InstancePtr is a pointer to the Usb_DevData instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 IsSuperSpeed(struct Usb_DevData *InstancePtr)
{
	return XUsbPsu_IsSuperSpeed((struct XUsbPsu *)InstancePtr->PrivateData);
}

/****************************************************************************/
/**
* Set the Config state
*
* @param	InstancePtr is a private member of Usb_DevData instance.
* @param	Flag is the config value.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void SetConfigDone(void *InstancePtr, u8 Flag)
{
	((struct XUsbPsu *)InstancePtr)->IsConfigDone = Flag;
}

/****************************************************************************/
/**
* Get the Config state
*
* @param	InstancePtr is a private member of Usb_DevData instance.
*
* @return	Current configuration value
*
* @note		None.
*
*****************************************************************************/
u8 GetConfigDone(void *InstancePtr)
{
	return (((struct XUsbPsu *)InstancePtr)->IsConfigDone);
}

void Ep0StallRestart(void *InstancePtr)
{
	XUsbPsu_Ep0StallRestart((struct XUsbPsu *)InstancePtr);
}

/******************************************************************************/
/**
 * This function sets Endpoint Interval.
 *
 * @param	InstancePtr is a private member of Usb_DevData instance.
 * @param	UsbEpnum is Endpoint Number.
 * @param	Dir is Endpoint Direction(In/Out).
 * @param	Interval is the data transfer service interval
 *
 * @return 	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SetEpInterval(void *InstancePtr, u8 UsbEpNum, u8 Dir, u32 Interval)
{
	u32 PhyEpNum;
	struct XUsbPsu_Ep *Ept;

	PhyEpNum = PhysicalEp(UsbEpNum, Dir);
	Ept = &((struct XUsbPsu *)InstancePtr)->eps[PhyEpNum];
	Ept->Interval = Interval;
}

void StopTransfer(void *InstancePtr, u8 EpNum, u8 Dir)
{
	XUsbPsu_StopTransfer((struct XUsbPsu *)InstancePtr, EpNum, Dir, TRUE);
}

s32 StreamOn(void *InstancePtr, u8 EpNum, u8 Dir, u8 *BufferPtr)
{
	(void)InstancePtr;
	(void)EpNum;
	(void)Dir;
	(void)BufferPtr;
	/* Streaming will start on TxferNotReady Event */
	return XST_SUCCESS;
}

void StreamOff(void *InstancePtr, u8 EpNum, u8 Dir)
{
	StopTransfer((struct XUsbPsu *)InstancePtr, EpNum, Dir);
}
