/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.14  pm     21/06/23 Added support for system device-tree flow.
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xusb_wrapper.h"

/************************** Variable Definitions *****************************/

struct XUsbPsu PrivateData;	/**< Private data for the USB PSU instance. */

/************************** Function Prototypes ******************************/
#ifndef SDT
/**
* @brief Lookup USB configuration by device ID.
*
* @param[in] DeviceId is the USB device ID.
*
* @return	Pointer to the USB configuration, or NULL if not found.
*/
Usb_Config *LookupConfig(u16 DeviceId)
{
	return XUsbPsu_LookupConfig(DeviceId);
}
#else
/**
* @brief Lookup USB configuration by base address.
*
* @param[in] BaseAddress is the base address of the controller.
*
* @return	Pointer to the USB configuration, or NULL if not found.
*/
Usb_Config *LookupConfig(UINTPTR BaseAddress)
{
	return XUsbPsu_LookupConfig(BaseAddress);
}
#endif

/**
* @brief CacheInit function.
*/
void CacheInit(void)
{

}

/**
* @brief CfgInitialize function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] ConfigPtr is a pointer to the USB configuration.
* @param[in] BaseAddress is the base address of the controller.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 CfgInitialize(struct Usb_DevData *InstancePtr,
		  Usb_Config *ConfigPtr, u32 BaseAddress)
{
	PrivateData.AppData = InstancePtr;
	InstancePtr->PrivateData = (void *)&PrivateData;

	return XUsbPsu_CfgInitialize((struct XUsbPsu *)InstancePtr->PrivateData,
				     ConfigPtr, BaseAddress);
}

/**
* @brief Set_Ch9Handler function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] func is the Chapter 9 handler callback function pointer.
*/
void Set_Ch9Handler(
	void *InstancePtr,
	void (*func)(struct Usb_DevData *, SetupPacket *))
{
	XUsbPsu_set_ch9handler((struct XUsbPsu *)InstancePtr, func);
}

/**
* @brief Set_RstHandler function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] func is the reset handler callback function pointer.
*/
void Set_RstHandler(void *InstancePtr, void (*func)(struct Usb_DevData *))
{
	XUsbPsu_set_rsthandler((struct XUsbPsu *)InstancePtr, func);
}

/**
* @brief Set_Disconnect function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] func is the disconnect handler callback function pointer.
*/
void Set_Disconnect(void *InstancePtr, void (*func)(struct Usb_DevData *))
{
	XUsbPsu_set_disconnect((struct XUsbPsu *)InstancePtr, func);
}

/**
* @brief EpConfigure function.
*
* @param[in] UsbInstance is a pointer to the USB instance.
* @param[in] EndpointNo is the endpoint number.
* @param[in] dir is the endpoint direction.
* @param[in] Type is the endpoint type.
*/
void EpConfigure(void *UsbInstance, u8 EndpointNo, u8 dir, u32 Type)
{
	(void)UsbInstance;
	(void)EndpointNo;
	(void)dir;
	(void)Type;
}

/**
* @brief ConfigureDevice function.
*
* @param[in] UsbInstance is a pointer to the USB instance.
* @param[in] MemPtr is a pointer to the memory buffer.
* @param[in] memSize is the size of the memory buffer.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 ConfigureDevice(void *UsbInstance, u8 *MemPtr, u32 memSize)
{
	(void)UsbInstance;
	(void)MemPtr;
	(void)memSize;
	return XST_SUCCESS;
}

/**
* @brief SetEpHandler function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Epnum is the endpoint number.
* @param[in] Dir is the endpoint direction.
* @param[in] Handler is the handler function pointer.
*/
void SetEpHandler(void *InstancePtr, u8 Epnum,
		  u8 Dir, void (*Handler)(void *, u32, u32))
{
	XUsbPsu_SetEpHandler((struct XUsbPsu *)InstancePtr, Epnum, Dir, Handler);
}

/**
* @brief Usb_Start function.
*
* @param[in] InstancePtr is a pointer to the instance.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 Usb_Start(void *InstancePtr)
{
	return XUsbPsu_Start((struct XUsbPsu *)InstancePtr);
}

/**
* @brief Retrieve the driver data pointer.
*
* @param[in] InstancePtr is a pointer to the instance.
*
* @return	Pointer to the driver data, or NULL if not set.
*/
void *Get_DrvData(void *InstancePtr)
{
	return XUsbPsu_get_drvdata((struct XUsbPsu *)InstancePtr);
}

/**
* @brief Set_DrvData function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] data is the driver data pointer.
*/
void Set_DrvData(void *InstancePtr, void *data)
{
	XUsbPsu_set_drvdata((struct XUsbPsu *)InstancePtr, data);
}

/**
* @brief IsEpStalled function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Epnum is the endpoint number.
* @param[in] Dir is the endpoint direction.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 IsEpStalled(void *InstancePtr, u8 Epnum, u8 Dir)
{
	return XUsbPsu_IsEpStalled((struct XUsbPsu *)InstancePtr, Epnum, Dir);
}

/**
* @brief EpClearStall function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Epnum is the endpoint number.
* @param[in] Dir is the endpoint direction.
*/
void EpClearStall(void *InstancePtr, u8 Epnum, u8 Dir)
{
	XUsbPsu_EpClearStall((struct XUsbPsu *)InstancePtr, Epnum, Dir);
}

/**
* @brief EpBufferSend function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] UsbEp is the USB endpoint number.
* @param[in] BufferPtr is a pointer to the data buffer.
* @param[in] BufferLen is the buffer length.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 EpBufferSend(void *InstancePtr, u8 UsbEp,
		 u8 *BufferPtr, u32 BufferLen)
{
	if (UsbEp == 0 && BufferLen == 0) {
		return XST_SUCCESS;
	} else
		return XUsbPsu_EpBufferSend((struct XUsbPsu *)InstancePtr,
					    UsbEp, BufferPtr, BufferLen);

}

/**
* @brief EpBufferRecv function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] UsbEp is the USB endpoint number.
* @param[in] BufferPtr is a pointer to the data buffer.
* @param[in] Length is the transfer length.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 EpBufferRecv(void *InstancePtr, u8 UsbEp,
		 u8 *BufferPtr, u32 Length)
{
	return XUsbPsu_EpBufferRecv((struct XUsbPsu *)InstancePtr, UsbEp,
				    BufferPtr, Length);
}

/**
* @brief EpSetStall function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Epnum is the endpoint number.
* @param[in] Dir is the endpoint direction.
*/
void EpSetStall(void *InstancePtr, u8 Epnum, u8 Dir)
{
	if (!Epnum) {
		XUsbPsu_Ep0StallRestart((struct XUsbPsu *)InstancePtr);
	} else  {
		XUsbPsu_EpSetStall((struct XUsbPsu *)InstancePtr, Epnum, Dir);
	}
}

/**
* @brief SetBits function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] TestSel is the test mode selector.
*/
void SetBits(void *InstancePtr, u32 TestSel)
{
	(void)InstancePtr;
	(void)TestSel;
}

/**
* @brief SetDeviceAddress function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Addr is the USB device address.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 SetDeviceAddress(void *InstancePtr, u16 Addr)
{
	return XUsbPsu_SetDeviceAddress((struct XUsbPsu *)InstancePtr, Addr);
}

/**
* @brief SetU1SleepTimeout function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Sleep is the sleep timeout value.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 SetU1SleepTimeout(void *InstancePtr, u8 Sleep)
{
	return XUsbPsu_SetU1SleepTimeout((struct XUsbPsu *)InstancePtr, Sleep);
}

/**
* @brief SetU2SleepTimeout function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Sleep is the sleep timeout value.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 SetU2SleepTimeout(void *InstancePtr, u8 Sleep)
{
	return XUsbPsu_SetU2SleepTimeout((struct XUsbPsu *)InstancePtr, Sleep);
}

/**
* @brief AcceptU1U2Sleep function.
*
* @param[in] InstancePtr is a pointer to the instance.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 AcceptU1U2Sleep(void *InstancePtr)
{
	return XUsbPsu_AcceptU1U2Sleep((struct XUsbPsu *)InstancePtr);

}

/**
* @brief U1SleepEnable function.
*
* @param[in] InstancePtr is a pointer to the instance.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 U1SleepEnable(void *InstancePtr)
{
	return XUsbPsu_U1SleepEnable((struct XUsbPsu *)InstancePtr);
}

/**
* @brief U2SleepEnable function.
*
* @param[in] InstancePtr is a pointer to the instance.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 U2SleepEnable(void *InstancePtr)
{
	return XUsbPsu_U2SleepEnable((struct XUsbPsu *)InstancePtr);
}

/**
* @brief U1SleepDisable function.
*
* @param[in] InstancePtr is a pointer to the instance.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 U1SleepDisable(void *InstancePtr)
{
	return XUsbPsu_U1SleepDisable((struct XUsbPsu *)InstancePtr);
}

/**
* @brief U2SleepDisable function.
*
* @param[in] InstancePtr is a pointer to the instance.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 U2SleepDisable(void *InstancePtr)
{
	return XUsbPsu_U2SleepDisable((struct XUsbPsu *)InstancePtr);
}

/**
* @brief EpEnable function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] UsbEpNum is the USB endpoint number.
* @param[in] Dir is the endpoint direction.
* @param[in] Maxsize is the maximum packet size.
* @param[in] Type is the endpoint type.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 EpEnable(void *InstancePtr, u8 UsbEpNum, u8 Dir, u16 Maxsize, u8 Type)
{
	return XUsbPsu_EpEnable((struct XUsbPsu *)InstancePtr, UsbEpNum, Dir,
				Maxsize, Type, FALSE);
}

/**
* @brief EpDisable function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] UsbEpNum is the USB endpoint number.
* @param[in] Dir is the endpoint direction.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 EpDisable(void *InstancePtr, u8 UsbEpNum, u8 Dir)
{
	return XUsbPsu_EpDisable((struct XUsbPsu *)InstancePtr, UsbEpNum, Dir);
}

/**
* @brief Usb_SetSpeed function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] Speed is the USB speed value.
*/
void Usb_SetSpeed(void *InstancePtr, u32 Speed)
{
	XUsbPsu_SetSpeed((struct XUsbPsu *)InstancePtr, Speed);
}

/****************************************************************************/
/**
* Handler called from polled mode for handling usb events
*
* @param[in] InstancePtr is a pointer to the XUsbPsu instance.
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
* @param[in] InstancePtr is a pointer to the XUsbPsu instance.
* @param[in] Mask is the bitmask of events to enable.
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
* @param[in] InstancePtr is a pointer to the Usb_DevData instance.
*
* @return	XST_SUCCESS if super speed, XST_FAILURE otherwise.
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
* @param[in] InstancePtr is a private member of Usb_DevData instance.
* @param[in] Flag is the config value.
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
* @param[in] InstancePtr is a private member of Usb_DevData instance.
*
* @return	Configuration done flag value.
*
*****************************************************************************/
u8 GetConfigDone(void *InstancePtr)
{
	return (((struct XUsbPsu *)InstancePtr)->IsConfigDone);
}

/**
* @brief Ep0StallRestart function.
*
* @param[in] InstancePtr is a pointer to the instance.
*/
void Ep0StallRestart(void *InstancePtr)
{
	XUsbPsu_Ep0StallRestart((struct XUsbPsu *)InstancePtr);
}

/******************************************************************************/
/**
 * This function sets Endpoint Interval.
 *
 * @param[in] InstancePtr is a private member of Usb_DevData instance.
 * @param[in] UsbEpNum is the endpoint number.
 * @param[in] Dir is Endpoint Direction(In/Out).
 * @param[in] Interval is the data transfer service interval
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

/**
* @brief StopTransfer function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] EpNum is the endpoint number.
* @param[in] Dir is the endpoint direction.
*/
void StopTransfer(void *InstancePtr, u8 EpNum, u8 Dir)
{
	XUsbPsu_StopTransfer((struct XUsbPsu *)InstancePtr, EpNum, Dir, TRUE);
}

/**
* @brief StreamOn function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] EpNum is the endpoint number.
* @param[in] Dir is the endpoint direction.
* @param[in] BufferPtr is a pointer to the data buffer.
 *
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*/
s32 StreamOn(void *InstancePtr, u8 EpNum, u8 Dir, u8 *BufferPtr)
{
	(void)InstancePtr;
	(void)EpNum;
	(void)Dir;
	(void)BufferPtr;
	/* Streaming will start on TxferNotReady Event */
	return XST_SUCCESS;
}

/**
* @brief StreamOff function.
*
* @param[in] InstancePtr is a pointer to the instance.
* @param[in] EpNum is the endpoint number.
* @param[in] Dir is the endpoint direction.
	 */
void StreamOff(void *InstancePtr, u8 EpNum, u8 Dir)
{
	StopTransfer((struct XUsbPsu *)InstancePtr, EpNum, Dir);
}
