/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/****************************************************************************/
/**
 *
 * @file xusb_freertos_audio.c
 *
 * This file implements the ISO IN and ISO OUT data transfer. It transfers and
 * receives audio data on High or Super Speed bus based on connection.
 *
 * @setup requirement
 * zcu102 board in usb device mode, connected with host using USB 3.0 cable
 *
 * Following other files require to run this example
 *  o xusb_ch9.c, xusb_ch9.h
 *  o xusb_freertos_ch9_audio.c, xusb_freerots_ch9_audio.h
 *  o xusb_freertos_class_audio.c, xusb_freertos_class_audio.h
 *
 * @validation
 *  o on success example will be detected as audio device
 *  o use aplay/arecord for play or record audio
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   26/03/18 First release
 * 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 * 1.15  pm   12/15/23 Added support for system device-tree flow.
 * 1.18  ka   08/21/25 Fixed GCC warnings.
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "FreeRTOS.h"
#include "task.h"
#include "xparameters.h"

#include "xusb_freertos_ch9_audio.h"
#include "xusb_freertos_class_audio.h"

/************************** Constant Definitions ****************************/
#ifndef SDT
#define USB_INTR_ID			XPAR_XUSBPS_0_INTR
#else
#define INTRNAME_DWC3USB3	0 /* Interrupt-name - USB */
#define XUSBPSU_BASEADDRESS	XPAR_XUSBPSU_0_BASEADDR /* USB base address */
#endif

/* The following constants are to be modified to get different size of memory */
#define RAMDISKSECTORS		0x400   /* 1KB */
#define RAMBLOCKS		4096
#define MEMORY_SIZE		(64 * 1024)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
struct Usb_DevData UsbInstance;

#ifdef __ICCARM__
#pragma data_alignment = 32
u8 Buffer[MEMORY_SIZE];
#else
u8 Buffer[MEMORY_SIZE] ALIGNMENT_CACHELINE;
#endif

/* Supported AUDIO sampling frequencies */
u8 audio_freq[MAX_AUDIO_FREQ][3] = {
	{ 0x40, 0x1F, 0x00 },	/* sample frequency 8000  */
	{ 0x44, 0xAC, 0x00 },	/* sample frequency 44100 */
	{ 0x80, 0xBB, 0x00 },	/* sample frequency 48000 */
	{ 0x00, 0x77, 0x01,},	/* sample frequency 96000 */
};

u8 BufferPtrTemp[1024];
u8 VirtFlash[0x10000000];

struct audio_dev audio_dev = {
	.virtualdisk = VirtFlash,
	.disksize = sizeof(VirtFlash),
};

/* Initialize a ch9 data structure */
static USBCH9_DATA iso_data = {
	.ch9_func = {
		/* Set the chapter9 hooks */
		.Usb_Ch9SetupDevDescReply = Usb_Ch9SetupDevDescReply,
		.Usb_Ch9SetupCfgDescReply = Usb_Ch9SetupCfgDescReply,
		.Usb_Ch9SetupBosDescReply = Usb_Ch9SetupBosDescReply,
		.Usb_Ch9SetupStrDescReply = Usb_Ch9SetupStrDescReply,
		.Usb_SetConfiguration = Usb_SetConfiguration,
		.Usb_SetConfigurationApp = Usb_SetConfigurationApp,
		.Usb_SetInterfaceHandler = Usb_SetInterfaceHandler,
		.Usb_ClassReq = Usb_ClassReq,
	},
	.data_ptr = (void *) &audio_dev,
};

/****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The USB controller could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	InstPtr is a pointer to the XUsb instance.
* @param	UsbIntrId is the interrupt ID of the USB controller
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void SetupInterruptSystem(struct XUsbPsu *InstancePtr, u16 UsbIntrId)
{
	xPortInstallInterruptHandler(UsbIntrId,
				     (XInterruptHandler)XUsbPsu_IntrHandler,
				     (void *)InstancePtr);

	XUsbPsu_EnableIntr(InstancePtr, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
			   XUSBPSU_DEVTEN_WKUPEVTEN |
			   XUSBPSU_DEVTEN_ULSTCNGEN |
			   XUSBPSU_DEVTEN_CONNECTDONEEN |
			   XUSBPSU_DEVTEN_USBRSTEN |
			   XUSBPSU_DEVTEN_DISCONNEVTEN);

	vPortEnableInterrupt(UsbIntrId);
}

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
static void set_audio_transfer_size(struct audio_dev *f_audio)
{
	u32 Rate = 0, AudioFreq = 0, MaxPacketsize = 0;

	/* Audio sampling frequency which filled in TYPE One Format desc */
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
static void Usb_IsoOutHandler(void *CallBackRef, u32 RequestedBytes,
			      u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *) Get_DrvData(InstancePtr->PrivateData);
	struct audio_dev *dev = (struct audio_dev *)(ch9_ptr->data_ptr);
	BaseType_t xHigherPriorityTaskWoken;

	(void)RequestedBytes;

	dev->bytesRecv = BytesTxed;
	xSemaphoreGiveFromISR(dev->xSemaphorePlay, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
static void Usb_IsoInHandler(void *CallBackRef, u32 RequestedBytes,
			     u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *) Get_DrvData(InstancePtr->PrivateData);
	struct audio_dev *dev = (struct audio_dev *)(ch9_ptr->data_ptr);
	BaseType_t xHigherPriorityTaskWoken;

	(void)RequestedBytes;
	(void)BytesTxed;

	xSemaphoreGiveFromISR(dev->xSemaphoreRecord, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/****************************************************************************/
/**
* This function is implementing USB audio example.
*
* @param	UsbInstPtr USB instance pointer.
*		dev audio device instance pointer
*		DeviceId USB Device ID
*		UsbIntrId USB Interrupt ID
*
* @return	- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
static int XUsbAudioExample(struct Usb_DevData *UsbInstPtr,
			    struct audio_dev *dev, u16 DeviceId, u16 UsbIntrId)
#else
static int XUsbAudioExample(struct Usb_DevData *UsbInstPtr,
			    struct audio_dev *dev)
#endif
{
	s32 Status;
	Usb_Config *UsbConfigPtr;

	xil_printf("FreeRTOS Xilinx Audio Start...\r\n");

#ifndef SDT
	UsbConfigPtr = LookupConfig(DeviceId);
#else
	UsbConfigPtr = LookupConfig(XUSBPSU_BASEADDRESS);
#endif
	if (NULL == UsbConfigPtr) {
		xil_printf("LookupConfig failed\r\n");
		return XST_FAILURE;
	}

	Status = CfgInitialize(UsbInstPtr, UsbConfigPtr,
			       UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("CfgInitialize failed: %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Hook up chapter9 handler */
	Set_Ch9Handler(UsbInstPtr->PrivateData, Ch9Handler);

	/* Assign the data to USB driver */
	Set_DrvData(UsbInstPtr->PrivateData, &iso_data);

	EpConfigure(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_IN,
		    USB_EP_TYPE_ISOCHRONOUS);
	EpConfigure(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_OUT,
		    USB_EP_TYPE_ISOCHRONOUS);

	Status = ConfigureDevice(UsbInstPtr->PrivateData, &Buffer[0],
				 MEMORY_SIZE);
	if (XST_SUCCESS != Status) {
		xil_printf("ConfigureDevice failed: %d\r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * set endpoint handlers
	 * XUsbPsu_IsoInHandler -  to be called when data is sent
	 * XUsbPsu_IsoOutHandler -  to be called when data is received
	 */
	SetEpHandler(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_IN,
		     Usb_IsoInHandler);

	SetEpHandler(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_OUT,
		     Usb_IsoOutHandler);

	/* Setup interrupts */
	SetupInterruptSystem(UsbInstPtr->PrivateData,
#ifndef SDT
			     UsbIntrId
#else
			     UsbConfigPtr->IntrId[INTRNAME_DWC3USB3]
#endif
			     );

	set_audio_transfer_size(&audio_dev);

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstPtr->PrivateData);

	while (1) {
		BaseType_t Ret;
		u32 Notification;

		Ret = xTaskNotifyWait(0x00, 0x00, &Notification, portMAX_DELAY);
		if (Ret == pdTRUE) {
			if (Notification & RECORD_START)
				xTaskCreate(prvRecordTask,
					    (const char *) "Record Task",
					    configMINIMAL_STACK_SIZE,
					    UsbInstPtr,
					    tskIDLE_PRIORITY + 2,
					    &(dev->xRecordTask));

			if (Notification & PLAY_START)
				xTaskCreate(prvPlayBackTask,
					    (const char *) "PlayBack Task",
					    configMINIMAL_STACK_SIZE,
					    UsbInstPtr,
					    tskIDLE_PRIORITY + 2,
					    &(dev->xPlayTask));

			if (Notification & RECORD_STOP) {
				StreamOff(UsbInstPtr->PrivateData, ISO_EP,
					  USB_EP_DIR_IN);
				EpDisable(UsbInstPtr->PrivateData, ISO_EP,
					  USB_EP_DIR_IN);
				vSemaphoreDelete(dev->xSemaphoreRecord);
				vTaskDelete(dev->xRecordTask);
			}

			if (Notification & PLAY_STOP) {
				StreamOff(UsbInstPtr->PrivateData, ISO_EP,
					  USB_EP_DIR_OUT);
				EpDisable(UsbInstPtr->PrivateData, ISO_EP,
					  USB_EP_DIR_OUT);
				vSemaphoreDelete(dev->xSemaphorePlay);
				vTaskDelete(dev->xPlayTask);
			}
		}
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This task implements audio functionality
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
static void prvMainTask(void *pvParameters)
{
	s32 Status;

	(void)pvParameters;

#ifndef SDT
	Status = XUsbAudioExample(&UsbInstance, &audio_dev, USB_DEVICE_ID,
				  USB_INTR_ID);
#else
	Status = XUsbAudioExample(&UsbInstance, &audio_dev);
#endif
	if (Status == XST_FAILURE) {
		xil_printf("FreeRTOS USB Audio Example failed\r\n");
		vTaskDelete(NULL);
	}
}

/****************************************************************************/
/**
 * This function is the main function of the USB audio example.
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if successful,
 *		- XST_FAILURE if unsuccessful.
 *
 * @note	None.
 *
 *
 *****************************************************************************/
int main(void)
{
	CacheInit();

	xTaskCreate(prvMainTask, (const char *) "Audio",
		    configMINIMAL_STACK_SIZE, NULL,	tskIDLE_PRIORITY,
		    &audio_dev.xMainTask);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return XST_SUCCESS;
}
