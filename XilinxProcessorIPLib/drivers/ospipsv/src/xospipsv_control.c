/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xospipsv_control.c
* @addtogroup xospipsv_v1_2
* @{
*
* This file implements the low level functions used by the functions in
* xospipsv.c and xospipsv_options.c files.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.2   sk  02/20/20 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xospipsv_control.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/
#define MAX_DELAY_CNT	10000U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Flash command based data reading using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
* @note		This operation is IO mode of reading.
*
******************************************************************************/
u32 XOspiPsv_Stig_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Reqaddr;
	u32 Status;

	if (InstancePtr->RxBytes <= 0U) {
		Status = (u32)XST_FAILURE;
		goto ERROR_PATH;
	}

	if (Msg->Addrvalid != 0U) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_ADDR_REG, Msg->Addr);
		Reqaddr = 1;
	} else {
		Reqaddr = 0U;
	}

	XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
		1, (u32)InstancePtr->RxBytes - 1, Reqaddr, 0, (u32)Msg->Addrsize - 1,
		0, 0, (u32)Msg->Dummy, 0);

	/* Execute command */
	Status = XOspiPsv_Exec_Flash_Cmd(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto ERROR_PATH;
	}

	XOspiPsv_FifoRead(InstancePtr, Msg);

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Flash command based data write using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE if fails.
*
* @note		This operation is IO mode of writing.
*
******************************************************************************/
u32 XOspiPsv_Stig_Write(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Reqaddr;
	u32 Reqwridataen;
	u32 ByteCount;
	u32 Status;

	if (Msg->Addrvalid != 0U) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_ADDR_REG, Msg->Addr);
		Reqaddr = 1;
	} else {
		Reqaddr = 0U;
	}
	if (InstancePtr->TxBytes != 0U) {
		Reqwridataen = 1;
		ByteCount = InstancePtr->TxBytes;
		XOspiPsv_FifoWrite(InstancePtr, Msg);
	} else {
		Reqwridataen = 0U;
		ByteCount = 1;
	}
	XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
		0, 0, Reqaddr, 0, (u32)Msg->Addrsize - 1,
		Reqwridataen, (u32)ByteCount - 1, 0, 0);

	/* Exec cmd */
	Status = XOspiPsv_Exec_Flash_Cmd(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function Read the data using DMA
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
* @note		None
*
******************************************************************************/
u32 XOspiPsv_Dma_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;

	if ((Msg->ByteCount % 4U) != 0U) {
		InstancePtr->IsUnaligned = 1;
	}

	if (Msg->ByteCount >= (u32)4) {
		Msg->ByteCount -= (Msg->ByteCount % 4U);
		XOspiPsv_Config_Dma(InstancePtr,Msg);
		XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
		Status = XOspiPsv_Exec_Dma(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR_PATH;
		}
		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheInvalidateRange((UINTPTR)Msg->RxBfrPtr, Msg->ByteCount);
		}
		if (InstancePtr->IsUnaligned != 0U) {
			InstancePtr->RecvBufferPtr += Msg->ByteCount;
			Msg->Addr += Msg->ByteCount;
		}
	}

	if (InstancePtr->IsUnaligned != 0U) {
		Msg->ByteCount = 4;
		Msg->RxBfrPtr = InstancePtr->UnalignReadBuffer;
		InstancePtr->RxBytes = (InstancePtr->RxBytes % 4U);
		XOspiPsv_Config_Dma(InstancePtr,Msg);
		XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
		Status = XOspiPsv_Exec_Dma(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR_PATH;
		}
		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheInvalidateRange((UINTPTR)Msg->RxBfrPtr, Msg->ByteCount);
		}
		Xil_MemCpy(InstancePtr->RecvBufferPtr, InstancePtr->UnalignReadBuffer,
				InstancePtr->RxBytes);
		InstancePtr->IsUnaligned = 0U;
	}

	Status = (u32)XST_SUCCESS;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function reads the data using Linear controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE for invalid address.
*
* @note		None
*
******************************************************************************/
u32 XOspiPsv_Dac_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;
	u32 *Addr = (u32 *)XOSPIPSV_LINEAR_ADDR_BASE + Msg->Addr;

	if (Addr >= (u32 *)(XOSPIPSV_LINEAR_ADDR_BASE + SIZE_512MB)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	Xil_MemCpy(Msg->RxBfrPtr, Addr, InstancePtr->RxBytes);
	InstancePtr->RxBytes = 0U;

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function writes the data Using Linear controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE for invalid address.
*
* @note		None
*
******************************************************************************/
u32 XOspiPsv_Dac_Write(XOspiPsv *InstancePtr, const XOspiPsv_Msg *Msg)
{
	u32 Status;
	u32 *Addr = (u32 *)XOSPIPSV_LINEAR_ADDR_BASE + Msg->Addr;

	if (Addr >= (u32 *)(XOSPIPSV_LINEAR_ADDR_BASE + SIZE_512MB)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	Xil_MemCpy(Addr, Msg->TxBfrPtr, InstancePtr->TxBytes);
	InstancePtr->TxBytes = 0U;

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Check for OSPI idle which means Serial interface and low level SPI pipeline
* is IDLE.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
u32 XOspiPsv_CheckOspiIdle(XOspiPsv *InstancePtr)
{
	u32 ReadReg;
	u32 Status;
	u32 DelayCount;

	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);
	DelayCount = 0U;
	while ((ReadReg & XOSPIPSV_CONFIG_REG_IDLE_FLD_MASK) == 0U) {
		if (DelayCount == MAX_DELAY_CNT) {
			Status = XST_FAILURE;
			goto ERROR_PATH;
		} else {
			/* Wait for 1 usec */
			usleep(1);
			DelayCount++;
			ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_CONFIG_REG);
		}
	}

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/** @} */
