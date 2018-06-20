/******************************************************************************
*
* Copyright (C) 2009 - 2018 Xilinx, Inc.  All rights reserved.
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
* @file xnandps.c
* @addtogroup nandps_v2_4
* @{
*
* This file contains the implementation of the interface functions for
* XNandPs driver. Refer to the header file xnandps.h for more detailed
* information.
*
* This module supports for NAND flash memory devices that conform to the
* "Open NAND Flash Interface" (ONFI) Specification. This modules implements
* basic flash operations like read, write and erase.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* 1.01a nm     28/02/2012  Fixed 16-bit issue with ONFI commands like
*                          read, write and read status command. The config
*                          structure width is updated after ONFI query
*                          with the parameter page width.
* 1.02a nm     20/09/2012  Removed setting of set_cycles and set_opmode
*                          register values as it is now done in FSBL using
*                          the PCW generated files. CR#678949.
* 1.03a nm     10/22/2012  Fixed CR# 673348.
* 1.04a nm     04/15/2013  Fixed CR# 704401. Removed warnings when compiled
* 			   with -Wall and -Wextra option in bsp.
*	       04/25/2013  Implemented PR# 699544. Added page cache read
*			   and program support. Added API's XNandPs_ReadCache
*			   and XNandPs_WriteCache for page cache support.
*			   Added ECC handling functions XNandPs_EccSetCfg,
*			   XNandPs_EccSetMemCmd1...etc, to support better
*			   usage of ECC block for page cache commands.
*			   Modified Read/Write API's so that there is common
*			   code for normal read/write and page cache commands.
*			   Disabling/Re-enabling ECC block in read/write API's
*			   of spare bytes since we don't calculate ECC for
*			   spare bytes.
* 2.01 kpc    07/24/2014   Fixed CR#808770. Update command register twice only
*                          if flash device requires >= four address cycles.
* 2.2  sb     01/31/2015   Use the address cycles defined in onfi parameter
*			   page than hardcoding this value to 5 for read and
*			   write operations.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xnandps.h"
#include "xnandps_bbm.h"
#include "xnandps_onfi.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XNandPs_EccHwInit(XNandPs *InstancePtr);

static int XNandPs_EccSwInit(XNandPs *InstancePtr);

static int XNandPs_ReadPage_HwEcc(XNandPs *InstancePtr, u8 *DstPtr);

static int XNandPs_ReadPage(XNandPs *InstancePtr, u8 *DstPtr);

static int XNandPs_WritePage_HwEcc(XNandPs *InstancePtr, u8 *SrcPtr);

static int XNandPs_WritePage(XNandPs *InstancePtr, u8 *SrcPtr);

static int XNandPs_EccCalculate(XNandPs *InstancePtr, u8 *EccData);

static int XNandPs_EccCorrect(u8 *Buf, u8 *EccCalc, u8 *EccCode);

static void XNandPs_ReadBuf(XNandPs *InstancePtr, u8 *Buf, u32 Length);

static void XNandPs_WriteBuf(XNandPs *InstancePtr, u8 *Buf, u32 Length);

static int XNandPs_IsBusy(XNandPs *InstancePtr);

void XNandPs_SendCommand(XNandPs *InstancePtr, XNandPs_CommandFormat
		*Command, int Page, int Column);

static void XNandPs_EccSetCfg(XNandPs *InstancePtr, u32 EccConfig);

static void XNandPs_EccSetMemCmd1(XNandPs *InstancePtr, u32 EccCmd);

static void XNandPs_EccSetMemCmd2(XNandPs *InstancePtr, u32 EccCmd);

static void XNandPs_EccDisable(XNandPs *InstancePtr);

/* Bad block management routines */
extern void XNandPs_InitBbtDesc(XNandPs *InstancePtr);

extern int XNandPs_ScanBbt(XNandPs *InstancePtr);
/* ONFI routines */
extern u8 Onfi_CmdReadStatus(XNandPs *InstancePtr);

extern int Onfi_NandInit(XNandPs *InstancePtr);
/************************** Variable Definitions *****************************/
/* ECC data position in the spare data area  for different page sizes */
u32 NandOob16[] = {13, 14, 15};	/**< Ecc position for 16 bytes spare area */

u32 NandOob32[] = {26, 27, 28, 29, 30, 31};
				/**< Ecc position for 32 bytes spare area */

u32 NandOob64[] = {52, 53, 54, 55, 56, 57,
		   58, 59, 60, 61, 62, 63};
				/**< Ecc position for 64 bytes spare area */

extern XNandPs_CommandFormat OnfiCommands[];	/**< ONFI commands */

/*****************************************************************************/
/**
*
* This function initializes a specific XNandPs device/instance. This function
* must be called prior to using the flash device to read or write any data.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	ConfigPtr points to the XNandPs device configuration structure.
* @param	SmcBaseAddr is the base address of SMC controller.
* @param	FlashBaseAddr is the base address of NAND flash.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		The user needs to first call the XNandPs_LookupConfig() API
*		which returns the Configuration structure pointer which is
*		passed as a parameter to the XNandPs_CfgInitialize() API.
*
******************************************************************************/
int XNandPs_CfgInitialize(XNandPs *InstancePtr, XNandPs_Config *ConfigPtr,
				u32 SmcBaseAddr, u32 FlashBaseAddr)
{
	u32 PageSize;
	int Status;

	/*
	 * Assert the input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Set the values read from the device config and the base address.
	 */
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
	InstancePtr->Config.SmcBase = SmcBaseAddr;
	InstancePtr->Config.FlashBase = FlashBaseAddr;
	InstancePtr->Config.FlashWidth = ConfigPtr->FlashWidth;

	XNandPs_WriteReg(InstancePtr->Config.SmcBase +
		XNANDPS_MEMC_CLR_CONFIG_OFFSET,
		XNANDPS_CLR_CONFIG);	/* Disable interrupts */

	/*
	 * ONFI query to get geometry
	 */
	Status = Onfi_NandInit(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Updating Config structure flash width
	 */
	InstancePtr->Config.FlashWidth = InstancePtr->Geometry.FlashWidth;

	/*
	 * Fill the spare buffer pointer
	 */
	PageSize = InstancePtr->Geometry.BytesPerPage;
	InstancePtr->SpareBufPtr = &InstancePtr->DataBuf[PageSize];

	/*
	 * Initialize ECC Block Parameters
	 */
	switch (InstancePtr->EccMode)
	{
		case XNANDPS_ECC_NONE:
			/* Fall through */
		case XNANDPS_ECC_ONDIE:
			/* Bypass the ECC block in the SMC controller */
			XNandPs_EccDisable(InstancePtr);

			/* Initialize the Read/Write page routines */
			InstancePtr->ReadPage = XNandPs_ReadPage;
			InstancePtr->WritePage = XNandPs_WritePage;
			break;
		case XNANDPS_ECC_HW:
			/* Use SMC ECC controller ECC block */
			Status = XNandPs_EccHwInit(InstancePtr);
			if (Status != XST_SUCCESS)
				return Status;

			/* Initialize ECC SW parameters */
			Status = XNandPs_EccSwInit(InstancePtr);
			if (Status != XST_SUCCESS)
				return Status;

			/* Initialize the Read/Write page routines */
			InstancePtr->ReadPage = XNandPs_ReadPage_HwEcc;
			InstancePtr->WritePage = XNandPs_WritePage_HwEcc;
			break;
		default:
			return XST_FAILURE;
	}

	/*
	 * Indicate the instance is now ready to use, initialized without error.
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Scan for the bad block table(bbt) stored in the flash & load it in
	 * memory(RAM).  If bbt is not found, create bbt by scanning factory
	 * marked bad blocks and store it in last good blocks of flash.
	 */
	XNandPs_InitBbtDesc(InstancePtr);
	Status = XNandPs_ScanBbt(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the HW ECC block based on flash.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	EccConfig is the value of ECC config to update.
*
* @return	None
*
* @note		None
*
*****************************************************************************/
static void XNandPs_EccSetCfg(XNandPs *InstancePtr, u32 EccConfig)
{
	/*
	 * Check the busy status of the ECC block
	 */
	while (XNandPs_ReadReg(InstancePtr->Config.SmcBase +
		XNANDPS_ECC_STATUS_OFFSET(XNANDPS_IF1_ECC_OFFSET)) &
		XNANDPS_ECC_STATUS_MASK);
	/*
	 * Write ECC configuration register
	 */
	XNandPs_WriteReg(InstancePtr->Config.SmcBase +
		(XNANDPS_ECC_MEMCFG_OFFSET(XNANDPS_IF1_ECC_OFFSET)),
		EccConfig);
}

/*****************************************************************************/
/**
*
* This function writes ECC MEM CMD1 register with EccCmd value.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	EccCmd is register value to write.
*
* @return	None
*
* @note		None
*
*****************************************************************************/
static void XNandPs_EccSetMemCmd1(XNandPs *InstancePtr, u32 EccCmd)
{
	/*
	 * Set the ECC mem command1 register
	 */
	XNandPs_WriteReg(InstancePtr->Config.SmcBase +
		(XNANDPS_ECC_MEMCMD1_OFFSET(XNANDPS_IF1_ECC_OFFSET)),
		EccCmd);
}

/*****************************************************************************/
/**
*
* This function writes ECC MEM CMD2 register with EccCmd value.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	EccCmd is register value to write.
*
* @return	None
*
* @note		None
*
*****************************************************************************/
static void XNandPs_EccSetMemCmd2(XNandPs *InstancePtr, u32 EccCmd)
{
	/*
	 * Set the ECC mem command2 register
	 */
	XNandPs_WriteReg(InstancePtr->Config.SmcBase +
		(XNANDPS_ECC_MEMCMD2_OFFSET(XNANDPS_IF1_ECC_OFFSET)),
		EccCmd);
}

/*****************************************************************************/
/**
*
* This function disables ECC block.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return	None
*
* @note		None
*
*****************************************************************************/
static void XNandPs_EccDisable(XNandPs *InstancePtr)
{
	u32 EccConfig = 0;
	/*
	 * Bypass the ECC block in the SMC controller
	 */
	EccConfig = XNandPs_ReadReg(InstancePtr->Config.SmcBase +
		(XNANDPS_ECC_MEMCFG_OFFSET(XNANDPS_IF1_ECC_OFFSET)));

	EccConfig &= ~XNANDPS_ECC_MEMCFG_ECC_MODE_MASK;
	XNandPs_EccSetCfg(InstancePtr, EccConfig);
}

/*****************************************************************************/
/**
*
* This function initializes the HW ECC block based on flash.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if the flash is not supported.
*
* @note		None
*
*****************************************************************************/
static int XNandPs_EccHwInit(XNandPs *InstancePtr)
{
	u32 PageSize;
	u32 EccConfig = 0;

	PageSize = InstancePtr->Geometry.BytesPerPage;
	/*
	 * Set the ECC mem command1 and ECC mem command2 register
	 */
	XNandPs_EccSetMemCmd1(InstancePtr, XNANDPS_ECC_CMD1);
	XNandPs_EccSetMemCmd2(InstancePtr, XNANDPS_ECC_CMD2);
	/*
	 * Configure HW ECC block
	 */
	switch(PageSize) {
		case XNANDPS_PAGE_SIZE_512:
			EccConfig = (XNANDPS_ECC_MEMCFG |
					XNANDPS_ECC_MEMCFG_PAGE_SIZE_512);
			break;
		case XNANDPS_PAGE_SIZE_1024:
			EccConfig = (XNANDPS_ECC_MEMCFG |
					XNANDPS_ECC_MEMCFG_PAGE_SIZE_1024);
			break;
		case XNANDPS_PAGE_SIZE_2048:
			EccConfig = (XNANDPS_ECC_MEMCFG |
					XNANDPS_ECC_MEMCFG_PAGE_SIZE_2048);
			break;
		default:
			/*
			 * Page size 256 bytes & 4096 bytes not supported
			 * by ECC block
			 */
			return XST_FAILURE;
	}
	XNandPs_EccSetCfg(InstancePtr, EccConfig);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the software variables related
* to ECC generation, ECC checking and writing ECC bytes in spare bytes.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if the flash is not supported.
*
* @note		None
*
*****************************************************************************/
static int XNandPs_EccSwInit(XNandPs *InstancePtr)
{
	u32 PageSize;
	u32 SpareBytesSize;
	u32 Index;

	PageSize = InstancePtr->Geometry.BytesPerPage;
	SpareBytesSize = InstancePtr->Geometry.SpareBytesPerPage;

	/*
	 * Initialize ECC config structure parameters
	 */
	InstancePtr->EccConfig.BytesPerBlock = XNANDPS_ECC_BYTES;
	InstancePtr->EccConfig.BlockSize = XNANDPS_ECC_BLOCK_SIZE;
	InstancePtr->EccConfig.TotalBytes = (PageSize/XNANDPS_ECC_BLOCK_SIZE)
		* XNANDPS_ECC_BYTES;
	InstancePtr->EccConfig.NumSteps = PageSize/XNANDPS_ECC_BLOCK_SIZE;

	/*
	 * Ecc write position in spare data area as per Linux mtd subsystem
	 */
	switch(SpareBytesSize) {
		case XNANDPS_SPARE_SIZE_16:
			for(Index = 0; Index <
					InstancePtr->EccConfig.TotalBytes;
					Index++) {
				InstancePtr->EccConfig.EccPos[Index] =
					NandOob16[Index];
			}
			break;
		case XNANDPS_SPARE_SIZE_32:
			for(Index = 0; Index <
					InstancePtr->EccConfig.TotalBytes;
					Index++) {
				InstancePtr->EccConfig.EccPos[Index] =
					NandOob32[Index];
			}
			break;
		case XNANDPS_SPARE_SIZE_64:
			for(Index = 0; Index <
					InstancePtr->EccConfig.TotalBytes;
					Index++) {
				InstancePtr->EccConfig.EccPos[Index] =
					NandOob64[Index];
			}
			break;
		default:
			return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the data from the Flash device and copies it into the
* specified user buffer. It doesn't check for the bad blocks while reading
* the flash pages that cross block boundary. User must take care of handling
* bad blocks.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Offset is the flash data address to read from.
* @param	Length is number of bytes to read.
* @param	DestPtr is the destination address to copy data to.
* @param	UserSparePtr is the user buffer to which spare data must be
*		copied.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		This function reads sequential pages from the Flash device.
*
******************************************************************************/
int XNandPs_Read(XNandPs *InstancePtr, u64 Offset, u32 Length, void *DestPtr,
			u8 *UserSparePtr)
{
	u32 Page;
	u32 Col;
	u32 PartialBytes;
	u32 NumOfBytes;
	int Status;
	u32 PartialPageRead = 0;
	u32 CopyOffset;
	u8 *BufPtr;
	u8 *Ptr = (u8 *)DestPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DestPtr != NULL);
	Xil_AssertNonvoid((Offset + Length) < InstancePtr->Geometry.DeviceSize);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Length != 0);

	Page = (u32) (Offset/InstancePtr->Geometry.BytesPerPage);
	Col = (u32) (Offset & (InstancePtr->Geometry.BytesPerPage - 1));
	PartialBytes = InstancePtr->Geometry.BytesPerPage - Col;
	NumOfBytes = (PartialBytes < Length) ? PartialBytes:Length;
	CopyOffset = InstancePtr->Geometry.BytesPerPage - PartialBytes;

	/*
	 * Restore the ECC mem command1 and ECC mem command2 register
	 * if the previous command is read page cache.
	 */
	XNandPs_EccSetMemCmd1(InstancePtr, XNANDPS_ECC_CMD1);
	XNandPs_EccSetMemCmd2(InstancePtr, XNANDPS_ECC_CMD2);

	while (Length) {
		/*
		 * Check if partial read
		 */
		if (NumOfBytes < InstancePtr->Geometry.BytesPerPage) {
			BufPtr = &InstancePtr->DataBuf[0];
			PartialPageRead = 1;
		} else {
			BufPtr = (u8 *)Ptr;
			PartialPageRead = 0;
		}

		/*
		 * Send the ONFI Read command
		 */
		XNandPs_SendCommand(InstancePtr, &OnfiCommands[READ], Page, 0);

		/*
		 * Poll the Memory controller status register
		 */
		while (XNandPs_IsBusy(InstancePtr) == TRUE) {
		}

		/*
		 * Clear the interrupt condition
		 */
		XNandPs_WriteReg((InstancePtr->Config.SmcBase +
					XNANDPS_MEMC_CLR_CONFIG_OFFSET),
				XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);

		/*
		 *  Read the page data
		 */
		Status = InstancePtr->ReadPage(InstancePtr, BufPtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Fill the partial data in the buffer
		 */
		if (PartialPageRead) {
			memcpy(Ptr, BufPtr + CopyOffset, NumOfBytes);
		}

		Ptr += NumOfBytes;
		Length -= NumOfBytes;
		Page++;
		NumOfBytes = (Length > InstancePtr->Geometry.BytesPerPage) ?
				InstancePtr->Geometry.BytesPerPage:Length;
		CopyOffset = 0;
	}

	/*
	 * Copy the spare data to user spare buffer
	 */
	if (UserSparePtr) {
		memcpy(UserSparePtr, InstancePtr->SpareBufPtr,
				InstancePtr->Geometry.SpareBytesPerPage);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the data from the Flash device using read page cache
* command and copies it into the specified user buffer.
* It doesn't check for the bad blocks while reading the flash pages that
* cross block boundary. User must take care of handling bad blocks.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Offset is the flash data address to read from.
* @param	Length is number of bytes to read.
* @param	DestPtr is the destination address to copy data to.
* @param	UserSparePtr is the user buffer to which spare data must be
*		copied.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		This function reads sequential pages from the Flash device.
*
******************************************************************************/
int XNandPs_ReadCache(XNandPs *InstancePtr, u64 Offset, u32 Length,
			void *DestPtr, u8 *UserSparePtr)
{
	u32 Page;
	u32 Col;
	u32 PartialBytes;
	u32 NumOfBytes;
	int Status;
	u32 PartialPageRead = 0;
	u32 CopyOffset;
	u8 *BufPtr;
	u8 *Ptr = (u8 *)DestPtr;
	u32 NumPages;
	u32 EccConfig = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DestPtr != NULL);
	Xil_AssertNonvoid((Offset + Length) < InstancePtr->Geometry.DeviceSize);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Length != 0);

	/*
	 * Check if the flash supports read cache
	 */
	if (!InstancePtr->Features.ReadCache) {
		return XNandPs_Read(InstancePtr, Offset, Length, DestPtr,
							UserSparePtr);
	}

	Page = (u32) (Offset/InstancePtr->Geometry.BytesPerPage);
	Col = (u32) (Offset & (InstancePtr->Geometry.BytesPerPage - 1));
	PartialBytes = InstancePtr->Geometry.BytesPerPage - Col;
	NumOfBytes = (PartialBytes < Length) ? PartialBytes:Length;
	CopyOffset = InstancePtr->Geometry.BytesPerPage - PartialBytes;
	/*
	 * Calculate number of pages to read
	 */
	NumPages = Length/InstancePtr->Geometry.BytesPerPage;
	NumPages += (Length % InstancePtr->Geometry.BytesPerPage) ? 1:0;
	/*
	 * Read, Read Cache start, Read Cache end
	 */
	if (NumPages <= 1) {
		return XNandPs_Read(InstancePtr, Offset, Length, DestPtr,
							UserSparePtr);
	}

	/*
	 * Change ECC commands in ECC registers for page cache support
	 */
	EccConfig |= ONFI_CMD_PAGE_CACHE_PROGRAM1;
	EccConfig |= ONFI_CMD_READ_CACHE_ENHANCED1 << 8;
	EccConfig |= ONFI_CMD_READ_CACHE_ENHANCED2 << 16;
	EccConfig |= (XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_VALID_MASK);
	XNandPs_EccSetMemCmd1(InstancePtr, EccConfig);

	/*
	 * Send the ONFI Read command
	 */
	XNandPs_SendCommand(InstancePtr, &OnfiCommands[READ], Page, 0);

	/*
	 * Poll the Memory controller status register
	 */
	while (XNandPs_IsBusy(InstancePtr) == TRUE) {
	}

	/*
	 * Clear the interrupt condition
	 */
	XNandPs_WriteReg((InstancePtr->Config.SmcBase +
				XNANDPS_MEMC_CLR_CONFIG_OFFSET),
			XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);

	/*
	 * Check ONFI Status Register
	 */
	Status = Onfi_CmdReadStatus(InstancePtr);
	if (Status & ONFI_STATUS_FAIL) {
		return XST_FAILURE;
	}

	while (Length && (NumPages > 0)) {
		/*
		 * Check if partial read
		 */
		if (NumOfBytes < InstancePtr->Geometry.BytesPerPage) {
			BufPtr = &InstancePtr->DataBuf[0];
			PartialPageRead = 1;
		} else {
			BufPtr = (u8 *)Ptr;
			PartialPageRead = 0;
		}

		/* Increment the page */
		Page++;

		if (NumPages <= 1) {
			/*
			 * Change ECC commands in ECC registers to check
			 * change read column for ECC calculation
			 */
			EccConfig = 0;
			EccConfig |= ONFI_CMD_PAGE_CACHE_PROGRAM1;
			EccConfig |= ONFI_CMD_CHANGE_READ_COLUMN1 << 8;
			EccConfig |= ONFI_CMD_CHANGE_READ_COLUMN2 << 16;
			EccConfig |=
				XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_VALID_MASK;
			XNandPs_EccSetMemCmd1(InstancePtr, EccConfig);

			/*
			 * Send NAND page cache end command 0x3F
			 */
			XNandPs_SendCommand(InstancePtr,
					&OnfiCommands[READ_CACHE_END_SEQ],
					XNANDPS_PAGE_NOT_VALID,
					XNANDPS_COLUMN_NOT_VALID);
		} else {
			XNandPs_SendCommand(InstancePtr,
					&OnfiCommands[READ_CACHE_RANDOM],
					Page, 0);
		}

		/*
		 * Poll the Memory controller status register
		 */
		while (XNandPs_IsBusy(InstancePtr) == TRUE) {
		}

		/*
		 * Clear the interrupt condition
		 */
		XNandPs_WriteReg((InstancePtr->Config.SmcBase +
					XNANDPS_MEMC_CLR_CONFIG_OFFSET),
				XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);

		if (NumPages <= 1) {
			XNandPs_SendCommand(InstancePtr,
					&OnfiCommands[CHANGE_READ_COLUMN],
					XNANDPS_PAGE_NOT_VALID,
					0);
		}

		/*
		 *  Read the page data
		 */
		Status = InstancePtr->ReadPage(InstancePtr, BufPtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Fill the partial data in the buffer
		 */
		if (PartialPageRead) {
			memcpy(Ptr, BufPtr + CopyOffset, NumOfBytes);
		}

		Ptr += NumOfBytes;
		Length -= NumOfBytes;
		NumPages--;
		NumOfBytes = (Length > InstancePtr->Geometry.BytesPerPage) ?
				InstancePtr->Geometry.BytesPerPage:Length;
		CopyOffset = 0;
	}

	/*
	 * Copy the spare data to user spare buffer
	 */
	if (UserSparePtr) {
		memcpy(UserSparePtr, InstancePtr->SpareBufPtr,
				InstancePtr->Geometry.SpareBytesPerPage);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function programs the flash device(s) with data specified in the user
* buffer. The source and destination address must be aligned to the width of the
* flash's data bus. It doesn't check for the bad blocks while writing to
* the flash pages that cross block boundary. User must take care of handling
* bad blocks.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Offset is the flash data address to write to.
* @param	Length is number of bytes to write.
* @param	SrcPtr is the source address to write the data from.
* @param	UserSparePtr is the user buffer which contains buffer to write
*		into spare data area.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*		- XST_NAND_WRITE_PROTECTED if the flash is write protected.
*
* @note		This function writes number of sequential pages into the
*		Flash device.
*
******************************************************************************/
int XNandPs_Write(XNandPs *InstancePtr, u64 Offset, u32 Length, void *SrcPtr,
			u8 *UserSparePtr)
{
	u32 Page;
	u32 Col;
	u32 PartialBytes;
	u32 NumOfBytes;
	u32 CopyOffset;
	u32 Status;
	u8 *BufPtr;
	u8 OnfiStatus;
	u8 *Ptr = (u8 *)SrcPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SrcPtr != NULL);
	Xil_AssertNonvoid((Offset + Length) < InstancePtr->Geometry.DeviceSize);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Length != 0);

	/*
	 * Check if the flash is write protected
	 */
	OnfiStatus = Onfi_CmdReadStatus(InstancePtr);
	if (!(OnfiStatus & ONFI_STATUS_WP)) {
		return XST_NAND_WRITE_PROTECTED;
	}

	/*
	 * Copy the user spare data buffer
	 */
	if (UserSparePtr == NULL) {
		memset(InstancePtr->SpareBufPtr, 0xff,
				InstancePtr->Geometry.SpareBytesPerPage);
	} else {
		memcpy(InstancePtr->SpareBufPtr, UserSparePtr,
				InstancePtr->Geometry.SpareBytesPerPage);
	}

	Page = (u32) (Offset/InstancePtr->Geometry.BytesPerPage);
	Col = (u32) (Offset & (InstancePtr->Geometry.BytesPerPage - 1));
	PartialBytes = InstancePtr->Geometry.BytesPerPage - Col;
	NumOfBytes = (PartialBytes < Length) ? PartialBytes:Length;
	CopyOffset = InstancePtr->Geometry.BytesPerPage - PartialBytes;

	while (Length)
	{
		/*
		 * Partial write, fill the remaining buffer with 0xff
		 */
		if (NumOfBytes < InstancePtr->Geometry.BytesPerPage) {
			BufPtr = &InstancePtr->DataBuf[0];
			memset(BufPtr, 0xff,
					InstancePtr->Geometry.BytesPerPage);
			memcpy(BufPtr + CopyOffset, Ptr, NumOfBytes);
		} else {
			BufPtr = (u8 *)Ptr;
		}

		/*
		 * Send ONFI Program command
		 */
		XNandPs_SendCommand(InstancePtr, &OnfiCommands[PAGE_PROGRAM],
					Page, 0);

		/*
		 * Write the page data
		 */
		Status = InstancePtr->WritePage(InstancePtr, BufPtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		Ptr += NumOfBytes;
		Length -= NumOfBytes;
		Page++;
		NumOfBytes = (Length > InstancePtr->Geometry.BytesPerPage) ?
				InstancePtr->Geometry.BytesPerPage:Length;
		CopyOffset = 0;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function programs the flash device(s) with data specified in the user
* buffer using program cache command.
* The source and destination address must be aligned to the width of the
* flash's data bus. It doesn't check for the bad blocks while writing to
* the flash pages that cross block boundary. User must take care of handling
* bad blocks.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Offset is the flash data address to write to.
* @param	Length is number of bytes to write.
* @param	SrcPtr is the source address to write the data from.
* @param	UserSparePtr is the user buffer which contains buffer to write
*		into spare data area.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*		- XST_NAND_WRITE_PROTECTED if the flash is write protected.
*
* @note		This function writes number of sequential pages into the
*		Flash device.
*
******************************************************************************/
int XNandPs_WriteCache(XNandPs *InstancePtr, u64 Offset, u32 Length,
				void *SrcPtr, u8 *UserSparePtr)
{
	u32 Page;
	u32 Col;
	u32 PartialBytes;
	u32 NumOfBytes;
	u32 CopyOffset;
	u32 Status;
	u8 *BufPtr;
	u8 OnfiStatus;
	u8 *Ptr = (u8 *)SrcPtr;
	u32 NumPages;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SrcPtr != NULL);
	Xil_AssertNonvoid((Offset + Length) < InstancePtr->Geometry.DeviceSize);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Length != 0);


	/*
	 * Check if the flash is write protected
	 */
	OnfiStatus = Onfi_CmdReadStatus(InstancePtr);
	if (!(OnfiStatus & ONFI_STATUS_WP)) {
		return XST_NAND_WRITE_PROTECTED;
	}

	/*
	 * Copy the user spare data buffer
	 */
	if (UserSparePtr == NULL) {
		memset(InstancePtr->SpareBufPtr, 0xff,
				InstancePtr->Geometry.SpareBytesPerPage);
	} else {
		memcpy(InstancePtr->SpareBufPtr, UserSparePtr,
				InstancePtr->Geometry.SpareBytesPerPage);
	}

	Page = (u32) (Offset/InstancePtr->Geometry.BytesPerPage);
	Col = (u32) (Offset & (InstancePtr->Geometry.BytesPerPage - 1));
	PartialBytes = InstancePtr->Geometry.BytesPerPage - Col;
	NumOfBytes = (PartialBytes < Length) ? PartialBytes:Length;
	CopyOffset = InstancePtr->Geometry.BytesPerPage - PartialBytes;
	/*
	 * Calculate number of pages to write
	 */
	NumPages = Length/InstancePtr->Geometry.BytesPerPage;
	NumPages += (Length % InstancePtr->Geometry.BytesPerPage) ? 1:0;
	/*
	 * Check for enough pages for cache programming
	 */
	if (NumPages <= 1) {
		return XNandPs_Write(InstancePtr, Offset, Length, SrcPtr,
							UserSparePtr);
	}

	while (Length && (NumPages > 0))
	{
		/*
		 * Partial write, fill the remaining buffer with 0xff
		 */
		if (NumOfBytes < InstancePtr->Geometry.BytesPerPage) {
			BufPtr = &InstancePtr->DataBuf[0];
			memset(BufPtr, 0xff,
					InstancePtr->Geometry.BytesPerPage);
			memcpy(BufPtr + CopyOffset, Ptr, NumOfBytes);
		} else {
			BufPtr = (u8 *)Ptr;
		}

		if (NumPages > 1) {
			/*
			 * Send ONFI Program cache command
			 */
			XNandPs_SendCommand(InstancePtr,
					&OnfiCommands[PAGE_CACHE_PROGRAM],
						Page, 0);
			/*
			 * Write the page data
			 */
			Status = InstancePtr->WritePage(InstancePtr, BufPtr);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		} else {
			/*
			 * Send ONFI Program command
			 */
			XNandPs_SendCommand(InstancePtr,
						&OnfiCommands[PAGE_PROGRAM],
						Page, 0);
			/*
			 * Write the page data
			 */
			Status = InstancePtr->WritePage(InstancePtr, BufPtr);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}

		Ptr += NumOfBytes;
		Length -= NumOfBytes;
		Page++;
		NumPages--;
		NumOfBytes = (Length > InstancePtr->Geometry.BytesPerPage) ?
				InstancePtr->Geometry.BytesPerPage:Length;
		CopyOffset = 0;
	}

	return XST_SUCCESS;
}

/**************************************************************************/
/**
*
* This function sends a NAND command to the flash device.
*
* @param	InstancePtr is the pointer to XNandPs struture
* @param	Command is the NAND command to send
* @param	Page is the page offset required for specific commands
* @param	Column the column offset required for specific commands
*
* @return	None
*
* @note		None
*
***************************************************************************/
void XNandPs_SendCommand(XNandPs *InstancePtr, XNandPs_CommandFormat
		*Command, int Page, int Column)
{
	u32 EndCmdReq = 0;
	u32 EccLast = 0;
	u32 ClearCs = 0;
	u32 CmdPhaseAddr;
	u32 DataPhaseAddr;
	u32 CmdPhaseData=0;
	u32 PageShift;

	Xil_AssertVoid(Command != NULL);

	if (Command->EndCmdValid == XNANDPS_CMD_PHASE) {
		EndCmdReq = 1;
	}

	if ((Command->StartCmd == ONFI_CMD_READ1) ||
		(Command->StartCmd == ONFI_CMD_PAGE_PROG1)) {
		Command->AddrCycles = InstancePtr->Geometry.RowAddrCycles +
					InstancePtr->Geometry.ColAddrCycles;
	}
	if ((Command->StartCmd == ONFI_CMD_BLOCK_ERASE1)) {
		Command->AddrCycles = InstancePtr->Geometry.RowAddrCycles;
	}

	/*
	 * Construct command phase address
	 */
	CmdPhaseAddr = InstancePtr->Config.FlashBase			|
		(Command->AddrCycles << XNANDPS_ADDR_CYCLES_SHIFT)	|
		(EndCmdReq << XNANDPS_END_CMD_VALID_SHIFT)	|
		XNANDPS_COMMAND_PHASE_MASK			|
		(Command->EndCmd << XNANDPS_END_CMD_SHIFT)	|
		(Command->StartCmd << XNANDPS_START_CMD_SHIFT);

	InstancePtr->CommandPhaseAddr = CmdPhaseAddr;

	EndCmdReq = 0;

	/*
	 * Some NAND commands require end command to be sent after data phase
	 */
	if (Command->EndCmdValid == XNANDPS_DATA_PHASE) {
		EndCmdReq = 1;
	}

	/*
	 * Construct data phase address
	 */
	DataPhaseAddr = InstancePtr->Config.FlashBase			|
			  (ClearCs << XNANDPS_CLEAR_CS_SHIFT)		|
			  (EndCmdReq << XNANDPS_END_CMD_VALID_SHIFT)	|
			  XNANDPS_DATA_PHASE_MASK			|
			  (Command->EndCmd << XNANDPS_END_CMD_SHIFT)	|
			  (EccLast << XNANDPS_ECC_LAST_SHIFT);

	InstancePtr->DataPhaseAddr = DataPhaseAddr;

	/*
	 * Command phase data
	 */
	if (Column != XNANDPS_COLUMN_NOT_VALID && Page !=
			XNANDPS_PAGE_NOT_VALID) {
		if (InstancePtr->Geometry.FlashWidth ==
				XNANDPS_FLASH_WIDTH_16) {
			Column >>= 1;
		}
		CmdPhaseData = Column;
		PageShift = InstancePtr->Geometry.ColAddrCycles * 8;
		CmdPhaseData |= Page << PageShift;
		if (Command->AddrCycles > 4) {
			/*
			 * Send lower bytes of page address in first address
			 * cycle
			 */
			XNandPs_WriteReg(CmdPhaseAddr, CmdPhaseData);
			/*
			 * Send the upper bytes of the page address in second
			 * address cycle
			 */
			CmdPhaseData = Page >> (32 - PageShift);
		}
	} else if (Page != XNANDPS_PAGE_NOT_VALID) {
		CmdPhaseData = Page;
	} else {
		if (InstancePtr->Geometry.FlashWidth ==
				XNANDPS_FLASH_WIDTH_16) {
			Column >>= 1;
		}
		CmdPhaseData = Column;
	}

	/*
	 * Send command phase
	 */
	XNandPs_WriteReg(CmdPhaseAddr, CmdPhaseData);
}

/*****************************************************************************/
/**
*
* This function reads the spare area of a page.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Page is the page number from where spare data is read.
* @param	Buf is pointer to the buffer where the spare data is filled.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None.
*
******************************************************************************/
int XNandPs_ReadSpareBytes(XNandPs *InstancePtr, u32 Page, u8 *Buf)
{
	u32 Col;
	u32 Length;
	u32 DataPhaseAddr;
	u32 ZeroCommand;
	u32 Status;

	Xil_AssertNonvoid(Buf != NULL);

	/*
	 * Bypass the ECC block in the SMC controller since
	 * we don't calculate ECC for spare bytes.
	 */
	if (InstancePtr->EccMode == XNANDPS_ECC_HW) {
		XNandPs_EccDisable(InstancePtr);
	}

	Col = InstancePtr->Geometry.BytesPerPage;
	Length = InstancePtr->Geometry.SpareBytesPerPage;

	XNandPs_SendCommand(InstancePtr, &OnfiCommands[READ], Page, Col);
	/*
	 * Poll the Memory controller status register for BUSY input signal
	 */
	while (XNandPs_IsBusy(InstancePtr) == TRUE) {
	}

	/*
	 * Clear the interrupt condition
	 */
	XNandPs_WriteReg((InstancePtr->Config.SmcBase +
			XNANDPS_MEMC_CLR_CONFIG_OFFSET),
			XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);

	/*
	 * Check ONFI Status Register
	 */
	Status = Onfi_CmdReadStatus(InstancePtr);
	if (Status & ONFI_STATUS_FAIL) {
		return XST_FAILURE;
	}

	/*
	 * ONFI : Reissue the 0x00 on the command line to start
	 * reading data
	 */
	ZeroCommand = InstancePtr->Config.FlashBase |
			(0 << XNANDPS_ADDR_CYCLES_SHIFT)|
			(0 << XNANDPS_END_CMD_VALID_SHIFT)|
			(XNANDPS_COMMAND_PHASE_MASK)|
			(0 << XNANDPS_END_CMD_SHIFT)|
			(0 << XNANDPS_START_CMD_SHIFT);

	/*
	 * AXI transaction for sending command 0x00 to the flash
	 */
	Xil_Out32(ZeroCommand, 0x00);

	/*
	 * Read the spare data
	 */
	XNandPs_ReadBuf(InstancePtr, Buf, (Length - XNANDPS_AXI_DATA_WIDTH));

	/*
	 * Clear chip select for last AXI transaction
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_CLR_CS;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	Buf += (Length - XNANDPS_AXI_DATA_WIDTH);
	XNandPs_ReadBuf(InstancePtr, Buf, XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Re-enable ECC block in the SMC controller
	 */
	if (InstancePtr->EccMode == XNANDPS_ECC_HW) {
		Status = XNandPs_EccHwInit(InstancePtr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function write to the spare area of a page.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Page is the page number to write.
* @param	Buf is pointer to the buffer which holds the data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None.
*
******************************************************************************/
int XNandPs_WriteSpareBytes(XNandPs *InstancePtr, u32 Page, u8 *Buf)
{
	u32 Col;
	u32 Length;
	u32 DataPhaseAddr;
	u32 Status;

	Xil_AssertNonvoid(Buf != NULL);

	/*
	 * Bypass the ECC block in the SMC controller since
	 * we don't calculate ECC for spare bytes.
	 */
	if (InstancePtr->EccMode == XNANDPS_ECC_HW) {
		XNandPs_EccDisable(InstancePtr);
	}

	Col = InstancePtr->Geometry.BytesPerPage;
	Length = InstancePtr->Geometry.SpareBytesPerPage;

	XNandPs_SendCommand(InstancePtr, &OnfiCommands[PAGE_PROGRAM], Page, Col);

	/*
	 * Write to the spare area
	 */
	XNandPs_WriteBuf(InstancePtr, Buf, (Length -
				XNANDPS_AXI_DATA_WIDTH));
	/*
	 * Last transaction clear chip select
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_CLR_CS;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	Buf += (Length - XNANDPS_AXI_DATA_WIDTH);
	XNandPs_WriteBuf(InstancePtr, Buf, XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Poll the Memory controller status register for BUSY input signal
	 */
	while (XNandPs_IsBusy(InstancePtr) == TRUE) {
	}

	/*
	 * Clear the interrupt condition
	 */
	XNandPs_WriteReg((InstancePtr->Config.SmcBase +
			XNANDPS_MEMC_CLR_CONFIG_OFFSET),
			XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);
	/*
	 * Check SR[0] bit
	 */
	Status = Onfi_CmdReadStatus(InstancePtr);
	if (Status & ONFI_STATUS_FAIL) {
		return XST_FAILURE;
	}

	/*
	 * Re-enable ECC block in the SMC controller
	 */
	if (InstancePtr->EccMode == XNANDPS_ECC_HW) {
		Status = XNandPs_EccHwInit(InstancePtr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks whether SMC controller busy in processing a request.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return	- TRUE if SMC is busy
*		- FALSE if SMC is free
*
* @note		None.
*
******************************************************************************/
static int XNandPs_IsBusy(XNandPs *InstancePtr)
{
	u32 Status;

	/*
	 * Read the memory controller status register
	 */
	Status = XNandPs_ReadReg(InstancePtr->Config.SmcBase +
			XNANDPS_MEMC_STATUS_OFFSET) &
		XNANDPS_MEMC_STATUS_RAW_INT_STATUS1_MASK;

	if (Status) {
		return FALSE;
	} else {
		return TRUE;
	}
}
/*****************************************************************************/
/**
*
* This function calculates the ECC value from the ECC registers.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	EccData is the buffer to fill the ECC value.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None
*
******************************************************************************/
static int XNandPs_EccCalculate(XNandPs *InstancePtr, u8 *EccData)
{
	u32 EccReg;
	u32 EccValue;
	u32 EccByte;
	u32 EccStatus;

	/*
	 * Check the busy status of the ECC block
	 */
	while (XNandPs_ReadReg(InstancePtr->Config.SmcBase +
		XNANDPS_ECC_STATUS_OFFSET(XNANDPS_IF1_ECC_OFFSET)) &
		XNANDPS_ECC_STATUS_MASK);

	for(EccReg = 0; EccReg < 4; EccReg++) {

		EccValue = XNandPs_ReadReg(InstancePtr->Config.SmcBase +
			XNANDPS_ECC_VALUE0_OFFSET(XNANDPS_IF1_ECC_OFFSET +
				(EccReg * 4)));
		EccStatus = (EccValue >> 24) & 0xFF;

		/*
		 * Check if the ECC value not valid
		 */
		if ((EccStatus >> 6) & 0x1) {
			for(EccByte = 0; EccByte < 3; EccByte++) {
				*EccData = EccValue & 0xFF;
				EccValue = EccValue >> 8;
				EccData++;
			}
		} else {
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function corrects the ECC errors.
*
* @param	Buf is the buffer which holds the data read from the page.
* @param	EccCalc is the calculated ECC value.
* @param	EccCode is the ECC read from the spare area.
*
* @return
*		- XST_SUCCESS if the ECC error is corrected.
*		- XST_FAILURE if the ECC error is not corrected.
*
* @note		None
*
******************************************************************************/
static int XNandPs_EccCorrect(u8 *Buf, u8 *EccCalc, u8 *EccCode)
{
	u8 BitPos;
	u32 BytePos;
	u16 EccOdd, EccEven;
	u16 ReadEccLow, ReadEccHigh;
	u16 CalcEccLow, CalcEccHigh;

	/*
	 * Lower 12 bits of ECC Read
	 */
	ReadEccLow = (EccCode[0] | (EccCode[1] << 8)) & 0xfff;
	/*
	 * Upper 12 bits of ECC Read
	 */
	ReadEccHigh = ((EccCode[1] >> 4) | (EccCode[2] << 4)) & 0xfff;

	/*
	 * Lower 12 bits of ECC calculated
	 */
	CalcEccLow = (EccCalc[0] | (EccCalc[1] << 8)) & 0xfff;
	/*
	 * Upper 12 bits of ECC Calculated
	 */
	CalcEccHigh = ((EccCalc[1] >> 4) | (EccCalc[2] << 4)) & 0xfff;

	EccOdd = ReadEccLow ^ CalcEccLow;
	EccEven = ReadEccHigh ^ CalcEccHigh;

	/*
	 * No Error
	 */
	if ((EccOdd == 0) && (EccEven == 0)) {
		return XST_SUCCESS;
	}

	/*
	 * Single bit error, correct it
	 */
	if (EccOdd == (~EccEven & 0xfff)) {
		BytePos = (EccOdd >> 3) & XNANDPS_ECC_CORRECT_BYTE_MASK;
		BitPos = EccOdd & XNANDPS_ECC_CORRECT_BIT_MASK;
		/*
		 * Toggling error bit
		 */
		Buf[BytePos] ^= (1 << BitPos);
		return XST_SUCCESS;
	}

	/*
	 * Parity error
	 */
	if (OneHot((EccOdd | EccEven)) == XST_SUCCESS) {
		return XST_SUCCESS;
	}

	/*
	 * Multiple bit errors
	 */
	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function reads a specific page from NAND device using HW ECC block.
* It checks for the ECC errors and corrects single bit errors. The multiple bit
* error are reported as failure.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	DstPtr is a pointer to the destination buffer.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None
*
******************************************************************************/
static int XNandPs_ReadPage_HwEcc(XNandPs *InstancePtr, u8 *DstPtr)
{
	u32 Status;
	u32 BytesPerPage;
	u32 SpareBytesPerPage;
	u32 EccSteps;
	u32 EccOffset;
	u32 DataPhaseAddr;
	u32 Index;
	u32 *EccPos;
	u8 *EccCode;
	u8 *EccCalc;
	u8 *Ptr = DstPtr;
	u8 *SparePtr = InstancePtr->SpareBufPtr;

	BytesPerPage = InstancePtr->Geometry.BytesPerPage;
	SpareBytesPerPage = InstancePtr->Geometry.SpareBytesPerPage;

	EccSteps = InstancePtr->EccConfig.NumSteps;
	EccCode = &InstancePtr->EccCode[0];
	EccCalc = &InstancePtr->EccCalc[0];

	/*
	 * Read page sized bytes in one less AXI data width
	 */
	XNandPs_ReadBuf(InstancePtr, Ptr,
				(BytesPerPage - XNANDPS_AXI_DATA_WIDTH));

	Ptr += (BytesPerPage - XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Set the ECC Last bit
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_ECC_LAST;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;

	/*
	 * Read transaction with ECC enabled
	 */
	XNandPs_ReadBuf(InstancePtr, Ptr, XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Calculate the hardware ECC
	 */
	Ptr = DstPtr;
	Status = XNandPs_EccCalculate(InstancePtr, EccCalc);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr &= ~XNANDPS_ECC_LAST;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	XNandPs_ReadBuf(InstancePtr, SparePtr,
				(SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH));

	/*
	 * Clear chip select for last AXI transaction
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_CLR_CS;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	SparePtr += (SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH);
	XNandPs_ReadBuf(InstancePtr, SparePtr, XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Read the stored ECC code
	 */
	EccPos = &InstancePtr->EccConfig.EccPos[0];
	for(Index = 0; Index < InstancePtr->EccConfig.TotalBytes; Index++) {
		EccCode[Index] = ~(InstancePtr->SpareBufPtr[EccPos[Index]]);
	}

	/*
	 * Check for ECC errors
	 */
	EccOffset = 0;
	for(; EccSteps; EccSteps--) {
		Status = XNandPs_EccCorrect(DstPtr,
				&EccCalc[EccOffset],&EccCode[EccOffset]);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		DstPtr += InstancePtr->EccConfig.BlockSize;
		EccOffset += InstancePtr->EccConfig.BytesPerBlock;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads a specific page from NAND device. This doesn't use the
* HW ECC block for checking ECC errors.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	DstPtr is a pointer to the destination buffer.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None
*
******************************************************************************/
static int XNandPs_ReadPage(XNandPs *InstancePtr, u8 *DstPtr)
{
	u32 Status;
	u32 ZeroCommand;
	u32 BytesPerPage;
	u16 SpareBytesPerPage;
	u32 DataPhaseAddr;
	u8 *Ptr = DstPtr;
	u8 *SparePtr = InstancePtr->SpareBufPtr;

	BytesPerPage = InstancePtr->Geometry.BytesPerPage;
	SpareBytesPerPage = InstancePtr->Geometry.SpareBytesPerPage;

	/*
	 * Check ONFI Status Register
	 */
	Status = Onfi_CmdReadStatus(InstancePtr);
	if (Status & ONFI_STATUS_FAIL) {
		return XST_FAILURE;
	}

	/*
	 * ONFI : Reissue the 0x00 on the command line to start
	 * reading data
	 */
	ZeroCommand = InstancePtr->Config.FlashBase |
			(0 << XNANDPS_ADDR_CYCLES_SHIFT)|
			(0 << XNANDPS_END_CMD_VALID_SHIFT)|
			(XNANDPS_COMMAND_PHASE_MASK)|
			(0 << XNANDPS_END_CMD_SHIFT)|
			(0 << XNANDPS_START_CMD_SHIFT);

	/*
	 * AXI transaction for sending command 0x00 to the flash
	 */
	Xil_Out32(ZeroCommand, 0x00);

	/*
	 * Read page data
	 */
	XNandPs_ReadBuf(InstancePtr, Ptr, BytesPerPage);

	/*
	 * Read spare bytes in one less AXI data width
	 */
	XNandPs_ReadBuf(InstancePtr, SparePtr,
			(SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH));

	/*
	 * Clear chip select for last AXI transaction
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_CLR_CS;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	SparePtr += (SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH);
	XNandPs_ReadBuf(InstancePtr, SparePtr, XNANDPS_AXI_DATA_WIDTH);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes a specific page in the NAND device using HW ECC block.
* The ECC code is written into the spare bytes of the page.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	SrcPtr is a pointer to the source buffer.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None
*
******************************************************************************/
static int XNandPs_WritePage_HwEcc(XNandPs *InstancePtr, u8 *SrcPtr)
{
	u32 Status;
	u32 BytesPerPage;
	u32 SpareBytesPerPage;
	u32 DataPhaseAddr;
	u32 Index;
	u32 *EccPos;
	u8 *EccCalc;
	u8 *Ptr = SrcPtr;
	u8 *SparePtr = InstancePtr->SpareBufPtr;

	BytesPerPage = InstancePtr->Geometry.BytesPerPage;
	SpareBytesPerPage = InstancePtr->Geometry.SpareBytesPerPage;

	EccCalc = &InstancePtr->EccCalc[0];

	/*
	 * Transfer page sized bytes in one less AXI data width
	 */
	XNandPs_WriteBuf(InstancePtr, Ptr,
				(BytesPerPage - XNANDPS_AXI_DATA_WIDTH));

	Ptr += (BytesPerPage - XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Last page transaction with ECC set
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_ECC_LAST;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	XNandPs_WriteBuf(InstancePtr, Ptr, XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Calculate the ECC
	 */
	Ptr = SrcPtr;
	Status = XNandPs_EccCalculate(InstancePtr, EccCalc);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Fill the Spare buffer with calculated ECC
	 */
	EccPos = &InstancePtr->EccConfig.EccPos[0];
	for(Index = 0; Index < InstancePtr->EccConfig.TotalBytes; Index++) {
		InstancePtr->SpareBufPtr[EccPos[Index]] = ~(EccCalc[Index]);
	}

	/*
	 * Write the spare area with the ECC
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr &= ~XNANDPS_ECC_LAST;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	XNandPs_WriteBuf(InstancePtr, SparePtr,
				(SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH));

	/*
	 * Clear chip select for last AXI transaction
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_CLR_CS;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	SparePtr += (SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH);
	XNandPs_WriteBuf(InstancePtr, SparePtr, XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Poll the Memory controller status register
	 */
	while (XNandPs_IsBusy(InstancePtr) == TRUE) {
	}

	/*
	 * Clear the interrupt condition
	 */
	XNandPs_WriteReg((InstancePtr->Config.SmcBase +
				XNANDPS_MEMC_CLR_CONFIG_OFFSET),
				XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);

	/*
	 * Check SR[0] bit
	 */
	Status = Onfi_CmdReadStatus(InstancePtr);
	if (Status & ONFI_STATUS_FAIL) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes a specific page in the NAND device. This doesn't use the
* HW ECC block for ECC code generation.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	SrcPtr is a pointer to the source buffer.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None
*
******************************************************************************/
static int XNandPs_WritePage(XNandPs *InstancePtr, u8 *SrcPtr)
{
	u32 Status;
	u32 DataPhaseAddr;
	u32 BytesPerPage;
	u16 SpareBytesPerPage;
	u8 *Ptr = SrcPtr;
	u8 *SparePtr = InstancePtr->SpareBufPtr;

	BytesPerPage = InstancePtr->Geometry.BytesPerPage;
	SpareBytesPerPage = InstancePtr->Geometry.SpareBytesPerPage;

	/*
	 * Transfer page sized bytes
	 */
	XNandPs_WriteBuf(InstancePtr, Ptr, BytesPerPage);

	/*
	 * Write the spare data bytes
	 */
	XNandPs_WriteBuf(InstancePtr, SparePtr,
			(SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH));

	/*
	 * Clear chip select for last AXI transaction
	 */
	DataPhaseAddr = InstancePtr->DataPhaseAddr;
	DataPhaseAddr |= XNANDPS_CLR_CS;
	InstancePtr->DataPhaseAddr = DataPhaseAddr;
	SparePtr += (SpareBytesPerPage - XNANDPS_AXI_DATA_WIDTH);
	XNandPs_WriteBuf(InstancePtr, SparePtr, XNANDPS_AXI_DATA_WIDTH);

	/*
	 * Poll the Memory controller status register
	 */
	while (XNandPs_IsBusy(InstancePtr) == TRUE) {
	}

	/*
	 * Clear the interrupt condition
	 */
	XNandPs_WriteReg((InstancePtr->Config.SmcBase +
				XNANDPS_MEMC_CLR_CONFIG_OFFSET),
				XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);

	/*
	 * Check SR[0] bit
	 */
	Status = Onfi_CmdReadStatus(InstancePtr);
	if (Status & ONFI_STATUS_FAIL) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases a specific block in the NAND device.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	BlockNum is the block number of the device.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*		- XST_NAND_WRITE_PROTECTED if the flash is write
*			protected.
*
* @note		None
*
******************************************************************************/
int XNandPs_EraseBlock(XNandPs *InstancePtr, u32 BlockNum)
{
	u8 OnfiStatus;
	u32 Status;
	u32 Page;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(BlockNum < InstancePtr->Geometry.NumBlocks);

	/*
	 * Check if the flash is write protected
	 */
	OnfiStatus = Onfi_CmdReadStatus(InstancePtr);
	if (!(OnfiStatus & ONFI_STATUS_WP)) {
		return XST_NAND_WRITE_PROTECTED;
	}

	Page = BlockNum * InstancePtr->Geometry.PagesPerBlock;
	XNandPs_SendCommand(InstancePtr, &OnfiCommands[BLOCK_ERASE], Page,
			XNANDPS_COLUMN_NOT_VALID);

	/*
	 * Poll the Memory controller status register
	 */
	while (XNandPs_IsBusy(InstancePtr) == TRUE) {
	}

	/*
	 * Clear the interrupt condition
	 */
	XNandPs_WriteReg((InstancePtr->Config.SmcBase +
				XNANDPS_MEMC_CLR_CONFIG_OFFSET),
			XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK);

	/*
	 * Check the SR[0] whether the erase operation is successful or not
	 */
	Status = Onfi_CmdReadStatus(InstancePtr);
	if (Status & ONFI_STATUS_FAIL) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function reads the page data from the AXI Data Phase Address.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Buf is the buffer pointer to store the byte.
* @param	Length is the number of bytes to read.
*
* @return
*		- None.
*
******************************************************************************/
static void XNandPs_ReadBuf(XNandPs *InstancePtr, u8 *Buf, u32 Length)
{
	u32 Index;
	u32 AxiLen = Length >> 2;
	u32 *Ptr = (u32 *)Buf;

	for(Index = 0; Index < AxiLen; Index++) {
		Ptr[Index] = XNandPs_ReadReg(InstancePtr->DataPhaseAddr);
	}
}

/*****************************************************************************/
/**
*
* This function writes the data to the AXI Data Phase Address.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Buf is the buffer pointer to write the data from.
* @param	Length is the number of bytes to write.
*
* @return
*		- None.
*
******************************************************************************/
static void XNandPs_WriteBuf(XNandPs *InstancePtr, u8 *Buf, u32 Length)
{
	u32 Index;
	u32 AxiLen = Length >> 2;
	u32 *Ptr = (u32 *)Buf;

	for(Index = 0; Index < AxiLen; Index++) {
		XNandPs_WriteReg(InstancePtr->DataPhaseAddr, Ptr[Index]);
	}
}
/** @} */
