/******************************************************************************
*
* Copyright (C) 2012 - 2017 Xilinx, Inc.  All rights reserved.
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
* @file xilisf_read.c
*
* This file contains the library functions to read data from the Serial Flash
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
* 5.0   sb	 08/05/14 Updated support for > 128 MB flash for PSQSPI
*			  interface.
*			  Changed API:
*				ReadData()
*				FastReadData()
*
* 5.2  asa       05/12/15 Added support for Micron (N25Q256A) flash part
* 						  which supports 4 byte addressing.
* 5.3  sk        06/01/15 Used Half of Actual byte count for calculating
*                         Real Byte count in parallel mode. CR# 859979.
* 5.3  sk   08/07/17 Added QSPIPSU flash interface support for ZynqMP.
* 5.5  sk   01/14/16 Used 4byte Fast read command in 4 byte addressing mode.
* 5.8  nsk  03/02/17 Update WriteBuffer index to 10 in FastReadData, CR#968476
* 5.9  nsk  97/11/17 Add Micron 4Byte addressing support in Xisf_Read, CR#980169
*      ms   08/03/17 Added tags and modified comment lines style for doxygen.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilisf.h"

/************************** Constant Definitions *****************************/

#define FAST_READ_NUM_DUMMY_BYTES	1
#define SIXTEENMB	0x1000000	/**< Sixteen MB */
#define BANKMASK 	0xF000000	/**< Bank mask */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

extern int XIsf_Transfer(XIsf *InstancePtr, u8 *WritePtr, u8* ReadPtr,
			 u32 ByteCount);
extern u32 GetRealAddr(XIsf_Iface *QspiPtr, u32 Address);

#ifdef XPAR_XISF_INTERFACE_PSQSPI
extern int SendBankSelect(XIsf *InstancePtr, u32 BankSel);
#endif

static int ReadData(XIsf *InstancePtr, u32 Address, u8 *ReadPtr,
		u32 ByteCount);
static int FastReadData(XIsf *InstancePtr, u8 Command, u32 Address,
		u8 *ReadPtr,
			u32 ByteCount, int NumDummyBytes);
static int FlashToBufTransfer(XIsf *InstancePtr, u8 BufferNum, u32 Address);
static int BufferRead(XIsf *InstancePtr, u8 BufferNum, u8 *ReadPtr,
			u32 ByteOffset, u32 NumBytes);
static int FastBufferRead(XIsf *InstancePtr, u8 BufferNum, u8 *ReadPtr,
			  u32 ByteOffset, u32 NumBytes);
static int ReadOTPData(XIsf *InstancePtr, u32 Address, u8 *ReadPtr,
			u32 ByteCount);

/************************** Variable Definitions *****************************/
extern u32 XIsf_StatusEventInfo;
extern unsigned int XIsf_ByteCountInfo;
/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
* @brief
* This API reads the data from the Serial Flash.
*
* @param	InstancePtr	Pointer to the XIsf instance.
* @param	Operation	Type of the read operation to be performed
*				on the Serial Flash.
*				The different operations are
*				- XISF_READ: Normal Read
*				- XISF_FAST_READ: Fast Read
*				- XISF_PAGE_TO_BUF_TRANS: Page to Buffer
*				  Transfer
*				- XISF_BUFFER_READ: Buffer Read
*				- XISF_FAST_BUFFER_READ: Fast Buffer Read
*				- XISF_OTP_READ: One Time Programmable Area
*				  (OTP) Read
*				- XISF_DUAL_OP_FAST_READ: Dual Output Fast
*				  Read
*				- XISF_DUAL_IO_FAST_READ: Dual Input/Output
*				  Fast Read
*				- XISF_QUAD_OP_FAST_READ: Quad Output Fast
*				  Read
*				- XISF_QUAD_IO_FAST_READ: Quad Input/Output
*				  Fast Read
* @param	OpParamPtr	Pointer to structure variable which contains
*				operational parameter of specified operation.
*				This parameter type is dependant on the type
*				of Operation to be performed.
*
*				- Normal Read (XISF_READ), Fast Read
*				(XISF_FAST_READ), One Time Programmable Area
*				Read (XISF_OTP_READ), Dual Output Fast Read
*				(XISF_CMD_DUAL_OP_FAST_READ), Dual Input/
*				Output Fast Read (XISF_CMD_DUAL_IO_FAST_READ),
*				Quad Output Fast Read
*				(XISF_CMD_QUAD_OP_FAST_READ) and Quad Input/
*				Output Fast Read (XISF_CMD_QUAD_IO_FAST_READ):
*				The OpParamPtr must be of type struct
*				XIsf_ReadParam.
*				OpParamPtr->Address is start address in the
*				Serial Flash.
*				OpParamPtr->ReadPtr is a pointer to the
*				memory where the data read from the Serial
*				Flash is stored.
*				OpParamPtr->NumBytes is number of bytes to
*				read.
*				OpParamPtr->NumDummyBytes is the number of
*				dummy bytes to be transmitted for the Read
*				command. This parameter is only used in case
*				of Dual and Quad reads.
*				Normal Read and Fast Read operations are
*				supported for Atmel, Intel, STM, Winbond and
*				Spansion Serial Flash.
*				Dual and quad reads are supported for Winbond
*				(W25QXX), Numonyx(N25QXX) and Spansion
*				(S25FL129) quad flash.
*				OTP Read operation is only supported in Intel
*				Serial Flash.
*
*				- Page To Buffer Transfer
*				(XISF_PAGE_TO_BUF_TRANS):
*				The OpParamPtr must be of type struct
*				XIsf_FlashToBufTransferParam .
*				OpParamPtr->BufferNum specifies the internal
*				SRAM Buffer of the Serial Flash. The valid
*				values are XISF_PAGE_BUFFER1 or XISF_PAGE_BUFFER2
*				XISF_PAGE_BUFFER2 is not valid in case of
*				AT45DB011D Flash as it contains a single buffer.
*				OpParamPtr->Address is start address in the
*				Serial Flash.
*				This operation is only supported in Atmel
*				Serial Flash.
*
*				- Buffer Read (XISF_BUFFER_READ) and Fast
*				Buffer Read(XISF_FAST_BUFFER_READ):
*				The OpParamPtr must be of type struct
*				XIsf_BufferReadParam.
*				OpParamPtr->BufferNum specifies the internal
*				SRAM Buffer of the Serial Flash. The valid
*				values are XISF_PAGE_BUFFER1 or XISF_PAGE_BUFFER2
*				XISF_PAGE_BUFFER2 is not valid in case of
*				AT45DB011D Flash as it contains a single buffer.
*				OpParamPtr->ReadPtr is pointer to the memory
*				where data read from the SRAM buffer is to be
*				stored.
*				OpParamPtr->ByteOffset is byte offset in the
*				SRAM buffer from where the first byte is read.
*				OpParamPtr->NumBytes is the number of bytes to
*				be read from the Buffer.
*				These operations are supported only in Atmel
*				Serial Flash.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if it fails.
*
* @note
*		- Application must fill the structure elements of the third
*		argument and pass its pointer by type casting it with void
*		pointer.
*		- The valid data is available from the fourth location pointed
*		to by the ReadPtr for Normal Read and Buffer Read operations.
*		- The valid data is available from fifth location pointed to
*		by the ReadPtr for Fast Read, Fast Buffer Read and OTP Read
*		operations.
*		- The valid data is available from the (4 + NumDummyBytes)th
*		location pointed to by ReadPtr for Dual/Quad Read operations.
*
******************************************************************************/
int XIsf_Read(XIsf *InstancePtr, XIsf_ReadOperation Operation,
		void *OpParamPtr)
{
	int Status = (int)(XST_FAILURE);
	u8 Mode;
	XIsf_ReadParam *ReadParamPtr;
	XIsf_FlashToBufTransferParam *FlashToBufTransferParamPtr;
	XIsf_BufferReadParam *BufferReadParamPtr;

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
	}

	if (NULL == OpParamPtr) {
		return (int)(XST_FAILURE);
	}

	switch (Operation) {
		case XISF_READ:
			ReadParamPtr = (XIsf_ReadParam*) OpParamPtr;
			Status = ReadData(InstancePtr,
					ReadParamPtr->Address,
					ReadParamPtr->ReadPtr,
					ReadParamPtr->NumBytes);
			break;

		case XISF_FAST_READ:
			ReadParamPtr = (XIsf_ReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(ReadParamPtr != NULL);
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
			if (InstancePtr->FourByteAddrMode == TRUE) {
				Status = FastReadData(InstancePtr,
							XISF_CMD_FAST_READ_4BYTE,
							ReadParamPtr->Address,
							ReadParamPtr->ReadPtr,
							ReadParamPtr->NumBytes,
							ReadParamPtr->NumDummyBytes);
			} else {
#endif
				Status = FastReadData(InstancePtr,
						XISF_CMD_FAST_READ,
						ReadParamPtr->Address,
						ReadParamPtr->ReadPtr,
						ReadParamPtr->NumBytes,
						ReadParamPtr->NumDummyBytes);
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
			}
#endif
			break;

		case XISF_PAGE_TO_BUF_TRANS:
			FlashToBufTransferParamPtr =
			(XIsf_FlashToBufTransferParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(FlashToBufTransferParamPtr != NULL);
			Status = FlashToBufTransfer(InstancePtr,
					FlashToBufTransferParamPtr->BufferNum,
					FlashToBufTransferParamPtr->Address);
			break;

		case XISF_BUFFER_READ:
			BufferReadParamPtr =
			(XIsf_BufferReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(BufferReadParamPtr != NULL);
			Status = BufferRead(InstancePtr,
					BufferReadParamPtr->BufferNum,
					BufferReadParamPtr->ReadPtr,
					BufferReadParamPtr->ByteOffset,
					BufferReadParamPtr->NumBytes);
			break;

		case XISF_FAST_BUFFER_READ:
			BufferReadParamPtr =
			(XIsf_BufferReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(BufferReadParamPtr != NULL);
			Status = FastBufferRead(InstancePtr,
					BufferReadParamPtr->BufferNum,
					BufferReadParamPtr->ReadPtr,
					BufferReadParamPtr->ByteOffset,
					BufferReadParamPtr->NumBytes);
			break;

		case XISF_OTP_READ:
			ReadParamPtr = (XIsf_ReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(ReadParamPtr != NULL);
			Status = ReadOTPData(InstancePtr,
					ReadParamPtr->Address,
					ReadParamPtr->ReadPtr,
					ReadParamPtr->NumBytes);
			break;

#if ((XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == STM) \
     || (XPAR_XISF_FLASH_FAMILY == SPANSION))
		case XISF_DUAL_OP_FAST_READ:
			ReadParamPtr = (XIsf_ReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(ReadParamPtr != NULL);
				Status = FastReadData(InstancePtr,
					XISF_CMD_DUAL_OP_FAST_READ,
					ReadParamPtr->Address,
					ReadParamPtr->ReadPtr,
					ReadParamPtr->NumBytes,
					ReadParamPtr->NumDummyBytes);
			break;

		case XISF_DUAL_IO_FAST_READ:
			ReadParamPtr = (XIsf_ReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(ReadParamPtr != NULL);
				Status = FastReadData(InstancePtr,
					XISF_CMD_DUAL_IO_FAST_READ,
					ReadParamPtr->Address,
					ReadParamPtr->ReadPtr,
					ReadParamPtr->NumBytes,
					ReadParamPtr->NumDummyBytes);
			break;

		case XISF_QUAD_OP_FAST_READ:
			ReadParamPtr = (XIsf_ReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(ReadParamPtr != NULL);
#if (!defined(XPAR_XISF_INTERFACE_PSQSPI))
			if (InstancePtr->FourByteAddrMode == TRUE) {
				Status = FastReadData(InstancePtr,
							XISF_CMD_FAST_READ_4BYTE,
							ReadParamPtr->Address,
							ReadParamPtr->ReadPtr,
							ReadParamPtr->NumBytes,
							ReadParamPtr->NumDummyBytes);
			} else {
#endif
				Status = FastReadData(InstancePtr,
					XISF_CMD_QUAD_OP_FAST_READ,
					ReadParamPtr->Address,
					ReadParamPtr->ReadPtr,
					ReadParamPtr->NumBytes,
					ReadParamPtr->NumDummyBytes);
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
			}
#endif
			break;

		case XISF_QUAD_IO_FAST_READ:
			ReadParamPtr = (XIsf_ReadParam*)(void *) OpParamPtr;
			Xil_AssertNonvoid(ReadParamPtr != NULL);
				Status = FastReadData(InstancePtr,
					XISF_CMD_QUAD_IO_FAST_READ,
					ReadParamPtr->Address,
					ReadParamPtr->ReadPtr,
					ReadParamPtr->NumBytes,
					ReadParamPtr->NumDummyBytes);
			break;
#endif /*((XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	  (XPAR_XISF_FLASH_FAMILY == STM) ||
	  (XPAR_XISF_FLASH_FAMILY == SPANSION))*/

		default:
			break;
	}

	/*
	 * Get the Transfer Mode
	 */
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
* This function reads data from the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the starting address in the Serial Flash from where
*		the data is to be read.
* @param	ReadPtr is a pointer to the memory where the data read from
*		the Serial Flash is stored.
* @param	ByteCount is the number of bytes to be read from the Serial
*		Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- This operation is supported for Atmel, Intel, STM, Winbond
*		  and Spansion Serial Flash.
*		- Minimum of one byte and a maximum of an entire Serial Flash
*		Array can be read.
*		- The valid data is available from the fourth location pointed
*		to by the ReadPtr.
*
******************************************************************************/
static int ReadData(XIsf *InstancePtr, u32 Address, u8 *ReadPtr, u32 ByteCount)
{
	u8 Mode;
	u32 BankSel;
	u32 RealAddr;
	int Status;
	u32 RealByteCnt;
	u32 BufferIndex;
	u8 ShiftSize;
	u32 TotalByteCnt = ByteCount;
	u32 LocalByteCnt = ByteCount;
	u32 LocalAddress = Address;
	u32 TempByteCnt = LocalByteCnt;
	u8 WriteBuffer[10] = {0};
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	u32 DieSize, BusWidth, DieTempData;
	u8 DiscardByteCnt;
	int DieNo;
	u8 * NULLPtr = NULL;
	XQspiPsu_Msg FlashMsg[3];
#endif

	if (LocalByteCnt <= 0 ) {
		return (int)XST_FAILURE;
	}

	if (ReadPtr == NULL) {
		return (int)XST_FAILURE;
	}

	while(((s32)(LocalByteCnt)) > 0) {

	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(InstancePtr->SpiInstPtr, LocalAddress);

#ifdef XPAR_XISF_INTERFACE_QSPIPSU

	DieNo = RealAddr/DieSize;

	DieTempData = (DieSize * (DieNo+1)) - (RealAddr);

	if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
					XISF_QSPIPS_CONNECTION_MODE_PARALLEL) {
		DieTempData = DieTempData*2;
	}

	/* For Dual Stacked, split and read for boundary crossing */
	if(LocalByteCnt > DieTempData) {
		RealByteCnt = DieTempData;
	} else {
#endif

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	if(InstancePtr->DeviceIDMemSize > 0x18U) {

		/*
		 * Get the Transfer Mode
		 */
		Mode = XIsf_GetTransferMode(InstancePtr);

		/*
		 * Seting the transfer mode to Polled Mode before
		 * performing the Bank Select operation.
		 */
		XIsf_SetTransferMode(InstancePtr, XISF_POLLING_MODE);

		BankSel = RealAddr/SIXTEENMB;

		(void)SendBankSelect(InstancePtr, BankSel);

		/*
		 * Restoring the transfer mode back
		 */
		XIsf_SetTransferMode(InstancePtr, Mode);
	}

	if(InstancePtr->SpiInstPtr->Config.ConnectionMode ==
			XISF_QSPIPS_CONNECTION_MODE_PARALLEL) {
		TempByteCnt = LocalByteCnt/2;
	}

	/*
	 * If data to be read spans beyond the current bank, then
	 * calculate RealByteCnt in current bank. Else
	 * RealByteCnt is the same as ByteCount
	 */
	if((RealAddr & BANKMASK) != ((RealAddr+TempByteCnt) & BANKMASK)) {
		RealByteCnt = ((RealAddr & BANKMASK) + SIXTEENMB) - RealAddr;
	}
	else {
#endif
		RealByteCnt = LocalByteCnt;

#if defined (XPAR_XISF_INTERFACE_PSQSPI) || \
	defined (XPAR_XISF_INTERFACE_QSPIPSU)
	}
#endif
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	if (InstancePtr->FourByteAddrMode == TRUE) {
		WriteBuffer[BYTE1] = XISF_CMD_RANDOM_READ_4BYTE;
		WriteBuffer[BYTE2] = (u8) (RealAddr >> XISF_ADDR_SHIFT24);
		WriteBuffer[BYTE3] = (u8) (RealAddr >> XISF_ADDR_SHIFT16);
		WriteBuffer[BYTE4] = (u8) (RealAddr >> XISF_ADDR_SHIFT8);
		WriteBuffer[BYTE5] = (u8) (RealAddr);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		DiscardByteCnt = 5;
#endif
	} else {
#endif
		WriteBuffer[BYTE1] = XISF_CMD_RANDOM_READ;
		WriteBuffer[BYTE2] = (u8)((RealAddr & 0xFF0000) >> XISF_ADDR_SHIFT16);
		WriteBuffer[BYTE3] = (u8)((RealAddr & 0xFF00) >> XISF_ADDR_SHIFT8);
		WriteBuffer[BYTE4] = (u8)(RealAddr & 0xFF);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		DiscardByteCnt = 4;
#endif
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	}
#endif

#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	FlashMsg[0].TxBfrPtr = WriteBuffer;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = DiscardByteCnt;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = ReadPtr;
	FlashMsg[1].ByteCount = RealByteCnt;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

	if(InstancePtr->SpiInstPtr->Config.ConnectionMode ==
			XISF_QSPIPS_CONNECTION_MODE_PARALLEL){
		FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}
	InstancePtr->SpiInstPtr->Msg = FlashMsg;
	Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 2);
#else
	if (InstancePtr->FourByteAddrMode == TRUE) {
		Status = XIsf_Transfer(InstancePtr, WriteBuffer,
			&(ReadPtr[TotalByteCnt - LocalByteCnt]),
			RealByteCnt + XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE);
	} else {
		Status = XIsf_Transfer(InstancePtr, WriteBuffer,
				&(ReadPtr[TotalByteCnt - LocalByteCnt]),
				RealByteCnt + XISF_CMD_SEND_EXTRA_BYTES);
	}
#endif
	if (Status != (int)(XST_SUCCESS)) {
		return (int)XST_FAILURE;
	}

#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	LocalAddress += RealByteCnt;
	ReadPtr += RealByteCnt;
#endif

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	/*
	 * To discard the first 4 dummy bytes, shift the data in read buffer
	 */
	ShiftSize =  XISF_CMD_SEND_EXTRA_BYTES;
	BufferIndex = (TotalByteCnt - LocalByteCnt);
	for(;
		BufferIndex < ((TotalByteCnt - LocalByteCnt) + RealByteCnt);
		BufferIndex++) {
		ReadPtr[BufferIndex] = ReadPtr[BufferIndex + ShiftSize];
	}

	/*
	 * Increase address to next bank
	 */
	LocalAddress = (LocalAddress & BANKMASK) + SIXTEENMB;
#endif
	/*
	 * Decrease byte count by bytes already read.
	 */
	LocalByteCnt = LocalByteCnt - RealByteCnt;

	}

	return (int)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads data from the Serial Flash at a higher speed than normal
* Read operation.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Command is the fast read command used to read data from the
*		flash. It could be one of XISF_CMD_DUAL/QUAD_*_FAST_READ or
*		XISF_CMD_FAST_READ.
* @param	Address is the starting address in the Serial Flash from where
*		the data is to be read.
* @param	ReadPtr is a pointer to the memory where the data read from
*		the Serial Flash is stored.
* @param	ByteCount is the number of bytes to be read from the Serial
*		Flash.
* @param	NumDummyBytes is the number of dummy bytes associated with the
*		fast read commands.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- XISF_CMD_FAST_READ operation is supported for Atmel, Intel,
*		  STM, Winbond and Spansion Serial Flash.
*		- XISF_CMD_DUAL/QUAD_*_FAST_READ operations are supported on
*		  Winbond (W25QXX), Numonyx (N25QXX) and Spansion (S25FL129)
*		  quad flash devices.
*		- Minimum of one byte and a maximum of an entire Serial Flash
*		  Array can be read.
*		- The valid data is available from the (4 + NumDummyBytes)th
*		  location pointed to by the ReadPtr.
*
******************************************************************************/
static int FastReadData(XIsf *InstancePtr, u8 Command, u32 Address,
			u8 *ReadPtr, u32 ByteCount, int NumDummyBytes)
{
	u8 Mode;
	int Index;
	int Status;
	u32 BankSel;
	u32 RealAddr;
	u32 RealByteCnt;
	u32 BufferIndex;
	u8 ShiftSize;
	u32 TotalByteCnt = ByteCount;
	u32 LocalByteCnt = ByteCount;
	u32 LocalAddress = Address;
	u32 TempByteCnt = LocalByteCnt;
	u8 WriteBuffer[10]= {0};
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	u32 DieSize, BusWidth, DieTempData;
	u32 FlashMsgCnt;
	u8 DiscardByteCnt;
	int DieNo;
	u8 * NULLPtr = NULL;
	XQspiPsu_Msg FlashMsg[3];
#endif

	if (LocalByteCnt <= 0 ) {
		return (int)XST_FAILURE;
	}

	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (NumDummyBytes <= 0) {
		return (int)(XST_FAILURE);
	}

	while(((s32)(LocalByteCnt)) > 0) {

	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(InstancePtr->SpiInstPtr, LocalAddress);

#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	DieNo = InstancePtr->NumDie;

	switch(InstancePtr->DeviceIDMemSize){
		case XISF_MICRON_ID_BYTE2_128:
		default:
			DieSize = FLASH_SIZE_128;
			break;
		case XISF_MICRON_ID_BYTE2_256:
			DieSize = FLASH_SIZE_256;
			break;
		case XISF_MICRON_ID_BYTE2_512:
			DieSize = FLASH_SIZE_512;
			break;
		case XISF_MICRON_ID_BYTE2_1G:
			DieSize = FLASH_SIZE_1G;
			break;
	}

	DieTempData = (DieSize * (DieNo+1)) - (RealAddr);

	if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
					XISF_QSPIPS_CONNECTION_MODE_PARALLEL) {
		DieTempData = DieTempData*2;
	}

	/* For Dual Stacked, split and read for boundary crossing */
	if(LocalByteCnt > DieTempData) {
		RealByteCnt = DieTempData;
	} else {
#endif

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	if(InstancePtr->DeviceIDMemSize > 0x18U) {
		/*
		 * Get the Transfer Mode
		 */
		Mode = XIsf_GetTransferMode(InstancePtr);

		/*
		 * Seting the transfer mode to Polled Mode before
		 * performing the Bank Select operation.
		 */
		XIsf_SetTransferMode(InstancePtr, XISF_POLLING_MODE);

		BankSel = RealAddr/SIXTEENMB;

		(void)SendBankSelect(InstancePtr, BankSel);

		/*
		 * Restoring the transfer mode back
		 */
		XIsf_SetTransferMode(InstancePtr, Mode);
	}

	if(InstancePtr->SpiInstPtr->Config.ConnectionMode ==
			XISF_QSPIPS_CONNECTION_MODE_PARALLEL) {
		TempByteCnt = LocalByteCnt/2;
	}
	/*
	 * If data to be read spans beyond the current bank, then
	 * calculate RealByteCnt in current bank. Else
	 * RealByteCnt is the same as ByteCount
	 */
	if((RealAddr & BANKMASK) != ((RealAddr+TempByteCnt) & BANKMASK)) {
		RealByteCnt = ((RealAddr & BANKMASK) + SIXTEENMB) - RealAddr;
	}
	else {
#endif
		RealByteCnt = LocalByteCnt;

#if defined (XPAR_XISF_INTERFACE_PSQSPI) || \
	defined (XPAR_XISF_INTERFACE_QSPIPSU)
	}
#endif
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	if (InstancePtr->FourByteAddrMode == TRUE) {
		WriteBuffer[BYTE1] = Command;
		WriteBuffer[BYTE2] = (u8) (RealAddr >> XISF_ADDR_SHIFT24);
		WriteBuffer[BYTE3] = (u8) (RealAddr >> XISF_ADDR_SHIFT16);
		WriteBuffer[BYTE4] = (u8) (RealAddr >> XISF_ADDR_SHIFT8);
		WriteBuffer[BYTE5] = (u8) RealAddr;
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		DiscardByteCnt = 5;
#else
		for (Index = 0; Index < NumDummyBytes; Index++) {
			WriteBuffer[Index + BYTE5 + 1] = (u8) (XISF_DUMMYBYTE);
		}
#endif
	} else {
#endif
		WriteBuffer[BYTE1] = Command;
		WriteBuffer[BYTE2] = (u8) (RealAddr >> XISF_ADDR_SHIFT16);
		WriteBuffer[BYTE3] = (u8) (RealAddr >> XISF_ADDR_SHIFT8);
		WriteBuffer[BYTE4] = (u8) RealAddr;
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		DiscardByteCnt = 4;
#else
		for (Index = 0; Index < NumDummyBytes; Index++) {
			WriteBuffer[Index + BYTE5] = (u8) (XISF_DUMMYBYTE);
		}
#endif
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	}
#endif

#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	FlashMsg[0].TxBfrPtr = WriteBuffer;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = DiscardByteCnt;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsgCnt = 1;
	BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	/* It is recommended to have a separate entry for dummy */
	if ((Command == XISF_CMD_FAST_READ) ||
			(Command == XISF_CMD_DUAL_OP_FAST_READ) ||
			(Command == XISF_CMD_QUAD_OP_FAST_READ) ||
			(Command == XISF_CMD_FAST_READ_4BYTE) ||
			(Command == XISF_CMD_DUAL_OP_FAST_READ_4B) ||
			(Command == XISF_CMD_QUAD_OP_FAST_READ_4B)) {
		/* Update Dummy cycles as per flash specs for QUAD IO */

		/*
		 * It is recommended that Bus width value during dummy
		 * phase should be same as data phase
		 */
		if ((Command == XISF_CMD_FAST_READ) ||
					(Command == XISF_CMD_FAST_READ_4BYTE)) {
			BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		}

		if ((Command == XISF_CMD_DUAL_OP_FAST_READ) ||
				(Command == XISF_CMD_DUAL_OP_FAST_READ_4B)) {
			BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
		}

		if ((Command == XISF_CMD_QUAD_OP_FAST_READ) ||
				(Command == XISF_CMD_QUAD_OP_FAST_READ_4B)) {
			BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
		}

		FlashMsg[1].BusWidth = BusWidth;
		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = 8;
		FlashMsg[1].Flags = 0;

		FlashMsgCnt++;
	}
	FlashMsg[FlashMsgCnt].BusWidth = BusWidth;
	FlashMsg[FlashMsgCnt].TxBfrPtr = NULL;
	FlashMsg[FlashMsgCnt].RxBfrPtr = ReadPtr;
	FlashMsg[FlashMsgCnt].ByteCount = RealByteCnt;
	FlashMsg[FlashMsgCnt].Flags = XQSPIPSU_MSG_FLAG_RX;

	if(InstancePtr->SpiInstPtr->Config.ConnectionMode ==
			XISF_QSPIPS_CONNECTION_MODE_PARALLEL){
		FlashMsg[FlashMsgCnt].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}
	InstancePtr->SpiInstPtr->Msg = FlashMsg;
	Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, FlashMsgCnt+1);
#else
	RealByteCnt += NumDummyBytes;
	if (InstancePtr->FourByteAddrMode == TRUE) {
		Status = (int)XIsf_Transfer(InstancePtr, WriteBuffer,
			&(ReadPtr[TotalByteCnt - LocalByteCnt]),
			RealByteCnt + XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE);
	} else {
		Status = (int)XIsf_Transfer(InstancePtr, WriteBuffer,
			&(ReadPtr[TotalByteCnt - LocalByteCnt]),
			RealByteCnt + XISF_CMD_SEND_EXTRA_BYTES);
	}
#endif
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	LocalAddress += RealByteCnt;
	LocalByteCnt -= RealByteCnt;
	ReadPtr += RealByteCnt;
#endif

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	/*
	 * To discard the first 5 dummy bytes, shift the data in read buffer
	 */
	ShiftSize =  XISF_CMD_SEND_EXTRA_BYTES + (u8)NumDummyBytes;
	BufferIndex = (TotalByteCnt - LocalByteCnt);
	for(;
		BufferIndex < ((TotalByteCnt - LocalByteCnt) + RealByteCnt);
		BufferIndex++) {
		ReadPtr[BufferIndex] = ReadPtr[BufferIndex + ShiftSize];
	}

	/*
	 * Increase address to next bank
	 */
	LocalAddress = (LocalAddress & BANKMASK) + SIXTEENMB;
#endif

#ifndef XPAR_XISF_INTERFACE_QSPIPSU
	/*
	 * Decrease byte count by bytes already read.
	 */
	LocalByteCnt = LocalByteCnt - (RealByteCnt - NumDummyBytes);
#endif
	}

	return (int)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function copies the data from the Serial Flash to the specified SRAM
* buffer.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferNum specifies the internal SRAM Buffer to which the data
*		from Serial Flash is to be transferred. The valid values are
*		XISF_PAGE_BUFFER1 or XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2 is
*		not valid in the case of AT45DB011D Flash as it contains a
*		single buffer.
* @param	Address specifies any address within the Page of the Serial
*		Flash from where the Page of data is to be copied.
*		Byte address in this Address is ignored as an entire Page of
*		data is transferred/copied.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- This operation is only supported in Atmel Serial Flash.
*		- This function reads one complete Page from the Serial Flash.
*		- Read the Spartan-3AN In-system Flash User Guide/Atmel
*		  AT45XXXD Data sheets for more information.
*
******************************************************************************/
static int FlashToBufTransfer(XIsf *InstancePtr, u8 BufferNum, u32 Address)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Address != 0);
	Xil_AssertNonvoid(BufferNum != 0);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Check whether the buffer number is valid or not.
	 */
	if ((BufferNum == XISF_PAGE_BUFFER1) ||
		(BufferNum == XISF_PAGE_BUFFER2)) {
		if ((InstancePtr->DeviceCode  == XISF_ATMEL_DEV_AT45DB011D) &&
			(BufferNum != XISF_PAGE_BUFFER1)) {
			return (int)(XST_FAILURE);
		}
	} else {
		return (int)(XST_FAILURE);
	}

	if (BufferNum == XISF_PAGE_BUFFER1) {
		/*
		 * Page to Buffer 1 Transfer.
		 */
		InstancePtr->WriteBufPtr[BYTE1] =
			XISF_CMD_PAGETOBUF1_TRANS;
	} else {
		/*
		 * Page to Buffer 2 Transfer.
		 */
		InstancePtr->WriteBufPtr[BYTE1] =
			XISF_CMD_PAGETOBUF2_TRANS;
	}

	InstancePtr->WriteBufPtr[BYTE2] = (u8)(Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8)(Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8)(XISF_DUMMYBYTE);

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;

}

/*****************************************************************************/
/**
*
* This function reads the data available in the SRAM buffer of Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferNum specifies the internal SRAM Buffer from which data
*		is to be read. The valid values are XISF_PAGE_BUFFER1 or
*		XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2 is not valid in case of
*		AT45DB011D Flash as it contains a single buffer.
* @param	ReadPtr is a pointer to the memory where the data read from the
*		SRAM buffer is stored.
* @param	ByteOffset is the byte offset in the buffer from where the
*		first byte is read.
* @param	NumBytes is the number of bytes to be read from the Buffer.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- This operation is only supported in Atmel Serial Flash.
*		- This function reads a Minimum of one Byte and a Maximum of an
*		entire SRAM buffer (1 Page) from the Serial Flash.
*		- The valid data is available from the fourth location pointed
*		to by the ReadPtr.
*		- Read the Spartan-3AN In-system Flash User Guide/Atmel
*		AT45XXXD Data sheets for more information.
*
******************************************************************************/
static int BufferRead(XIsf *InstancePtr, u8 BufferNum, u8 *ReadPtr,
			u32 ByteOffset, u32 NumBytes)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferNum != 0);
	Xil_AssertNonvoid(ReadPtr != NULL);
	Xil_AssertNonvoid(ByteOffset != 0);
	Xil_AssertNonvoid(NumBytes != 0);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)

	/*
	 * Check if the buffer number is valid or not.
	 */
	if ((BufferNum == XISF_PAGE_BUFFER1) ||
			(BufferNum == XISF_PAGE_BUFFER2)) {
		if ((InstancePtr->DeviceCode  == XISF_ATMEL_DEV_AT45DB011D) &&
			(BufferNum != XISF_PAGE_BUFFER1)) {
			return (int)(XST_FAILURE);
		}
	} else {
		return (int)(XST_FAILURE);
	}

	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (ByteOffset > InstancePtr->BytesPerPage){
		return (int)(XST_FAILURE);
	}

	if ((NumBytes <= 0) || (NumBytes > InstancePtr->BytesPerPage)) {
		return (int)(XST_FAILURE);
	}

	if (BufferNum == XISF_PAGE_BUFFER1) {
		ReadPtr[BYTE1] = XISF_CMD_BUF1_READ; /* Buffer 1 Read. */
	} else {
		ReadPtr[BYTE1] = XISF_CMD_BUF2_READ; /* Buffer 2 Read.*/
	}
	ReadPtr[BYTE2] = (u8) (0x00);
	ReadPtr[BYTE3] = (u8) (ByteOffset >> XISF_ADDR_SHIFT8);
	ReadPtr[BYTE4] = (u8) ByteOffset;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, ReadPtr, ReadPtr,
				NumBytes + XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;

}
/*****************************************************************************/
/**
*
* This function reads the data from the internal SRAM page buffer of the Serial
* Flash memory at higher speed than normal Buffer Read operation.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferNum specifies the internal SRAM Buffer from which data
*		is to be read. The valid values are XISF_PAGE_BUFFER1 or
*		XISF_PAGE_BUFFER2. XISF_PAGE_BUFFER2 is not valid in case of
*		AT45DB011D Flash as it contains a single buffer.
* @param	ReadPtr is a pointer to the memory where the data read from the
*		SRAM buffer is stored.
* @param	ByteOffset is the byte offset in the buffer from where the
*		first byte is read.
* @param	NumBytes is the number of bytes to be read from the Buffer.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- This operation is only supported in Atmel Serial Flash.
*		- This function reads a Minimum of one Byte and a Maximum of an
*		entire SRAM buffer (1 Page) from the Serial Flash.
*		- The valid data is available from the fifth location pointed
*		to by the ReadPtr.
*		- Read the Spartan-3AN In-system Flash User Guide/Atmel
*		AT45XXXD Data sheets for more information.
*
******************************************************************************/
static int FastBufferRead(XIsf *InstancePtr, u8 BufferNum, u8 *ReadPtr,
				u32 ByteOffset, u32 NumBytes)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferNum != 0);
	Xil_AssertNonvoid(ReadPtr != NULL);
	Xil_AssertNonvoid(ByteOffset != 0);
	Xil_AssertNonvoid(NumBytes != 0);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Check if the buffer number is valid or not.
	 */
	if ((BufferNum == XISF_PAGE_BUFFER1) ||
			(BufferNum == XISF_PAGE_BUFFER2)) {
		if ((InstancePtr->DeviceCode  == XISF_ATMEL_DEV_AT45DB011D) &&
			(BufferNum != XISF_PAGE_BUFFER1)) {
			return (int)(XST_FAILURE);
		}
	} else {
		return (int)(XST_FAILURE);
	}

	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (ByteOffset > InstancePtr->BytesPerPage){
		return (int)(XST_FAILURE);
	}

	if ((NumBytes <= 0) || (NumBytes > InstancePtr->BytesPerPage)) {
		return (int)(XST_FAILURE);
	}

	if (BufferNum == XISF_PAGE_BUFFER1) {
		/*
		 * Buffer 1 Fast Read.
		 */
		ReadPtr[BYTE1] = XISF_CMD_FAST_BUF1_READ;
	} else {
		/*
		 * Buffer 2 Fast Read.
		 */
		ReadPtr[BYTE1] = XISF_CMD_FAST_BUF2_READ;
	}

	ReadPtr[BYTE2] = (u8) (0x00);
	ReadPtr[BYTE3] = (u8) (ByteOffset >> XISF_ADDR_SHIFT8);
	ReadPtr[BYTE4] = (u8) ByteOffset;
	ReadPtr[BYTE5] = (u8) (XISF_DUMMYBYTE);

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, ReadPtr, ReadPtr,
			NumBytes + XISF_CMD_FAST_READ_EXTRA_BYTES);
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function reads the data from OTP area of the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the starting address in OTP area of the Serial Flash
*		from which the data is to be read.
* @param	ReadPtr is a pointer to the memory where the data read from the
*		Serial Flash is stored.
* @param	ByteCount is the number of bytes to be read from the Serial
*		Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- This operation is only supported for Intel Serial Flash.
*		- Minimum of one byte and a maximum of an entire Serial Flash
*		  array can be read.
******************************************************************************/
static int ReadOTPData(XIsf *InstancePtr, u32 Address, u8 *ReadPtr,
			u32 ByteCount)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Address != 0);
	Xil_AssertNonvoid(ReadPtr != NULL);
	Xil_AssertNonvoid(ByteCount != 0);

#if (XPAR_XISF_FLASH_FAMILY == INTEL)
	if (ByteCount <= 0 ) {
		return (int)(XST_FAILURE);
	}

	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
	}

	ReadPtr[BYTE1] = XISF_CMD_OTP_READ;
	ReadPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	ReadPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	ReadPtr[BYTE4] = (u8) Address;
	ReadPtr[BYTE5] = (u8) (XISF_DUMMYBYTE);


	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, ReadPtr, ReadPtr,
				ByteCount + XISF_OTP_RDWR_EXTRA_BYTES);
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == INTEL) */

	return Status;
}
