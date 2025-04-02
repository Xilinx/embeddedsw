/******************************************************************************
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xhwicap.c
* @addtogroup hwicap Overview
* @{
*
* This file contains the functions of the XHwIcap driver. See xhwicap.h for a
* detailed description of the driver.
*
* @note
*
* 7 series device, Zynq device, Ultrascale and ZynqMP Ultrascale are supported.
*
* In a Zynq device the ICAP needs to be selected using the
* XDcfg_SelectIcapInterface API of the DevCfg driver (clear the PCAP_PR bit of
* Control register in the Device Config Interface)  before it can be
* accessed using the HwIcap.
* In case of ZynqMP clear the PCAP_PR bit of pcap_ctrl register in Module
* Configuration Security Unit(CSU) using register write.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date      Changes
* ----- ---- -------- -------------------------------------------------------
* 2.00a sv   09/11/07  Initial version.
* 2.01a ecm  04/08/08  Updated data structures to include the V5FXT parts.
* 3.00a sv   11/28/08  Added the API for initiating Abort while reading/writing
*		       from the ICAP.
* 4.00a hvm  12/1/09   Added support for V6 and updated with HAL phase 1
*		       modifications
* 5.00a hvm  04/02/10  Added support for S6 device.
* 5.01a hvm  07/06/10  In XHwIcap_DeviceRead function, a read bit mask
*		       verification is added after all the data bytes are read
*		       from READ FIFO.The Verification of the read bit mask
*		       at the begining of reading of bytes is removed.
* 5.03a hvm  15/4/11   Updated with V6 CXT device definitions.
* 6.00a hvm  08/01/11  Added support for K7 devices.
* 7.00a bss  03/14/12  Added support for 8/16/32 ICAP Data Widths - CR 620085
*		       Added support for Lite Mode(no Write FIFO) - CR 601748
*		       Added Virtex 7, Artix 7 and Zynq Idcodes in Device look
*		        up table - CR 647140, CR 643295
* 8.01a bss  04/18/13  Updated to fix compiler warnings. CR#704814
* 9.0   bss  02/20/14  Updated to support Kintex8, kintexu and virtex72000T
*	 		family devices.
* 10.0  bss  6/24/14  Removed support for families older than 7 series
*		      Removed IDCODE lookup logic in XHwIcap_CfgInitialize.
* 10.0  bss  7/10/14  Fix compilation failure for designs other than 32 bit
*		      data width of HWICAP.
* 10.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                      Changed the prototype of XHwIcap_CfgInitialize API.
* 10.2   mi   09/22/16 Fixed compilation warnings.
* 11.0   MNK  12/06/16 Added support for 8-series family devices.
* 11.1   sg   08/29/17 Updated software reset and fifo flush api by adding
*			delay as per IP specifications
* 11.2	Nava  02/01/19 Updated the Number of words per frame as mention in the
*		       ug570
* 11.5  Nava  09/30/22 Added New IDCODE's as mentioned in the ug570 Doc.
* 11.6  Nava  06/28/23 Added support for system device-tree flow.
* 11.7  Nava  12/09/24 Added the missing ID code for Artix UltraScale+ FPGAs
*                      to the series_ultra_plus_idcodes array.
* 11.7  Nava  04/01/25 Updates the series_ultra_plus_idcodes array to include
*                      the ID code for UltraScale+ FPGAs, ensuring accurate
*                      identification and compliance with the latest UG570 Doc.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include <xil_types.h>
#include <xil_assert.h>
#include "xhwicap.h"
#include "xparameters.h"
#include <sleep.h>

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/
#define DEVICE_7SERIES_WORDS_PER_FRAME		101
#define DEVICE_ULTRA_WORDS_PER_FRAME		123
#define DEVICE_ULTRA_PLUS_WORDS_PER_FRAME	93

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/
/*
 * 7-series family number information taken for ug470_7series_config.pdf
 */

static u32 series_7idcodes[NUM_7SERIES_IDCODES] = {
	0x3622093, 0x3620093, 0x37C4093, 0x362F093, 0x37C8093, 0x37C7093,
	0x37C3093, 0x362E093, 0x37C2093, 0x362D093, 0x362C093, 0x3632093,
	0x3631093, 0x3636093, 0x3647093, 0x364C093, 0x3651093, 0x3747093,
	0x3656093, 0x3752093, 0x3751093, 0x3671093, 0x36B3093, 0x3667093,
	0x3682093, 0x3687093, 0x3692093, 0x3691093, 0x3696093, 0x36D5093,
	0x36D9093, 0x36DB093
};

/*
 * 8-series family number information taken for ug570_7series_config.pdf
 */

static u32 series_ultra_idcodes[NUM_ULTRA_SERIES_IDCODES] = {
	0x3824093, 0x3823093, 0x3822093, 0x3919093, 0x380F093, 0x3844093,
	0x390D093, 0x3939093, 0x3843093, 0x3842093, 0x392D093, 0x3933093,
	0x3931093, 0x396D093
};

static u32 series_ultra_plus_idcodes[NUM_ULTRA_PLUS_SERIES_IDCODES] = {
	0x4A63093, 0x4A62093, 0x484A093, 0x4A4E093, 0x4A52093, 0x4A56093,
	0x4B39093, 0x4B2B093, 0x4B29093, 0x4B31093, 0x4B49093, 0x4B51093,
	0x4AC2093, 0x4AC4093, 0x4A65093, 0x4A64093, 0x4ACF093, 0x4BA1093,
	0x4ACE093, 0x4B43093, 0x4B41093, 0x4B6B093, 0x4B69093, 0x4B71093,
	0x4B79093, 0x4B73093, 0x4B7B093, 0x4B61093, 0x4AF6093
};


/************************** Function Prototypes *****************************/
static void StubStatusHandler(void *CallBackRef, u32 StatusEvent,
			      u32 ByteCount);
static u32 FindDeviceType(u32 IdCode);

/****************************************************************************/
/**
*
* This function initializes a specific XHwIcap instance.
* The IDCODE is read from the FPGA and based on the IDCODE the information
* about the resources in the FPGA is filled in the instance structure.
*
* The HwIcap device will be in put in a reset state before exiting this
* function.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	ConfigPtr points to the XHwIcap device configuration structure.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
int XHwIcap_CfgInitialize(XHwIcap *InstancePtr, XHwIcap_Config *ConfigPtr,
			  UINTPTR EffectiveAddr)
{
	int Status;
	u32 DeviceIdCode;
	u32 TempDevId;
	u32 DeviceType;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Set some default values.
	 */
	InstancePtr->IsReady = FALSE;
	InstancePtr->IsTransferInProgress = FALSE;
	InstancePtr->IsPolled = TRUE; /* Polled Mode */

	/*
	 * Set the device base address and stub handler.
	 */
	InstancePtr->HwIcapConfig.BaseAddress = EffectiveAddr;
	InstancePtr->StatusHandler = (XHwIcap_StatusHandler) StubStatusHandler;

	/** Set IcapWidth **/

	InstancePtr->HwIcapConfig.IcapWidth = ConfigPtr->IcapWidth;

	/** Set IsLiteMode **/
	InstancePtr->HwIcapConfig.IsLiteMode = ConfigPtr->IsLiteMode;

#ifdef __aarch64__
	/* Controls the method for PL partial reconfiguraiton,0x0 - ICAP */
	XHwIcap_Out32(PCAP_CR_OFFSET, 0);
#endif

	/*
	 * Read the IDCODE from ICAP.
	 */

	/*
	 * Setting the IsReady of the driver temporarily so that
	 * we can read the IdCode of the device.
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Dummy Read of the IDCODE as the first data read from the
	 * ICAP has to be discarded (Due to the way the HW is designed).
	 */
	Status = XHwIcap_GetConfigReg(InstancePtr, XHI_IDCODE, &TempDevId);
	if (Status != XST_SUCCESS) {
		InstancePtr->IsReady = 0;
		return XST_FAILURE;
	}

	/*
	 * Read the IDCODE and mask out the version section of the DeviceIdCode.
	 */
	Status = XHwIcap_GetConfigReg(InstancePtr, XHI_IDCODE, &DeviceIdCode);
	if (Status != XST_SUCCESS) {
		InstancePtr->IsReady = 0;
		return XST_FAILURE;
	}

	DeviceIdCode = DeviceIdCode & XHI_DEVICE_ID_CODE_MASK;

	if ((DeviceIdCode == XHI_DEVICE_ID_CODE_MASK) ||
	    (DeviceIdCode == 0x0)) {
		return XST_FAILURE;
	}


	Status = XHwIcap_CommandDesync(InstancePtr);
	InstancePtr->IsReady = 0;
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	DeviceType = FindDeviceType (DeviceIdCode);
	InstancePtr->DeviceFamily = DeviceType;

	switch (DeviceType) {
		case DEVICE_TYPE_7SERIES :
			InstancePtr->WordsPerFrame = DEVICE_7SERIES_WORDS_PER_FRAME;
			break;
		case DEVICE_TYPE_ULTRA :
			InstancePtr->WordsPerFrame = DEVICE_ULTRA_WORDS_PER_FRAME;
			break;
		case DEVICE_TYPE_ULTRA_PLUS :
			InstancePtr->WordsPerFrame = DEVICE_ULTRA_PLUS_WORDS_PER_FRAME;
			break;
		default:
			return XST_FAILURE;
	}

	InstancePtr->BytesPerFrame = (InstancePtr->WordsPerFrame * 4);
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Reset the device.
	 */
	XHwIcap_Reset(InstancePtr);

	return XST_SUCCESS;
} /* end XHwIcap_CfgInitialize() */


/****************************************************************************/
/**
*
* This function writes the given user data to the Write FIFO in both the
* polled mode and the interrupt mode and starts the transfer of the data to
* the ICAP device.
*
* In the polled mode, this function will write the specified number of words
* into the FIFO before returning.
*
* In the interrupt mode, this function will write the words upto the size
* of the Write FIFO and starts the transfer, then subsequent transfer of the
* data is performed by the interrupt service routine until the entire buffer
* has been transferred. The status callback function is called when the entire
* buffer has been sent.
* In order to use interrupts, it is necessary for the user to connect the driver
* interrupt handler, XHwIcap_IntrHandler(), to the interrupt system of
* the application and enable the interrupts associated with the Write FIFO.
* The user has to enable the interrupts each time this function is called
* using the XHwIcap_IntrEnable macro.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	FrameBuffer is a pointer to the data to be written to the
*			ICAP device.
* @param	NumWords is the number of words (16 bit for S6 and 32 bit
*		for all other devices)to write to the ICAP device.
*
* @return	XST_SUCCESS or XST_FAILURE
*
* @note		This function is a blocking for the polled mode of operation
*		and is non-blocking for the interrupt mode of operation.
*		Use the function XHwIcap_DeviceWriteFrame for writing a frame
*		of data to the ICAP device.
*
*****************************************************************************/
int XHwIcap_DeviceWrite(XHwIcap *InstancePtr, u32 *FrameBuffer, u32 NumWords)
{
#if XPAR_HWICAP_0_ICAP_DWIDTH == 8
	u8 Fifo[NumWords * 4];
#elif XPAR_HWICAP_0_ICAP_DWIDTH == 16
	u16 Fifo[NumWords * 2];
#endif

#if (XPAR_HWICAP_0_ICAP_DWIDTH == 8) || (XPAR_HWICAP_0_ICAP_DWIDTH == 16)
	u32 Index; /* Array Index */
	u32 Fifo[4]; /** Icap Width of 32 does not use Fifo but declared
			 to overcome compilation error. Size of 4 is used
			 to overcome compiler warnings */
#endif

#if (XPAR_HWICAP_0_MODE == 0)
	u32 WrFifoVacancy;
	u32 IntrStatus;
#endif

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(FrameBuffer != NULL);
	Xil_AssertNonvoid(NumWords > 0);

	/*
	 * Make sure that the last Read/Write by the driver is complete.
	 */
	if (XHwIcap_IsTransferDone(InstancePtr) == FALSE) {
		return XST_FAILURE;
	}

	/*
	 * Check if the ICAP device is Busy with the last Read/Write
	 */
	if (XHwIcap_IsDeviceBusy(InstancePtr) == TRUE) {
		return XST_FAILURE;
	}

	/*
	 * Set the flag, which will be cleared when the transfer
	 * is entirely done from the FIFO to the ICAP.
	 */
	InstancePtr->IsTransferInProgress = TRUE;

	/*
	 * Disable the Global Interrupt.
	 */
	XHwIcap_IntrGlobalDisable(InstancePtr);

#if (XPAR_HWICAP_0_ICAP_DWIDTH == 8) || (XPAR_HWICAP_0_ICAP_DWIDTH == 16)
	/* 16 bit */
	if (InstancePtr->HwIcapConfig.IcapWidth == 16) {
		for (Index = 0; Index < (NumWords * 2); Index = Index + 2) {
			Fifo[Index + 1] = *FrameBuffer;
			Fifo[Index]	= *FrameBuffer >> 16;
			FrameBuffer++;
		}
		InstancePtr->RequestedWords = NumWords * 2;
		InstancePtr->RemainingWords = NumWords * 2;
		InstancePtr->SendBufferPtr = &Fifo[0];
	}

	/* 8 bit */
	else {
		for (Index = 0; Index < (NumWords * 4); Index = Index + 4) {
			Fifo[Index + 3] = *FrameBuffer;
			Fifo[Index + 2] = *FrameBuffer >> 8;
			Fifo[Index + 1] = *FrameBuffer >> 16;
			Fifo[Index]	= *FrameBuffer >> 24;
			FrameBuffer++;
		}
		InstancePtr->RequestedWords = NumWords * 4;
		InstancePtr->RemainingWords = NumWords * 4;
		InstancePtr->SendBufferPtr = &Fifo[0];
	}
#else
	/*
	 * Set up the buffer pointer and the words to be transferred.
	 */
	InstancePtr->SendBufferPtr = FrameBuffer;
	InstancePtr->RequestedWords = NumWords;
	InstancePtr->RemainingWords = NumWords;
#endif
	/*
	 * Fill the FIFO with as many words as it will take (or as many as we
	 * have to send.
	 */

#if (XPAR_HWICAP_0_MODE == 1)
	/* If Lite Mode then write one by one word in WriteFIFO register */
	while (InstancePtr->RemainingWords > 0) {

		XHwIcap_FifoWrite(InstancePtr, *InstancePtr->SendBufferPtr);

		XHwIcap_StartConfig(InstancePtr);

		while ((XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
					XHI_CR_OFFSET)) & XHI_CR_WRITE_MASK);

		InstancePtr->RemainingWords--;

		InstancePtr->SendBufferPtr++;
	}

	/*
	 * Clear the flag to indicate the write has been done
	 */
	InstancePtr->IsTransferInProgress = FALSE;
	InstancePtr->RequestedWords = 0x0;

#else
	/* If FIFOs are enabled, fill the FIFO and initiate transfer */

	WrFifoVacancy = XHwIcap_GetWrFifoVacancy(InstancePtr);
	while ((WrFifoVacancy != 0) &&
	       (InstancePtr->RemainingWords > 0)) {

		XHwIcap_FifoWrite(InstancePtr, *InstancePtr->SendBufferPtr);
		InstancePtr->RemainingWords--;
		WrFifoVacancy--;
		InstancePtr->SendBufferPtr++;
	}

	/*
	 * Start the transfer of the data from the FIFO to the ICAP device.
	 */
	XHwIcap_StartConfig(InstancePtr);

	while ((XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
				XHI_CR_OFFSET)) & XHI_CR_WRITE_MASK);
	/*
	 * Check if there is more data to be written to the ICAP
	 */
	if (InstancePtr->RemainingWords != 0U) {

		/*
		 * Check whether it is polled or interrupt mode of operation.
		 */
		if (InstancePtr->IsPolled == FALSE) { /* Interrupt Mode */

			/*
			 * If it is interrupt mode of operation then the
			 * transfer of the remaining data will be done in the
			 * interrupt handler.
			 */

			/*
			 * Clear the interrupt status of the earlier interrupts
			 */
			IntrStatus  = XHwIcap_IntrGetStatus(InstancePtr);
			XHwIcap_IntrClear(InstancePtr, IntrStatus);


			/*
			 * Enable the interrupts by enabling the
			 * Global Interrupt.
			 */
			XHwIcap_IntrGlobalEnable(InstancePtr);

		}

		else { /* Polled Mode */

			while (InstancePtr->RemainingWords > 0) {

				WrFifoVacancy =
					XHwIcap_GetWrFifoVacancy(InstancePtr);
				while ((WrFifoVacancy != 0) &&
				       (InstancePtr->RemainingWords > 0)) {
					XHwIcap_FifoWrite(InstancePtr,
							  *InstancePtr->SendBufferPtr);
					InstancePtr->RemainingWords--;
					WrFifoVacancy--;
					InstancePtr->SendBufferPtr++;
				}

				XHwIcap_StartConfig(InstancePtr);
				while ((XHwIcap_ReadReg(InstancePtr->
							HwIcapConfig.BaseAddress,
							XHI_CR_OFFSET)) & XHI_CR_WRITE_MASK);
			}

			/*
			 * Clear the flag to indicate the write has
			 * been done
			 */
			InstancePtr->IsTransferInProgress = FALSE;
			InstancePtr->RequestedWords = 0x0;
		}
	}

	else {
		/*
		 * Clear the flag to indicate the write has been done
		 */
		InstancePtr->IsTransferInProgress = FALSE;
		InstancePtr->RequestedWords = 0x0;

	}

#endif
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function reads the specified number of words from the ICAP device in
* the polled mode. Interrupt mode is not supported in reading data from the
* ICAP device.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	FrameBuffer is a pointer to the memory where the frame read
*		from the ICAP device is stored.
* @param	NumWords is the number of words (16 bit for S6 and 32 bit for
* 			all other devices) to write to the ICAP device.
*
* @return
*		- XST_SUCCESS if the specified number of words have been read
*		from the ICAP device
*		- XST_FAILURE if the device is busy with the last Read/Write or
*		if the requested number of words have not been read from the
*		ICAP device, or there is a timeout.
*
* @note		This is a blocking function.
*
*****************************************************************************/
int XHwIcap_DeviceRead(XHwIcap *InstancePtr, u32 *FrameBuffer, u32 NumWords)
{
	u32 Retries = 0;
	u32 Index = 0; /* Array Index */
#if XPAR_HWICAP_0_ICAP_DWIDTH == 8
	u8 Data[NumWords * 4];
#elif XPAR_HWICAP_0_ICAP_DWIDTH == 16
	u16 Data[NumWords * 2];
#else
	u32 *Data = FrameBuffer;
#endif
	u32 RdFifoOccupancy = 0;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(FrameBuffer != NULL);
	Xil_AssertNonvoid(NumWords > 0);

	/*
	 * Make sure that the last Read/Write by the driver is complete.
	 */
	if (XHwIcap_IsTransferDone(InstancePtr) == FALSE) {
		return XST_FAILURE;
	}

	/*
	 * Check if the ICAP device is Busy with the last Write/Read
	 */
	if (XHwIcap_IsDeviceBusy(InstancePtr) == TRUE) {
		return XST_FAILURE;
	}

	/*
	 * Set the flag, which will be cleared by the driver
	 * when the transfer is entirely done.
	 */
	InstancePtr->IsTransferInProgress = TRUE;

	/* 8 bit */
	if (InstancePtr->HwIcapConfig.IcapWidth == 8) {
		InstancePtr->RequestedWords = NumWords * 4;
		InstancePtr->RemainingWords = NumWords * 4;
		XHwIcap_SetSizeReg(InstancePtr, NumWords * 4);
	}
	/* 16 bit */
	else if (InstancePtr->HwIcapConfig.IcapWidth == 16) {
		InstancePtr->RequestedWords = NumWords * 2;
		InstancePtr->RemainingWords = NumWords * 2;
		XHwIcap_SetSizeReg(InstancePtr, NumWords * 2);
	}

	/* 32 bit */
	else {
		InstancePtr->RequestedWords = NumWords;
		InstancePtr->RemainingWords = NumWords;
		XHwIcap_SetSizeReg(InstancePtr, NumWords);
	}

	XHwIcap_StartReadBack(InstancePtr);

	/*
	 * Read the data from the Read FIFO into the buffer provided by
	 * the user.
	 */

	/* As long as there is still data to read... */
	while (InstancePtr->RemainingWords > 0) {
		/* Wait until we have some data in the fifo. */
		while (RdFifoOccupancy == 0) {
			RdFifoOccupancy =
				XHwIcap_GetRdFifoOccupancy(InstancePtr);
			Retries++;
			if (Retries > XHI_MAX_RETRIES) {
				break;
			}
		}

		/* Read the data from the Read FIFO. */
#if (XPAR_HWICAP_0_ICAP_DWIDTH == 8) || (XPAR_HWICAP_0_ICAP_DWIDTH == 16)
		while ((RdFifoOccupancy != 0) &&
		       (InstancePtr->RemainingWords > 0)) {
			Data[Index] = XHwIcap_FifoRead(InstancePtr);
			InstancePtr->RemainingWords--;
			RdFifoOccupancy--;
			Index++;
		}
#else
		while ((RdFifoOccupancy != 0) &&
		       (InstancePtr->RemainingWords > 0)) {
			*Data++ = XHwIcap_FifoRead(InstancePtr);
			InstancePtr->RemainingWords--;
			RdFifoOccupancy--;
		}
#endif
	}
	while ((XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
				XHI_CR_OFFSET)) &
	       XHI_CR_READ_MASK);

	/* 8 bit */
	if (InstancePtr->HwIcapConfig.IcapWidth == 8) {
		for (Index = 0 ; Index < (NumWords * 4) ; Index = Index + 4) {
			*FrameBuffer = Data[Index] << 24;
			*FrameBuffer = *FrameBuffer | Data[Index + 1] << 16;
			*FrameBuffer = *FrameBuffer | Data[Index + 2] << 8;
			*FrameBuffer = *FrameBuffer | Data[Index + 3];
			FrameBuffer++;
		}
	}
	/* 16 bit */
	else if (InstancePtr->HwIcapConfig.IcapWidth == 16) {
		for (Index = 0 ; Index < (NumWords * 2) ; Index = Index + 2) {
			*FrameBuffer = Data[Index] << 16;
			*FrameBuffer = *FrameBuffer | Data[Index + 1];
			FrameBuffer++;
		}
	}

	/*
	 * If the requested number of words have not been read from
	 * the device then indicate failure.
	 */
	if (InstancePtr->RemainingWords != 0) {
		return XST_FAILURE;
	}

	InstancePtr->IsTransferInProgress = FALSE;
	InstancePtr->RequestedWords = 0x0;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function forces the software reset of the complete HWICAP device.
* All the registers will return to the default value and the FIFO is also
* flushed as a part of this software reset.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHwIcap_Reset(XHwIcap *InstancePtr)
{
	u32 RegData;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Reset the device by setting/clearing the RESET bit in the
	 * Control Register.
	 */
	RegData = XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
				  XHI_CR_OFFSET);

	XHwIcap_WriteReg(InstancePtr->HwIcapConfig.BaseAddress, XHI_CR_OFFSET,
			 RegData | XHI_CR_SW_RESET_MASK);

	/*
	 * Reset pulse of atleast 3 slower clock cycle
	 */
	usleep(10);

	XHwIcap_WriteReg(InstancePtr->HwIcapConfig.BaseAddress, XHI_CR_OFFSET,
			 RegData & (~ XHI_CR_SW_RESET_MASK));

}

/*****************************************************************************/
/**
*
* This function flushes the FIFOs in the device.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHwIcap_FlushFifo(XHwIcap *InstancePtr)
{
	u32 RegData;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Flush the FIFO by setting/clearing the FIFO Clear bit in the
	 * Control Register.
	 */
	RegData = XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
				  XHI_CR_OFFSET);

	XHwIcap_WriteReg(InstancePtr->HwIcapConfig.BaseAddress, XHI_CR_OFFSET,
			 RegData | XHI_CR_FIFO_CLR_MASK);

	/*
	 * Reset pulse of atleast 3 slower clock cycle
	 */
	usleep(10);

	XHwIcap_WriteReg(InstancePtr->HwIcapConfig.BaseAddress, XHI_CR_OFFSET,
			 RegData & (~ XHI_CR_FIFO_CLR_MASK));

}

/*****************************************************************************/
/**
*
* This function initiates the Abort Sequence by setting the Abort bit in the
* control register.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHwIcap_Abort(XHwIcap *InstancePtr)
{
	u32 RegData;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Initiate the Abort sequence in the ICAP by setting the Abort bit in
	 * the Control Register.
	 */
	RegData = XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
				  XHI_CR_OFFSET);

	XHwIcap_WriteReg(InstancePtr->HwIcapConfig.BaseAddress, XHI_CR_OFFSET,
			 RegData | XHI_CR_SW_ABORT_MASK);

}

/******************************************************************************
* This function is used to identify the device family.
* @param        Device ID Code.
* @return       Device Family ID( 7-series or 8-series).
* @note         None.
*
*******************************************************************************/
static u32 FindDeviceType(u32 IdCode)
{
	u32 i = 0;
	u32 DeviceType = 0;

	for (i = 0; i < NUM_7SERIES_IDCODES; i++) {

		if ( series_7idcodes[i] == IdCode ) {
			DeviceType = DEVICE_TYPE_7SERIES;
			goto END;
		}
	}

	for (i = 0; i < NUM_ULTRA_SERIES_IDCODES; i++) {

		if ( series_ultra_idcodes[i] == IdCode ) {
			DeviceType = DEVICE_TYPE_ULTRA;
			goto END;
		}
	}

	for (i = 0; i < NUM_ULTRA_PLUS_SERIES_IDCODES; i++) {

		if ( series_ultra_plus_idcodes[i] == IdCode ) {
			DeviceType = DEVICE_TYPE_ULTRA_PLUS;
			goto END;
		}
	}
END:
	return DeviceType;
}

/*****************************************************************************/
/**
*
* This is a stub for the status callback. The stub is here in case the upper
* layers forget to set the handler.
*
* @param	CallBackRef is a pointer to the upper layer callback reference
* @param	StatusEvent is the event that just occurred.
* @param	WordCount is the number of words (32 bit) transferred up until
*		the event occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubStatusHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount)
{
	(void) CallBackRef;
	(void) StatusEvent;
	(void) ByteCount;

	Xil_AssertVoidAlways();
}
/** @} */
