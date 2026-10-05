/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_wrapper.h
 *
 * This file contains declarations for USBPSU Driver wrappers.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  	Date     Changes
 * ----- ---- 	-------- -------------------------------------------------------
 *  1.0  BK	12/01/18 First release
 *	 MYK	12/01/18 Added hibernation support for device mode
 *	 vak	22/01/18 Added Microblaze support for usbpsu driver
 *	 vak	13/03/18 Moved the setup interrupt system calls from driver to
 *			 example.
 *  1.5  vak	02/06/19 Add UsbPollHandler and UsbEnableEvent API's
 *  1.7  pm	02/03/20 Add closure bracket for "extern c" c++ compilation
 *  1.14 pm     21/06/23 Added support for system device-tree flow.
 *
 * </pre>
 *
 *****************************************************************************/

#ifndef XUSB_WRAPPER_H  /* Prevent circular inclusions */
#define XUSB_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xusbpsu.h"

/************************** Constant Definitions ****************************/
#define USB_DEVICE_ID		XPAR_XUSBPSU_0_DEVICE_ID	/**< USB device instance ID. */

#define USB_EP_DIR_IN		XUSBPSU_EP_DIR_IN	/**< USB endpoint direction: IN. */
#define USB_EP_DIR_OUT		XUSBPSU_EP_DIR_OUT	/**< USB endpoint direction: OUT. */

/** @brief USB transfer direction: OUT (host to device). */
#define USB_DIR_OUT				0U
/** @brief USB transfer direction: IN (device to host). */
#define USB_DIR_IN				0x80U

/**
 * @name Endpoint Address
 * @{
 */
#define USB_EP1_IN		0x81	/**< Usb Ep1 In. */
#define USB_EP1_OUT		0x01	/**< Usb Ep1 Out. */
#define USB_EP2_IN		0x82	/**< Usb Ep2 In. */
#define USB_EP2_OUT		0x02	/**< Usb Ep2 Out. */
/** @} */

/**
 * @name Endpoint Type
 * @{
 */
#define USB_EP_TYPE_CONTROL			XUSBPSU_ENDPOINT_XFER_CONTROL	/**< Usb Ep Type Control. */
#define USB_EP_TYPE_ISOCHRONOUS		XUSBPSU_ENDPOINT_XFER_ISOC	/**< Usb Ep Type Isochronous. */
#define USB_EP_TYPE_BULK			XUSBPSU_ENDPOINT_XFER_BULK	/**< Usb Ep Type Bulk. */
#define USB_EP_TYPE_INTERRUPT		XUSBPSU_ENDPOINT_XFER_INT	/**< Usb Ep Type Interrupt. */
/** @} */

#define USB_ENDPOINT_NUMBER_MASK        0x0f	/**< Mask to extract endpoint number from address. */
#define USB_ENDPOINT_DIR_MASK           0x80	/**< Mask to extract endpoint direction from address. */

/*
 * Device States
 */
#define		USB_STATE_ATTACHED		XUSBPSU_STATE_ATTACHED	/**< USB device state: Attached. */
#define		USB_STATE_POWERED		XUSBPSU_STATE_POWERED	/**< USB device state: Powered. */
#define		USB_STATE_DEFAULT		XUSBPSU_STATE_DEFAULT	/**< USB device state: Default. */
#define		USB_STATE_ADDRESS		XUSBPSU_STATE_ADDRESS	/**< USB device state: Addressed. */
#define		USB_STATE_CONFIGURED	XUSBPSU_STATE_CONFIGURED	/**< USB device state: Configured. */
#define		USB_STATE_SUSPENDED		XUSBPSU_STATE_SUSPENDED	/**< USB device state: Suspended. */

#define XUSBPSU_REQ_REPLY_LEN	    1024	/**< Max size of reply buffer. */
#define USB_REQ_REPLY_LEN			XUSBPSU_REQ_REPLY_LEN	/**< Maximum USB request reply buffer length. */

/*
 * Device Speeds
 */
#define	USB_SPEED_UNKNOWN		XUSBPSU_SPEED_UNKNOWN	/**< USB speed: Unknown. */
#define	USB_SPEED_LOW			XUSBPSU_SPEED_LOW	/**< USB speed: Low speed. */
#define	USB_SPEED_FULL			XUSBPSU_SPEED_FULL	/**< USB speed: Full speed. */
#define	USB_SPEED_HIGH			XUSBPSU_SPEED_HIGH	/**< USB speed: High speed. */
#define	USB_SPEED_SUPER			XUSBPSU_SPEED_SUPER	/**< USB speed: SuperSpeed. */

/* Device Configuration Speed */
#define	USB_DCFG_SPEED_MASK		XUSBPSU_DCFG_SPEED_MASK	/**< Device configuration speed mask. */
#define	USB_DCFG_SUPERSPEED		XUSBPSU_DCFG_SUPERSPEED	/**< Device configuration: SuperSpeed. */
#define	USB_DCFG_HIGHSPEED		XUSBPSU_DCFG_HIGHSPEED	/**< Device configuration: High speed. */
/** @brief Device configuration: Full speed (alternate). */
#define	USB_DCFG_FULLSPEED2		XUSBPSU_DCFG_FULLSPEED2
#define	USB_DCFG_LOWSPEED		XUSBPSU_DCFG_LOWSPEED	/**< Device configuration: Low speed. */
#define	USB_DCFG_FULLSPEED1		XUSBPSU_DCFG_FULLSPEED1	/**< Device configuration: Full speed. */

#define	USB_TEST_J				XUSBPSU_TEST_J	/**< USB test mode: Test_J. */
#define	USB_TEST_K				XUSBPSU_TEST_K	/**< USB test mode: Test_K. */
#define	USB_TEST_SE0_NAK		XUSBPSU_TEST_SE0_NAK	/**< USB test mode: Test_SE0_NAK. */
#define	USB_TEST_PACKET			XUSBPSU_TEST_PACKET	/**< USB test mode: Test_Packet. */
#define	USB_TEST_FORCE_ENABLE	XUSBPSU_TEST_FORCE_ENABLE	/**< USB test mode: Test_Force_Enable. */

/* TODO: If we enable this macro, reconnection is failed with 2017.3 */
#define USB_LPM_MODE			XUSBPSU_LPM_MODE	/**< USB Link Power Management mode. */

/*
 * return Physical EP number as dwc3 mapping
 */
/** @brief Compute the physical endpoint number from logical endpoint and direction. */
#define PhysicalEp(epnum, direction)	(((epnum) << 1 ) | (direction))

/**************************** Type Definitions ******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef SDT
Usb_Config *LookupConfig(u16 DeviceId);	/**< Lookup Config(u16  Device Id). */
#else
Usb_Config *LookupConfig(UINTPTR BaseAddress);
#endif
void CacheInit(void);
s32 CfgInitialize(struct Usb_DevData *InstancePtr,
		  Usb_Config *ConfigPtr, u32 BaseAddress);
void Set_Ch9Handler(void *InstancePtr,
		    void (*func)(struct Usb_DevData *, SetupPacket *));
void Set_RstHandler(void *InstancePtr, void (*func)(struct Usb_DevData *));
void Set_Disconnect(void *InstancePtr, void (*func)(struct Usb_DevData *));
void EpConfigure(void *UsbInstance, u8 EndpointNo, u8 dir, u32 Type);
s32 ConfigureDevice(void *UsbInstance, u8 *MemPtr, u32 memSize);	/**< Configure Device(void * Usb Instance, u8 * Mem Ptr, u32 m... */
void SetEpHandler(void *InstancePtr, u8 Epnum,
		  u8 Dir, void (*Handler)(void *, u32, u32));
s32 Usb_Start(void *InstancePtr);
void UsbPollHandler(struct XUsbPsu *InstancePtr);
void UsbEnableEvent(struct XUsbPsu *InstancePtr, u32 Mask);
void *Get_DrvData(void *InstancePtr);	/**< Get_ Drv Data(void * Instance Ptr). */
void Set_DrvData(void *InstancePtr, void *data);
s32 IsEpStalled(void *InstancePtr, u8 Epnum, u8 Dir);	/**< Is Ep Stalled(void * Instance Ptr, u8  Epnum, u8  Dir). */
void EpClearStall(void *InstancePtr, u8 Epnum, u8 Dir);
s32 EpBufferSend(void *InstancePtr, u8 UsbEp,
		 u8 *BufferPtr, u32 BufferLen);
s32 EpBufferRecv(void *InstancePtr, u8 UsbEp,
		 u8 *BufferPtr, u32 Length);
void EpSetStall(void *InstancePtr, u8 Epnum, u8 Dir);
void SetBits(void *InstancePtr, u32 TestSel);
s32 SetDeviceAddress(void *InstancePtr, u16 Addr);	/**< Set Device Address(void * Instance Ptr, u16  Addr). */
s32 SetU1SleepTimeout(void *InstancePtr, u8 Sleep);	/**< Set U1 Sleep Timeout(void * Instance Ptr, u8  Sleep). */
s32 SetU2SleepTimeout(void *InstancePtr, u8 Sleep);	/**< Set U2 Sleep Timeout(void * Instance Ptr, u8  Sleep). */
s32 AcceptU1U2Sleep(void *InstancePtr);	/**< Accept U1 U2 Sleep(void * Instance Ptr). */
s32 U1SleepEnable(void *InstancePtr);	/**< U1 Sleep Enable(void * Instance Ptr). */
s32 U2SleepEnable(void *InstancePtr);	/**< U2 Sleep Enable(void * Instance Ptr). */
s32 U1SleepDisable(void *InstancePtr);	/**< U1 Sleep Disable(void * Instance Ptr). */
s32 U2SleepDisable(void *InstancePtr);	/**< U2 Sleep Disable(void * Instance Ptr). */
s32 EpEnable(void *InstancePtr, u8 UsbEpNum, u8 Dir, u16 Maxsize, u8 Type);	/**< Ep Enable(void * Instance Ptr, u8  Usb Ep Num, u8  Dir, u... */
s32 EpDisable(void *InstancePtr, u8 UsbEpNum, u8 Dir);	/**< Ep Disable(void * Instance Ptr, u8  Usb Ep Num, u8  Dir). */
void Usb_SetSpeed(void *InstancePtr, u32 Speed);
s32 IsSuperSpeed(struct Usb_DevData *InstancePtr);
void SetConfigDone(void *InstancePtr, u8 Flag);
u8 GetConfigDone(void *InstancePtr);
void Ep0StallRestart(void *InstancePtr);
void SetEpInterval(void *InstancePtr, u8 UsbEpNum, u8 Dir, u32 Interval);
void StopTransfer(void *InstancePtr, u8 EpNum, u8 Dir);
s32 StreamOn(void *InstancePtr, u8 EpNum, u8 Dir, u8 *BufferPtr);	/**< Stream On(void * Instance Ptr, u8  Ep Num, u8  Dir, u8 * ... */
void StreamOff(void *InstancePtr, u8 EpNum, u8 Dir);

#ifdef __cplusplus
}
#endif

#endif  /* End of protection macro. */
