/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_endpoint.c
* @addtogroup usb Overview
* @{
 *
 * This file contains the USB end point related function definitions.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release
 * 1.01a hvm  10/2/08 In function XUsb_EpDataRecv, the initialization of
 *			Buffer0Ready, Buffer1Ready and CurBufNum variables is
 *			moved before the buffer ready bit is set in the buffer
 *			ready register.
 *			Added the initialization of Buffer0Ready, Buffer1Ready
 *			and CurBufNum variables in the XUsb_EpDataSend function.
 * 2.00a hvm  12/2/08 Updated the XUsb_EpDataSend and XUsb_EpRecv  functions to
 *			provide support for DMA and non DMA modes of data
 *			transfer.
 * 3.00a hvm  11/18/09 Updated to use HAL processor APIs. Removed _m from the
 *			name of the macros.
 *
 * 3.02a hvm  8/5/10  Updated the XUsb_EpDataRecv function to ensure that the
 *			buffer ready bit setup is now made only during non-DMA
 *			case. CR570776.
 *
 * 4.01a hvm  8/23/11 Added an API to set the number of isochronous transfers in
 *			a microframe for a given endpoint.
 * </pre>
 ******************************************************************************/
/***************************** Include Files **********************************/

#include "xusb.h"

/************************** Constant Definitions ******************************/

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

/****************************************************************************/
/**
*
* This function enables the specified endpoint for all operations.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number which has to be enabled for
*		operations.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsb_EpEnable(const XUsb *InstancePtr, u8 EpNum)
{
	u32 EpConfigReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(EpNum < XUSB_MAX_ENDPOINTS);

	EpConfigReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				   InstancePtr->EndPointOffset[EpNum]);
	EpConfigReg |= XUSB_EP_CFG_VALID_MASK;

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      InstancePtr->EndPointOffset[EpNum], EpConfigReg);

}

/****************************************************************************/
/**
*
* This function disables the specified endpoint for all operations.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number which has to be disabled for
*		operations.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsb_EpDisable(const XUsb *InstancePtr, u8 EpNum)
{
	u32 EpConfigReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(EpNum < XUSB_MAX_ENDPOINTS);

	EpConfigReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				   InstancePtr->EndPointOffset[EpNum]);
	EpConfigReg &= ~XUSB_EP_CFG_VALID_MASK;

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      InstancePtr->EndPointOffset[EpNum], EpConfigReg);

}

/****************************************************************************/
/**
*
* This function stalls operations for the specified endpoint.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number which has to be stalled for
*		operations.
*
* @return	None.
*
* @note		This function does not guaranty the Stall operation, it only
*		sets the Stall bit in the Endpoint configuration register.
*
*****************************************************************************/
void XUsb_EpStall(const XUsb *InstancePtr, u8 EpNum)
{
	u32 EpConfigReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(EpNum < XUSB_MAX_ENDPOINTS);

	EpConfigReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				   InstancePtr->EndPointOffset[EpNum]);
	EpConfigReg |= XUSB_EP_CFG_STALL_MASK;

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      InstancePtr->EndPointOffset[EpNum], EpConfigReg);

}

/****************************************************************************/
/**
*
* This function unstalls operations for the specified endpoint.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number for which the unstall operations
* 		are to be carried out.
*
* @return	None.
*
* @note		This function does not guaranty the Stall operation, it only
*		sets the Stall bit in the Endpoint configuration register.
*
*****************************************************************************/
void XUsb_EpUnstall(const XUsb *InstancePtr, u8 EpNum)
{
	u32 EpConfigReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(EpNum < XUSB_MAX_ENDPOINTS);

	EpConfigReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				   InstancePtr->EndPointOffset[EpNum]);
	EpConfigReg &= ~XUSB_EP_CFG_STALL_MASK;

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      InstancePtr->EndPointOffset[EpNum], EpConfigReg);

}

/****************************************************************************/
/**
*
* This function configures a specific end point with the given configuration
* data.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number which has to be configured.
* @param	EpCfgPtr is a pointer to the endpoint configuration structure.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsb_EpConfigure(XUsb *InstancePtr, u8 EpNum, XUsb_EpConfig *EpCfgPtr)
{

	u32 EpCfgReg = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(EpNum < XUSB_MAX_ENDPOINTS);
	Xil_AssertVoid(EpCfgPtr != 0);

	/*
	 * Configure the end point direction, type, Max Packet Size and the
	 * EP buffer location.
	 */
	EpCfgReg |= ((EpCfgPtr->OutIn << XUSB_EP_CFG_OUT_IN_SHIFT) |
		     (EpCfgPtr->EpType << XUSB_EP_CFG_ISO_SHIFT) |
		     (EpCfgPtr->Size << XUSB_EP_CFG_PACKET_SIZE_SHIFT) |
		     (EpCfgPtr->RamBase));
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      InstancePtr->EndPointOffset[EpNum], EpCfgReg);

	/*
	 * Set the Buffer count and the Buffer ready bits.
	 */
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      (InstancePtr->EndPointOffset[EpNum] +
		       XUSB_EP_BUF0COUNT_OFFSET), EpCfgPtr->Buffer0Count);

	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      (InstancePtr->EndPointOffset[EpNum] +
		       XUSB_EP_BUF1COUNT_OFFSET), EpCfgPtr->Buffer1Count);

	if (EpCfgPtr->Buffer0Ready == 1) {
		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_BUFFREADY_OFFSET, 1 << EpNum);
	}

	if (EpCfgPtr->Buffer1Ready == 1) {
		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      XUSB_BUFFREADY_OFFSET, (1 <<
						      (EpNum +
						       XUSB_STATUS_EP_BUFF2_SHIFT)));
	}
}

/****************************************************************************/
/**
*
* This function copies the transmit data to the end point buffer and enables the
* buffer for transmission.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number.
* @param	BufferPtr is a pointer to buffer containing the data to be sent.
* @param	BufferLen is the number of data bytes to be sent.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if the send operation could not be performed due
*			to non availability of the ping pong buffers in the
*			DPRAM.
*
* @note		Success doesn't imply that the data is actually transmitted, it
*		only confirms that the DPRAM buffer is updated with send data
*		and the core is enabled for transmitting the data.
*
*****************************************************************************/
int XUsb_EpDataSend(XUsb *InstancePtr, u8 EpNum, u8 *BufferPtr, u32 BufferLen)
{
	UINTPTR *RamBase;
	u32 BytesToSend;
	u8 *TempRamBase;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(EpNum < XUSB_MAX_ENDPOINTS);
	Xil_AssertNonvoid(BufferPtr != NULL);

	BytesToSend = BufferLen;

	/*
	 * Put the transmit buffer into the correct ping-pong buffer.
	 */
	if ((InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum == 0) &&
	    (InstancePtr->DeviceConfig.Ep[EpNum].Buffer0Ready == 0)) {
		/*
		 * Get the Buffer address and copy the transmit data.
		 */
		RamBase = (UINTPTR *) (InstancePtr->Config.BaseAddress +
				       (InstancePtr->DeviceConfig.Ep[EpNum].
					RamBase));
		/*
		 * Set the Buffer count register with transmit length.
		 */
		XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			      (InstancePtr->EndPointOffset[EpNum] +
			       XUSB_EP_BUF0COUNT_OFFSET), BufferLen);

		InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum = 1;
		InstancePtr->DeviceConfig.Ep[EpNum].Buffer0Ready = 1;

		if (InstancePtr->Config.DmaEnabled) {

			/*
			 * Set the correct buffer ready mask and
			 * enable the DMA transfer.
			 */
			XUsb_WriteReg(InstancePtr->Config.BaseAddress,
				      XUSB_DMA_CONTROL_OFFSET,
				      (XUSB_DMA_BRR_CTRL | (1 << EpNum)));

			XUsb_DmaTransfer(InstancePtr, (UINTPTR *)BufferPtr,
					 RamBase, BytesToSend);

		} else {

			/*
			 * Copy the data into the USB DPRAM
			 */
			while (BytesToSend > 3) {

				*RamBase++ = *(UINTPTR *) BufferPtr;
				BufferPtr += 4;
				BytesToSend -= 4;
			}
			TempRamBase = (u8 *) RamBase;
			while (BytesToSend--) {
				*TempRamBase++ = *BufferPtr++;
			}

			/*
			 * Enable the transmission.
			 */
			XUsb_WriteReg(InstancePtr->Config.BaseAddress,
				      XUSB_BUFFREADY_OFFSET, (1 << EpNum));
		}

	} else {

		if ((InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum == 1) &&
		    (InstancePtr->DeviceConfig.Ep[EpNum].Buffer1Ready == 0)) {
			/*
			 * Get the Buffer address and copy the transmit data.
			 */
			RamBase = (UINTPTR *) (InstancePtr->Config.BaseAddress +
					       ((InstancePtr->DeviceConfig.
						 Ep[EpNum].RamBase)) +
					       (InstancePtr->DeviceConfig.Ep[EpNum].
						Size));
			/*
			 * Set the Buffer count register with transmit length
			 * and enable the buffer for transmission.
			 */
			XUsb_WriteReg(InstancePtr->Config.BaseAddress,
				      (InstancePtr->EndPointOffset[EpNum] +
				       XUSB_EP_BUF1COUNT_OFFSET), BufferLen);

			InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum = 0;
			InstancePtr->DeviceConfig.Ep[EpNum].Buffer1Ready = 1;

			if (InstancePtr->Config.DmaEnabled) {

				/*
				 * Set the correct buffer ready mask and
				 * enable the DMA transfer
				 */
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					      XUSB_DMA_CONTROL_OFFSET,
					      ( XUSB_DMA_BRR_CTRL |
						(1 << (EpNum +
						       XUSB_STATUS_EP_BUFF2_SHIFT))));

				XUsb_DmaTransfer(InstancePtr,
						 (UINTPTR *)BufferPtr, RamBase,
						 BytesToSend );

			} else {

				/*
				 * Copy the data into the USB DPRAM
				 */
				while (BytesToSend > 3) {

					*RamBase++ = *(UINTPTR *) BufferPtr;
					BufferPtr += 4;
					BytesToSend -= 4;
				}
				TempRamBase = (u8 *) RamBase;
				while (BytesToSend--) {
					*TempRamBase++ = *BufferPtr++;
				}

				/*
				 * Enable the Transmission.
				 */
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					      XUSB_BUFFREADY_OFFSET, (1 << (EpNum +
								      XUSB_STATUS_EP_BUFF2_SHIFT)));

			}


		} else {
			/*
			 * None of the ping-pong buffer is free. Reply a
			 * failure.
			 */
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function copies the received data from end point buffer to the buffer
* passed and then makes the device ready for receiving data again into the same
* end point buffer.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number.
* @param	BufferPtr is a pointer to buffer where data is to be copied.
* @param	BufferLen is the number of data bytes to be received.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if there is no received data in any of the
* 		ping pong buffers.
*
* @note		None
*
*****************************************************************************/
int XUsb_EpDataRecv(XUsb *InstancePtr, u8 EpNum, u8 *BufferPtr, u32 BufferLen)
{
	UINTPTR *RamBase;
	u32 RxBytesToRead;
	u8 *TempRamBase;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(EpNum < XUSB_MAX_ENDPOINTS);
	Xil_AssertNonvoid(BufferPtr != NULL);

	RxBytesToRead = BufferLen;

	if ((InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum == 0) &&
	    (InstancePtr->DeviceConfig.Ep[EpNum].Buffer0Ready == 0)) {
		/*
		 * Get the EP buffer address and copy the Received data.
		 */
		RamBase = (UINTPTR *) (InstancePtr->Config.BaseAddress +
				       (InstancePtr->DeviceConfig.Ep[EpNum].
					RamBase));
		InstancePtr->DeviceConfig.Ep[EpNum].Buffer0Ready = 1;
		InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum = 1;

		if (InstancePtr->Config.DmaEnabled) {

			/*
			 * Set the correct buffer ready mask and
			 * enable the DMA transfer.
			 */
			XUsb_WriteReg(InstancePtr->Config.BaseAddress,
				      XUSB_DMA_CONTROL_OFFSET,
				      (XUSB_DMA_BRR_CTRL |
				       XUSB_DMA_READ_FROM_DPRAM |
				       (1 << EpNum)));

			XUsb_DmaTransfer(InstancePtr, RamBase,
					 (UINTPTR *)BufferPtr,
					 RxBytesToRead);

		} else {
			/*
			 * Read the data from the USB DPRAM and set the
			 * buffer ready for receiving next packet.
			 */
			while (RxBytesToRead > 3) {

				*(UINTPTR *) BufferPtr = *RamBase++;
				BufferPtr += 4;
				RxBytesToRead -= 4;
			}
			TempRamBase = (u8 *) RamBase;
			while (RxBytesToRead--) {
				*BufferPtr++ = *TempRamBase++;
			}

			XUsb_WriteReg(InstancePtr->Config.BaseAddress,
				      XUSB_BUFFREADY_OFFSET, 1 << EpNum);
		}

	} else {

		if ((InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum == 1) &&
		    (InstancePtr->DeviceConfig.Ep[EpNum].Buffer1Ready == 0)) {
			/*
			 * Get the EP buffer address and copy the Received data.
			 */
			RamBase = (UINTPTR *) (InstancePtr->Config.BaseAddress +
					       ((InstancePtr->DeviceConfig.
						 Ep[EpNum].RamBase)) +
					       (InstancePtr->DeviceConfig.Ep[EpNum].
						Size));
			InstancePtr->DeviceConfig.Ep[EpNum].Buffer1Ready = 1;
			InstancePtr->DeviceConfig.Ep[EpNum].CurBufNum = 0;

			if (InstancePtr->Config.DmaEnabled) {

				/*
				 * Set the correct buffer ready mask and
				 * enable the DMA transfer
				 */
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					      XUSB_DMA_CONTROL_OFFSET, (
						      (XUSB_DMA_BRR_CTRL |
						       XUSB_DMA_READ_FROM_DPRAM |
						       (1 << (EpNum +
							      XUSB_STATUS_EP_BUFF2_SHIFT)))));

				XUsb_DmaTransfer(InstancePtr, RamBase,
						 (UINTPTR *)BufferPtr,
						 RxBytesToRead);

			} else {
				/*
				 * Read the data from the USB DPRAM and set the
				 * buffer ready for receiving next packet.
				 */
				while (RxBytesToRead > 3) {

					*(UINTPTR *) BufferPtr = *RamBase++;
					BufferPtr += 4;
					RxBytesToRead -= 4;
				}

				TempRamBase = (u8 *) RamBase;
				while (RxBytesToRead--) {
					*BufferPtr++ = *TempRamBase++;
				}
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					      XUSB_BUFFREADY_OFFSET,
					      (1 << (EpNum +
						     XUSB_STATUS_EP_BUFF2_SHIFT)));
			}
		} else {

			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function sets the number of isochronous transfers in a microframe
* for a given endpoint.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	EpNum is the endpoint number.
* @param	NoOfTransfers is the number of transfers in a microframe.
*
* @return	None
*
* @note		The values allowed for number of transfers is 1, 2 and 3.
*		This function should be called after all the endpoints are
*		configured. The allowed value for EpNum is 1 to 7
*
*****************************************************************************/
void XUsb_EpIsoTransferConfigure(XUsb *InstancePtr, u8 EpNum,
				 u8 NoOfTransfers)
{
	u32 EpConfigReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((EpNum < XUSB_MAX_ENDPOINTS) && (EpNum != 0));
	Xil_AssertVoid(NoOfTransfers < 2);

	EpConfigReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				   InstancePtr->EndPointOffset[EpNum]);


	EpConfigReg |= 	(NoOfTransfers << XUSB_EP_CFG_ISOTRANS_SHIFT);


	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
		      InstancePtr->EndPointOffset[EpNum], EpConfigReg);

}
/** @} */
