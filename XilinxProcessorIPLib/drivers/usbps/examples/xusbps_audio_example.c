/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbps_audio_example.c
 *
 * This file contains the implementation of chapter 9 specific code for
 * the example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who	Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   pm	20/02/20 First release
 * 2.8   pm	07/07/23 Added support for system device-tree flow.
 * 2.10  ka     21/08/25 Fixed GCC warnings
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"

#include "xusbps_ch9_audio.h"
#include "xusbps_class_audio.h"

#include "xil_exception.h"
#include "xpseudo_asm.h"
#include "xreg_cortexa9.h"
#include "xil_cache.h"

#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif

#ifdef XUSBPS_MICROPHONE
#include "xusbps_audiodata.h"
#endif

/************************** Constant Definitions ****************************/
#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))

/*
 * The following constants are to be modified to get different size of memory.
 */
#define RAMDISKSECTORS  	0x400		//1KB
#define RAMBLOCKS		4096

#define MEMORY_SIZE (64 * 1024)
#ifdef __ICCARM__
#pragma data_alignment = 32
u8 Buffer[MEMORY_SIZE];
#pragma data_alignment = 4
#else
u8 Buffer[MEMORY_SIZE] ALIGNMENT_CACHELINE;
#endif

#ifdef XUSBPS_UAC1

/*
 * Default is 8000Hz
 * Change this value to set different sampling rate.
 * 		u8 AudioFreq [MAX_AudioFreq][3] ={
 * 			{ 0x40, 0x1F, 0x00 }, // sample frequency 8000
 * 			{ 0x44, 0xAC, 0x00 }, // sample frequency 44100
 * 			{ 0x80, 0xBB, 0x00 }, // sample frequency 48000
 * 			{ 0x00, 0x77, 0x01,}, // sample frequency 96000
 *		};
 */
#define CUR_AUDIOFREQ		0x00

#else	/*	XUSPBS_UAC2 */

/*
 * Default is 44100Hz
 * Change this value to set different sampling rate.
 * 		u8 AudioFreq [MAX_AudioFreq][3] ={
 * 			{ 0x40, 0x1F, 0x00 }, // sample frequency 8000
 * 			{ 0x44, 0xAC, 0x00 }, // sample frequency 44100
 * 			{ 0x80, 0xBB, 0x00 }, // sample frequency 48000
 * 			{ 0x00, 0x77, 0x01,}, // sample frequency 96000
 *		};
 */
#define CUR_AUDIOFREQ		0x01

#endif

#ifndef SDT
#define USB_DEVICE_ID		XPAR_XUSBPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define	USB_INTR_ID		XPAR_XUSBPS_0_INTR
#else
#define USBPS_BASEADDR		XPS_USB0_BASEADDR /* USBPS base address */
#endif
/************************** Function Prototypes ******************************/
static void XUsbPs_IsoInHandler(void *CallBackRef, u32 RequestedBytes,
				u32 BytesTxed );
static void XUsbPs_IsoOutHandler(void *CallBackRef, u32 RequestedBytes,
				 u32 BytesTxed );
static void XUsbPs_AudioTransferSize(void);
static void XUsbPs_Ep0EventHandler(void *CallBackRef, u8 EpNum, u8 EventType, void *Data);
s32 XUsbPs_CfgInit(struct Usb_DevData *InstancePtr, Usb_Config *ConfigPtr,
		   u32 BaseAddress);

#ifndef SDT
s32 XUsbPs_SetupInterruptSystem(XUsbPs *InstancePtr, u16 IntcDeviceID,
				XScuGic *IntcInstancePtr);
#endif

/************************** Variable Definitions *****************************/
struct Usb_DevData UsbInstance;

Usb_Config *UsbConfigPtr;
XUsbPs PrivateData;

/*
 * Interrupt controller instance
 */
#ifndef SDT
XScuGic InterruptController;
#endif

XUsbPs_DeviceConfig DeviceConfig;

/*
 * A ram array
 */
u32 RamDisk[RAMDISKSECTORS * RAMBLOCKS] __attribute__ ((aligned(4)));
u8 *WrRamDiskPtr = (u8 *) & (RamDisk[0]);

u8 BufferPtrTemp[1024];

u32 Index = 0;
u8 FirstPktFrame = 1;

u32 Framesize = 0, Interval = 0, PacketSize = 0,
    PacketResidue = 0, Residue = 0;

#ifdef XUSBPS_MICROPHONE
static u32 FileSize = sizeof(Hello_wav);
#else
static u32 FileSize = sizeof(RamDisk);
#endif

/* Supported AUDIO sampling frequencies */
u8 AudioFreq [MAX_AUDIO_FREQ][3] = {
	{ 0x40, 0x1F, 0x00 },	/* sample frequency 8000  */
	{ 0x44, 0xAC, 0x00 },	/* sample frequency 44100 */
	{ 0x80, 0xBB, 0x00 },	/* sample frequency 48000 */
	{ 0x00, 0x77, 0x01,},	/* sample frequency 96000 */
};

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
	const u8 NumEndpoints = 2;
	u8 *MemPtr = NULL;
	s32 Status;

	xil_printf("Xilinx Audio Start...\r\n");

#ifndef SDT
	UsbConfigPtr = XUsbPs_LookupConfig(USB_DEVICE_ID);
#else
	UsbConfigPtr = XUsbPs_LookupConfig(USBPS_BASEADDR);
#endif
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	Status = XUsbPs_CfgInit(&UsbInstance, UsbConfigPtr,
				UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Assign the ep configuration to USB driver
	 */

	DeviceConfig.EpCfg[0].Out.Type = XUSBPS_EP_TYPE_CONTROL;
	DeviceConfig.EpCfg[0].Out.NumBufs = 2;
	DeviceConfig.EpCfg[0].Out.BufSize = 64;
	DeviceConfig.EpCfg[0].Out.MaxPacketSize = 64;
	DeviceConfig.EpCfg[0].In.Type = XUSBPS_EP_TYPE_CONTROL;
	DeviceConfig.EpCfg[0].In.NumBufs = 2;
	DeviceConfig.EpCfg[0].In.MaxPacketSize = 64;

	DeviceConfig.EpCfg[1].Out.Type = XUSBPS_EP_TYPE_ISOCHRONOUS;
	DeviceConfig.EpCfg[1].Out.NumBufs = 16;
	DeviceConfig.EpCfg[1].Out.BufSize = 1024;
	DeviceConfig.EpCfg[1].Out.MaxPacketSize = 1024;
	DeviceConfig.EpCfg[1].In.Type = XUSBPS_EP_TYPE_ISOCHRONOUS;
	DeviceConfig.EpCfg[1].In.NumBufs = 16;
	DeviceConfig.EpCfg[1].In.MaxPacketSize = 1024;

	DeviceConfig.NumEndpoints = NumEndpoints;

	MemPtr = (u8 *) &Buffer[0];
	memset(MemPtr, 0, MEMORY_SIZE);
	Xil_DCacheFlushRange((unsigned int) MemPtr, MEMORY_SIZE);

	/* Finish the configuration of the DeviceConfig structure and configure
	 * the DEVICE side of the controller.
	 */
	DeviceConfig.DMAMemPhys = (u32) MemPtr;

	Status = XUsbPs_ConfigureDevice(UsbInstance.PrivateData, &DeviceConfig);

	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Hook up chapter9 handler
	 */
	Status = XUsbPs_EpSetHandler(UsbInstance.PrivateData, 0,
				     XUSBPS_EP_DIRECTION_OUT,
				     (XUsbPs_EpHandlerFunc)XUsbPs_Ep0EventHandler,
				     UsbInstance.PrivateData);

	/*
	 * set endpoint handlers
	 * XUsbPsu_IsoInHandler -  to be called when data is sent
	 * XUsbPsu_IsoOutHandler -  to be called when data is received
	 */
	XUsbPs_EpSetIsoHandler(UsbInstance.PrivateData, ISO_EP,
			       XUSBPS_EP_DIRECTION_IN,
			       XUsbPs_IsoInHandler);

	XUsbPs_EpSetIsoHandler(UsbInstance.PrivateData, ISO_EP,
			       XUSBPS_EP_DIRECTION_OUT,
			       XUsbPs_IsoOutHandler);

	/*
	 * Setup interrupts
	 */
#ifndef SDT
	Status = XUsbPs_SetupInterruptSystem((XUsbPs *)UsbInstance.PrivateData,
					     INTC_DEVICE_ID,
					     &InterruptController);
#else
	Status = XSetupInterruptSystem((XUsbPs *)UsbInstance.PrivateData,
				       &XUsbPs_IntrHandler,
				       UsbConfigPtr->IntrId,
				       UsbConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);

	XUsbPs_IntrEnable((XUsbPs *)UsbInstance.PrivateData,
			  XUSBPS_IXR_UR_MASK | XUSBPS_IXR_UI_MASK);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XUsbPs_AudioTransferSize();

	/*
	 * Start the controller so that Host can see our device
	 */
	XUsbPs_Start((XUsbPs *)UsbInstance.PrivateData);

	while (1) {
		/*
		 * Rest is taken care by interrupts
		 */
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * This function calculates Data to be sent at every Interval
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None.
 *
 *
 *****************************************************************************/
static void XUsbPs_AudioTransferSize(void)

{
	u32 Rate = 0, AudioFreqTemp = 0, MaxPacketSize = 0;

	/*
	 * Audio sampling frequency which filled in TYPE One Format
	 * descriptors
	 */
	AudioFreqTemp = (u32)((u8)AudioFreq[CUR_AUDIOFREQ][0] |
			      (u8)AudioFreq[CUR_AUDIOFREQ][1] << 8 |
			      (u8)AudioFreq[CUR_AUDIOFREQ][2] << 16);

	/*
	 * Audio transmission Bytes required to send in one sec
	 * (Sampling Freq * Number of Channel * Audio frame size)
	 */
	Framesize = AUDIO_CHANNEL_NUM * AUDIO_FRAME_SIZE;
	Rate = AudioFreqTemp * Framesize;
	Interval = INTERVAL_PER_SECOND / (1 << (AUDIO_INTERVAL - 1));

	/*
	 * Audio data transfer size to be transferred at every interval
	 */
	MaxPacketSize = AUDIO_CHANNEL_NUM * AUDIO_FRAME_SIZE *
			DIV_ROUND_UP(AudioFreqTemp, INTERVAL_PER_SECOND /
				     (1 << (AUDIO_INTERVAL - 1)));
	PacketSize = ((Rate / Interval) < MaxPacketSize) ?
		     (Rate / Interval) : MaxPacketSize;

	if (PacketSize < MaxPacketSize) {
		PacketResidue = Rate % Interval;
	} else {
		PacketResidue = 0;
	}
}

/****************************************************************************/
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
static void XUsbPs_IsoInHandler(void *CallBackRef, u32 RequestedBytes,
				u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	u32 Size;

	(void)RequestedBytes;
        (void)BytesTxed;

	Size = PacketSize;
	Residue += PacketResidue;

	if ((Residue / Interval) >= Framesize) {
		Size += Framesize;
		Residue -= Framesize * Interval;
	}

	if ((Index + Size) > FileSize) {
		/* Buffer is completed, retransmitting the same file data */
		Index = 0;
	}

#ifdef XUSBPS_MICROPHONE
	if (XUsbPs_EpBufferSend((XUsbPs *)InstancePtr->PrivateData, ISO_EP,
				&Hello_wav[Index],
				Size) == XST_SUCCESS) {
#else	/* XUSPBS_UAC2 */
	if (XUsbPs_EpBufferSend((XUsbPs *)InstancePtr->PrivateData, ISO_EP,
				&WrRamDiskPtr[Index],
				Size) == XST_SUCCESS) {
#endif
		Index += Size;
		if (FirstPktFrame) {
			Size = PacketSize;
			Residue += PacketResidue;

			if ((Residue / Interval) >= Framesize) {
				Size += Framesize;
				Residue -= Framesize * Interval;
			}

			if ((Index + Size) > FileSize) {
				Index = 0;
			} else {
				Index += Size;
			}

			FirstPktFrame = 0;
		}
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
static void XUsbPs_IsoOutHandler(void *CallBackRef, u32 RequestedBytes,
				 u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;
	u32 Size;

	(void)RequestedBytes;

	Size = PacketSize;
	Residue += PacketResidue;

	if ((Residue / Interval) >= Framesize) {
		Size += Framesize;
		Residue -= Framesize * Interval;
	}

	if (FirstPktFrame) {
		FirstPktFrame = 0;
	} else {
		if ((Index + BytesTxed) > FileSize) {
			/* Buffer is full, overwriting the data */
			Index = 0;
		}

		/* Copy received to RAM array */
		memcpy(&WrRamDiskPtr[Index], BufferPtrTemp, BytesTxed);
		Index += BytesTxed;
	}

	XUsbPs_EpDataBufferReceive((XUsbPs *)InstancePtr->PrivateData, ISO_EP,
				   BufferPtrTemp, Size);
}


/*****************************************************************************/
/**
 * This function is registered to handle callbacks for endpoint 0 (Control).
 *
 * It is called from an interrupt context such that the amount of processing
 * performed should be minimized.
 *
 *
 * @param	CallBackRef is the reference passed in when the function
 *		was registered.
 * @param	EpNum is the Number of the endpoint on which the event occurred.
 * @param	EventType is type of the event that occurred.
 *
 * @return	None.
 *
 ******************************************************************************/
static void XUsbPs_Ep0EventHandler(void *CallBackRef, u8 EpNum, u8 EventType, void *Data)
{
	XUsbPs *InstancePtr;
	int Status;
	XUsbPs_SetupData SetupData;
	u8 *BufferPtr;
	u32 BufferLen;
	u32 Handle;

	(void)Data;

	Xil_AssertVoid(NULL != CallBackRef);

	InstancePtr = (XUsbPs *) CallBackRef;


	switch (EventType) {

		/* Handle the Setup Packets received on Endpoint 0. */
		case XUSBPS_EP_EVENT_SETUP_DATA_RECEIVED:
			Status = XUsbPs_EpGetSetupData(InstancePtr, EpNum, &SetupData);
			if (XST_SUCCESS == Status) {
				/* Handle the setup packet. */
				(int) XUsbPs_Ch9HandleSetupPacket((XUsbPs *)InstancePtr,
								  &SetupData);
			}
			break;

		/* We get data RX events for 0 length packets on endpoint 0.
		 * We receive and immediately release them again here, but
		 * there's no action to be taken.
		 */
		case XUSBPS_EP_EVENT_DATA_RX:
			/* Get the data buffer. */
			Status = XUsbPs_EpBufferReceive(InstancePtr, EpNum, &BufferPtr,
							&BufferLen, &Handle);
			if (XST_SUCCESS == Status) {
				/* Return the buffer. */
				XUsbPs_EpBufferRelease(Handle);
			}
			break;

		default:
			/* Unhandled event. Ignore. */
			break;
	}
}

/****************************************************************************/
/**
 * This function setups the interrupt system such that interrupts can occur.
 * This function is application specific since the actual system may or may not
 * have an interrupt controller.  The USB controller could be
 * directly connected to aprocessor without an interrupt controller.
 * The user should modify this function to fit the application.
 *
 * @param	InstancePtr is a pointer to the XUsbPs instance.
 * @param	IntcDeviceID is the unique ID of the interrupt controller
 * @param	IntcInstacePtr is a pointer to the interrupt controller
 *			instance.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *****************************************************************************/
#ifndef SDT
s32 XUsbPs_SetupInterruptSystem(XUsbPs *InstancePtr, u16 IntcDeviceID,
				XScuGic *IntcInstancePtr)
{
	s32 Status;
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(IntcDeviceID);
	if (IntcConfig == NULL) {
		return XST_FAILURE;
	}
	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Xil_ExceptionInit();
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, USB_INTR_ID,
				 (Xil_ExceptionHandler)XUsbPs_IntrHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	/*
	 * Enable the interrupt for the device.
	 */
	XScuGic_Enable(IntcInstancePtr, USB_INTR_ID);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

	/* Enable the interrupts. */
	XUsbPs_IntrEnable(InstancePtr, XUSBPS_IXR_UR_MASK | XUSBPS_IXR_UI_MASK);


	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function initializes a XUsbPs instance/driver.
*
* The initialization entails:
* - Initialize all members of the XUsbPs structure.
*
* @param	InstancePtr is a pointer to XUsbPs instance of the controller.
* @param	ConfigPtr is a pointer to a XUsbPs_Config configuration
*		structure. This structure will contain the requested
*		configuration for the device. Typically, this is a local
*		structure and the content of which will be copied into the
*		configuration structure within XUsbPs.
* @param	BaseAddress is the base address of the device.
*
* @return
*		- XST_SUCCESS no errors occurred.
*		- XST_FAILURE an error occurred during initialization.
*
* @note
*
******************************************************************************/

s32 XUsbPs_CfgInit(struct Usb_DevData *InstancePtr, Usb_Config *ConfigPtr,
		   u32 BaseAddress)
{
	PrivateData.AppData = InstancePtr;
	InstancePtr->PrivateData = (void *)&PrivateData;

	return XUsbPs_CfgInitialize((XUsbPs *)InstancePtr->PrivateData,
				    ConfigPtr, BaseAddress);
}
