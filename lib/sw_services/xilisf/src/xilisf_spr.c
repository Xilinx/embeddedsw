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
* @file xilisf_spr.c
*
* This file contains the library functions to operate on Sector Protection
* feature of the Serial Flash. Refer xilisf.h for a detailed description.
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
* 5.0   sb	 08/05/14 Added Call back to lib interrupt handler
*			  after XIsf_Transfer Calls.
*			  Changed API:
*			  - XIsf_SectorProtect()
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilisf.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

extern int XIsf_Transfer(XIsf *InstancePtr, u8 *WritePtr, u8* ReadPtr,
				u32 ByteCount);
extern int XIsf_GetStatus(XIsf *InstancePtr, u8 *ReadPtr);
static int SprRead(XIsf *InstancePtr, u8 *ReadPtr);
static int SprProgram(XIsf *InstancePtr, u8 *BufferPtr);

static int SprErase(XIsf *InstancePtr);
static int SpEnable(XIsf *InstancePtr);
static int SpDisable(XIsf *InstancePtr);

/************************** Variable Definitions *****************************/
extern u32 XIsf_StatusEventInfo;
extern unsigned int XIsf_ByteCountInfo;
/************************** Function Definitions ******************************/


/*****************************************************************************/
/**
*
* This API is used for performing Sector Protect related operations.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Operation is the type of Sector Protect operation to be
*		performed on the Serial Flash.
*		The different operations are
*		- XISF_SPR_READ: Read Sector Protection Register
*		- XISF_SPR_WRITE: Write Sector Protection Register
*		- XISF_SPR_ERASE: Erase Sector Protection Register
*		- XISF_SP_ENABLE: Enable Sector Protection
*		- XISF_SP_DISABLE: Disable Sector Protection
* @param	BufferPtr is a pointer to the memory where the SPR content is
*		read to/written from. This argument can be NULL if the
*		Operation is SprErase, SpEnable and SpDisable.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- The SPR content is stored at the fourth location pointed
*		by the BufferPtr when performing XISF_SPR_READ operation.
*		- For Intel, STM, Winbond and Spansion Serial Flash, the user
*		application must call the XIsf_WriteEnable() API by passing
*		XISF_WRITE_ENABLE as an argument, before calling the
*		XIsf_SectorProtect() API, for Sector Protect Register Write
*		(XISF_SPR_WRITE) operation.
*		- Atmel Flash supports all these Sector Protect operations.
*		- Intel, STM, Winbond and Spansion Flash support only Sector
*		Protect Read and Sector Protect Write operations.
*
******************************************************************************/
int XIsf_SectorProtect(XIsf *InstancePtr, XIsf_SpOperation Operation,
				u8 *BufferPtr)
{
	int Status = (int)(XST_FAILURE);
	u8 Mode;

	switch (Operation) {
		case XISF_SPR_READ:
			Status = SprRead(InstancePtr, BufferPtr);
			break;

		case XISF_SPR_WRITE:
			Status = SprProgram(InstancePtr, BufferPtr);
			break;

		case XISF_SPR_ERASE:
			Status = SprErase(InstancePtr);
			break;

		case XISF_SP_ENABLE:
			Status = SpEnable(InstancePtr);
			break;

		case XISF_SP_DISABLE:
			Status = SpDisable(InstancePtr);
			break;

		default:
			/* Added Comment for MISRA C */
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
* This function reads the content of the Sector Protection Register(SPR).
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	ReadPtr is a pointer to the memory where the SPR content is
*		copied.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- This operation is supported for Atmel, Intel, STM, Winbond
*		and Spansion Serial Flash.
*		- The SPR content is stored at the fourth location pointed
*		by the ReadPtr for Atmel Serial Flash and at second location
*		for STM/Intel/Winbond/Spansion Serial Flash.
*
******************************************************************************/
static int SprRead(XIsf *InstancePtr, u8 *ReadPtr)
{
	int Status;

	Xil_AssertNonvoid(InstancePtr != NULL);


	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
	}

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	u32 Index;

	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SPR_READ;

	for(Index = 1; Index < (InstancePtr->NumOfSectors +
				XISF_CMD_SEND_EXTRA_BYTES); Index++) {
		InstancePtr->WriteBufPtr[Index] = XISF_DUMMYBYTE;
	}

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, ReadPtr,
				InstancePtr->NumOfSectors +
				XISF_CMD_SEND_EXTRA_BYTES);

#else

	Status = XIsf_GetStatus(InstancePtr, ReadPtr);

#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	return (int)(XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function writes to the Sector Protection Register(SPR).
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferPtr is the pointer to the data to be written to SPR.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This operation is supported for Atmel, Intel, STM, Winbond and
*		Spansion Serial Flash.
*
******************************************************************************/
static int SprProgram(XIsf *InstancePtr, u8 *BufferPtr)
{
	int Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if (BufferPtr == NULL) {
		return (int)(XST_FAILURE);
	}

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	u32 Index;

	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SPR_BYTE1;
	InstancePtr->WriteBufPtr[BYTE2] = XISF_CMD_SPR_BYTE2;
	InstancePtr->WriteBufPtr[BYTE3] = XISF_CMD_SPR_BYTE3;
	InstancePtr->WriteBufPtr[BYTE4] = XISF_CMD_SPR_BYTE4_PROGRAM;

	for(Index = 4; Index < (InstancePtr->NumOfSectors +
			XISF_CMD_SEND_EXTRA_BYTES); Index++) {
		InstancePtr->WriteBufPtr[Index] = *BufferPtr++;
	}

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				InstancePtr->NumOfSectors +
				XISF_CMD_SEND_EXTRA_BYTES);
#else

	Status = XIsf_Write(InstancePtr, XISF_WRITE_STATUS_REG,
				(void*) BufferPtr);

#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	return (int)(XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function erases the content of the Sector Protection Register(SPR).
*
* @param	InstancePtr is a pointer to the XIsf instance.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int SprErase(XIsf *InstancePtr)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SPR_BYTE1;
	InstancePtr->WriteBufPtr[BYTE2] = XISF_CMD_SPR_BYTE2;
	InstancePtr->WriteBufPtr[BYTE3] = XISF_CMD_SPR_BYTE3;
	InstancePtr->WriteBufPtr[BYTE4] = XISF_CMD_SPR_BYTE4_ERASE;

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
* This function enables Sector Protection. Sectors specified for protection in
* the Sector Protection Register are protected by using this operation.
*
* @param	InstancePtr is a pointer to the XIsf instance.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int SpEnable(XIsf *InstancePtr)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);


#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SPR_BYTE1;
	InstancePtr->WriteBufPtr[BYTE2] = XISF_CMD_SPR_BYTE2;
	InstancePtr->WriteBufPtr[BYTE3] = XISF_CMD_SPR_BYTE3;
	InstancePtr->WriteBufPtr[BYTE4] = XISF_CMD_SPR_BYTE4_ENABLE;

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
* This function Disables Sector Protection.
*
* @param	InstancePtr is a pointer to the XIsf instance.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int SpDisable(XIsf *InstancePtr)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SPR_BYTE1;
	InstancePtr->WriteBufPtr[BYTE2] = XISF_CMD_SPR_BYTE2;
	InstancePtr->WriteBufPtr[BYTE3] = XISF_CMD_SPR_BYTE3;
	InstancePtr->WriteBufPtr[BYTE4] = XISF_CMD_SPR_BYTE4_DISABLE;

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
