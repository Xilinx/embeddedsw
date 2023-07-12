/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xusb_storage_polled_mode.c
 *
 * This file contains Mass storage device application related functions implemented
 * in polled mode.
 *
 * @note	The example is tested on MicroBlaze, with caches included in the
 *		H/W design. This example works for USB high speed interface only.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 4.00a hvm  05/24/11 Created based on the xusb_storage.c example.
 * 4.01a bss  11/01/11 Modified UsbIfIntrHandler function to unconditionally
 *		       reset when USB reset is asserted (CR 627574).
 * 4.02a bss  04/05/12 Modified the sequence of Cache Flush and Invalidation.
 *		       The cache flush happens just before the driver API
 *		       EPDataSend is called. Similarly the cache invalidation
 *	               is done after the call to EPDataReceive and after the
 *		       dma transfer is over.
 * 5.6   pm   07/05/23 Removed powerpc support.
 * 5.6   pm   07/05/23 Added support for system device-tree flow.
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb.h"
#include "xusb_storage.h"
#include "stdio.h"
#include "xparameters.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/


#ifndef SDT
#define USB_DEVICE_ID		XPAR_USB_0_DEVICE_ID
#else
#define XUSB_BASEADDRESS	XPAR_XUSB_0_BASEADDR
#endif /* SDT */

#define READ_COMMAND		1
#define WRITE_COMMAND		2

#undef XUSB_MS_DEBUG

/************************** Variable Definitions *****************************/

XUsb UsbInstance;		/* The instance of the USB device */
XUsb_Config *UsbConfigPtr;	/* Instance of the USB config structure */

volatile u8 CmdFlag = 0;

volatile u8 FirstPkt = 0;
u8  *WrRamDiskPtr;
u8 *RdRamDiskPtr;
volatile u8 RdIndex = 0;
volatile u8 WrIndex = 0;

volatile u32 IntrStatus;
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

	/* Enable caches for Microblaze, for
	 * ARM caches are enabled by BSP  */
#ifndef SDT
#ifdef __MICROBLAZE__
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();


	Xil_DCacheInvalidate();
	Xil_DCacheEnable();
#endif
#endif

	/*
	 * Initialize the USB driver.
	 */
#ifndef SDT
	UsbConfigPtr = XUsb_LookupConfig(USB_DEVICE_ID);
#else
	UsbConfigPtr = XUsb_LookupConfig(XUSB_BASEADDRESS);
#endif
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}


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


	XUsb_Start(&UsbInstance);

	/*
	 * Set the device configuration to unenumerated state
	 * and set the speed as High speed device.
	 */
	UsbInstance.DeviceConfig.CurrentConfiguration = 0;

	UsbInstance.DeviceConfig.CurrentSpeed = XUSB_EP_HIGH_SPEED;
	while (1) {


		/*
		 * Read the interrupt status register
		 */
		IntrStatus = XUsb_ReadReg(UsbInstance.Config.BaseAddress,
					  XUSB_STATUS_OFFSET);
		if (IntrStatus & (XUSB_STATUS_RESET_MASK |
				  XUSB_STATUS_DISCONNECT_MASK)) {
			UsbIfIntrHandler(&UsbInstance, IntrStatus);
			continue;
		}
		if (IntrStatus & XUSB_STATUS_EP0_BUFF1_COMP_MASK) {
			Ep0IntrHandler(&UsbInstance, 0,  IntrStatus);
			continue;
		}

		if (IntrStatus & (XUSB_STATUS_EP1_BUFF1_COMP_MASK |
				  XUSB_STATUS_EP1_BUFF2_COMP_MASK)) {
			Ep1IntrHandler(&UsbInstance, 1,  IntrStatus);
		}
		if (IntrStatus & (XUSB_STATUS_EP2_BUFF1_COMP_MASK |
				  XUSB_STATUS_EP2_BUFF2_COMP_MASK)) {
			Ep2IntrHandler(&UsbInstance, 2, IntrStatus);
		}

		/*
		 * Process Rx Commands on End point 2.
		 */
		if (CmdFlag != WRITE_COMMAND) {

			if (XUsb_EpDataRecv(&UsbInstance, 2,
					    (unsigned char *) (&CmdBlock), sizeof(CmdBlock)) ==
			    XST_SUCCESS) {
				if (UsbInstance.Config.DmaEnabled) {
					while ((XUsb_ReadReg(
							UsbInstance.Config.BaseAddress,
							XUSB_DMA_STATUS_OFFSET) &
						XUSB_DMA_DMASR_BUSY)
					       == XUSB_DMA_DMASR_BUSY);

					Xil_DCacheInvalidateRange(
						(u32) &CmdBlock, sizeof(CmdBlock));
				}

				ProcessRxCmd(&UsbInstance);
			}

		}

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
void InitUsbInterface(XUsb *InstancePtr)
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
					      (2 +
					       XUSB_STATUS_EP_BUFF2_SHIFT)));

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
 * This function processes the reset and disconnect conditions of the
 * USB mass storage device.
 *
 *
 * @param    	CallBackRef is the callback reference passed from to the
 *           	function, which in our case is a pointer to the driver instance.
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
 * This function processes the USB End point Zero events.
 *
 *
 * @param    	CallBackRef is the callback reference passed from the caller
 *           	function, which in our case is a pointer to the driver instance.
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
		} else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_RDY_MASK) {
			EP0ProcessOutToken(InstancePtr);
		} else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_FREE_MASK) {
			EP0ProcessInToken(InstancePtr);
		}
	}
}

/*****************************************************************************/
/**
 * This function processes the USB End point one events.
 *
 * @param    	CallBackRef is the callback reference passed from the caller
 *           	function, which in our case is a pointer to the driver instance.
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

	if (CmdFlag == READ_COMMAND) {


		if (FirstPkt == 0) {
			BlockCount.IntBlockCount--;
			Lba.IntLba++;
		}
		FirstPkt = 1;

		if (BlockCount.IntBlockCount > 0) {

			RdRamDiskPtr = (u8 *) & (RamDisk[Lba.IntLba][0]);

			if (InstancePtr->Config.DmaEnabled) {

				Xil_DCacheFlushRange((u32)RdRamDiskPtr, 512);
			}

			XUsb_EpDataSend(&UsbInstance, 1, RdRamDiskPtr, 512);
			if (InstancePtr->Config.DmaEnabled) {

				while ((XUsb_ReadReg(
						UsbInstance.Config.BaseAddress,
						XUSB_DMA_STATUS_OFFSET) &
					XUSB_DMA_DMASR_BUSY)
				       == XUSB_DMA_DMASR_BUSY);
			}

			BlockCount.IntBlockCount--;
			Lba.IntLba++;

		} else {
			FirstPkt = 0;
			CmdFlag = 0;
			CmdStatusBlock.bCSWStatus = CMD_PASSED;
			CmdStatusBlock.Residue.value = 0;

			if (InstancePtr->Config.DmaEnabled) {
				Xil_DCacheFlushRange((u32)&CmdStatusBlock,
						     USBCSW_LENGTH);
			}

			XUsb_EpDataSend(&UsbInstance, 1,
					(unsigned char *) &CmdStatusBlock, USBCSW_LENGTH);
			if (InstancePtr->Config.DmaEnabled) {

				while ((XUsb_ReadReg(
						UsbInstance.Config.BaseAddress,
						XUSB_DMA_STATUS_OFFSET) &
					XUSB_DMA_DMASR_BUSY)
				       == XUSB_DMA_DMASR_BUSY);
			}
		}

	}

}

/*****************************************************************************/
/**
 * This function processes the USB End point two events.
 *
 * @param    	CallBackRef is the callback reference passed from the caller
 *           	function, which in our case is a pointer to the driver instance.
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

	if (CmdFlag == WRITE_COMMAND) {

		while (BlockCount.IntBlockCount > 0) {

			WrRamDiskPtr = (u8 *) & (RamDisk[Lba.IntLba][0]);

			if (XUsb_EpDataRecv(InstancePtr, 2, WrRamDiskPtr,
					    512) == XST_SUCCESS) {
				if (InstancePtr->Config.DmaEnabled) {
					while ((XUsb_ReadReg(
							UsbInstance.Config.BaseAddress,
							XUSB_DMA_STATUS_OFFSET) &
						XUSB_DMA_DMASR_BUSY)
					       == XUSB_DMA_DMASR_BUSY);
					Xil_DCacheInvalidateRange(
						(u32)WrRamDiskPtr, 512);
				}


				BlockCount.IntBlockCount--;
				Lba.IntLba++;
			}
			IntrStatus = XUsb_ReadReg(UsbInstance.Config.BaseAddress,
						  XUSB_STATUS_OFFSET);
			if (IntrStatus & XUSB_BUFFREADY_EP2_BUFF1_MASK) {
				InstancePtr->DeviceConfig.Ep[2].Buffer0Ready = 0;

			}
			if (IntrStatus & XUSB_BUFFREADY_EP2_BUFF2_MASK) {
				InstancePtr->DeviceConfig.Ep[2].Buffer1Ready = 0;
			}
		}



		if (BlockCount.IntBlockCount == 0) {
			CmdFlag = 0;
			CmdStatusBlock.bCSWStatus = CMD_PASSED;
			CmdStatusBlock.Residue.value = 0;

			if (UsbInstance.Config.DmaEnabled) {
				Xil_DCacheFlushRange((u32)&CmdStatusBlock,
						     USBCSW_LENGTH);
			}

			XUsb_EpDataSend(&UsbInstance, 1,
					(unsigned char *) &CmdStatusBlock, USBCSW_LENGTH);
			if (UsbInstance.Config.DmaEnabled) {
				while ((XUsb_ReadReg(
						UsbInstance.Config.BaseAddress,
						XUSB_DMA_STATUS_OFFSET) &
					XUSB_DMA_DMASR_BUSY)
				       == XUSB_DMA_DMASR_BUSY);
			}
		}

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
void ProcessRxCmd(XUsb *InstancePtr)
{
	u8 Length;
	u8 *BufPtr;
	u8 SendResp = FALSE;
	u8 SendStatus = FALSE;

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
			if (BlockCount.IntBlockCount == 0) {
				SendStatus = TRUE;
				CmdFlag = 0;
			}
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


			} else {
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
			//xil_printf("SCSI UNIT READY/VERIFY \r\n");
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
			} else {
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
			break;

	}

	if (SendResp == TRUE) {


		if (InstancePtr->Config.DmaEnabled) {
			Xil_DCacheFlushRange((u32)BufPtr, Length);
		}

		while (XUsb_EpDataSend(InstancePtr, 1, BufPtr, Length) !=
		       XST_SUCCESS) {
			if (InstancePtr->DeviceConfig.Status ==
			    XUSB_DISCONNECTED) {
				/*
				 * We've been reset exit.
				 */
				goto FuncExit;
			}
		}

		if (InstancePtr->Config.DmaEnabled) {

			while ((XUsb_ReadReg(
					UsbInstance.Config.BaseAddress,
					XUSB_DMA_STATUS_OFFSET) &
				XUSB_DMA_DMASR_BUSY)
			       == XUSB_DMA_DMASR_BUSY);
		}

		CmdStatusBlock.bCSWStatus = CMD_PASSED;
		CmdStatusBlock.Residue.value = 0;
		SendStatus = TRUE;
	}


	if (SendStatus == TRUE) {

		if (InstancePtr->Config.DmaEnabled) {

			Xil_DCacheFlushRange(
				(u32)&CmdStatusBlock, USBCSW_LENGTH);
		}

		/*
		 * Send a Success Status.
		 */
		if (XUsb_EpDataSend(InstancePtr, 1,
				    (unsigned char *) &CmdStatusBlock, USBCSW_LENGTH) !=
		    XST_SUCCESS) {
			if (InstancePtr->DeviceConfig.Status ==
			    XUSB_DISCONNECTED) {
				/*
				 * We've been reset exit.
				 */
				goto FuncExit;
			}
		}

		if (InstancePtr->Config.DmaEnabled) {

			while ((XUsb_ReadReg(
					UsbInstance.Config.BaseAddress,
					XUSB_DMA_STATUS_OFFSET) &
				XUSB_DMA_DMASR_BUSY)
			       == XUSB_DMA_DMASR_BUSY);
		}
		SendStatus = FALSE;
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
void Read10(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock)
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


	if (BlockCount.IntBlockCount) {

		RamDiskPtr = (u8 *) & (RamDisk[Lba.IntLba][0]);

		if (InstancePtr->Config.DmaEnabled) {

			Xil_DCacheFlushRange((u32)RamDiskPtr, 512);
		}


		if (XUsb_EpDataSend(InstancePtr, 1, RamDiskPtr,
				    512) != XST_SUCCESS) {
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
	}

	if (InstancePtr->Config.DmaEnabled) {
		while ((XUsb_ReadReg(
				UsbInstance.Config.BaseAddress,
				XUSB_DMA_STATUS_OFFSET) &
			XUSB_DMA_DMASR_BUSY)
		       == XUSB_DMA_DMASR_BUSY);
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
void Write10(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock)
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

	if (BlockCount.IntBlockCount) {
		WrRamDiskPtr = (u8 *) & (RamDisk[Lba.IntLba][0]);


		if (XUsb_EpDataRecv(InstancePtr, 2, WrRamDiskPtr,
				    512) != XST_SUCCESS) {
			if (InstancePtr->DeviceConfig.Status ==
			    XUSB_DISCONNECTED) {
				/*
				 * We've been reset exit.
				 */
#ifdef 	XUSB_MS_DEBUG
				xil_printf("Wr Disconnected \r\n");
#endif
				pStatusBlock->bCSWStatus = CMD_FAILED;
				ErrCode = ERR_USBABORT;
				goto FuncExit;
			}

			if (Write10Abort == 1) {
#ifdef 	XUSB_MS_DEBUG
				xil_printf("Write Abort \r\n");
#endif
				pStatusBlock->bCSWStatus = CMD_FAILED;
				ErrCode = ERR_USBABORT;
				goto FuncExit;
			}
		} else {
			if (InstancePtr->Config.DmaEnabled) {
				while ((XUsb_ReadReg(
						UsbInstance.Config.BaseAddress,
						XUSB_DMA_STATUS_OFFSET) &
					XUSB_DMA_DMASR_BUSY)
				       == XUSB_DMA_DMASR_BUSY);

				Xil_DCacheInvalidateRange(
					(u32)WrRamDiskPtr, 512);
			}

			BlockCount.IntBlockCount--;
			Lba.IntLba++;
		}
	}



FuncExit:
	return;
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
void MassStorageReset(XUsb *InstancePtr)
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
void GetMaxLUN(XUsb *InstancePtr)
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
