/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xusb.c
* @addtogroup usb Overview
* @{
 *
 * The XUsb driver. Functions in this file are the minimum required functions
 * for this driver. See xusb.h for a detailed description of the driver.
 *
 * @note     None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release
 * 2.00a hvm  10/22/08 Added DMA APIs.
 * 3.00a hvm  12/3/09 Added XUsb_ReadErrorCounters API to return USB error
 *                     counters data. Updated to use HAL processor APIs.
 *		       XUsb_mReadReg is renamed to XUsb_ReadReg and
 *		       XUsb_mWriteReg is renamed to XUsb_WriteReg.
 * 3.02a hvm  7/15/10  Added Device ID initialization in XUsb_CfgInitialize
 *		       function (CR555996).
 * 4.00a hvm  10/21/10 Added ULPI PHY Read/Write APIs.
 * 			Added DMA handler initialization in XUsb_CfgInitialize
 *			function
 * 4.03a bss  06/20/10 Added SIE Reset API (XUsb_SieReset) to reset the SIE
 * 			state machine (CR 660602)
 * 5.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
 *                     Changed the prototype of XUsb_CfgInitialize API.
 * 5.2	MNK   29/03/15 Added 64bit changes for ZYNQMP.
 * 5.6   pm   07/05/23 Added support for system device-tree flow.
 * </pre>
 *****************************************************************************/

/***************************** Include Files *********************************/

#include "xusb.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static void StubHandler(void);

/*****************************************************************************/
/**
*
* This function initializes a XUsb instance/driver.
*
* The initialization entails:
* - Initialize all members of the XUsb structure.
*
* @param	InstancePtr is a pointer to the XUsb instance of the USB device.
* @param	ConfigPtr is a pointer to a XUsb_Config configuration structure.
* 		This structure will contain the requested configuration for the
* 		device. Typically, this is a local structure and the content of
* 		which will be copied into the configuration structure within
* 		XUsb.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return
*		- XST_SUCCESS no errors occurred.
*		- XST_FAILURE an error occurred during initialization.
*
* @note		After calling XUsb_CfgInitialize() the USB device IS NOT READY
*		for use. Before the USB device can be used its parameters must
*		be configured. See xusb.h for details.
*
******************************************************************************/
int XUsb_CfgInitialize(XUsb *InstancePtr, XUsb_Config *ConfigPtr,
		       UINTPTR EffectiveAddr)
{
	u8 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Initialize the XUsb structure to default values.
	 */
#ifndef SDT
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#else
	InstancePtr->Config.Name = ConfigPtr->Name;
#endif
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.DmaEnabled = ConfigPtr->DmaEnabled;
	InstancePtr->Config.AddrWidth = ConfigPtr->AddrWidth;
	InstancePtr->DeviceConfig.NumEndpoints = XUSB_MAX_ENDPOINTS;
	for (Index = 0; Index < XUSB_MAX_ENDPOINTS; Index++) {

		InstancePtr->EndPointOffset[Index] = XUSB_EP0_CONFIG_OFFSET +
						     (Index * 0x10);
	}
	InstancePtr->HandlerFunc = (XUsb_IntrHandlerFunc) StubHandler;
	InstancePtr->ErrHandlerFunc = (XUsb_IntrHandlerFunc) StubHandler;
	InstancePtr->DmaHandlerFunc = (XUsb_IntrHandlerFunc) StubHandler;
	InstancePtr->UlpiHandlerFunc = (XUsb_IntrHandlerFunc) StubHandler;
	InstancePtr->DeviceConfig.CurrentSpeed = XUSB_EP_FULL_SPEED;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes USB End points.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	CfgPtr is pointer to a XUsb_Config configuration structure.
*		This structure will contain the requested configuration for the
* 		device. Typically, this is a local structure and the content of
* 		which will be copied into the configuration structure within
* 		XUsb.
*
* @return
*		- XST_SUCCESS no errors occurred.
*		- XST_FAILURE an error occurred during initialization.
*
* @note		None.
*
******************************************************************************/
int XUsb_ConfigureDevice(XUsb *InstancePtr, XUsb_DeviceConfig *CfgPtr)
{

	int Index;

	/*
	 * Initialize the End points.
	 */
	for (Index = 0; Index < CfgPtr->NumEndpoints; Index++) {

		InstancePtr->DeviceConfig.Ep[Index].RamBase =
			CfgPtr->Ep[Index].RamBase;
		InstancePtr->DeviceConfig.Ep[Index].Size =
			CfgPtr->Ep[Index].Size;

		InstancePtr->DeviceConfig.Ep[Index].EpType =
			CfgPtr->Ep[Index].EpType;
		if (Index == 0) {
			InstancePtr->DeviceConfig.Ep[Index].Buffer0Ready = 1;
		} else {
			InstancePtr->DeviceConfig.Ep[Index].Buffer0Ready = 0;
		}

		InstancePtr->DeviceConfig.Ep[Index].Buffer0Count = 0;
		InstancePtr->DeviceConfig.Ep[Index].Buffer1Ready = 0;
		InstancePtr->DeviceConfig.Ep[Index].Buffer1Count = 0;
		InstancePtr->DeviceConfig.Ep[Index].OutIn =
			CfgPtr->Ep[Index].OutIn;
		InstancePtr->DeviceConfig.Ep[Index].CurBufNum = 0;

		XUsb_EpConfigure(InstancePtr, Index,
				 &InstancePtr->DeviceConfig.Ep[Index]);

	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function starts the USB Device.
*
* @param	InstancePtr is a pointer to the XUsb instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XUsb_Start(XUsb *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Start the USB Serial Interface Engine.
	 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      XUSB_CONTROL_OFFSET, XUSB_CONTROL_USB_READY_MASK);

}

/*****************************************************************************/
/**
* This function stops the USB device.
*
* @param	InstancePtr is a pointer to the XUsb instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XUsb_Stop(XUsb *InstancePtr)
{
	u32 CrRegValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	CrRegValue = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				  XUSB_CONTROL_OFFSET);

	CrRegValue &= ~XUSB_CONTROL_USB_READY_MASK;

	/*
	 * Stop the USB Serial Interface Engine.
	 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      XUSB_CONTROL_OFFSET, CrRegValue);
}

/*****************************************************************************/
/**
* This function returns the current frame number.
*
* @param	InstancePtr is a pointer to the XUsb instance.
*
* @return	The current frame number..
*
* @note		None.
*
******************************************************************************/
u32 XUsb_GetFrameNum(const XUsb *InstancePtr)
{
	u32 FrameNumPtr = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	FrameNumPtr = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				   XUSB_FRAMENUM_OFFSET);

	return (FrameNumPtr);

}

/*****************************************************************************/
/**
* This function sets the USB device address.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	Address is the device address to be set.
*
* @return
*		- XST_SUCCESS: Address set successfully.
*		- XST_INVALID_PARAM: Invalid parameter passed.
*
* @note		None.
*
******************************************************************************/
int XUsb_SetDeviceAddress(XUsb *InstancePtr, u8 Address)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	/*
	 * Check address range validity.
	 */
	if (Address > XUSB_DEVICEADDR_MAX) {
		return XST_INVALID_PARAM;
	}

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      XUSB_ADDRESS_OFFSET, Address);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function sets the USB device into a given test mode.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	TestMode is the type of test to be performed.
* @param	BufPtr is a pointer to the buffer containing the test packet.
*
* @return	None.
*
* @note		If the test mode is Test packet(TEST_PKT), then user needs
*		to pass the address of the buffer containing the test packet. In
*		other cases, the BufPtr parameter is not used and the user can
* 		send a NULL or any value. BufPtr parameter should be 32 bit
*		aligned.
*
******************************************************************************/
void XUsb_SetTestMode(XUsb *InstancePtr, u8 TestMode, u8 *BufPtr)
{

	u32 Count;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((TestMode == TEST_J) ||
		       (TestMode == TEST_K) ||
		       (TestMode == TEST_SE0_NAK) || (TestMode == TEST_PKT));

	/*
	 * Stop the SIE.
	 */
	XUsb_Stop(InstancePtr);

	if (TestMode == TEST_PKT) {

		volatile u32 *Src, *Dst;

		if (BufPtr == NULL) {
			/*
			 * Null pointer is passed.
			 */
			while (1) {
				/*
				 * Do a hardware reset to re-start the device.
				 */
			}

		}

		Src = (u32 *) BufPtr;
		Dst = (u32 *) (InstancePtr->Config.BaseAddress);
		Count = 14;

		/*
		 * Copy Leurker PKT to DP RAM at 0.
		 */
		while (Count--) {
			*Dst++ = *Src++;
		}
	}

	/*
	 * Set the test mode.
	 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress, XUSB_TESTMODE_OFFSET,
		      TestMode);
	/*
	 * Re-start the SIE.
	 */
	XUsb_Start(InstancePtr);

	while (1) {
		;		/*
				 * Only way out is through hardware reset!
				 */
	}

}

/******************************************************************************/
/**
* This function resets the DMA module of the USB device
*
* @param	InstancePtr is a pointer to the XUsb instance.
*
* @return	None.
*
* @note		After the DMA reset, only the DMA related logic part of the
*		USB device will be reset and all the DMA related registers will
*		be reset to the default values. Upon DMA Reset, any DMA
*		transfer in progress will be stopped.
*
******************************************************************************/
void XUsb_DmaReset(XUsb *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XUsb_WriteReg(InstancePtr->Config.BaseAddress, XUSB_DMA_RESET_OFFSET,
		      XUSB_DMA_RESET_VALUE);
}


/******************************************************************************/
/**
* This function sets the DMA registers with the given values to initiate a  DMA
* data transfer. This function is called by the XUsb_EpDataSend and
* XUsb_EpDataRecv functions.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	SrcAddr is the source address from where the data is to be
*		read.
* @param	DstAddr is the destination address to where the data is to be
*		written.
* @param	Length is the amount of data that can be transferred. The
*		maximum data transfer can be 1024.
*
* @return	None.
*
* @note		This function doesn't guarantee that the transfer is done
*		successfully. This function only initiates the DMA transfer.
*
******************************************************************************/
void XUsb_DmaTransfer(XUsb *InstancePtr, UINTPTR *SrcAddr, UINTPTR *DstAddr,
		      u16 Length)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SrcAddr != NULL);
	Xil_AssertVoid(DstAddr != NULL);
	Xil_AssertVoid(Length <= 1024);

	/*
	 * Set the addresses in the DMA source and destination
	 * registers and then set the length into the DMA length register.
	 */

	if (InstancePtr->Config.AddrWidth > 32) {

		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_DMA_DSAR_ADDR_OFFSET_LSB,
			      LOWER_32_BITS((UINTPTR)SrcAddr));
		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_DMA_DSAR_ADDR_OFFSET_MSB,
			      UPPER_32_BITS((UINTPTR)SrcAddr));


		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_DMA_DDAR_ADDR_OFFSET_LSB,
			      LOWER_32_BITS((UINTPTR)DstAddr));
		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_DMA_DDAR_ADDR_OFFSET_MSB,
			      UPPER_32_BITS((UINTPTR)DstAddr));
	} else {

		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_DMA_DSAR_ADDR_OFFSET,
			      (UINTPTR)SrcAddr);

		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_DMA_DDAR_ADDR_OFFSET,
			      (UINTPTR)DstAddr);
	}
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      XUSB_DMA_LENGTH_OFFSET,
		      Length);
}

/******************************************************************************/
/**
* This function reads the USB error counter register and returns the error
* counters information.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	BitStuffErrors is  a pointer to the 8 bit bitstuff error
*		counter.
* @param	PidErrors is a pointer to the 8 bit pid error counter.
* @param	CrcErrors is a pointer to the 8 bit crc error counter.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XUsb_ReadErrorCounters(XUsb *InstancePtr, u8 *BitStuffErrors,
			    u8 *PidErrors, u8 *CrcErrors)
{
	u32 ErrCounterReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BitStuffErrors != NULL);
	Xil_AssertVoid(PidErrors != NULL);
	Xil_AssertVoid(CrcErrors != NULL);

	ErrCounterReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				     XUSB_ECR_OFFSET);

	*BitStuffErrors = (ErrCounterReg & XUSB_ECR_BITSTUFF_ERRCNT_MASK) >>
			  XUSB_ECR_BITSTUFF_ERRCNT_SHIFT;
	*PidErrors = (ErrCounterReg & XUSB_ECR_PID_ERRCNT_MASK) >>
		     XUSB_ECR_PID_ERRCNT_SHIFT;
	*CrcErrors = (ErrCounterReg & XUSB_ECR_CRC_ERRCNT_MASK) >>
		     XUSB_ECR_CRC_ERRCNT_SHIFT;
}

/******************************************************************************/
/**
* This function initiates the USB ULPI PHY register read transaction. This
* function  returns the busy status if the earlier transaction is still in
* progress and returns the PHY register data upon successful read transaction.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	RegAddr is the address of the PHY register.
*
* @return
*		- Register data
*		- XST_DEVICE_BUSY: The previous PHY transaction is still in
*		  progress.
*
* @note		This function waits till the BUSY bit is cleared in the ULPI
*		PHY resgiter and then reads the register. The user of this API
*		should note that the PHY interrupt should be ignored during read
*		operation.
*
******************************************************************************/
u8 XUsb_UlpiPhyReadRegister(XUsb *InstancePtr, u8 RegAddr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Check whether the earlier transaction is complete.
	 */
	if (XUsb_ReadReg(InstancePtr->Config.BaseAddress,
			 XUSB_UPAR_OFFSET) & XUSB_UPAR_BUSY_MASK) {

		return XST_DEVICE_BUSY;
	}

	/*
	 * Initiate the read transaction for the given PHY register.
	 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress, XUSB_UPAR_OFFSET,
		      RegAddr);

	while (XUsb_ReadReg(InstancePtr->Config.BaseAddress,
			    XUSB_UPAR_OFFSET) & XUSB_UPAR_BUSY_MASK);

	return (((XUsb_ReadReg(InstancePtr->Config.BaseAddress,
			       XUSB_UPAR_OFFSET) & XUSB_UPAR_REG_DATA_MASK) >>
		 XUSB_UPAR_REG_DATA_SHIFT));

}

/******************************************************************************/
/**
* This function initiates the USB ULPI PHY register write transaction. This
* function  returns the busy status if the earlier transaction is still in
* progress and returns a success upon successful write transaction initiation.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	RegAddr is the address of the PHY register.
*		counter.
* @param	UlpiPhyRegData is the data to be written to PHY register.
*
* @return
*		- XST_SUCCESS: Read transaction initiated successfully.
*		- XST_DEVICE_BUSY: The previous PHY transaction is still in
*		  progress.
*
* @note		None.
*
******************************************************************************/
int XUsb_UlpiPhyWriteRegister(XUsb *InstancePtr, u8 RegAddr,
			      u8 UlpiPhyRegData)
{

	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Check whether the earlier transaction is complete.
	 */
	if (XUsb_ReadReg(InstancePtr->Config.BaseAddress,
			 XUSB_UPAR_OFFSET) & XUSB_UPAR_BUSY_MASK) {

		return XST_DEVICE_BUSY;
	}

	/*
	 * Initiate the write transaction for the given PHY register.
	 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress, XUSB_UPAR_OFFSET,
		      (RegAddr |
		       (XUSB_UPAR_READ_WRITE_MASK) | (UlpiPhyRegData <<
				       XUSB_UPAR_REG_DATA_SHIFT)));

	return XST_SUCCESS;

}

/******************************************************************************/
/**
* This routine is a stub for the asynchronous callbacks. The stub is here in
* case the upper layer forgot to set the handler. On initialization, all
* handlers are set to this callback. It is considered an error for this handler
* to be invoked.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubHandler(void)
{
	Xil_AssertVoidAlways();
}


/******************************************************************************/
/**
* This function resets the Serial Interface Engine
*
* @param	InstancePtr is a pointer to the XUsb instance.
*
* @return	None.
*
* @note		After the SIE reset, only the SIE state machine logic part of
*		the USB device will be reset and starts from Init state.
*
******************************************************************************/
void XUsb_SieReset(XUsb *InstancePtr)
{

	u32 RegData;
	Xil_AssertVoid(InstancePtr != NULL);

	RegData = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
			       XUSB_CONTROL_OFFSET);
	/* Reset by writing 1 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress, XUSB_CONTROL_OFFSET,
		      RegData | XUSB_CONTROL_SIE_RESET_MASK);
	/* Release from reset by writing 0 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress, XUSB_CONTROL_OFFSET,
		      RegData & (~(XUSB_CONTROL_SIE_RESET_MASK)));
}
/** @} */
