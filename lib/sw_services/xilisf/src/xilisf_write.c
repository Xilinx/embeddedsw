/******************************************************************************
*
* Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
/*****************************************************************************/
/**
*
* @file xilisf_write.c
*
* This file contains the library functions to write to the Serial Flash
* devices. Refer xilisf.h for detailed description.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------  -------- -----------------------------------------------
* 1.00a ksu/sdm  03/03/08 First release
* 2.01a sdm      01/04/10 Added Support for Winbond W25QXX/W25XX devices
* 2.04a sdm      08/17/10 Updated to support Numonyx (N25QXX) and Spansion
*			  flash memories
* 3.00a srt	 06/20/12 Updated to support interfaces SPI PS and QSPI PS.
*			  Added support to SST flash on SPI PS interface.
* 5.0   sb	 08/05/14 Updated support for > 128 MB flash for PSQSPI
*			  interface.
*			  Changed API:
*				WriteData()
*				XIsf_Write()
* 5.2  asa       05/12/15 Added support for Micron (N25Q256A) flash part
* 						  which supports 4 byte addressing.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilisf.h"

/************************** Constant Definitions *****************************/
#define SIXTEENMB	0x1000000	/**< Sixteen MB */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

extern int XIsf_Transfer(XIsf *InstancePtr, u8 *WritePtr, u8* ReadPtr,
			 u32 ByteCount);
extern u32 GetRealAddr(XIsf_Iface *QspiPtr, u32 Address);

#ifdef XPAR_XISF_INTERFACE_PSQSPI
extern int SendBankSelect(XIsf *InstancePtr, u32 BankSel);
#endif
static int WriteData(XIsf *InstancePtr, u8 Command, u32 Address,
			const u8 *BufferPtr, u32 ByteCount);
static int AutoPageWrite(XIsf *InstancePtr, u32 Address);
static int BufferWrite(XIsf *InstancePtr, u8 BufferNum, const u8 *WritePtr,
			u32 ByteOffset, u32 NumBytes);
static int BufferToFlashWriteWithErase(XIsf *InstancePtr, u8 BufferNum,
					u32 Address);
static int BufferToFlashWriteWithoutErase(XIsf *InstancePtr, u8 BufferNum,
					  u32 Address);
static int WriteSR(XIsf *InstancePtr, u8 SRData);
static int WriteSR2(XIsf *InstancePtr, u8 *SRData);
static int WriteOTPData(XIsf *InstancePtr, u32 Address, const u8 *BufferPtr);

/************************** Variable Definitions *****************************/
extern u32 XIsf_StatusEventInfo;
extern unsigned int XIsf_ByteCountInfo;
/************************** Function Definitions ******************************/


/*****************************************************************************/
/**
*
* This API writes the data to the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Operation is the type of write operation to be performed on the
*		Serial Flash.
*		The different operations are
*		- XISF_WRITE: Normal Write
*		- XISF_DUAL_IP_PAGE_WRITE: Dual Input Fast Program
*		- XISF_DUAL_IP_EXT_PAGE_WRITE: Dual Input Extended Fast Program
*		- XISF_QUAD_IP_PAGE_WRITE: Quad Input Fast Program
*		- XISF_QUAD_IP_EXT_PAGE_WRITE: Quad Input Extended Fast Program
*		- XISF_AUTO_PAGE_WRITE: Auto Page Write
*		- XISF_BUFFER_WRITE: Buffer Write
*		- XISF_BUF_TO_PAGE_WRITE_WITH_ERASE: Buffer to Page Transfer
*			with Erase
*		- XISF_BUF_TO_PAGE_WRITE_WITHOUT_ERASE: Buffer to Page Transfer
*			without Erase
*		- XISF_WRITE_STATUS_REG: Status Register Write
*		- XISF_WRITE_STATUS_REG2: 2 byte Status Register Write
*		- XISF_OTP_WRITE: OTP Write.
*
* @param	OpParamPtr is pointer to a structure variable which contains
*		operational parameters of the specified operation.
*		This parameter type is dependant on value of first argument
*		(Operation).
*
*		- Normal Write (XISF_WRITE), Dual Input Fast Program
*		(XISF_DUAL_IP_PAGE_WRITE), Dual Input Extended Fast Program
*		(XISF_DUAL_IP_EXT_PAGE_WRITE), Quad Input Fast Program
*		(XISF_QUAD_IP_PAGE_WRITE), Quad Input Extended Fast Program
*		(XISF_QUAD_IP_EXT_PAGE_WRITE):
*		The OpParamPtr must be of type struct XIsf_WriteParam.
* 		OpParamPtr->Address is the start address in the Serial Flash.
*		OpParamPtr->WritePtr is a pointer to the data to be written to
*		the Serial Flash.
*		OpParamPtr->NumBytes is the number of bytes to be written to
*		Serial Flash.
*		This operation is supported for Atmel, Intel, STM, Winbond and
*		Spansion Serial Flash.
*
*		- Auto Page Write (XISF_AUTO_PAGE_WRITE):
*		The OpParamPtr must be of 32 bit unsigned integer variable.
*		This is the address of page number in the Serial Flash which is
*		to be refreshed.
*		This operation is only supported for Atmel Serial Flash.
*
*		- Buffer Write (XISF_BUFFER_WRITE):
*		The OpParamPtr must be of type struct
*		XIsf_BufferToFlashWriteParam.
*		OpParamPtr->BufferNum specifies the internal SRAM Buffer of the
*		Serial Flash. The valid values are XISF_PAGE_BUFFER1 or
*		XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2 is not valid in case of
*		AT45DB011D Flash as it contains a single buffer.
*		OpParamPtr->WritePtr is a pointer to the data to be written to
*		the Serial Flash SRAM Buffer.
*		OpParamPtr->ByteOffset is byte offset in the buffer from where
*		the data is to be written.
*		OpParamPtr->NumBytes is number of bytes to be written to the
*		Buffer.
*		This operation is supported only for Atmel Serial Flash.
*
*		- Buffer To Memory Write With Erase
*			(XISF_BUF_TO_PAGE_WRITE_WITH_ERASE)/
*		  Buffer To Memory Write Without Erase
*			(XISF_BUF_TO_PAGE_WRITE_WITHOUT_ERASE):
*		The OpParamPtr must be  of type struct
*		XIsf_BufferToFlashWriteParam.
*		OpParamPtr->BufferNum specifies the internal SRAM Buffer of the
*		Serial Flash. The valid values are XISF_PAGE_BUFFER1 or
*		XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2 is not valid in case of
*		AT45DB011D Flash as it contains a single buffer.
*		OpParamPtr->Address is starting address in the Serial Flash
*		memory from where the data is to be written.
*		These operations are only supported for Atmel Serial Flash.
*
*		- Write Status Register (XISF_WRITE_STATUS_REG):
*		The OpParamPtr must be  of type of 8 bit unsigned integer
*		variable. This is the value to be written to the Status
*		Register.
*		This operation is only supported for Intel, STM Winbond and
*		Spansion Serial Flash.
*
*		- Write Status Register2 (XISF_WRITE_STATUS_REG2):
*		The OpParamPtr must be  of type (u8 *) and should point to two
*		8 bit unsigned integer values. This is the value to be written
*		to the 16 bit Status Register.
*		This operation is only supported in Winbond (W25Q) Serial Flash.
*
*		- One Time Programmable Area Write (XISF_OTP_WRITE):
*		The OpParamPtr must be of type struct XIsf_WriteParam.
*		OpParamPtr->Address is the address in the SRAM Buffer of the
*		Serial Flash to which the data is to be written.
*		OpParamPtr->WritePtr is a pointer to the data to be written to
*		the Serial Flash.
*		OpParamPtr->NumBytes should be set to 1 when performing
*		OTPWrite operation.
*		This operation is only supported for Intel Serial Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- Application must fill the structure elements of the third
*		argument and pass its pointer by type casting it with void
*		pointer.
*		- For Intel, STM, Winbond and Spansion Serial Flash, the user
*		application must call the XIsf_WriteEnable() API by passing
*		XISF_WRITE_ENABLE as an argument, before calling the
*		XIsf_Write() API.
*
******************************************************************************/
int XIsf_Write(XIsf *InstancePtr, XIsf_WriteOperation Operation,
			void *OpParamPtr)
{
	int Status = (int)XST_FAILURE;
	u8 Mode;
	XIsf_WriteParam *WriteParamPtr;
	XIsf_BufferWriteParam *BufferWriteParamPtr;
	XIsf_BufferToFlashWriteParam *BufferToFlashWriteParamPtr;

	if (InstancePtr == NULL) {
		return (int)XST_FAILURE;
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)XST_FAILURE;
	}

	if (NULL == OpParamPtr) {
		return (int)XST_FAILURE;
	}

	switch (Operation) {
		case XISF_WRITE:
			WriteParamPtr = (XIsf_WriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(WriteParamPtr != NULL);
			Status = WriteData(InstancePtr,
				XISF_CMD_PAGEPROG_WRITE,
				WriteParamPtr->Address,
				WriteParamPtr->WritePtr,
				WriteParamPtr->NumBytes);
			break;

		case XISF_AUTO_PAGE_WRITE:
			Status = AutoPageWrite(InstancePtr,
					*((u32*)(void *) OpParamPtr));
			break;

		case XISF_BUFFER_WRITE:
			BufferWriteParamPtr = (XIsf_BufferWriteParam*)
						(void *) OpParamPtr;
			Xil_AssertNonvoid(BufferWriteParamPtr != NULL);
			Status = BufferWrite(InstancePtr,
					BufferWriteParamPtr->BufferNum,
					BufferWriteParamPtr->WritePtr,
					BufferWriteParamPtr->ByteOffset,
					BufferWriteParamPtr->NumBytes);
			break;

		case XISF_BUF_TO_PAGE_WRITE_WITH_ERASE:
			BufferToFlashWriteParamPtr =
			(XIsf_BufferToFlashWriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(BufferToFlashWriteParamPtr != NULL);
			Status = BufferToFlashWriteWithErase(InstancePtr,
					BufferToFlashWriteParamPtr->BufferNum,
					BufferToFlashWriteParamPtr->Address);
			break;

		case XISF_BUF_TO_PAGE_WRITE_WITHOUT_ERASE:
			BufferToFlashWriteParamPtr =
			(XIsf_BufferToFlashWriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(BufferToFlashWriteParamPtr != NULL);
			Status = BufferToFlashWriteWithoutErase(InstancePtr,
					BufferToFlashWriteParamPtr->BufferNum,
					BufferToFlashWriteParamPtr->Address);
			break;

		case XISF_WRITE_STATUS_REG:
			Status = WriteSR(InstancePtr,
				*((u8*)(void *) OpParamPtr));
			break;

		case XISF_OTP_WRITE:
			WriteParamPtr = (XIsf_WriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(WriteParamPtr != NULL);
			if (1 == WriteParamPtr->NumBytes) {
				Status = WriteOTPData(InstancePtr,
					WriteParamPtr->Address,
					WriteParamPtr->WritePtr);
			}
			break;

		case XISF_WRITE_STATUS_REG2:
			Status = WriteSR2(InstancePtr,
				(u8*)(void *) OpParamPtr);
			break;

#if ((XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == STM) \
     || (XPAR_XISF_FLASH_FAMILY == SPANSION))
		case XISF_QUAD_IP_PAGE_WRITE:
			WriteParamPtr = (XIsf_WriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(WriteParamPtr != NULL);
			Status = WriteData(InstancePtr,
					XISF_CMD_QUAD_IP_PAGE_WRITE,
					WriteParamPtr->Address,
					WriteParamPtr->WritePtr,
					WriteParamPtr->NumBytes);
			break;

#endif /*((XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	  (XPAR_XISF_FLASH_FAMILY == STM) ||
	  (XPAR_XISF_FLASH_FAMILY == SPANSION))*/

#if (XPAR_XISF_FLASH_FAMILY == STM)
		case XISF_DUAL_IP_PAGE_WRITE:
			WriteParamPtr = (XIsf_WriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(WriteParamPtr != NULL);
			Status = WriteData(InstancePtr,
					XISF_CMD_DUAL_IP_PAGE_WRITE,
					WriteParamPtr->Address,
					WriteParamPtr->WritePtr,
					WriteParamPtr->NumBytes);
			break;

		case XISF_DUAL_IP_EXT_PAGE_WRITE:
			WriteParamPtr = (XIsf_WriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(WriteParamPtr != NULL);
			Status = WriteData(InstancePtr,
					XISF_CMD_DUAL_IP_EXT_PAGE_WRITE,
					WriteParamPtr->Address,
					WriteParamPtr->WritePtr,
					WriteParamPtr->NumBytes);
			break;

		case XISF_QUAD_IP_EXT_PAGE_WRITE:
			WriteParamPtr = (XIsf_WriteParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(WriteParamPtr != NULL);
			Status = WriteData(InstancePtr,
					XISF_CMD_QUAD_IP_EXT_PAGE_WRITE,
					WriteParamPtr->Address,
					WriteParamPtr->WritePtr,
					WriteParamPtr->NumBytes);
			break;
#endif /* (XPAR_XISF_FLASH_FAMILY == STM) */

		default:
			break;
	}

	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
		InstancePtr->StatusHandler(InstancePtr,
			XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes the data to the specified address locations in Serial
* Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the address in the Serial Flash memory, where the
*		data is to be written.
* @param	BufferPtr is a pointer to the data to be written to Serial
*		Flash.
* @param	ByteCount is the number of bytes to be written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- A minimum of one byte and a maximum of one Page can be
*		written using this function.
*		- This operation is supported for Atmel, Intel, STM, Winbond
*		and Spansion Serial Flash.
*
******************************************************************************/
static int WriteData(XIsf *InstancePtr, u8 Command, u32 Address,
			const u8 *BufferPtr, u32 ByteCount)
{
	u8 Mode;
	u32 Index;
	u32 BankSel;
	u32 RealAddr;
	int Status;
	u8 FlagStatus[2] = {0};
	u8 FlashStatus[2] = {0};
	u8 * NULLPtr = NULL;
	const u8 * LocalBufPtr = BufferPtr;
#ifdef XPAR_XISF_INTERFACE_PSQSPI
	u32 FlashMake = InstancePtr->ManufacturerID;
#endif
	u8 ReadStatusCmdBuf[] = { READ_STATUS_CMD, 0 };
	u8 ReadFlagSRCmd[] = {READ_FLAG_STATUS_CMD, 0};

	if ((ByteCount <= 0) || (ByteCount > InstancePtr->BytesPerPage)) {
		return (int)XST_FAILURE;
	}

	if (LocalBufPtr == NULL) {
		return (int)XST_FAILURE;
	}

	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(InstancePtr->SpiInstPtr, Address);

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	/*
	 * 0x18 is the DeviceIDMemSize for different make of
	 * flashes of size 16MB
	 */
	if(InstancePtr->DeviceIDMemSize > 0x18) {

		/*
		 * Get the Transfer Mode
		 */
		Mode = XIsf_GetTransferMode(InstancePtr);

		/*
		 * Seting the transfer mode to Polled Mode before
		 * performing the Bank Select operation.
		 */
		XIsf_SetTransferMode(InstancePtr, XISF_POLLING_MODE);

		/*
		 * Calculate bank
		 */
		BankSel = RealAddr/SIXTEENMB;
		/*
		 * Select bank
		 */
		(void)SendBankSelect(InstancePtr, BankSel);

		/*
		 * Restoring the transfer mode back
		 */
		XIsf_SetTransferMode(InstancePtr, Mode);
	}
#endif
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	if (InstancePtr->FourByteAddrMode == TRUE) {
		InstancePtr->WriteBufPtr[BYTE1] = Command;
		InstancePtr->WriteBufPtr[BYTE2] = (u8) (RealAddr >> XISF_ADDR_SHIFT24);
		InstancePtr->WriteBufPtr[BYTE3] = (u8) (RealAddr >> XISF_ADDR_SHIFT16);
		InstancePtr->WriteBufPtr[BYTE4] = (u8) (RealAddr >> XISF_ADDR_SHIFT8);
		InstancePtr->WriteBufPtr[BYTE5] = (u8) (RealAddr);
		for(Index = 5U; Index < (ByteCount + XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE);
													Index++) {
			InstancePtr->WriteBufPtr[Index] = *LocalBufPtr;
			LocalBufPtr += 1;
		}
	} else {
#endif
		InstancePtr->WriteBufPtr[BYTE1] = Command;
		InstancePtr->WriteBufPtr[BYTE2] = (u8) (RealAddr >> XISF_ADDR_SHIFT16);
		InstancePtr->WriteBufPtr[BYTE3] = (u8) (RealAddr >> XISF_ADDR_SHIFT8);
		InstancePtr->WriteBufPtr[BYTE4] = (u8) (RealAddr);
		for(Index = 4U; Index < (ByteCount + XISF_CMD_SEND_EXTRA_BYTES);
										Index++) {
			InstancePtr->WriteBufPtr[Index] = *LocalBufPtr;
			LocalBufPtr += 1;
		}
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	}
#endif

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	/*
	 * Enable write before transfer
	 */
	Status = XIsf_WriteEnable(InstancePtr, XISF_WRITE_ENABLE);
#endif
	if (InstancePtr->FourByteAddrMode == TRUE) {
		Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULLPtr,
							(ByteCount + XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE));
	} else {
		Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULLPtr,
					(ByteCount + XISF_CMD_SEND_EXTRA_BYTES));
	}
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	if((InstancePtr->NumDie > 1) &&
			(FlashMake == XISF_MANUFACTURER_ID_MICRON)) {
		Status = XIsf_Transfer(InstancePtr, ReadFlagSRCmd, FlagStatus,
					(u32)sizeof(ReadFlagSRCmd));
		if(Status != (int)XST_SUCCESS){
			return (int)XST_FAILURE;
		}
	}

	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the Flash to determine when it
		 * completes, by sending a read status command and receiving
		 * status byte
		 */
		Status = XIsf_Transfer(InstancePtr, ReadStatusCmdBuf,
				FlashStatus, (u32)sizeof(ReadStatusCmdBuf));
		if(Status != (int)XST_SUCCESS){
			return (int)XST_FAILURE;
		}

		/*
		 * If status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}

	if((InstancePtr->NumDie > 1) &&
			(FlashMake == XISF_MANUFACTURER_ID_MICRON)) {
		Status = XIsf_Transfer(InstancePtr, ReadFlagSRCmd, FlagStatus,
					(u32)sizeof(ReadFlagSRCmd));
		if(Status != (int)XST_SUCCESS){
			return (int)XST_FAILURE;
		}
	}
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* This function Auto rewrites the contents of a Page in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the address of the page to be refreshed.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int AutoPageWrite(XIsf *InstancePtr, u32 Address)
{
	int Status = (int)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Address != 0);


#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_AUTOPAGE_WRITE;
	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (XISF_DUMMYBYTE);

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes data to the specified SRAM buffer of the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferNum specifies the internal SRAM Buffer of the Serial
*		Flash. The valid values are XISF_PAGE_BUFFER1 or
*		XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2 is not valid in case of
*		Atmel AT45DB011D Serial Flash as it contains a single buffer.
* @param	WritePtr is the pointer to the data to be written to the
*		Serial Flash SRAM Buffer.
* @param	ByteOffset is the byte offset in the buffer from where the
*		data is to be written.
* @param	NumBytes is the number of bytes to be written to the Buffer.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- A minimum of one byte and a maximum of one SRAM buffer can be
*		written using this function.
*		- This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int BufferWrite(XIsf *InstancePtr, u8 BufferNum, const u8 *WritePtr,
			u32 ByteOffset, u32 NumBytes)
{
	int Status = (int)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferNum != 0);
	Xil_AssertNonvoid(WritePtr != NULL);
	Xil_AssertNonvoid(ByteOffset != 0);
	Xil_AssertNonvoid(NumBytes != 0);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	u16 Index;

	/*
	 * Check buffer number is valid or not.
	 */
	if ((BufferNum == XISF_PAGE_BUFFER1) ||
		(BufferNum == XISF_PAGE_BUFFER2)) {
		if ((InstancePtr->DeviceCode  == XISF_ATMEL_DEV_AT45DB011D) &&
			(BufferNum != XISF_PAGE_BUFFER1)) {
			return (int)XST_FAILURE;
		}
	}
	else{
		return (int)XST_FAILURE;
	}

	if (WritePtr == NULL) {
			return (int)XST_FAILURE;
	}

	if (ByteOffset > InstancePtr->BytesPerPage) {
		return (int)XST_FAILURE;
	}

	if ((NumBytes <= 0) || (NumBytes > InstancePtr->BytesPerPage)) {
		return (int)XST_FAILURE;
	}


	if (BufferNum == XISF_PAGE_BUFFER1) {
		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BUFFER1_WRITE;
	}
	else{
		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BUFFER2_WRITE;
	}
	InstancePtr->WriteBufPtr[BYTE2] = (u8) (0x00);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (ByteOffset >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) ByteOffset;


	for(Index = 4; Index < NumBytes + XISF_CMD_SEND_EXTRA_BYTES; Index++) {
		InstancePtr->WriteBufPtr[Index] = *WritePtr++;
	}

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				(NumBytes + XISF_CMD_SEND_EXTRA_BYTES));
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function first erases a page and then writes data from the specified
* internal SRAM buffer to the specified locations in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferNum specifies the internal SRAM Buffer, from which the
*		data needs to be written to the Serial Flash. The valid values
*		are XISF_PAGE_BUFFER1 or XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2
*		is not valid in the case of Atmel AT45DB011D Serial Flash as it
*		contains a single buffer.
* @param	Address is the starting address in the Serial Flash where
*		the data has to be written. Byte address in this address is
*		ignored	as an entire Page is transferred using this API.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- A minimum of one Page and a maximum of one Page can be
*		written using this function.
*		- This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int BufferToFlashWriteWithErase(XIsf *InstancePtr, u8 BufferNum,
					u32 Address)
{
	int Status = (int)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferNum != 0);
	Xil_AssertNonvoid(Address != 0);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Check whether the buffer number is valid or not.
	 */
	if ((BufferNum == XISF_PAGE_BUFFER1) ||
		(BufferNum == XISF_PAGE_BUFFER2)) {
		if ((InstancePtr->DeviceCode  == XISF_ATMEL_DEV_AT45DB011D) &&
			(BufferNum != XISF_PAGE_BUFFER1)) {
			return (int)XST_FAILURE;
		}
	}
	else {
		return (int)XST_FAILURE;
	}

	if (BufferNum == XISF_PAGE_BUFFER1) {
		/*
		 * Buffer 1 to Page Program With Erase.
		 */
		InstancePtr->WriteBufPtr[BYTE1] =
				XISF_CMD_ERASE_BUF1TOPAGE_WRITE;
	} else {
		/*
		 * Buffer 2 to Page Program With Erase.
		 */
		InstancePtr->WriteBufPtr[BYTE1] =
			XISF_CMD_ERASE_BUF2TOPAGE_WRITE;
	}
	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) (XISF_DUMMYBYTE);

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes data from the specified internal SRAM buffer to the
* specified locations in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferNum specifies the internal SRAM Buffer, from which the
*		data needs to be written to the Serial Flash. The valid values
*		are XISF_PAGE_BUFFER1 or XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2
*		is not valid in the case of Atmel AT45DB011D Serial Flash as it
*		contains a single buffer.
* @param	Address is the starting address in the Serial Flash where
*		data has to be written. Byte address in this address will be
*		ignored as an entire page of data is transferred using this
*		operation.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
*		- A minimum of one Page and a maximum of one Page can be
*		written using this function.
*		- This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int BufferToFlashWriteWithoutErase(XIsf *InstancePtr, u8 BufferNum,
					u32 Address)
{
	int Status = (int)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferNum != 0);
	Xil_AssertNonvoid(Address != 0);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Check whether the buffer number is valid or not.
	 */
	if ((BufferNum == XISF_PAGE_BUFFER1) ||
		(BufferNum == XISF_PAGE_BUFFER2)) {
		if ((InstancePtr->DeviceCode == XISF_ATMEL_DEV_AT45DB011D) &&
			(XISF_PAGE_BUFFER1 != 1)) {
			return (int)XST_FAILURE;
		}
	}
	else {
		return (int)XST_FAILURE;
	}

	if (BufferNum == XISF_PAGE_BUFFER1) {
		/*
		 * Buffer 1 to Page Program Without Erase.
		 */
		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BUF1TOPAGE_WRITE;
	}
	else {
		/*
		 * Buffer 2 to Page Program Without Erase.
		 */
		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BUF2TOPAGE_WRITE;
	}

	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) (XISF_DUMMYBYTE);

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes data to the Status Register of the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	SRData is the value to be written to the Status Register
*		of the Serial Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This operation is only supported in Intel, STM, Winbond and
*		Spansion Serial Flash. This is the write of Status Register 1
*		for Winbond devices, write of Status Register 2 is handled by
*		WriteSR2.
*
******************************************************************************/
static int WriteSR(XIsf *InstancePtr, u8 SRData)
{
	int Status = XST_FAILURE;

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || \
    (XPAR_XISF_FLASH_FAMILY == SPANSION) || (XPAR_XISF_FLASH_FAMILY == SST))
	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_STATUSREG_WRITE;
	InstancePtr->WriteBufPtr[BYTE2] = SRData;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
						XISF_STATUS_RDWR_BYTES);
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}
#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \\
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes data to Status Register of the Serial Flash. This API
* should be used to write to the 16 bit status register in Winbond Quad Flash
* (W25QXX).
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	SRData is of type (u8*) and points to the 16 bit value to be
*		written to the Status Register of the Serial Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This operation is only supported in Winbond (W25QXX) Serial
*		Flash.
*
******************************************************************************/
static int WriteSR2(XIsf *InstancePtr, u8 *SRData)
{
	int Status = (int)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SRData != NULL);

#if (XPAR_XISF_FLASH_FAMILY == WINBOND)
	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_STATUSREG_WRITE;
	InstancePtr->WriteBufPtr[BYTE2] = *SRData++;
	InstancePtr->WriteBufPtr[BYTE3] = *SRData;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
						XISF_STATUS_RDWR_BYTES + 1);
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == WINBOND) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes one byte of data to the OTP area in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the address in the OTP area, where to write data.
* @param	BufferPtr is the pointer to the data to be written into OTP
*		region of Serial Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- A minimum of one byte and a maximum of one byte can be
*		written using this function.
*		- This operation is supported only for Intel Serial Flash.
*
******************************************************************************/
static int WriteOTPData(XIsf *InstancePtr, u32 Address, const u8 *BufferPtr)
{
	int Status = (int)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Address != 0);
	Xil_AssertNonvoid(BufferPtr != NULL);


#if (XPAR_XISF_FLASH_FAMILY == INTEL)
	if (BufferPtr == NULL) {
		return (int)XST_FAILURE;
	}

	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_OTP_WRITE;
	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) Address;
	InstancePtr->WriteBufPtr[BYTE5] = *BufferPtr;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
					XISF_OTP_RDWR_EXTRA_BYTES);
	if (Status != (int)XST_SUCCESS) {
		return (int)XST_FAILURE;
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == INTEL) */

	return Status;
}
