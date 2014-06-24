/******************************************************************************
*
* Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xilisf_erase.c
*
* This file contains the library functions to Erase the Serial Flash.
* Refer xilisf.h for a detailed description.
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
* 3.02a srt	 04/25/13 Added Bulk Erase command support for SST and
*			  Spansion flashes.
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
static int PageErase(XIsf *InstancePtr, u32 Address);
static int BlockErase(XIsf *InstancePtr, u32 Address);
static int SectorErase(XIsf *InstancePtr, u32 Address);
static int BulkErase(XIsf *InstancePtr);

/************************** Variable Definitions *****************************/

/************************** Function Definitions ******************************/


/*****************************************************************************/
/**
*
* This API erases the contents of the specified memory in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Operation is the type of Erase operation to be performed on the
*		Serial Flash.
*		The different operations are
*		- XISF_PAGE_ERASE: Page Erase
*		- XISF_BLOCK_ERASE: Block Erase
*		- XISF_SECTOR_ERASE: Sector Erase
		- XISF_BULK_ERASE: Bulk Erase
* @param	Address is the address of the Page/Block/Sector to be erased.
*		The address can be either Page address, Block address or Sector
*		address based on the Erase operation to be performed.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- The erased bytes will read as 0xFF.
*		- For Intel, STM, Winbond or Spansion Serial Flash the user
*		application must call XIsf_WriteEnable() API by passing
*		XISF_WRITE_ENABLE as an argument before calling the XIsf_Erase()
*		API.
*		- Atmel Serial Flash support Page/Block/Sector Erase operations.
*		- Intel, Winbond, Numonyx (N25QXX) and Spansion Serial Flash
*		  support Sector/Block/Bulk Erase operations.
*		- STM (M25PXX) Serial Flash support Sector/Bulk Erase
*		  operations.
*
******************************************************************************/
int XIsf_Erase(XIsf *InstancePtr, XIsf_EraseOperation Operation, u32 Address)
{
	int Status = XST_FAILURE;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	switch (Operation) {

		case XISF_PAGE_ERASE:
			Status = PageErase(InstancePtr, Address);
			break;

		case XISF_BLOCK_ERASE:
			Status = BlockErase(InstancePtr, Address);
			break;

		case XISF_SECTOR_ERASE:
			Status = SectorErase(InstancePtr, Address);
			break;

		case XISF_BULK_ERASE:
			Status = BulkErase(InstancePtr);
			break;

		default:
			break;
	}

	return Status;
}


/*****************************************************************************/
/**
*
* This function erases the contents of the specified Page in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the address of the Page to be erased. This can be any
*		address in the Page to be erased. The Byte address values in
*		this address are ignored.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
* 		- The erased bytes will read as 0xFF.
*		- This operation is only supported for Atmel Serial Flash.
*
******************************************************************************/
static int PageErase(XIsf *InstancePtr, u32 Address)
{
	int Status = XST_FAILURE;

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_PAGE_ERASE;
	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULL, XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function erases the contents of the specified Block in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the address of the Block to be erased. This can be
* 		any address in the Block to be erased. The Page/Byte address
*		values in this address are ignored.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- The erased bytes will read as 0xFF.
*		- This operation is supported for Atmel, Intel, Winbond,
*		  Numonyx (N25QXX), and Spansion Serial Flash.
*
******************************************************************************/
static int BlockErase(XIsf *InstancePtr, u32 Address)
{
	int Status = XST_FAILURE;

#if ((XPAR_XISF_FLASH_FAMILY == ATMEL) || (XPAR_XISF_FLASH_FAMILY == INTEL) || \
     (XPAR_XISF_FLASH_FAMILY == WINBOND))

#if ((XPAR_XISF_FLASH_FAMILY == ATMEL) || (XPAR_XISF_FLASH_FAMILY == WINBOND))
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BLOCK_ERASE;

#else
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_PARAM_BLOCK_ERASE;

#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULL, XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* ((XPAR_XISF_FLASH_FAMILY==ATMEL)||(XPAR_XISF_FLASH_FAMILY==INTEL)) */

	return Status;
}

/*****************************************************************************/
/**
*
* This function erases the contents of the specified Sector in the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Address is the address of the Sector to be erased. This can be
* 		any address in the Sector to be erased.
*		The Block/Page/Byte address values in this address are ignored.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- The erased bytes will read as 0xFF.
*		- This operation is supported for Atmel, Intel, STM, Winbond and
*		  Spansion Serial Flash.
*
******************************************************************************/
static int SectorErase(XIsf *InstancePtr, u32 Address)
{
	int Status;

	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SECTOR_ERASE;
	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULL, XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the content of an entire Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- The erased bytes will read as 0xFF.
*		- This operation is supported for Intel, STM, Winbond,
*		  Spansion and SST Serial Flashes.
*
******************************************************************************/
static int BulkErase(XIsf *InstancePtr)
{
	int Status = XST_FAILURE;

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
     (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SST) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION))
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BULK_ERASE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULL, XISF_BULK_ERASE_BYTES);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL)||(XPAR_XISF_FLASH_FAMILY==STM)) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SST) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

	return Status;
}
