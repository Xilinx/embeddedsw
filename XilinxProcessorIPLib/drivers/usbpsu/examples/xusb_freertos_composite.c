/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * @file xusb_freertos_composite.c
 *
 * This file implements mass storage, hid, audio and dfu all in one composite
 * device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   28/03/18 First release
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xscugic.h"
#include "xusb_freertos_ch9_composite.h"
#include "xusb_freertos_class_composite.h"

/************************** Constant Definitions ****************************/
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define USB_INTR_ID             XPAR_XUSBPS_0_INTR
#define USB_WAKEUP_INTR_ID      XPAR_XUSBPS_0_WAKE_INTR

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

struct Usb_DevData UsbInstance;

XScuGic	InterruptController;	/* Interrupt controller instance */

/* Supported AUDIO sampling frequencies */
u8 audio_freq[MAX_AUDIO_FREQ][3] ={
	{ 0x40, 0x1F, 0x00 },	/* sample frequency 8000  */
	{ 0x44, 0xAC, 0x00 },	/* sample frequency 44100 */
	{ 0x80, 0xBB, 0x00 },	/* sample frequency 48000 */
	{ 0x00, 0x77, 0x01,},	/* sample frequency 96000 */
};

u8 BufferPtrTemp[1024];
u8 StorageDisk[0x6400000];	/* 100MB for storage */
u8 AudioDisk[0x3200000];	/* 50MB for audio */
u8 DfuDisk[0x3200000];		/* 50MB for DFU */

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
	.data_ptr = (void *)&comp_dev,
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

	/* Audio data transfer size to be transfered at every interval */
	MaxPacketsize = AUDIO_CHANNEL_NUM * AUDIO_FRAME_SIZE *
		DIV_ROUND_UP(AudioFreq, INTERVAL_PER_SECOND /
				(1 << (AUDIO_INTERVAL - 1)));
	f_audio->packetsize = ((Rate / f_audio->interval) < MaxPacketsize) ?
		(Rate / f_audio->interval) : MaxPacketsize;

	if (f_audio->packetsize < MaxPacketsize)
		f_audio->packetresidue = Rate % f_audio->interval;
	else
		f_audio->packetresidue = 0;
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
	struct composite_dev *dev = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct audio_if *f = &(dev->f_audio);
	BaseType_t xHigherPriorityTaskWoken;

	f->bytesRecv = BytesTxed;
	xSemaphoreGiveFromISR(f->xSemaphorePlay, &xHigherPriorityTaskWoken);
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
static void Usb_AudioInHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *) Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct audio_if *f = &(dev->f_audio);
	BaseType_t xHigherPriorityTaskWoken;

	xSemaphoreGiveFromISR(f->xSemaphoreRecord, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
	BaseType_t xHigherPriorityTaskWoken;

	xSemaphoreGiveFromISR(f->xSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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
	BaseType_t xHigherPriorityTaskWoken;

	xSemaphoreGiveFromISR(f->xSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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
	struct Usb_DevData *InstancePtr = CallBackRef;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev= (struct composite_dev *)(ch9_ptr->data_ptr);
	struct keyboard_if *f = &(dev->f_keyboard);
	BaseType_t xHigherPriorityTaskWoken;

	xSemaphoreGiveFromISR(f->xSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

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
	if (NULL == IntcConfig)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
			IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Connect to the interrupt controller */
	Status = XScuGic_Connect(IntcInstancePtr, UsbIntrId,
			(Xil_ExceptionHandler)XUsbPsu_IntrHandler,
			(void *)InstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

#ifdef XUSBPSU_HIBERNATION_ENABLE
	Status = XScuGic_Connect(IntcInstancePtr, USB_WAKEUP_INTR_ID,
			(Xil_ExceptionHandler)XUsbPsu_WakeUpIntrHandler,
			(void *)InstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
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
static int XUsbCompositeExample(struct Usb_DevData *UsbInstPtr, XScuGic *IntrInstPtr,
		struct composite_dev* dev, u16 DeviceId, u16 IntcDeviceID, u16 UsbIntrId)
{
	s32 Status;
	Usb_Config *UsbConfigPtr;

	xil_printf("USB Composite Device Start...\r\n");

	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
	UsbConfigPtr = LookupConfig(DeviceId);
	if (NULL == UsbConfigPtr)
		return XST_FAILURE;

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(UsbInstPtr, UsbConfigPtr,
			UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status)
		return XST_FAILURE;

	/* hook up chapter9 handler */
	Set_Ch9Handler(UsbInstPtr->PrivateData, Ch9Handler);

	/* Set the disconnect event handler */
	Set_Disconnect(UsbInstPtr->PrivateData, Usb_DisconnectHandler);

	/* Set the reset event handler */
	Set_RstHandler(UsbInstPtr->PrivateData, Usb_ResetHandler);

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstPtr->PrivateData, &composite_data);

	/* Initialize the DFU instance structure */
	dev->f_dfu.InstancePtr = UsbInstPtr;

	/* Set DFU state to APP_IDLE */
	Usb_DfuSetState(&dev->f_dfu, STATE_APP_IDLE);

	/* Set the DFU descriptor pointers, so we can use it when in DFU mode */
	dev->f_dfu.total_transfers = 0;
	dev->f_dfu.total_bytes_dnloaded = 0;
	dev->f_dfu.total_bytes_uploaded = 0;

	/* set handler for in endpoint, will be called when data is sent*/
	SetEpHandler(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_IN,
			Usb_AudioInHandler);

	/* set handler for out endpoint, will be called when data is recv*/
	SetEpHandler(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_OUT,
			Usb_AudioOutHandler);

	set_audio_transfer_size(&dev->f_audio);

	/* Mass storage endpoint event hadler registration */
	SetEpHandler(UsbInstance.PrivateData, STORAGE_EP, USB_EP_DIR_OUT,
			Usb_StorageOutHandler);
	SetEpHandler(UsbInstance.PrivateData, STORAGE_EP, USB_EP_DIR_IN,
			Usb_StorageInHandler);

	/* Keyboard endpoint event hadler */
	SetEpHandler(UsbInstPtr->PrivateData, KEYBOARD_EP, USB_EP_DIR_IN,
			Usb_KeyboardInHabdler);
	/* setup interrupts */
	Status = SetupInterruptSystem(UsbInstPtr->PrivateData, IntcDeviceID,
			UsbIntrId, (void *)IntrInstPtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstPtr->PrivateData);

	while (1) {
		BaseType_t Ret;
		u32 Notification;

		Ret = xTaskNotifyWait(0x00, 0x00, &Notification, portMAX_DELAY);
		if (Ret == pdTRUE) {
			if (Notification & KEYBOARD_CONFIG) {
				struct keyboard_if *f = &dev->f_keyboard;

				xTaskCreate(prvKeyboardTask, (const char *) "Keyboard Task",
						configMINIMAL_STACK_SIZE,
						UsbInstPtr, tskIDLE_PRIORITY,
						&(f->xKeyboardTask));
			}

			if (Notification & KEYBOARD_UNCONFIG) {
				struct keyboard_if *f = &dev->f_keyboard;

				EpDisable(UsbInstPtr->PrivateData, KEYBOARD_EP, USB_EP_DIR_IN);
				vSemaphoreDelete(f->xSemaphore);
				vTaskDelete(f->xKeyboardTask);
			}

			if (Notification & MSG_CONFIG) {
				struct storage_if *f = &dev->f_storage;

				xTaskCreate(prvSCSITask, (const char *) "SCSI Task",
						configMINIMAL_STACK_SIZE, UsbInstPtr,
						tskIDLE_PRIORITY + 2, &(f->xSCSITask));
			}

			if (Notification & MSG_UNCONFIG) {
				struct storage_if *f = &dev->f_storage;

				EpDisable(UsbInstPtr->PrivateData, STORAGE_EP,
						USB_EP_DIR_IN);
				EpDisable(UsbInstPtr->PrivateData, STORAGE_EP,
						USB_EP_DIR_OUT);

				vSemaphoreDelete(f->xSemaphore);
				vTaskDelete(f->xSCSITask);
			}

			if (Notification & RECORD_START) {
				struct audio_if *f = &(dev->f_audio);

				xTaskCreate(prvRecordTask, (const char *) "Record Task",
						configMINIMAL_STACK_SIZE, UsbInstPtr,
						tskIDLE_PRIORITY + 3, &(f->xRecordTask));
			}

			if (Notification & PLAY_START) {
				struct audio_if *f = &(dev->f_audio);

				xTaskCreate(prvPlayBackTask, (const char *) "PlayBack Task",
						configMINIMAL_STACK_SIZE, UsbInstPtr,
						tskIDLE_PRIORITY + 3, &(f->xPlayTask));
			}

			if (Notification & RECORD_STOP) {
				struct audio_if *f = &(dev->f_audio);

				StreamOff(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_IN);
				EpDisable(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_IN);
				if (f->xSemaphoreRecord) {
					vSemaphoreDelete(f->xSemaphoreRecord);
					vTaskDelete(f->xRecordTask);
				}
			}

			if (Notification & PLAY_STOP) {
				struct audio_if *f = &(dev->f_audio);

				StreamOff(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_OUT);
				EpDisable(UsbInstPtr->PrivateData, ISO_EP, USB_EP_DIR_OUT);
				if (f->xSemaphorePlay) {
					vSemaphoreDelete(f->xSemaphorePlay);
					vTaskDelete(f->xPlayTask);
				}
			}
		}
	}
}

/****************************************************************************/
/**
* This task implements mass storage functionality
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
static void prvMainTask( void *pvParameters)
{
	s32 Status;
	struct composite_dev *dev = pvParameters;

	Status = XUsbCompositeExample(&UsbInstance, &InterruptController, dev,
			USB_DEVICE_ID, INTC_DEVICE_ID, USB_INTR_ID);
	if (Status == XST_FAILURE) {
		xil_printf("USB Composite Example failed\r\n");
		vTaskDelete(NULL);
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

	xTaskCreate( 	prvMainTask,	 		/* The function that implements the task. */
			(const char *) "Mass Storage",	/* Text name for the task, provided to assist debugging only. */
			configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
			&comp_dev, 			/* The task parameter is not used, so set to NULL. */
			tskIDLE_PRIORITY + 4,		/* The task runs at the idle priority. */
			&comp_dev.xMainTask);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return XST_SUCCESS;
}
