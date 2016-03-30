/* $Id: xusb_dma_intr_storage.c,v 1.1.2.1 2011/08/23 11:35:44 vidhum Exp $ */
/******************************************************************************
*
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* (c) Copyright 2007-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
 * @file xusb_dma_intr_storage.c
 *
 * This file has the USB example Mass storage device application function with
 * SCSI command processing and related response preparation being implemented as
 * a part of the interrupt handler.
 *
 * @note	The example is tested on MicroBlaze system on SP605 board,
 *		with caches included in the H/W design.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release.
 * 4.00a hvm  06/01/11 Signature parameter of Command Status word is
 *			initialized with 'U''S''B''S' in ProcessRxCmd function.
 *			CR611761 fix.
 * 4.01a bss  11/01/11 Modified UsbIfIntrHandler function to unconditionally
 *			reset when USB reset is asserted (CR 627574).
 * 4.03a bss  02/05/13 Updated the example to support Zynq
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb.h"
#include "xusb_storage.h"
#include "xenv_standalone.h"
#include "xil_exception.h"
#include "xil_cache.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif


/************************** Constant Definitions *****************************/

#define USB_DEVICE_ID		XPAR_USB_0_DEVICE_ID


#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define USB_INTR		XPAR_INTC_0_USB_0_VEC_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define USB_INTR			XPAR_FABRIC_AXI_USB2_DEVICE_1_USB_IRPT_INTR
#endif /* XPAR_INTC_0_DEVICE_ID */

#define READ_COMMAND		1
#define WRITE_COMMAND		2
#undef XUSB_MS_DEBUG

/************************** Variable Definitions *****************************/

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */


XUsb UsbInstance;		/* The instance of the USB device */
XUsb_Config *UsbConfigPtr;	/* Instance of the USB config structure */

INTC InterruptController;	/* Instance of the Interrupt Controller */


volatile u8 CmdFlag = 0;

volatile u8 FirstPkt = 0;
u8  *WrRamDiskPtr;
u8 *RdRamDiskPtr;
volatile u8 RdIndex = 0;
volatile u8 WrIndex = 0;

volatile u8 TxDmaTransfer = FALSE;
volatile u8 RxDmaTransfer = FALSE;
volatile u8 ResponseSent = FALSE;
volatile u8 SendStatus = FALSE;
volatile u8 RdResponseSent = FALSE;
volatile u8 WrDataPhase = FALSE;
volatile u8 TwoRxInterrupts = FALSE;
volatile u8 TwoTxInterrupts = FALSE;

void ReadTransfer(XUsb *InstancePtr);
void WriteTransfer(XUsb *InstancePtr);

void DmaIntrHandler(void *CallBackRef, u32 IntrStatus);

/*****************************************************************************/
/**
 * This main function starts the USB application.
 *
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if test fails.
 * @note	None.
 *
 *****************************************************************************/
int main()
{
	int Status;

	/*
	 * Initialize the USB driver.
	 */
	UsbConfigPtr = XUsb_LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}
#ifdef __PPC__

	Xil_ICacheEnableRegion (0x80000001);
	Xil_DCacheEnableRegion (0x80000001);
#endif
#ifdef __MICROBLAZE__
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();


	Xil_DCacheInvalidate();
	Xil_DCacheEnable();
#endif
#ifdef __aarch64__
	Xil_DCacheInvalidate();
	Xil_DCacheDisable();
#endif

	/*
	 * We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example. For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = XUsb_CfgInitialize(&UsbInstance,
				    UsbConfigPtr, UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the USB instance as required for the mass storage
	 * application.
	 */
	InitUsbInterface(&UsbInstance);

	/*
	 * Set our function address to 0 which is the unenumerated state.
	 */
	Status = XUsb_SetDeviceAddress(&UsbInstance, 0);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Setup the interrupt handlers.
	 */
	XUsb_IntrSetHandler(&UsbInstance, (void *) UsbIfIntrHandler,
			    &UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 0,
			  (XUsb_EpHandlerFunc *) Ep0IntrHandler, &UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 1,
			  (XUsb_EpHandlerFunc *) Ep1IntrHandler, &UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 2,
			  (XUsb_EpHandlerFunc *) Ep2IntrHandler, &UsbInstance);

	XUsb_DmaIntrSetHandler(&UsbInstance,
			  (XUsb_IntrHandlerFunc *) DmaIntrHandler,
			  &UsbInstance);
	/*
	 * Setup the interrupt system.
	 */
	Status = SetupInterruptSystem(&UsbInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts.
	 */
	XUsb_IntrEnable(&UsbInstance, XUSB_STATUS_GLOBAL_INTR_MASK |
			XUSB_STATUS_RESET_MASK |
			XUSB_STATUS_SUSPEND_MASK |
			XUSB_STATUS_DISCONNECT_MASK |
			XUSB_STATUS_FIFO_BUFF_RDY_MASK |
			XUSB_STATUS_FIFO_BUFF_FREE_MASK |
			XUSB_STATUS_EP0_BUFF1_COMP_MASK |
			XUSB_STATUS_EP1_BUFF1_COMP_MASK |
			XUSB_STATUS_EP2_BUFF1_COMP_MASK |
			XUSB_STATUS_EP1_BUFF2_COMP_MASK |
			XUSB_STATUS_EP2_BUFF2_COMP_MASK |
			XUSB_STATUS_DMA_EVENT_MASK);


	XUsb_Start(&UsbInstance);

	/*
	 * Set the device configuration to unenumerated state.
	 */
	UsbInstance.DeviceConfig.CurrentConfiguration = 0;


	while (1) {

	}
}

/*****************************************************************************/
/**
 * This is the USB initialization function. This example initializes the device
 * for Mass Storage Application. The following configuration is done.
 *	- EP0 : CONTROL end point, Bidirectional, Packet size 64 bytes.
 *	- EP1 : NON_ISOCHRONOUS, BULK_IN, packet size 512 bytes.
 *	- EP2 : NON_ISOCHRONOUS, BULK_OUT, packet size 512 bytes
 *
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void InitUsbInterface(XUsb * InstancePtr)
{

	XUsb_DeviceConfig DeviceConfig;

	/*
	 * Setup Endpoint 0.
	 */
	DeviceConfig.Ep[0].RamBase = 0x22;
	DeviceConfig.Ep[0].Size = 0x40;
	DeviceConfig.Ep[0].EpType = 0;
	DeviceConfig.Ep[0].OutIn = XUSB_EP_DIRECTION_OUT;


	/*
	 * Setup EP 1  512 byte packets, BULK IN.
	 */
	DeviceConfig.Ep[1].RamBase = 0x1000;
	DeviceConfig.Ep[1].Size = 0x200;
	DeviceConfig.Ep[1].EpType = 0;
	DeviceConfig.Ep[1].OutIn = XUSB_EP_DIRECTION_IN;

	/*
	 * Setup EP 2  512 byte packets, BULK OUT.
	 */
	DeviceConfig.Ep[2].RamBase = 0x1100;
	DeviceConfig.Ep[2].Size = 0x200;
	DeviceConfig.Ep[2].EpType = 0;
	DeviceConfig.Ep[2].OutIn = XUSB_EP_DIRECTION_OUT;

	InstancePtr->DeviceConfig.NumEndpoints = 3;
	DeviceConfig.NumEndpoints = 3;

	/*
	 * Initialize the device configuration.
	 */
	XUsb_ConfigureDevice(InstancePtr, &DeviceConfig);

	XUsb_EpEnable(InstancePtr, 0);
	XUsb_EpEnable(InstancePtr, 1);
	XUsb_EpEnable(InstancePtr, 2);

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		       XUSB_BUFFREADY_OFFSET, 1 << 2);

	InstancePtr->DeviceConfig.Ep[2].Buffer0Ready = 1;

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		       XUSB_BUFFREADY_OFFSET, (1 <<
				(2 + XUSB_STATUS_EP_BUFF2_SHIFT)));

	InstancePtr->DeviceConfig.Ep[2].Buffer1Ready = 1;

	MaxControlSize = 64;


	/*
	 * Store the actual RAM address offset in the device structure, so as to
	 * avoid the multiplication during processing.
	 */
	InstancePtr->DeviceConfig.Ep[1].RamBase <<= 2;
	InstancePtr->DeviceConfig.Ep[2].RamBase <<= 2;
	InstancePtr->DeviceConfig.Status = XUSB_RESET;


}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB mass storage device
 * application.
 *
 *
 * @param    	CallBackRef is the callback reference passed from the interrupt
 *           	handler, which in our case is a pointer to the driver instance.
 * @param    	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return   	None.
 *
 * @note        None.
 *
 ******************************************************************************/
void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus)
{

	XUsb *InstancePtr;
	u8 Index;

	InstancePtr = (XUsb *) CallBackRef;

	if (IntrStatus & XUSB_STATUS_RESET_MASK) {

			XUsb_Stop(InstancePtr);
			InstancePtr->DeviceConfig.CurrentConfiguration = 0;
			InstancePtr->DeviceConfig.Status = XUSB_RESET;
			for (Index = 0; Index < 3; Index++) {
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					       InstancePtr->
					       EndPointOffset[Index], 0);
			}
			/*
			 * Re-initialize the device and set the device address
			 * to 0 and re-start the device.
			 */
			InitUsbInterface(InstancePtr);
			XUsb_SetDeviceAddress(InstancePtr, 0);
			XUsb_Start(InstancePtr);

		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_RESET_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_DISCONNECT_MASK |
					      XUSB_STATUS_SUSPEND_MASK));
	}
	if (IntrStatus & XUSB_STATUS_SUSPEND_MASK) {
		/*
		 * Process the suspend event.
		 */
		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_SUSPEND_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_RESET_MASK |
					      XUSB_STATUS_DISCONNECT_MASK));
	}

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB End point Zero events.
 *
 *
 * @param    	CallBackRef is the callback reference passed from the interrupt.
 *           	handler, which in our case is a pointer to the driver instance.
 * @param	EpNum is the end point number.
 * @param    	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	EpNum is not used in this function as the handler is attached
 *		specific to end point zero. This parameter is useful when a
 *		single handler is used for processing all end point interrupts.
 *
 ******************************************************************************/
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;
	int SetupRequest;

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process the end point zero buffer interrupt.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP0_BUFF_MASK) {
		if (IntrStatus & XUSB_STATUS_SETUP_PACKET_MASK) {
			/*
			 * Received a setup packet. Execute the chapter 9
			 * command.
			 */
			XUsb_IntrEnable(InstancePtr,
					(XUSB_STATUS_DISCONNECT_MASK |
					 XUSB_STATUS_SUSPEND_MASK |
					 XUSB_STATUS_RESET_MASK));
			SetupRequest = Chapter9(InstancePtr);
			if (SetupRequest != XST_SUCCESS) {
				switch (SetupRequest) {
					case MS_RESET:
						MassStorageReset(InstancePtr);
						break;

					case MS_GETMAXLUN:
						GetMaxLUN(InstancePtr);
						break;
					default:
						/*
						 * Unsupported command. Stall
						 * the end point.
						 */
						XUsb_EpStall(InstancePtr, 0);
				}

			}
		}
		else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_RDY_MASK) {
			EP0ProcessOutToken(InstancePtr);
		}
		else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_FREE_MASK) {
			EP0ProcessInToken(InstancePtr);
		}
	}
}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB End point one events.
 *
 * @param    	CallBackRef is the callback reference passed from the interrupt
 *           	handler, which in our case is a pointer to the driver instance.
 * @param	EpNum is the end point number.
 * @param    	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	EpNum is not used in this function as the handler is attached
 *		specific to end point one. This parameter is useful when a
 *		single handler is used for processing all end point interrupts.
 *
 ******************************************************************************/
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process the End point 1 interrupts.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF1_MASK) {
		InstancePtr->DeviceConfig.Ep[1].Buffer0Ready = 0;
	}
	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF2_MASK) {
		InstancePtr->DeviceConfig.Ep[1].Buffer1Ready = 0;
	}

	if (((IntrStatus & XUSB_BUFFREADY_EP1_BUFF1_MASK) ==
			XUSB_BUFFREADY_EP1_BUFF1_MASK) &&
		((IntrStatus & XUSB_BUFFREADY_EP1_BUFF2_MASK) ==
			XUSB_BUFFREADY_EP1_BUFF2_MASK)){
		TwoTxInterrupts = TRUE;
	}
	if (CmdFlag == READ_COMMAND) {
		ReadTransfer (InstancePtr);
	}

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB End point two events.
 *
 * @param    	CallBackRef is the callback reference passed from the interrupt
 *           	handler, which in our case is a pointer to the driver instance.
 * @param	EpNum is the end point number.
 * @param    	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	EpNum is not used in this function as the handler is attached
 *		specific to end point two. This parameter is useful when a
 *		single handler is used for processing all end point interrupts.
 *
 ******************************************************************************/
void Ep2IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process end point 2 interrupts.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP2_BUFF1_MASK) {
		InstancePtr->DeviceConfig.Ep[2].Buffer0Ready = 0;

	}
	if (IntrStatus & XUSB_BUFFREADY_EP2_BUFF2_MASK) {
		InstancePtr->DeviceConfig.Ep[2].Buffer1Ready = 0;
	}

	if (((IntrStatus & XUSB_BUFFREADY_EP2_BUFF1_MASK) ==
			XUSB_BUFFREADY_EP2_BUFF1_MASK) &&
		((IntrStatus & XUSB_BUFFREADY_EP2_BUFF2_MASK) ==
			XUSB_BUFFREADY_EP2_BUFF2_MASK)){
		TwoRxInterrupts = TRUE;

	}
	if (CmdFlag != WRITE_COMMAND) {
		Xil_DCacheInvalidateRange(							(u32) &CmdBlock, sizeof(CmdBlock));

		RxDmaTransfer = TRUE;
		XUsb_EpDataRecv(InstancePtr, 2,
			(unsigned char *) (&CmdBlock),
				sizeof(CmdBlock));
	} else {

		WriteTransfer(InstancePtr);
	}

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB DMA events.
 *
 * @param    	CallBackRef is the callback reference passed from the interrupt
 *           	handler, which in our case is a pointer to the driver instance.
 * @param    	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void DmaIntrHandler(void *CallBackRef, u32 IntrStatus)
{

	XUsb *InstancePtr;

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process the DMA events.
	 */
	if (IntrStatus & XUSB_STATUS_DMA_DONE_MASK) {

		if (TxDmaTransfer == TRUE) {

			TxDmaTransfer = FALSE;
			if (CmdFlag == READ_COMMAND) {

				ReadTransfer (InstancePtr);

			} else	{
				if (CmdFlag != WRITE_COMMAND){
					if (ResponseSent == FALSE) {
					Xil_DCacheFlushRange(
						(u32)&CmdStatusBlock,
						USBCSW_LENGTH);
					/*
					 * Send a Success Status.
					 */
						TxDmaTransfer = TRUE;
					if (XUsb_EpDataSend(&UsbInstance,
						1,
						(unsigned char *) &CmdStatusBlock, USBCSW_LENGTH)
						!= XST_SUCCESS) {
						TxDmaTransfer = FALSE;
					xil_printf("NormalResp Failure\n");
						if (
						UsbInstance.DeviceConfig.Status
						==
						XUSB_DISCONNECTED) {
						/*
						 * We've been reset exit.
						 */
#ifdef XUSB_MS_DEBUG
						xil_printf("Disconnected \r\n");
#endif
						}
					} else {
						ResponseSent = TRUE;
						}
					}
				}
			}
			if (SendStatus == TRUE) {
				SendStatus = FALSE;
			}
		}
		if (RxDmaTransfer == TRUE) {
			RxDmaTransfer = FALSE;
			if (CmdFlag != WRITE_COMMAND) {
				ProcessRxCmd(InstancePtr);
			} else {
				if (WrDataPhase == TRUE) {

					WriteTransfer(InstancePtr);

				}
				if (TwoRxInterrupts == TRUE) {
					TwoRxInterrupts = FALSE;
					WriteTransfer(InstancePtr);
				}
			}
		}
		if (TwoTxInterrupts == TRUE) {
			TwoTxInterrupts = FALSE;
			if (CmdFlag == WRITE_COMMAND) {
				WriteTransfer(InstancePtr);
			}
			if (CmdFlag == READ_COMMAND) {
				ReadTransfer(InstancePtr);
			}
		}
	}

	if (IntrStatus & XUSB_STATUS_DMA_ERROR_MASK) {

	/*
	 * Code can be added here to handle the DMA error event.
	 */
#ifdef XUSB_MS_DEBUG
		xil_printf("DMA error\r\n");
#endif

	}

}

/*****************************************************************************/
/**
 * This function processes mass storage specific commands and sends the required
 * response.
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void ProcessRxCmd(XUsb * InstancePtr)
{
	u8 Length;
	u8 *BufPtr;
	u8 SendResp = FALSE;

	/*
	 * Setup status block.
	 */
	CmdStatusBlock.dCBWTag = CmdBlock.dCBWTag;
	CmdStatusBlock.dCBWSignature[0] = 'U';
	CmdStatusBlock.dCBWSignature[1] = 'S';
	CmdStatusBlock.dCBWSignature[2] = 'B';
	CmdStatusBlock.dCBWSignature[3] = 'S';

	/*
	 * Process the command.
	 */
	switch (CmdBlock.OpCode) {
	case SCSI_READ_10:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI READ \r\n");
#endif
		Read10(InstancePtr, &CmdBlock, &CmdStatusBlock);
		if (Read10Abort == 1) {
			Read10Abort = 0;
			goto FuncExit;
		}
		break;

	case SCSI_WRITE_10:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI WRITE \r\n");
#endif
		Write10(InstancePtr, &CmdBlock, &CmdStatusBlock);
		if (Write10Abort == 1) {
			Write10Abort = 0;
			goto FuncExit;
		}
		break;

	case SCSI_INQUIRY:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI INQUIRY \r\n");
#endif
		BufPtr = (u8 *) (&(Piq.device_type));
		Length = sizeof(INQUIRY);
		SendResp = TRUE;
		break;

	case SCSI_READ_FORMAT_CAPACITIES:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI READ FORMAT \r\n");
#endif
		BufPtr = (u8 *) (&(Pcl.caplstlen[0]));
		Length = sizeof(CAPACITY_LIST);
		SendResp = TRUE;
		break;

	case SCSI_READ_CAPACITY:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI READ CAPACITY\r\n");
#endif
		Prc.lastLBA[3] = (RAMDISKSECTORS - 1) & 0xFF;
		Prc.lastLBA[2] = ((RAMDISKSECTORS - 1) & 0xFF00) >> 8;
		Prc.lastLBA[1] = ((RAMDISKSECTORS - 1) & 0xFF0000) >> 16;
		Prc.lastLBA[0] = ((RAMDISKSECTORS - 1) & 0xFF000000) >> 24;
		Prc.blocklength[3] = BLOCK_SIZE & 0xFF;
		Prc.blocklength[2] = (BLOCK_SIZE & 0xFF00) >> 8;
		Prc.blocklength[1] = (BLOCK_SIZE & 0xFF0000) >> 16;
		Prc.blocklength[0] = (BLOCK_SIZE & 0xFF000000) >> 24;
		BufPtr = (u8 *) (&Prc.lastLBA[0]);
		Length = sizeof(READ_CAPACITY);
		SendResp = TRUE;
		break;

	case SCSI_MODE_SENSE:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI MODE SENSE \r\n");
#endif
		Pms_cdb = (PSCSI_MODESENSE_CDB)
			(&(CmdBlock.dCBWFlags));

		if (Pms_cdb->pagecode != MODESENSE_RETURNALL) {
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI MODE SENSE Reply Short\r\n");
#endif
			BufPtr = (u8 *) (&(Pmsd_s.mpl.mode_data_length));
			Length = sizeof(MODE_SENSE_REPLY_SHORT);


		}
		else {
			/*
			 * Load up full mode sense data.
			 */
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI MODE SENSE Reply All\r\n");
#endif
			BufPtr = (u8 *) (&(Pmsd_l.mpl.mode_data_length));
			Length = sizeof(MODE_SENSE_REPLY_ALL);
		}
		SendResp = TRUE;
		break;

	case SCSI_TEST_UNIT_READY:
	case SCSI_VERIFY:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI UNIT READY/VERIFY \r\n");
#endif
		/*
		 * We are always ready.
		 */
		CmdStatusBlock.bCSWStatus = CMD_PASSED;
		SendStatus = TRUE;
		break;

	case SCSI_REQUEST_SENSE:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI REQUEST SENSE \r\n");
#endif
		BufPtr = (u8 *) (&(Prss.error_code));
		Length = sizeof(REQUEST_SENSE);
		SendResp = TRUE;

		break;

	case SCSI_MEDIA_REMOVAL:
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI REMOVAL \r\n");
#endif
		Pmr = (PSCSI_MEDIA_REMOVAL_TYPE)
			& (CmdBlock.dCBWFlags);

		if (Pmr->prevent == 0x1) {
			/*
			 * We cannot prevent removal.
			 */
#ifdef XUSB_MS_DEBUG
		xil_printf("SCSI REMOVAL failed\r\n");
#endif
			CmdStatusBlock.bCSWStatus = CMD_FAILED;
		}
		else {
			CmdStatusBlock.bCSWStatus = CMD_PASSED;
		}
		CmdStatusBlock.Residue.value = 0;
		SendStatus = TRUE;
		break;

	default:
		/*
		 * Set status to failure.
		 */
		CmdStatusBlock.bCSWStatus = CMD_FAILED;
		CmdStatusBlock.Residue.value = 0;
		SendStatus = TRUE;
		break;

	}

	if (SendResp == TRUE) {

		Xil_DCacheFlushRange((u32)BufPtr, Length);
		if (XUsb_EpDataSend(InstancePtr, 1, BufPtr, Length) !=
		       XST_SUCCESS) {
			xil_printf("SendResp failure\r\n");
			if (InstancePtr->DeviceConfig.Status ==
			    XUSB_DISCONNECTED) {
				/*
				 * We've been reset exit.
				 */
				goto FuncExit;
			}
		} else {
			TxDmaTransfer = TRUE;
			ResponseSent = FALSE;
		}

		CmdStatusBlock.bCSWStatus = CMD_PASSED;
		CmdStatusBlock.Residue.value = 0;
	}


	if (SendStatus == TRUE) {
		Xil_DCacheFlushRange((u32)&CmdStatusBlock,
				USBCSW_LENGTH);
		if (XUsb_EpDataSend(&UsbInstance, 1,
			(unsigned char *) &CmdStatusBlock,
				USBCSW_LENGTH) != XST_SUCCESS){

			xil_printf("SendStatus Failure\n");
			SendStatus = FALSE;
			TxDmaTransfer = FALSE;
		} else {
			TxDmaTransfer = TRUE;
		}
	}
	if (CmdStatusBlock.bCSWStatus != CMD_PASSED){
		xil_printf("command failed \r\n");
	}

      FuncExit:
	return;
}


/*****************************************************************************/
/**
 * This function implements the transmission of data from the device to the READ
 * request from the USB Host.
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 * @param	pCmdBlock is a pointer to the Mass Storage Command Block
 *		wrapper.
 * @param	pStatusBlock is a pointer to the Mass Storage Status wrapper.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void Read10(XUsb * InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock)
{
	u8 *RamDiskPtr;


	/*
	 * Get the starting Logical block address and the number of blocks to
	 * transfer
	 */
#ifdef __LITTLE_ENDIAN__
	Lba.CharLba[0] = pCmdBlock->lba[3];
	Lba.CharLba[1] = pCmdBlock->lba[2];
	Lba.CharLba[2] = pCmdBlock->lba[1];
	Lba.CharLba[3] = pCmdBlock->lba[0];
	BlockCount.CharBlockCount[3] = 0;
	BlockCount.CharBlockCount[2] = 0;
	BlockCount.CharBlockCount[1] = pCmdBlock->transfer_length[0];
	BlockCount.CharBlockCount[0] = pCmdBlock->transfer_length[1];
#else
	Lba.CharLba[0] = pCmdBlock->lba[0];
	Lba.CharLba[1] = pCmdBlock->lba[1];
	Lba.CharLba[2] = pCmdBlock->lba[2];
	Lba.CharLba[3] = pCmdBlock->lba[3];
	BlockCount.CharBlockCount[0] = 0;
	BlockCount.CharBlockCount[1] = 0;
	BlockCount.CharBlockCount[2] = pCmdBlock->transfer_length[0];
	BlockCount.CharBlockCount[3] = pCmdBlock->transfer_length[1];
#endif
	CmdFlag = READ_COMMAND;
	RdResponseSent = FALSE;
	if (BlockCount.IntBlockCount) {

		RamDiskPtr = (u8 *) &(RamDisk[Lba.IntLba][0]);

		if (InstancePtr->DeviceConfig.CurrentSpeed ==
		    XUSB_EP_HIGH_SPEED) {
			Xil_DCacheFlushRange((u32)RamDiskPtr, 512);

			if (XUsb_EpDataSend(InstancePtr, 1, RamDiskPtr,
					       512) != XST_SUCCESS) {
				TxDmaTransfer = FALSE;
				if (InstancePtr->DeviceConfig.Status ==
				    XUSB_DISCONNECTED) {
					/*
					 * We've been reset exit.
					 */
#ifdef 	XUSB_MS_DEBUG
					xil_printf("Rd Disconnected \r\n");
#endif
					goto FuncExit;
				}

				if (Read10Abort == 1) {
#ifdef 	XUSB_MS_DEBUG
						xil_printf("Read Abort \r\n");
#endif
						ErrCode = ERR_USBABORT;
					goto FuncExit;
				}

			} else {
				TxDmaTransfer = TRUE;
			}
		}
		else {
			/*
			 * Full speed is 64 bytes a packet, so 8 make up 512
			 * bytes.
			 */
			Xil_DCacheFlushRange((u32)RamDiskPtr, 64);

			if (XUsb_EpDataSend(InstancePtr, 1,
					       RamDiskPtr,
					       64) != XST_SUCCESS) {
				if (InstancePtr->DeviceConfig.Status ==
				    XUSB_DISCONNECTED) {
						/*
						 * We've been reset exit.
						 */
#ifdef 	XUSB_MS_DEBUG
					xil_printf("Rd Disconnected \r\n");
#endif
						goto FuncExit;
				}
				if (Read10Abort == 1) {
#ifdef 	XUSB_MS_DEBUG
					xil_printf("Read Abort \r\n");
#endif
					ErrCode = ERR_USBABORT;
					goto FuncExit;
				}
			}
			RdIndex = 1;

		}
	}

FuncExit:
	return;
}

/*****************************************************************************/
/**
 * This function implements the reception of data in the USB device for the
 * write request from the USB Host.
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 * @param	pCmdBlock is a pointer to the Mass Storage Command Block
 *		wrapper.
 * @param	pStatusBlock is a pointer to the Mass Storage Status wrapper.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void Write10(XUsb * InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock)
{

	/*
	 * Get the starting logical block address and the number of blocks to
	 * transfer.
	 */
#ifdef __LITTLE_ENDIAN__
	Lba.CharLba[0] = pCmdBlock->lba[3];
	Lba.CharLba[1] = pCmdBlock->lba[2];
	Lba.CharLba[2] = pCmdBlock->lba[1];
	Lba.CharLba[3] = pCmdBlock->lba[0];
	BlockCount.CharBlockCount[3] = 0;
	BlockCount.CharBlockCount[2] = 0;
	BlockCount.CharBlockCount[1] = pCmdBlock->transfer_length[0];
	BlockCount.CharBlockCount[0] = pCmdBlock->transfer_length[1];
#else
	Lba.CharLba[0] = pCmdBlock->lba[0];
	Lba.CharLba[1] = pCmdBlock->lba[1];
	Lba.CharLba[2] = pCmdBlock->lba[2];
	Lba.CharLba[3] = pCmdBlock->lba[3];
	BlockCount.CharBlockCount[0] = 0;
	BlockCount.CharBlockCount[1] = 0;
	BlockCount.CharBlockCount[2] = pCmdBlock->transfer_length[0];
	BlockCount.CharBlockCount[3] = pCmdBlock->transfer_length[1];
#endif

	CmdFlag = WRITE_COMMAND;

}

/*****************************************************************************/
/**
 * This function implements the reception of data in the USB device for the
 * READ request from the USB Host.
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void ReadTransfer(XUsb *InstancePtr)
{

	if (InstancePtr->DeviceConfig.CurrentSpeed ==
		    XUSB_EP_HIGH_SPEED) {
		if (FirstPkt == 0){
				BlockCount.IntBlockCount--;
				Lba.IntLba++;
		}
		FirstPkt = 1;
	}
	if (BlockCount.IntBlockCount > 0) {
			RdRamDiskPtr = (u8 *) &(RamDisk[Lba.IntLba][0]);
			Xil_DCacheFlushRange((u32)RdRamDiskPtr, 512);
			if (InstancePtr->DeviceConfig.CurrentSpeed ==
				XUSB_EP_HIGH_SPEED) {
				if (XUsb_EpDataSend(&UsbInstance, 1,
					RdRamDiskPtr, 512) == XST_SUCCESS){
					TxDmaTransfer = TRUE;
					BlockCount.IntBlockCount--;
					Lba.IntLba++;
				}
			} else {

				RdRamDiskPtr += (64 * RdIndex);
				Xil_DCacheFlushRange((u32)RdRamDiskPtr,
				64);
				XUsb_EpDataSend(&UsbInstance, 1, RdRamDiskPtr,
					64);
				RdIndex += 1;
				if (RdIndex == 8){
					RdIndex = 0;
					BlockCount.IntBlockCount--;
					Lba.IntLba++;
				}
			}
	} else {
			if (RdResponseSent == FALSE) {
				Xil_DCacheFlushRange((u32)&CmdStatusBlock,
					USBCSW_LENGTH);
					CmdStatusBlock.bCSWStatus = CMD_PASSED;
					CmdStatusBlock.Residue.value = 0;
				if (XUsb_EpDataSend(&UsbInstance, 1,
				(unsigned char *) &CmdStatusBlock, USBCSW_LENGTH) == XST_SUCCESS) {
					FirstPkt = 0;
					CmdFlag = 0;
					TxDmaTransfer = TRUE;
					RdResponseSent = TRUE;
				}
			}
	}
}

/*****************************************************************************/
/**
 * This function implements the reception of data in the USB device for the
 * WRITE request from the USB Host.
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void WriteTransfer(XUsb *InstancePtr)
{

	if (BlockCount.IntBlockCount > 0) {

		RxDmaTransfer = TRUE;
		WrRamDiskPtr = (u8 *) &(RamDisk[Lba.IntLba][0]);
		Xil_DCacheInvalidateRange((u32)WrRamDiskPtr, 512);
		if (InstancePtr->DeviceConfig.CurrentSpeed ==
				XUSB_EP_HIGH_SPEED) {
			if (XUsb_EpDataRecv(&UsbInstance, 2, WrRamDiskPtr,
					512) == XST_SUCCESS){
			if (BlockCount.IntBlockCount == 1){
				WrDataPhase = TRUE;
			}
			BlockCount.IntBlockCount--;
			Lba.IntLba++;
			} else {
			xil_printf("WrData recv failure\n");
			}
		} else {
			WrRamDiskPtr += (64 * WrIndex);
			Xil_DCacheInvalidateRange((u32)WrRamDiskPtr,
						64);
			XUsb_EpDataRecv(&UsbInstance, 2, WrRamDiskPtr,
						64);
			WrIndex += 1;
			if (WrIndex == 8){
				WrIndex = 0;
				BlockCount.IntBlockCount--;
				Lba.IntLba++;
			}
		}
	} else {
			if (TxDmaTransfer == FALSE) {
				WrDataPhase = FALSE;
				CmdStatusBlock.bCSWStatus = CMD_PASSED;
				CmdStatusBlock.Residue.value = 0;
				Xil_DCacheFlushRange((u32)&CmdStatusBlock,
					USBCSW_LENGTH);
				if (XUsb_EpDataSend(&UsbInstance, 1,
				(unsigned char *) &CmdStatusBlock,
				USBCSW_LENGTH) == XST_SUCCESS){
					TxDmaTransfer = TRUE;
					CmdFlag = 0;
				} else {
					xil_printf("Write Resp Failure\n");
				}
			}
	}
}
/******************************************************************************/
/**
* This routine is called when a RESET class command is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note    	None.
*
******************************************************************************/
void MassStorageReset(XUsb * InstancePtr)
{
	switch (CmdBlock.OpCode) {
	case SCSI_READ_10:
		Read10Abort = 1;
		break;

	case SCSI_WRITE_10:
		Write10Abort = 1;
		break;

	default:
		break;
	}

	/*
	 * Set the basic control status words.
	 */
	SetupControlWriteStatusStage(InstancePtr);
	return;
}

/* Class Commands */
/******************************************************************************/
/**
* This routine is called when a GETMAXLUN class command is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note		None.
*
******************************************************************************/
void GetMaxLUN(XUsb * InstancePtr)
{
	u32 *RamBase;

	Ch9_CmdBuf.ContWriteCount = 0;
	RamBase = (u32 *) (InstancePtr->Config.BaseAddress +
			   ((InstancePtr->DeviceConfig.Ep[0].RamBase) << 2));
	UsbMemData.Byte.Zero = 0;

	*RamBase = UsbMemData.Word;

	InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			(InstancePtr->EndPointOffset[0] +
			XUSB_EP_BUF0COUNT_OFFSET), 1);
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			XUSB_BUFFREADY_OFFSET, 1);
}

/******************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the USB. This function is application specific since the actual
* system may or may not have an interrupt controller. The USB could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	InstancePtr contains a pointer to the instance of the USB
*		component, which is going to be connected to the interrupt
*		controller.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE. if it fails.
*
* @note		None
*
*******************************************************************************/
static int SetupInterruptSystem(XUsb * InstancePtr)
{
	int Status;


#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the USB device occurs.
	 */
	Status = XIntc_Connect(&InterruptController, USB_INTR,
			       (XInterruptHandler) XUsb_IntrHandler,
			       (void *) InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the USB can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the USB.
	 */
	XIntc_Enable(&InterruptController, USB_INTR);

#else
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	XScuGic_SetPriorityTriggerType(&InterruptController, USB_INTR,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(&InterruptController, USB_INTR,
				 (Xil_ExceptionHandler)XUsb_IntrHandler,
				 InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the USB device.
	 */
	XScuGic_Enable(&InterruptController, USB_INTR);


#endif

	/*
	 * Initialize the exception table
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER,
				&InterruptController);

	/*
	 * Enable non-critical exceptions
	 */
	Xil_ExceptionEnable();




	return XST_SUCCESS;
}


