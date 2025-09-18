/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/****************************************************************************/
/**
 *
 * @file xusb_composite_example.c
 *
 * This file implements mass storage, hid, audio and dfu all in one composite
 * device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   05/03/18 First release
 * 1.14  pm   21/06/23 Added support for system device-tree flow.
 * 1.18  ka   21/08/25 Fixed GCC warnings
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xusb_ch9_composite.h"
#include "xusb_class_composite.h"

#include "xparameters.h"
#include "xscugic.h"

#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
/************************** Constant Definitions ****************************/
#ifndef SDT
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define USB_INTR_ID             XPAR_XUSBPS_0_INTR
#define USB_WAKEUP_INTR_ID      XPAR_XUSBPS_0_WAKE_INTR
#else
#define INTRNAME_DWC3USB3	0 /* Interrupt-name - USB */
#define INTRNAME_HIBER		2 /* Interrupt-name - Hiber */
#define XUSBPSU_BASEADDRESS	XPAR_XUSBPSU_0_BASEADDR /* USB base address */
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

struct Usb_DevData UsbInstance;
XScuGic	InterruptController;	/* Interrupt controller instance */

/* Supported AUDIO sampling frequencies */
u8 audio_freq[MAX_AUDIO_FREQ][3] = {
	{ 0x40, 0x1F, 0x00 },	/* sample frequency 8000  */
	{ 0x44, 0xAC, 0x00 },	/* sample frequency 44100 */
	{ 0x80, 0xBB, 0x00 },	/* sample frequency 48000 */
	{ 0x00, 0x77, 0x01,},	/* sample frequency 96000 */
};

static u32 KeySent = 1;

u8	BufferPtrTemp[1024];
u8	StorageDisk[0x6400000];		// 100MB for storage
u8	AudioDisk[0x3200000];		// 50MB for audio
u8	DfuDisk[0x3200000];		// 50MB for DFU

struct composite_dev comp_dev = {
	.f_dfu.disk = DfuDisk,
	.f_dfu.disksize = sizeof(DfuDisk),
	.f_audio.disk = AudioDisk,
	.f_audio.disksize = sizeof(AudioDisk),
	.f_storage.disk = StorageDisk,
	.f_storage.disksize = sizeof(StorageDisk),
};

/* Initialize a Composite data structure */
static USBCH9_DATA composite_data = {
	.ch9_func = {
		.Usb_Ch9SetupDevDescReply = Usb_Ch9SetupDevDescReply,
		.Usb_Ch9SetupCfgDescReply = Usb_Ch9SetupCfgDescReply,
		.Usb_Ch9SetupBosDescReply = Usb_Ch9SetupBosDescReply,
		.Usb_Ch9SetupStrDescReply = Usb_Ch9SetupStrDescReply,
		.Usb_SetConfiguration = Usb_SetConfiguration,
		.Usb_SetConfigurationApp = Usb_SetConfigurationApp,
		.Usb_SetInterfaceHandler = Usb_SetIntf,
		.Usb_ClassReq = Usb_ClassReq,
		.Usb_GetDescReply = Usb_GetDescReply,
	},
	.data_ptr = (void *) &comp_dev,
};

/****************************************************************************/
/**
 * This function calculates Data to be sent at every Interval
 *
 * @param	f_audio audio interface instance pointer
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
static void set_audio_transfer_size(struct audio_if *f_audio)
{
	u32 Rate = 0, AudioFreq = 0, MaxPacketsize = 0;

	/*
	 * Audio sampling frequency which filled in TYPE One Format
	 * descriptors
	 */
	AudioFreq = (u32)((u8)audio_freq[CUR_AUDIO_FREQ][0] |
			  (u8)audio_freq[CUR_AUDIO_FREQ][1] << 8 |
			  (u8)audio_freq[CUR_AUDIO_FREQ][2] << 16);

	/*
	 * Audio transmission Bytes required to send in one sec
	 * (Sampling Freq * Number of Channel * Audio frame size)
	 */
	f_audio->framesize = AUDIO_CHANNEL_NUM * AUDIO_FRAME_SIZE;
	Rate = AudioFreq * f_audio->framesize;
	f_audio->interval = INTERVAL_PER_SECOND / (1 << (AUDIO_INTERVAL - 1));

	/* Audio data transfer size to be transferred at every interval */
	MaxPacketsize = AUDIO_CHANNEL_NUM * AUDIO_FRAME_SIZE *
			DIV_ROUND_UP(AudioFreq, INTERVAL_PER_SECOND /
				     (1 << (AUDIO_INTERVAL - 1)));
	f_audio->packetsize = ((Rate / f_audio->interval) < MaxPacketsize) ?
			      (Rate / f_audio->interval) : MaxPacketsize;

	if (f_audio->packetsize < MaxPacketsize) {
		f_audio->packetresidue = Rate % f_audio->interval;
	} else {
		f_audio->packetresidue = 0;
	}
}

/****************************************************************************/
/**
 * This function is ISO OUT Endpoint handler/Callback function, called by driver
 * when data is received from host.
 *
 * @param	CallBackRef is pointer to Usb_DevData instance.
 * @param	RequestedBytes is number of bytes requested to send.
 * @param	BytesTxed is actual number of bytes sent to Host.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
static void Usb_AudioOutHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *) Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *interface = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct audio_if *f = &(interface->f_audio);
	u32 Size;

	(void)RequestedBytes;

	Size = f->packetsize;
	f->residue += f->packetresidue;

	if ((f->residue / f->interval) >= f->framesize) {
		Size += f->framesize;
		f->residue -= f->framesize * f->interval;
	}

	if (f->firstpkt) {
		f->firstpkt = 0;
	} else {
		if ((f->index + BytesTxed) > f->disksize) {
			f->index = 0;
		}

		/* Copy received to RAM array */
		memcpy(f->disk + f->index, BufferPtrTemp, BytesTxed);
		f->index += BytesTxed;
	}

	EpBufferRecv(InstancePtr->PrivateData, ISO_EP,
		     BufferPtrTemp, Size);
}

/*****************************************************************************/
/**
 * This function is ISO IN Endpoint handler/Callback function, called by driver
 * when data is sent to host.
 *
 * @param	CallBackRef is pointer to Usb_DevData instance.
 * @param	RequestedBytes is number of bytes requested to send.
 * @param	BytesTxed is actual number of bytes sent to Host.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
static void Usb_AudioInHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *) Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *interface = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct audio_if *f = &(interface->f_audio);
	u32 Size;

	(void)RequestedBytes;
	(void)BytesTxed;

	Size = f->packetsize;
	f->residue += f->packetresidue;

	if ((f->residue / f->interval) >= f->framesize) {
		Size += f->framesize;
		f->residue -= f->framesize * f->interval;
	}

	/* Buffer is completed, retransmitting the same file data */
	if ((f->index + Size) > f->disksize) {
		f->index = 0;
	}

	if (EpBufferSend(InstancePtr->PrivateData, ISO_EP,
			 f->disk + f->index, Size) == XST_SUCCESS) {
		f->index += Size;

		if (f->firstpkt) {
			Size = f->packetsize;
			f->residue += f->packetresidue;

			if ((f->residue / f->interval) >= f->framesize) {
				Size += f->framesize;
				f->residue -= f->framesize * f->interval;
			}

			/* Buffer is completed, retransmitting the same file data */
			if ((f->index + Size) > f->disksize) {
				f->index = 0;
			} else {
				f->index += Size;
			}

			f->firstpkt = 0;
		}
	}
}

/****************************************************************************/
/**
* This function is Bulk Out Endpoint handler/Callback called by driver when
* data is received.
*
* @param	CallBackRef is pointer to Usb_DevData instance.
* @param	RequestedBytes is number of bytes requested for reception.
* @param	BytesTxed is actual number of bytes received from Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
static void Usb_StorageOutHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *) Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct storage_if *f = &(dev->f_storage);

	(void)RequestedBytes;
	(void)BytesTxed;

	if (f->phase == USB_EP_STATE_COMMAND) {
		ParseCBW(InstancePtr, f);

	} else if (f->phase == USB_EP_STATE_DATA_OUT) {
		/* WRITE command */
		switch (f->cbw.CBWCB[0]) {
			case USB_RBC_WRITE:
				f->diskptr += BytesTxed;
				f->bytesleft -= BytesTxed;
				break;
			default:
				break;
		}
		SendCSW(InstancePtr, f, 0);
	}
}

/****************************************************************************/
/**
* This function is Bulk In Endpoint handler/Callback called by driver when
* data is sent.
*
* @param	CallBackRef is pointer to Usb_DevData instance.
* @param	RequestedBytes is number of bytes requested to send.
* @param	BytesTxed is actual number of bytes sent to Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
static void Usb_StorageInHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *) Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct storage_if *f = &(dev->f_storage);

	(void)RequestedBytes;
	(void)BytesTxed;

	if (f->phase == USB_EP_STATE_DATA_IN) {
		/* Send the status */
		SendCSW(InstancePtr, f, 0);

	} else if (f->phase == USB_EP_STATE_STATUS) {
		f->phase = USB_EP_STATE_COMMAND;
		/* Receive next CBW */
		EpBufferRecv(InstancePtr->PrivateData, STORAGE_EP,
			     (u8 *)&f->cbw, sizeof(f->cbw));
	}
}

/****************************************************************************/
/**
* This function is IN Endpoint handler/Callback called by driver when
* data is sent.
*
* @param	CallBackRef is pointer to Usb_DevData instance.
* @param	RequestedBytes is number of bytes requested to send.
* @param	BytesTxed is actual number of bytes sent to Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
static void Usb_KeyboardInHabdler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	(void)CallBackRef;
	(void)RequestedBytes;
	(void)BytesTxed;

	KeySent = 1;
}

#ifndef SDT
/****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The USB controller could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	IntcDeviceID is the unique ID of the interrupt controller
* @param	USB_INTR_ID is the interrupt ID of the USB controller
* @param	IntcPtr is a pointer to the interrupt controller
*			instance.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static s32 SetupInterruptSystem(struct XUsbPsu *InstancePtr, u16 IntcDeviceID,
				u16 UsbIntrId, void *IntcPtr)
{
	s32 Status;

	XScuGic_Config *IntcConfig; /* The configuration parameters of the
				       interrupt controller */

	XScuGic *IntcInstancePtr = (XScuGic *)IntcPtr;

	/* Initialize the interrupt controller driver */
	IntcConfig = XScuGic_LookupConfig(IntcDeviceID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Connect to the interrupt controller */
	Status = XScuGic_Connect(IntcInstancePtr, UsbIntrId,
				 (Xil_ExceptionHandler)XUsbPsu_IntrHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef XUSBPSU_HIBERNATION_ENABLE
	Status = XScuGic_Connect(IntcInstancePtr, USB_WAKEUP_INTR_ID,
				 (Xil_ExceptionHandler)XUsbPsu_WakeUpIntrHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/* Enable the interrupt for the USB */
	XScuGic_Enable(IntcInstancePtr, UsbIntrId);
#ifdef XUSBPSU_HIBERNATION_ENABLE
	XScuGic_Enable(IntcInstancePtr, USB_WAKEUP_INTR_ID);
#endif

	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr(InstancePtr, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
			   XUSBPSU_DEVTEN_WKUPEVTEN |
			   XUSBPSU_DEVTEN_ULSTCNGEN |
			   XUSBPSU_DEVTEN_CONNECTDONEEN |
			   XUSBPSU_DEVTEN_USBRSTEN |
			   XUSBPSU_DEVTEN_DISCONNEVTEN);

#ifdef XUSBPSU_HIBERNATION_ENABLE
	if (InstancePtr->HasHibernation)
		XUsbPsu_EnableIntr(InstancePtr,
				   XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN);
#endif

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);

	/* Enable interrupts in the ARM */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
* This function is implementing USB AUDIO-DFU composte example.
*
* @param	UsbInstPtr USB instance pointer.
*		IntrInstPtr Interrupt controller instance pointer
*		DeviceId USB Device ID
*		IntcDeviceID Interrupt controller Device ID
*
* @return 	- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
static int XUsbCompositeExample(struct Usb_DevData *UsbInstPtr,
				XScuGic *IntrInstPtr, u16 DeviceId, u16 IntcDeviceID, u16 UsbIntrId)
#else
static int XUsbCompositeExample(struct Usb_DevData *UsbInstPtr)
#endif
{
	s32 Status;
	u8 SendKey = 1;
	u8 NoKeyData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	Usb_Config *UsbConfigPtr;

	xil_printf("USB Composite Device Start...\r\n");

	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
#ifndef SDT
	UsbConfigPtr = LookupConfig(DeviceId);
#else
	UsbConfigPtr = LookupConfig(XUSBPSU_BASEADDRESS);
#endif
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(UsbInstPtr, UsbConfigPtr,
			       UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/* hook up chapter9 handler */
	Set_Ch9Handler(UsbInstPtr->PrivateData, Ch9Handler);

	/* Set the disconnect event handler */
	Set_Disconnect(UsbInstPtr->PrivateData, Usb_DisconnectHandler);

	/* Set the reset event handler */
	Set_RstHandler(UsbInstPtr->PrivateData, Usb_ResetHandler);

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstPtr->PrivateData, &composite_data);

	/* Initialize the DFU instance structure */
	comp_dev.f_dfu.InstancePtr = UsbInstPtr;

	/* Set DFU state to APP_IDLE */
	Usb_DfuSetState(&comp_dev.f_dfu, STATE_APP_IDLE);

	/* Set the DFU descriptor pointers, so we can use it when in DFU mode */
	comp_dev.f_dfu.total_transfers = 0;
	comp_dev.f_dfu.total_bytes_dnloaded = 0;
	comp_dev.f_dfu.total_bytes_uploaded = 0;

	/* set handler for in endpoint, will be called when data is sent*/
	SetEpHandler(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_IN,
		     Usb_AudioInHandler);

	/* set handler for out endpoint, will be called when data is recv*/
	SetEpHandler(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_OUT,
		     Usb_AudioOutHandler);

	set_audio_transfer_size(&comp_dev.f_audio);

	/* Mass storage endpoint event hadler registration */
	SetEpHandler(UsbInstance.PrivateData, STORAGE_EP, USB_EP_DIR_OUT,
		     Usb_StorageOutHandler);
	SetEpHandler(UsbInstance.PrivateData, STORAGE_EP, USB_EP_DIR_IN,
		     Usb_StorageInHandler);

	/* Keyboard endpoint event hadler */
	SetEpHandler(UsbInstPtr->PrivateData, KEYBOARD_EP, USB_EP_DIR_IN,
		     Usb_KeyboardInHabdler);


#ifndef SDT
	/* setup interrupts */
	Status = SetupInterruptSystem(UsbInstPtr->PrivateData, IntcDeviceID,
				      UsbIntrId, (void *)IntrInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstPtr->PrivateData);
#else
	Status = XSetupInterruptSystem(UsbInstance.PrivateData,
				       &XUsbPsu_IntrHandler,
				       UsbConfigPtr->IntrId[INTRNAME_DWC3USB3],
				       UsbConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef XUSBPSU_HIBERNATION_ENABLE
	Status = XSetupInterruptSystem(UsbInstance.PrivateData,
				       &XUsbPsu_WakeUpIntrHandler,
				       UsbConfigPtr->IntrId[INTRNAME_HIBER],
				       UsbConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr(UsbInstance.PrivateData,
			   XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
			   XUSBPSU_DEVTEN_WKUPEVTEN |
			   XUSBPSU_DEVTEN_ULSTCNGEN |
			   XUSBPSU_DEVTEN_CONNECTDONEEN |
			   XUSBPSU_DEVTEN_USBRSTEN |
			   XUSBPSU_DEVTEN_DISCONNEVTEN);

#ifdef XUSBPSU_HIBERNATION_ENABLE
	struct XUsbPsu *InstancePtr = UsbInstance.PrivateData;

	if (InstancePtr->HasHibernation)
		XUsbPsu_EnableIntr(UsbInstance.PrivateData,
				   XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN);
#endif
	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstance.PrivateData);
#endif
	while (1) {

		/* After configuration done start sending key */
		while (GetConfigDone(UsbInstPtr->PrivateData)) {
			if (KeySent) {
				if (SendKey) {
					static char KeyData[8] = {2, 0, 0, 0, 0, 0, 0, 0};

					/* get key from serial and send key */
					KeyData[2] = (inbyte() - ('a' - 4));
					EpBufferSend(UsbInstance.PrivateData, KEYBOARD_EP,
						     (u8 *)&KeyData[0], 8);
					SendKey = 0;
				} else {
					/* send key release */
					EpBufferSend(UsbInstance.PrivateData, KEYBOARD_EP,
						     (u8 *)&NoKeyData[0], 8);
					SendKey = 1;
				}
				KeySent = 0;
			}
		}
	}
}

/****************************************************************************/
/**
* This function is the main function of the AUDI-DFU composite example.
*
* @param	None.
*
* @return 	- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{
#ifndef SDT
	if (XUsbCompositeExample(&UsbInstance, &InterruptController,
				 USB_DEVICE_ID, INTC_DEVICE_ID, USB_INTR_ID)) {
#else
	if (XUsbCompositeExample(&UsbInstance)) {
#endif
		xil_printf("USB Composite Example failed\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
