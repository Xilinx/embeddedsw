/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xilflash_intel.c
*
* This file implements the Intel CFI Version of the XFlash Library.
*
* @note
*
* - Special consideration has to be given to varying data bus widths. To boost
*	performance, multiple devices in parallel on the data bus are accessed
*	in parallel. Therefore to reduce complexity and increase performance,
*	many local primitive functions are duplicated with the only difference
*	being the width of writes to the devices.
*   <br><br>
*	Even with the performance boosting optimizations, the overhead
*	associated is rather high due to the general purpose nature of its
*	design.
*   <br><br>
*	Flash block erasing is a time consuming operation with nearly all
*	latency occurring due to the devices' themselves. It takes on the order
*	of 1 second to erase each block.
*   <br><br>
*	 Writes by comparison are much quicker so library overhead becomes an
*	issue.
*	The write algorithm has been optimized for bulk data programming and
*	should provide relatively better performance.
* - The code/comments refers to WSM frequently. This stands for Write State
*	Machine. WSM is the internal programming engine of the devices.
* - This library and the underlying Intel flash memory does not allow re-
*	programming while code is executing from the same memory.
* - If hardware is flakey or fails, then this library could hang a thread of
*	execution.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  10/25/07 First release
* 1.00a mta  10/25/07 Updated to flash library
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 1.02a ksu  06/16/09 Added support for multiple banks in Intel flash
*                     Added Reset Bank function
*                     Added support for 0xF0 reset command
*                     Added support for Xilinx Platform Flash XL.
*                     Added XFL_DEVCTL_SET_CONFIG_REG IOCTL to write to the
*                     Configuration Register of the Xilinx Platform Flash XL
*                     which can be used to set the Flash in Sync/Async mode.
*                     The Xilinx Platform Flash XL is set to Async mode during
*                     the initialization of the library.
*                     Added bank(s) reset function at the top of the read
*                     function.
*                     Updated Lock and Unlock operations for multiple blocks.
* 1.03a ksu  10/07/09 Added support for large buffer size flash (CR535564)
* 3.01a srt  03/02/12 Added support for Micron G18 Flash device to fix
*		      CRs 648372, 648282.
*		      Modified XFlashIntel_Reset function to reset all the
*		      partitions.
* 3.02a srt  05/30/12 Changed Implementation for Micron G18 Flash, which
*		      fixes the CR 662317.
*		      CR 662317 Description - Xilinx Platform Flash on ML605
*		      fails to work.
* 3.03a srt  11/04/12 Fixed CR 679937 -
*		      Description: Non-word aligned data write to flash fails
*		      with AXI interface.
* 4.1	nsk  08/06/15 Fixed CR 835008.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilflash.h"

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
#include "include/xilflash_intel.h"
#include "include/xilflash_cfi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*
 * Define a single type used to access the status register irregardless of the
 * width of the devices
 */
typedef union {
	u8 Mask8;
	u16 Mask16;
	u32 Mask32;
	Xuint64 Mask64;
} StatReg;

/*
 * Define Intel specific data to be part of the instance. This structure will
 * overlay the XFlash_PartData structure attribute of the base class.
 */
typedef struct XFlashVendorData_IntelTag {
	u32 WriteBufferWordCount;	/* This value is written to the WSM when
					   telling it how many words will be
					   programmed (always will be the
					   maximum
					   allowed) */
	StatReg SR_WsmReady; /* Status register bitmask for WSM ready */
	StatReg SR_LastError; /* Status register bitmask for error condition */

	/*
	 * The following functions are specific to the width of the data bus and
	 * will be assigned during initialization.
	 *
	 * SendCmd - Writes a single command to the devices.
	 * SendCmdSeq - Writes two commands in successive bus cycles to the
	 *		devices.
	 * WriteBuffer - Programming algorithm optimized to perform bulk writes
	 *		to devices.
	 * GetStatus - Retrieve and interpret the status registers of the
	 *		devices
	 * PollSR - Poll the status register of the devices until the WSM
	 *		signals ready.
	 */
	void (*SendCmd) (u32 BaseAddr, u32 Offset, u32 Cmd);
	void (*SendCmdSeq) (u32 BaseAddr, u32 Offset, u32 Cmd1, u32 Cmd2);

	int (*WriteBuffer) (XFlash *InstancePtr, void *DestPtr,
			    void *SrcPtr, u32 Bytes);

	int (*GetStatus) (XFlash *InstancePtr, u32 Offset);
	int (*PollSR) (XFlash *InstancePtr, u32 Offset);

} XFlashVendorData_Intel;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************
* GET_PARTDATA - Type safe way to convert the base component VendorData type
*		to the family specific VendorData_Intel type.
*
* Macro signature:
*
*	XFlashVendorData_Intel *GET_PARTDATA(XFlash_PartData *BaseComponent)
*****************************************************************************/
#define GET_PARTDATA(BaseComponent) \
	((XFlashVendorData_Intel*)&((BaseComponent)->VendorData))

/************************** Function Prototypes ******************************/

static void SendCmd8(u32 BaseAddr, u32 Offset, u32 Cmd);
static void SendCmd16(u32 BaseAddr, u32 Offset, u32 Cmd);
static void SendCmd32(u32 BaseAddr, u32 Offset, u32 Cmd);
static void SendCmd64(u32 BaseAddr, u32 Offset, u32 Cmd);

static void SendCmdSeq8(u32 BaseAddr, u32 Offset, u32 Cmd1, u32 Cmd2);
static void SendCmdSeq16(u32 BaseAddr, u32 Offset, u32 Cmd1, u32 Cmd2);
static void SendCmdSeq32(u32 BaseAddr, u32 Offset, u32 Cmd1, u32 Cmd2);
static void SendCmdSeq64(u32 BaseAddr, u32 Offset, u32 Cmd1, u32 Cmd2);

static int GetStatus8(XFlash *InstancePtr, u32 Offset);
static int GetStatus16(XFlash *InstancePtr, u32 Offset);
static int GetStatus32(XFlash *InstancePtr, u32 Offset);
static int GetStatus64(XFlash *InstancePtr, u32 Offset);

static int WriteBuffer8(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);
static int WriteBuffer16(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);
static int WriteBuffer32(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);
static int WriteBuffer64(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);

static int PollSR8(XFlash *InstancePtr, u32 Offset);
static int PollSR16(XFlash *InstancePtr, u32 Offset);
static int PollSR32(XFlash *InstancePtr, u32 Offset);
static int PollSR64(XFlash *InstancePtr, u32 Offset);

static int SetRYBY(XFlash *InstancePtr, u32 Mode);
static void GetPartID(XFlash *InstancePtr);
static int XFlashIntel_ResetBank(XFlash *InstancePtr, u32 Offset, u32 Bytes);
static u16 EnqueueEraseBlocks(XFlash *InstancePtr, u16 *RegionPtr,
				u16 *BlockPtr, u16 MaxBlocks);

extern int XFlashGeometry_ToBlock(XFlashGeometry *InstancePtr,
				u32 AbsoluteOffset,
				u16 *Region, u16 *Block, u32 *BlockOffset);
extern int XFlashGeometry_ToAbsolute(XFlashGeometry *InstancePtr,
				u16 Region,
				u16 Block,
				u32 BlockOffset, u32 *AbsoluteOffsetPtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initializes a XFlash instance with device family specific details. The
* initialization entails:
*
* - Assign part access primitive functions depending on bus width.
* - Reset the device.
*
* @param	InstancePtr is a pointer to the XFlash instance.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XFLASH_PART_NOT_SUPPORTED if the command set algorithm or
*		  Layout is not supported by any specific flash device family
*		  compiled into the system.
*		- XST_FAILURE if error.
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_Initialize(XFlash *InstancePtr)
{
	XFlashVendorData_Intel *DevDataPtr;
	u32 Layout;
	int BusWidthBytes;

	/*
	 * Verify inputs are valid
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Grab layout and get Vendor specific part information
	 */
	Layout = InstancePtr->Geometry.MemoryLayout;
	DevDataPtr = GET_PARTDATA(InstancePtr);

	/*
	 * Setup alignment of the write buffer.
	 */
	if (InstancePtr->Properties.ProgCap.WriteBufferSize != 0) {
		InstancePtr->Properties.ProgCap.WriteBufferAlignmentMask =
			InstancePtr->Properties.ProgCap.WriteBufferSize - 1;
	}


	/*
	 * Setup layout dependent attributes. These include:
	 *	- Part access primitive functions optimized for a specific bus
	 *	  width.
	 *	- SR_WsmRead.MaskX is set according to the SR_WSM_READY constant
	 *	  defined as 0x80. We don't use that constant here because it is
	 *	  simpler to define with a magic number rather than use complex
	 *	  masks and shifting operations.
	 */
	switch (Layout) {

		case XFL_LAYOUT_X16_X8_X1:
			DevDataPtr->SR_WsmReady.Mask8 = 0x80;
			DevDataPtr->GetStatus = GetStatus8;
			DevDataPtr->SendCmd = SendCmd8;
			DevDataPtr->SendCmdSeq = SendCmdSeq8;
			DevDataPtr->PollSR = PollSR8;
			DevDataPtr->WriteBuffer = WriteBuffer8;
			break;

		case XFL_LAYOUT_X16_X16_X1:
			DevDataPtr->SR_WsmReady.Mask16 = 0x0080;
			DevDataPtr->GetStatus = GetStatus16;
			DevDataPtr->SendCmd = SendCmd16;
			DevDataPtr->SendCmdSeq = SendCmdSeq16;
			DevDataPtr->PollSR = PollSR16;
			DevDataPtr->WriteBuffer = WriteBuffer16;
			break;

		case XFL_LAYOUT_X16_X16_X2:
			DevDataPtr->SR_WsmReady.Mask32 = 0x00800080;
			DevDataPtr->GetStatus = GetStatus32;
			DevDataPtr->SendCmd = SendCmd32;
			DevDataPtr->SendCmdSeq = SendCmdSeq32;
			DevDataPtr->PollSR = PollSR32;
			DevDataPtr->WriteBuffer = WriteBuffer32;
			break;

		case XFL_LAYOUT_X16_X16_X4:
			DevDataPtr->SR_WsmReady.Mask32 = 0x00800080;
			DevDataPtr->GetStatus = GetStatus64;
			DevDataPtr->SendCmd = SendCmd64;
			DevDataPtr->SendCmdSeq = SendCmdSeq64;
			DevDataPtr->PollSR = PollSR64;
			DevDataPtr->WriteBuffer = WriteBuffer64;
			break;

		default:
			return (XFLASH_PART_NOT_SUPPORTED);
	}

	/*
	 * Calculate data written during a buffer write that tells the buffer
	 * how many words are going to be written to the buffer. This value is
	 * based on ProgCap.WriteBufferSize which was taken from the standard
	 * CFI query at	offset 2Ah. Dividing this value by the width of the data
	 * bus gives us the maximum number of writes to the buffer per Intel
	 * literature.
	 */
	BusWidthBytes = (Layout & XFL_LAYOUT_NUM_PARTS_MASK) *
			((Layout & XFL_LAYOUT_PART_MODE_MASK) >> 8);
	DevDataPtr->WriteBufferWordCount =
			(InstancePtr->Properties.ProgCap.WriteBufferSize /
							BusWidthBytes) - 1;

	/*
	 * Get part ID.
	 */
	GetPartID(InstancePtr);

	/*
	 * Reset the part.
	 */
	(void) XFlashIntel_Reset(InstancePtr);

	/*
	 * Zero out error data and return.
	 */
	XUINT64_MSW(DevDataPtr->SR_LastError.Mask64) = 0;
	XUINT64_LSW(DevDataPtr->SR_LastError.Mask64) = 0;

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* The routine reads the data from the Intel flash device and copies it into
* user buffer.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset into the device(s) address space from
*		which to read.
* @param	Bytes is the number of bytes to copy.
* @param	DestPtr is the destination address to copy data to.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ADDRESS_ERROR if the source address does not start
*		  within the addressable areas of the device(s).
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_Read(XFlash *InstancePtr, u32 Offset, u32 Bytes, void *DestPtr)
{
	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if(DestPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Check to make sure start address is within the device.
	 */
	if (!XFL_GEOMETRY_IS_ABSOLUTE_VALID(&InstancePtr->Geometry, Offset)) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	if (XFlashIntel_ResetBank(InstancePtr, Offset, Bytes)!= XST_SUCCESS) {
		return (XST_FAILURE);
	}

	/*
	 * Perform copy to the user buffer from the buffer.
	 */
	memcpy(DestPtr,	(void *) (InstancePtr->Geometry.BaseAddress + Offset),
									Bytes);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Programs the Intel flash device with data stored in the user buffer.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset into the device(s) address space from which
*		to begin programming. Must be aligned to the width of the
*		flash's data bus.
* @param	Bytes is the number of bytes to program.
* @param	SrcPtr is the source address containing data to be programmed.
*		Must be aligned to the width of the flash's data bus.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific. Use XFlash_DeviceControl() to
*		  retrieve specific error conditions. When this error is
*		  returned, it is possible that the target address range was
*		  only partially programmed.
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_Write(XFlash *InstancePtr, u32 Offset, u32 Bytes, void *SrcPtr)
{
	XFlashVendorData_Intel *DevDataPtr;
	int Status;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if(SrcPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Nothing specified to be programmed.
	 */
	if (Bytes == 0) {
		return (XST_SUCCESS);
	}

	/*
	 * Verify the address range is within the part.
	 */
	if (!XFL_GEOMETRY_IS_ABSOLUTE_VALID(&InstancePtr->Geometry, Offset) ||
		!XFL_GEOMETRY_IS_ABSOLUTE_VALID(&InstancePtr->Geometry,
						Offset + Bytes - 1)) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Call the proper write buffer function.
	 */
	DevDataPtr = GET_PARTDATA(InstancePtr);
	Status = DevDataPtr->WriteBuffer(InstancePtr, (void *)
					 (InstancePtr->Geometry.BaseAddress +
					  Offset), SrcPtr, Bytes);

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	(void) XFlashIntel_ResetBank(InstancePtr, Offset, Bytes);

	return (Status);
}

/*****************************************************************************/
/**
*
* Erases the specified address range in the Intel Flash device.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset into the device(s) address space from which
*		to begin erasure.
* @param	Bytes is the number of bytes to erase.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ADDRESS_ERROR if the destination address range is
*		  not completely within the addressable areas of the device(s).
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_Erase(XFlash *InstancePtr, u32 Offset, u32 Bytes)
{
	u16 StartRegion, EndRegion;
	u16 StartBlock, EndBlock;
	u16 BlocksLeft, BlocksQueued;
	u32 Dummy;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Intel *DevDataPtr;
	int Status;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	GeomPtr = &InstancePtr->Geometry;

	/*
	 * Handle case when zero bytes is provided.
	 */
	if (Bytes == 0) {
		return (XST_SUCCESS);
	}

	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset, &StartRegion,
					&StartBlock, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Convert the ending address to block coordinates. This also verifies
	 * the ending address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset + Bytes - 1,
					&EndRegion, &EndBlock, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Erase loop. Queue up as many blocks at a time until all are erased.
	 */
	DevDataPtr = GET_PARTDATA(InstancePtr);
	BlocksLeft = XFL_GEOMETRY_BLOCK_DIFF(GeomPtr, StartRegion, StartBlock,
					     EndRegion, EndBlock);

	while (BlocksLeft > 0) {
		BlocksQueued = EnqueueEraseBlocks(InstancePtr, &StartRegion,
						  &StartBlock, BlocksLeft);
		BlocksLeft -= BlocksQueued;

		Status = DevDataPtr->PollSR(InstancePtr, Offset);
		if (Status != XFLASH_READY) {
			(void) XFlashIntel_ResetBank(InstancePtr, Offset,
						     Bytes);
			return (Status);
		}
	}

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	(void) XFlashIntel_ResetBank(InstancePtr, Offset, Bytes);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Locks the blocks in the specified range of the Intel flash device.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset into the device(s) address space from which
*		to begin block locking. The first three bytes of every block is
*		reserved for special purpose. The offset should be atleast three
*		bytes from start of the block.
* @param	Bytes indicates the number of bytes to Lock in the Block
*		starting from Offset.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XFLASH_ADDRESS_ERROR if the destination address range is
*		  not completely within the addressable areas of the device(s).
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_Lock(XFlash *InstancePtr, u32 Offset, u32 Bytes)
{
	volatile u16 StatusReg;
	XFlashVendorData_Intel *DevDataPtr;
	u16 StartRegion, EndRegion;
	u16 StartBlock, EndBlock;
	u16 Region, Block;
	u16 BlocksLeft;
	u32 Dummy;
	u32 BaseAddress;
	u32 BlockAddress;
	int Status;
	XFlashGeometry *GeomPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	GeomPtr = &InstancePtr->Geometry;
	BaseAddress = GeomPtr->BaseAddress;

	/*
	 * Handle case when zero bytes is provided.
	 */
	if (Bytes == 0) {
		/*
		 * Lock 1 block (for backward compatibility)
		 */
		Bytes = 1;
	}

	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset, &StartRegion,
					&StartBlock, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Convert the ending address to block coordinates. This also verifies
	 * the ending address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset + Bytes - 1,
					&EndRegion, &EndBlock, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Find total number of blocks to be locked.
	 */
	DevDataPtr = GET_PARTDATA(InstancePtr);
	BlocksLeft = XFL_GEOMETRY_BLOCK_DIFF(GeomPtr, StartRegion, StartBlock,
					     EndRegion, EndBlock);

	/*
	 * Sample the register(s). Wait till the device is ready.
	 */
	DevDataPtr->SendCmd(BaseAddress, Offset, XFL_INTEL_CMD_READ_STATUS_REG);
	StatusReg = READ_FLASH_16(BaseAddress + Offset);
	while(!(StatusReg & XFL_INTEL_SR_WSM_READY)) {
		StatusReg = READ_FLASH_8(BaseAddress + Offset);
	}

	Region = StartRegion;
	Block = StartBlock;

	while (BlocksLeft > 0) {
		/*
		 * Find the block address.
		 */
		(void) XFlashGeometry_ToAbsolute(GeomPtr, Region, Block, 0,
						 &BlockAddress);

		/*
		 * Clear any latched status register content.
		 */
		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_CLEAR_STATUS_REG);

		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_LOCK_BLOCK_SET);
		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_LOCK_BLOCK_SET_CONFIRM);
		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_READ_STATUS_REG);
		/*
		 * Check if the Locking is done and device is ready.
		 */
		StatusReg = READ_FLASH_16(BaseAddress + BlockAddress);
		while(!(StatusReg & XFL_INTEL_SR_WSM_READY)) {
			StatusReg = READ_FLASH_16(BaseAddress + BlockAddress);
		}

		/*
		 * Check if any Lock error has occurred.
		 */
		if(StatusReg & XFL_INTEL_SR_PROG_OR_LOCK_ERROR) {
			return XST_FAILURE;
		}

		/*
		 * Increment Region/Block.
		 */
		XFL_GEOMETRY_INCREMENT(GeomPtr, Region, Block);

		BlocksLeft--;
	}

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	(void) XFlashIntel_ResetBank(InstancePtr, Offset, Bytes);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Unlocks the blocks in the specified range of the Intel flash device.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset into the device(s) address space from which
*		to begin block UnLocking. The first three bytes of every block
*		is reserved for special purpose. The offset should be atleast
*		three bytes from start of the block.
* @param	Bytes indicates the number of bytes to UnLock in the Block
*		starting from Offset.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ADDRESS_ERROR if the destination address range is
*		  not completely within the addressable areas of the device(s).
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_Unlock(XFlash *InstancePtr, u32 Offset, u32 Bytes)
{

	volatile u16 StatusReg;
	XFlashVendorData_Intel *DevDataPtr;
	u16 StartRegion, EndRegion;
	u16 StartBlock, EndBlock;
	u16 Region, Block;
	u16 BlocksLeft;
	u32 Dummy;
	u32 BaseAddress;
	u32 BlockAddress;
	int Status;
	XFlashGeometry *GeomPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	GeomPtr = &InstancePtr->Geometry;
	BaseAddress = GeomPtr->BaseAddress;

	/*
	 * Handle case when zero bytes is provided.
	 */
	if (Bytes == 0) {
		/*
		 * Unlock 1 block (for backward compatibility)
		 */
		Bytes = 1;
	}

	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset, &StartRegion,
							&StartBlock, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Convert the ending address to block coordinates. This also verifies
	 * the ending address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset + Bytes - 1,
						&EndRegion, &EndBlock, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Find total number of blocks to be unlocked.
	 */
	DevDataPtr = GET_PARTDATA(InstancePtr);
	BlocksLeft = XFL_GEOMETRY_BLOCK_DIFF(GeomPtr, StartRegion, StartBlock,
					     EndRegion, EndBlock);

	/*
	 * Sample the register(s). Wait till the device is ready.
	 */
	DevDataPtr->SendCmd(BaseAddress, Offset, XFL_INTEL_CMD_READ_STATUS_REG);
	StatusReg = READ_FLASH_16(BaseAddress + Offset);
	while(!(StatusReg & XFL_INTEL_SR_WSM_READY)) {
		StatusReg = READ_FLASH_8(BaseAddress + Offset);
	}

	Region = StartRegion;
	Block = StartBlock;

	while (BlocksLeft > 0) {
		/*
		 * Find the block address.
		 */
		(void) XFlashGeometry_ToAbsolute(GeomPtr, Region, Block, 0,
						 &BlockAddress);

		/*
		 * Clear any latched status register content.
		 */
		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_CLEAR_STATUS_REG);

		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_LOCK_BLOCK_CLEAR);
		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_LOCK_BLOCK_CLEAR_CONFIRM);
		DevDataPtr->SendCmd(BaseAddress, BlockAddress,
				    XFL_INTEL_CMD_READ_STATUS_REG);

		/*
		 * Check if the Unlocking is done and device is ready.
		 */
		StatusReg = READ_FLASH_16(BaseAddress + BlockAddress);
		while(!(StatusReg & XFL_INTEL_SR_WSM_READY)) {
			StatusReg = READ_FLASH_16(BaseAddress + BlockAddress);
		}

		/*
		 * Check if any Unlock error has occurred.
		 */
		if(StatusReg & XFL_INTEL_SR_ERASE_OR_UNLOCK_ERROR) {
			return XST_FAILURE;
		}

		/*
		 * Increment Region/Block.
		 */
		XFL_GEOMETRY_INCREMENT(GeomPtr, Region, Block);

		BlocksLeft--;
	}

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	(void) XFlashIntel_ResetBank(InstancePtr, Offset, Bytes);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Clears the Intel flash device status register(s) and place the device(s) into
* read mode.
*
* @param	InstancePtr is the pointer to the XFlash instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_BUSY if the flash devices were in the middle of an
*		  operation and could not be reset.
*		- XFLASH_ERROR if the device(s) have experienced an internal
*		  error during the operation. XFlash_DeviceControl() must be
*		  used to access the cause of the device specific error
*		  condition.
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_Reset(XFlash *InstancePtr)
{

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Send reset for all region/bank(s) in the device.
	 */
	return (XFlashIntel_ResetBank(InstancePtr, 0,
				InstancePtr->Geometry.DeviceSize));

}

/*****************************************************************************/
/**
*
* Clears the Intel flash bank status register and place the bank into read mode.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the bank address.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_BUSY if the flash bank were in the middle of an
*		  operation and could not be reset.
*		- XFLASH_ERROR if the bank have experienced an internal error
*		  during the operation. XFlash_DeviceControl() must be used to
*		  access the cause of the device specific error condition.
*
* @note		None.
*
******************************************************************************/
static int XFlashIntel_ResetBank(XFlash *InstancePtr, u32 Offset, u32 Bytes)
{
	XFlashVendorData_Intel *DevDataPtr;
	int Status;
	u16 Region, Block;
	u32 BaseAddress;
	u32 Dummy;
	XFlashGeometry *GeomPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Handle case when zero bytes is provided.
	 */
	if (Bytes == 0) {
		return XST_SUCCESS;
	}

	GeomPtr = &InstancePtr->Geometry;
	BaseAddress = GeomPtr->BaseAddress;
	DevDataPtr = GET_PARTDATA(InstancePtr);

	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset, &Region,
							&Block, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}


	while ((GeomPtr->EraseRegion[Region].AbsoluteOffset <=
		(Offset + Bytes - 1)) &&
		(Region < InstancePtr->Geometry.NumEraseRegions)) {
		/*
		 * Send the clear status register command. Use the max write
		 * width to notify parts of all layouts.
		 */
		DevDataPtr->SendCmd(BaseAddress,
				    GeomPtr->EraseRegion[Region].AbsoluteOffset,
				    XFL_INTEL_CMD_CLEAR_STATUS_REG);
		DevDataPtr->SendCmd(BaseAddress,
				    GeomPtr->EraseRegion[Region].AbsoluteOffset,
				    XFL_INTEL_CMD_READ_STATUS_REG);

		/*
		 * Sample the status of the parts, then place them into
		 * read-array mode.
		 */
		Status = DevDataPtr->GetStatus(InstancePtr,
				GeomPtr->EraseRegion[Region].AbsoluteOffset);

		/*
		 * Some Intel CFI chips support 0xF0 command instead of 0xFF as
		 * reset/read array command.
		 */
		DevDataPtr->SendCmd(BaseAddress,
				    GeomPtr->EraseRegion[Region].AbsoluteOffset,
				    XFL_INTEL_CMD_RESET_0xF0);
		DevDataPtr->SendCmd(BaseAddress,
				    GeomPtr->EraseRegion[Region].AbsoluteOffset,
				    XFL_INTEL_CMD_READ_ARRAY);

		/*
		 * If the status is not ready, then something is wrong.
		 */
		if (Status != XFLASH_READY) {
			if (Status != XFLASH_BUSY) {
				return (XFLASH_ERROR);
			}
			else {
				return (XFLASH_BUSY);
			}
		}

		/*
		 * Increment the region/bank.
		 */
		Region++;
	}

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
*
* Performs the Intel device specific control functions.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Command is the device specific command to issue.
* @param	Parameters specifies the arguments passed to the device control
*		function.
*
* @return
*		- XST_SUCCESS if successful
*		- XFLASH_NOT_SUPPORTED if the command is not
*		  recognized/supported by the device(s).
*
* @note		None.
*
******************************************************************************/
int XFlashIntel_DeviceControl(XFlash *InstancePtr, u32 Command,
			      DeviceCtrlParam *Parameters)
{
	int Status;
	XFlashVendorData_Intel *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);

	switch (Command) {
		case XFL_DEVCTL_GET_LAST_ERROR:
			Parameters->LastErrorParam.Error =
						DevDataPtr->SR_LastError.Mask32;
			return (XST_SUCCESS);

		case XFL_DEVCTL_GET_GEOMETRY:
			Parameters->GeometryParam.GeometryPtr =
					&InstancePtr->Geometry;
			return XST_SUCCESS;

		case XFL_DEVCTL_GET_PROPERTIES:
			Parameters->PropertiesParam.PropertiesPtr =
					&InstancePtr->Properties;
			return XST_SUCCESS;

		case XFL_DEVCTL_SET_RYBY:
			if (InstancePtr->Properties.PartID.CommandSet !=
						XFL_CMDSET_INTEL_EXTENDED) {
				return (XFLASH_NOT_SUPPORTED);
			}

			Status = SetRYBY(InstancePtr,
						Parameters->RyByParam.Param);
			return (Status);

		case XFL_DEVCTL_SET_CONFIG_REG:
			if (InstancePtr->IsPlatformFlash == 1) {
				DevDataPtr->SendCmdSeq(
					InstancePtr->Geometry.BaseAddress,
					Parameters->ConfigRegParam.Value,
					XFL_INTEL_CMD_CONFIG_REG_SETUP,
					XFL_INTEL_CMD_CONFIG_REG_CONFIRM);
				return (XST_SUCCESS);
			}

			return (XFLASH_NOT_SUPPORTED);

		default:
			return (XFLASH_NOT_SUPPORTED);
	}
}

/*****************************************************************************/
/**
*
* Retrieves and interprets status register data from part arrays that occupy a
* 8-bit bus.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return
*		- XFLASH_READY if the flash device is ready.
*		- XFLASH_BUSY if the flash device is Busy.
*		- XFLASH_ERROR if some error has occurred during flash
*		  operation.
*
* @note		No attempt is made to determine the exact cause of an error.
*		Instead that determination is left up to the user.
*
******************************************************************************/
static int GetStatus8(XFlash *InstancePtr, u32 Offset)
{
	u8 RegData;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Sample the register(s).
	 */
	RegData = READ_FLASH_8(InstancePtr->Geometry.BaseAddress + Offset);

	/*
	 * First determine if the device(s) are ready without errors indicated.
	 */
	if (RegData == DevDataPtr->SR_WsmReady.Mask8) {
		return (XFLASH_READY);
	}

	/*
	 * Next determine if the device(s) are still busy.
	 */
	if ((RegData & DevDataPtr->SR_WsmReady.Mask8) !=
	    DevDataPtr->SR_WsmReady.Mask8) {
		return (XFLASH_BUSY);
	}

	/*
	 * If control reaches this point, then an error is pending. Copy the
	 * entire set of registers to the SR_LastError attribute.
	 */
	DevDataPtr->SR_LastError.Mask8 = RegData;

	return (XFLASH_ERROR);
}

/*****************************************************************************/
/**
*
* Retrieves and interprets status register data from part arrays that occupy a
* 16-bit bus.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return
*		- XFLASH_READY if the flash device is ready.
*		- XFLASH_BUSY if the flash device is Busy.
*		- XFLASH_ERROR if some error has occurred during flash
*		  operation.
*
* @note		No attempt is made to determine the exact cause of an error.
*		Instead that determination is left up to the user.
*
******************************************************************************/
static int GetStatus16(XFlash *InstancePtr, u32 Offset)
{
	u16 RegData;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Sample the register(s).
	 */
	RegData = READ_FLASH_16(InstancePtr->Geometry.BaseAddress + Offset);

	/*
	 * First determine if the device(s) are ready without errors indicated.
	 */
	if (RegData == DevDataPtr->SR_WsmReady.Mask16) {
		return (XFLASH_READY);
	}

	/*
	 * Next determine if the device(s) are still busy.
	 */
	if ((RegData & DevDataPtr->SR_WsmReady.Mask16) !=
	    DevDataPtr->SR_WsmReady.Mask16) {
		return (XFLASH_BUSY);
	}

	/*
	 * If control reaches this point, then an error is pending. Copy the
	 * entire set of registers to the SR_LastError attribute.
	 */
	DevDataPtr->SR_LastError.Mask16 = RegData;

	return (XFLASH_ERROR);
}

/*****************************************************************************/
/**
*
* Retrieves and interprets status register data from part arrays that occupy a
* 32-bit bus.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return
*		- XFLASH_READY if the flash device is ready.
*		- XFLASH_BUSY if the flash device is Busy.
*		- XFLASH_ERROR if some error has occurred during flash
*		  operation.
*
* @note		No attempt is made to determine the exact cause of an error.
*		Instead that determination is left up to the user.
*
******************************************************************************/
static int GetStatus32(XFlash *InstancePtr, u32 Offset)
{
	u32 RegData;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Sample the register(s).
	 */
	RegData = READ_FLASH_32(InstancePtr->Geometry.BaseAddress + Offset);

	/*
	 * First, determine if the device(s) are ready without errors indicated.
	 */
	if (RegData == DevDataPtr->SR_WsmReady.Mask32) {
		return (XFLASH_READY);
	}

	/*
	 * Next, determine if the device(s) are still busy.
	 */
	if ((RegData & DevDataPtr->SR_WsmReady.Mask32) !=
	    DevDataPtr->SR_WsmReady.Mask32) {
		return (XFLASH_BUSY);
	}

	/*
	 * If control reaches this point, then an error is pending. Copy the
	 * entire set of registers to the SR_LastError attribute.
	 */
	DevDataPtr->SR_LastError.Mask32 = RegData;

	return (XFLASH_ERROR);
}

/*****************************************************************************/
/**
*
* Retrieves and interprets status register data from part arrays that occupy a
* 64-bit bus.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return
*		- XFLASH_READY if the flash device is ready.
*		- XFLASH_BUSY if the flash device is Busy.
*		- XFLASH_ERROR if some error has occurred during flash
*		  operation.
*
* @note		No attempt is made to determine the exact cause of an error.
*		Instead that determination is left up to the user.
*
******************************************************************************/
static int GetStatus64(XFlash *InstancePtr, u32 Offset)
{
	Xuint64 RegData;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Sample the status register(s) and OR the 32 bit halves together.
	 */
	READ_FLASH_64(InstancePtr->Geometry.BaseAddress + Offset, RegData);

	/*
	 * First, determine if the device(s) are ready without errors indicated.
	 */
	if ((XUINT64_MSW(RegData) == DevDataPtr->SR_WsmReady.Mask32) &&
	    (XUINT64_LSW(RegData) == DevDataPtr->SR_WsmReady.Mask32)) {
		return (XFLASH_READY);
	}

	/*
	 * Next, determine if the device(s) are still busy.
	 */
	if (((XUINT64_MSW(RegData) & DevDataPtr->SR_WsmReady.Mask32) !=
	     DevDataPtr->SR_WsmReady.Mask32) && ((XUINT64_LSW(RegData)
						  & DevDataPtr->SR_WsmReady.
						  Mask32) !=
						 DevDataPtr->SR_WsmReady.
						 Mask32)) {
		return (XFLASH_BUSY);
	}

	/*
	 * If control reaches this point, then an error is pending. Copy the
	 * entire set of registers to the SR_LastError attribute.
	 */
	XUINT64_MSW(DevDataPtr->SR_LastError.Mask64) = XUINT64_MSW(RegData);
	XUINT64_LSW(DevDataPtr->SR_LastError.Mask64) = XUINT64_LSW(RegData);

	return (XFLASH_ERROR);
}

/*****************************************************************************/
/**
*
* Polls the status register until the WSM is ready. The device(s) are polled
* by repeatedly reading the status register.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return	- XFLASH_READY if WSM is ready.
*		- XFLASH_ERROR if WSM is ready, but an error condition
*		  exists.
*
* @note		For Intel parts, the status register can be accessed from any
*	 	addressable location in the bank, in command mode. So we pick
*		the address where operation was performed.
*
******************************************************************************/
static int PollSR8(XFlash *InstancePtr, u32 Offset)
{
	u8 StatusReg;
	u8 ReadyMask;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Wait until WSM indicates that it is complete.
	 */
	ReadyMask = DevDataPtr->SR_WsmReady.Mask8;
	StatusReg = READ_FLASH_8(InstancePtr->Geometry.BaseAddress + Offset);
	while ((StatusReg & ReadyMask) != ReadyMask) {
		StatusReg = READ_FLASH_8(InstancePtr->Geometry.BaseAddress +
					 Offset);
	}

	/*
	 * WSM is ready, see if an error bit is set.
	 */
	if (StatusReg != ReadyMask) {
		DevDataPtr->SR_LastError.Mask8 = StatusReg;
		return (XFLASH_ERROR);
	}
	else {
		return (XFLASH_READY);
	}
}

/*****************************************************************************/
/**
*
* Polls the status register until the WSM is ready. The device(s) are polled
* by repeatedly reading the status register.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return	- XFLASH_READY if WSM is ready.
*		- XFLASH_ERROR if WSM is ready, but an error condition
*		  exists.
*
* @note		For Intel parts, the status register can be accessed from any
*	 	addressable location in the bank, in command mode. So we pick
*		the address where operation was performed.
*
******************************************************************************/
static int PollSR16(XFlash *InstancePtr, u32 Offset)
{
	u16 StatusReg;
	u16 ReadyMask;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Wait until WSM indicates that it is complete.
	 */
	ReadyMask = DevDataPtr->SR_WsmReady.Mask16;
	StatusReg = READ_FLASH_16(InstancePtr->Geometry.BaseAddress + Offset);
	while ((StatusReg & ReadyMask) != ReadyMask) {
		StatusReg = READ_FLASH_16(InstancePtr->Geometry.BaseAddress +
					  Offset);
	}

	/*
	 * WSM is ready, see if an error bit is set.
	 */
	if (StatusReg != ReadyMask) {
		DevDataPtr->SR_LastError.Mask16 = StatusReg;
		return (XFLASH_ERROR);
	}
	else {
		return (XFLASH_READY);
	}
}

/*****************************************************************************/
/**
*
* Polls the status register until the WSM is ready. The
* device(s) are polled by repeatedly reading the status register.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return	- XFLASH_READY if WSM is ready.
*		- XFLASH_ERROR if WSM is ready, but an error condition
*		  exists.
*
* @note		For Intel parts, the status register can be accessed from any
*	 	addressable location in the bank, in command mode. So we pick
*		the address where operation was performed.
*
******************************************************************************/
static int PollSR32(XFlash *InstancePtr, u32 Offset)
{
	u32 StatusReg;
	u32 ReadyMask;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Wait until WSM indicates that it is complete.
	 */
	ReadyMask = DevDataPtr->SR_WsmReady.Mask32;
	StatusReg = READ_FLASH_32(InstancePtr->Geometry.BaseAddress + Offset);
	while ((StatusReg & ReadyMask) != ReadyMask) {
		StatusReg = READ_FLASH_32(InstancePtr->Geometry.BaseAddress +
					  Offset);
	}

	/*
	 * WSM is ready, see if an error bit is set.
	 */
	if (StatusReg != ReadyMask) {
		DevDataPtr->SR_LastError.Mask32 = StatusReg;
		return (XFLASH_ERROR);
	}
	else {
		return (XFLASH_READY);
	}
}

/*****************************************************************************/
/**
*
* Polls the status register until the WSM is ready. The device(s) are polled
* by repeatedly reading the status register.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset address in the flash device.
*
* @return	- XFLASH_READY if WSM is ready.
*		- XFLASH_ERROR if WSM is ready, but an error condition
*		  exists.
*
* @note		For Intel parts, the status register can be accessed from any
*	 	addressable location in the bank, in command mode. So we pick
*		the address where operation was performed.
*
******************************************************************************/
static int PollSR64(XFlash *InstancePtr, u32 Offset)
{
	Xuint64 StatusReg;
	u32 ReadyMask;
	XFlashVendorData_Intel *DevDataPtr =  GET_PARTDATA(InstancePtr);

	/*
	 * Wait until WSM indicates that it is complete.
	 */
	ReadyMask = DevDataPtr->SR_WsmReady.Mask32;
	READ_FLASH_64(InstancePtr->Geometry.BaseAddress + Offset, StatusReg);
	while (((XUINT64_MSW(StatusReg) & ReadyMask) != ReadyMask) ||
		((XUINT64_LSW(StatusReg) & ReadyMask) != ReadyMask)) {
		READ_FLASH_64(InstancePtr->Geometry.BaseAddress + Offset,
			      StatusReg);
	}

	/*
	 * WSM is ready, see if an error bit is set.
	 */
	if ((XUINT64_MSW(StatusReg) != ReadyMask) ||
	    (XUINT64_LSW(StatusReg) != ReadyMask)) {
		XUINT64_MSW(DevDataPtr->SR_LastError.Mask64) =
			XUINT64_MSW(StatusReg);
		XUINT64_LSW(DevDataPtr->SR_LastError.Mask64) =
			XUINT64_LSW(StatusReg);
		return (XFLASH_ERROR);
	}
	else {
		return (XFLASH_READY);
	}
}

/*****************************************************************************/
/**
*
* Writes a command using a 8-bit bus cycle.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd is the command/data to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmd8(u32 BaseAddress, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_8(BaseAddress + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command using a 16-bit bus cycle.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd is the command/data to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmd16(u32 BaseAddress, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_16(BaseAddress + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command using a 32-bit bus cycle.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd is the command/data to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmd32(u32 BaseAddress, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_32(BaseAddress + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command using a 64-bit bus cycle. Depending on the architecture,
* this may be a single write or two 32 bit writes.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd is the command/data to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmd64(u32 BaseAddress, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_64x2(BaseAddress + Offset, Cmd, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command sequence using 8-bit bus cycles.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd1 is the first command to write at BaseAddress + Offset
* @param	Cmd2 is the second command to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmdSeq8(u32 BaseAddress, u32 Offset, u32 Cmd1, u32 Cmd2)
{
	WRITE_FLASH_8(BaseAddress + Offset, Cmd1);
	WRITE_FLASH_8(BaseAddress + Offset, Cmd2);
}

/*****************************************************************************/
/**
*
* Writes a command sequence using 16-bit bus cycles.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd1 is the first command to write at BaseAddress + Offset
* @param	Cmd2 is the second command to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmdSeq16(u32 BaseAddress, u32 Offset, u32 Cmd1, u32 Cmd2)
{
	WRITE_FLASH_16(BaseAddress + Offset, Cmd1);
	WRITE_FLASH_16(BaseAddress + Offset, Cmd2);
}


/*****************************************************************************/
/**
*
* Writes a command sequence using 32-bit bus cycles.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd1 is the first command to write at BaseAddress + Offset
* @param	Cmd2 is the second command to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmdSeq32(u32 BaseAddress, u32 Offset, u32 Cmd1, u32 Cmd2)
{
	WRITE_FLASH_32(BaseAddress + Offset, Cmd1);
	WRITE_FLASH_32(BaseAddress + Offset, Cmd2);
}

/*****************************************************************************/
/**
*
* Writes a command sequence using 64-bit bus cycles. Depending on the
* architecture, this may be a single write or two 32 bit writes.
*
* @param	BaseAddress is the base address of device
* @param	Offset is the offset into device
* @param	Cmd1 is the first command to write at BaseAddress + Offset
* @param	Cmd2 is the second command to write at BaseAddress + Offset
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmdSeq64(u32 BaseAddress, u32 Offset, u32 Cmd1, u32 Cmd2)
{
	WRITE_FLASH_64x2(BaseAddress + Offset, Cmd1, Cmd1);
	WRITE_FLASH_64x2(BaseAddress + Offset, Cmd2, Cmd2);
}

/*****************************************************************************/
/**
*
* Program the device(s) using the faster write to buffer mechanism. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on
* @param	DestPtr is the physical destination address in flash memory
*		space
* @param	SrcPtr is the source data
* @param	Bytes is the number of bytes to program
*
* @return
*
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific. Use XFlash_DeviceControl() to
*		  retrieve specific error conditions. When this error is
*		  returned, it is possible that the target address range was
*		  only partially programmed.
*
* @note
*
* This algorithm programs only full buffers at a time and must take care
* of the following situations:
*	- Partial first buffer programming with pre and/or post padding
*	  required.
*	- Multiple full buffer programming.
*	- Partial last buffer programming with post padding.
* <br><br>
* When padding, 0xFF is written to each byte to be padded. This in effect does
* nothing to the current contents of the flash memory and saves us from having
* to merge real flash data with user data on partial buffer writes.
* <br><br>
* If hardware is failing, then this routine could get stuck in an endless loop.
*
******************************************************************************/
static int WriteBuffer8(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	XFlashVendorData_Intel *DevDataPtr;
	u8 *SrcWordPtr = (u8*)SrcPtr;
	u8 *DestWordPtr = (u8*)DestPtr;
	u8 StatusReg;
	u8 ReadyMask;
	u32 BytesLeft = Bytes;
	u32 BaseAddress;
	u32 PartialBytes;
	u32 Count;
	int Status = XST_SUCCESS;
	u32 Index;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;
	ReadyMask = DevDataPtr->SR_WsmReady.Mask8;

	/*
	 * Determine if a partial first buffer must be programmed.
	 */
	PartialBytes = (u32) DestWordPtr &
		InstancePtr->Properties.ProgCap.WriteBufferAlignmentMask;

	/*
	 * This write cycle programs the first partial write buffer.
	 */
	if (PartialBytes) {
		/*
		 * Backup DestWord to the beginning of a buffer alignment area
		 * Count is the number of write cycles left after pre-filling
		 * the write buffer with 0xFFFF.
		 */
		DestWordPtr = (u8*)((u32) DestWordPtr - PartialBytes);
		Count = InstancePtr->Properties.ProgCap.WriteBufferSize;

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available. Write number of words to be written (always the
		 * maximum).
		 */
		WRITE_FLASH_8(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);

		StatusReg = READ_FLASH_8(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_8(DestWordPtr);

		}

		WRITE_FLASH_8(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Write 0xFFFF padding until we get to the start of the
		 * original DestWord.
		 */
		while (PartialBytes > 1) {
			WRITE_FLASH_8(DestWordPtr++, 0xFFFF);
			PartialBytes -= 1;
			Count--;
		}

		/*
		 * Write the remainder of this write buffer.
		 */
		Index = 0;
		while (Count--) {
			/*
			 * Write Byte
			 */
			if (BytesLeft >= 1) {
				WRITE_FLASH_8(&DestWordPtr[Index],
						SrcWordPtr[Index]);
				BytesLeft -= 1;
			}

			/*
			 * End of SrcWords
			 */
			else {
				WRITE_FLASH_8(&DestWordPtr[Index], 0xFF);
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_8(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR8(InstancePtr, ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination to next buffer
		 */
		SrcWordPtr += Index;
		DestWordPtr += Index;
	}

	/*
	 * At this point DestWordPtr and SrcWordPtr are aligned to a write
	 * buffer boundary. The next batch of writes utilize write cycles full
	 * of SrcData.
	 */
	Count = InstancePtr->Properties.ProgCap.WriteBufferSize;
	while (BytesLeft >= InstancePtr->Properties.ProgCap.WriteBufferSize) {

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available. Write number of words to be written (always the
		 * maximum).
		 */
		WRITE_FLASH_8(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);
		StatusReg = READ_FLASH_8(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_8(DestWordPtr);
		}
		WRITE_FLASH_8(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer
		 */
		for (Index = 0; Index < Count; Index++) {
			WRITE_FLASH_8(&DestWordPtr[Index], SrcWordPtr[Index]);
			BytesLeft -= 1;
		}

		/*
		 * Send confirmation and wait for status
		 */
		WRITE_FLASH_8(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR8(InstancePtr, ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination.
		 */
		SrcWordPtr += Count;
		DestWordPtr += Count;
	}

	/*
	 * The last phase is to write a partial last buffer.
	 */
	if (BytesLeft) {

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available. Write number of words to be written (always the
		 * maximum).
		 */
		WRITE_FLASH_8(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);

		StatusReg = READ_FLASH_8(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_8(DestWordPtr);
		}

		WRITE_FLASH_8(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer
		 */
		Index = 0;
		while (Count--) {
			/*
			 * Write Byte
			 */
			if (BytesLeft >= 1) {
				WRITE_FLASH_8(&DestWordPtr[Index],
							SrcWordPtr[Index]);
				BytesLeft -= 1;
			}

			/*
			 * End of SrcWords
			 */
			else {
				WRITE_FLASH_8(&DestWordPtr[Index], 0xFF);
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_8(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR8(InstancePtr, ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Program the device(s) using the faster write to buffer mechanism. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on
* @param	DestPtr is the physical destination address in flash memory
*		space
* @param	SrcPtr is the source data
* @param	Bytes is the number of bytes to program
*
* @return
*
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific. Use XFlash_DeviceControl() to
*		  retrieve specific error conditions. When this error is
*		  returned, it is possible that the target address range was
*		  only partially programmed.
*		- XFLASH_ALIGNMENT_ERROR if the source and destination pointers
*		  are not aligned to a 16-bit word.
* @note
*
* This algorithm programs only full buffers at a time and must take care
* of the following situations:
*	- Partial first buffer programming with pre and/or post padding
*	  required.
*	- Multiple full buffer programming.
*	- Partial last buffer programming with post padding.
* <br><br>
* When padding, 0xFF is written to each byte to be padded. This in effect does
* nothing to the current contents of the flash memory and saves us from having
* to merge real flash data with user data on partial buffer writes.
* <br><br>
* If hardware is failing, then this routine could get stuck in an endless loop.
*
******************************************************************************/
static int WriteBufferStrataFlashDevice(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	XFlashVendorData_Intel *DevDataPtr;
	u16 *SrcWordPtr = (u16*)SrcPtr;
	u16 *DestWordPtr = (u16*)DestPtr;
	u32 BaseAddress;
	u32 BytesLeft = Bytes;
	u32 PartialBytes;
	u32 Count;
	int Status = XST_SUCCESS;
	u32 Index;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;

	/*
	 * Make sure DestPtr and SrcPtr are aligned to a 16-bit word.
	 */
	if (((int) SrcWordPtr & 1) || ((int) DestWordPtr & 1)) {
		return (XFLASH_ALIGNMENT_ERROR);
	}

	/*
	 * Determine if a partial first buffer must be programmed.
	 */
	PartialBytes = (u32) DestWordPtr &
		InstancePtr->Properties.ProgCap.WriteBufferAlignmentMask;

	/*
	 * This write cycle programs the first partial write buffer.
	 */
	if (PartialBytes) {
		/*
		 * Backup DestWord to the beginning of a buffer alignment area
		 * Count is the number of write cycles left after pre-filling
		 * the write buffer with 0xFFFF.
		 */
		DestWordPtr = (u16*)((u32) DestWordPtr - PartialBytes);
		Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 1;

		/*
		 * Send command to write buffer. Write number of words to
		 * be written (always the maximum).
		 */
		WRITE_FLASH_16(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);

		WRITE_FLASH_16(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Write 0xFFFF padding until we get to the start of the
		 * original DestWord.
		 */
		while (PartialBytes > 1) {
			WRITE_FLASH_16(DestWordPtr++, 0xFFFF);
			PartialBytes -= 2;
			Count--;
		}

		/*
		 * Write the remainder of this write buffer.
		 */
		Index = 0;
		while (Count--) {
			/* Full word */
			if (BytesLeft > 1) {
				WRITE_FLASH_16(&DestWordPtr[Index],
						SrcWordPtr[Index]);
				BytesLeft -= 2;
			}

			/* End of SrcWords */
			else if (BytesLeft == 0) {
				WRITE_FLASH_16(&DestWordPtr[Index], 0xFFFF);
			}

			/* Partial word */
			else {	/* BytesLeft == 1 */
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0xFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0x00FF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_16(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR16(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination to next buffer
		 */
		SrcWordPtr += Index;
		DestWordPtr += Index;
	}

	/*
	 * At this point DestWordPtr and SrcWordPtr are aligned to a write
	 * buffer boundary. The next batch of writes utilize write cycles full
	 * of SrcData.
	 */
	Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 1;
	while (BytesLeft > InstancePtr->Properties.ProgCap.WriteBufferSize) {

		/*
		 * Send command to write buffer.Write number of words to
		 * be written (always the maximum).
		 */
		WRITE_FLASH_16(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);

		WRITE_FLASH_16(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer
		 */
		for (Index = 0; Index < Count; Index++) {
			WRITE_FLASH_16(&DestWordPtr[Index], SrcWordPtr[Index]);
			BytesLeft -= 2;
		}

		/*
		 * Send confirmation and wait for status
		 */
		WRITE_FLASH_16(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR16(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination.
		 */
		SrcWordPtr += Count;
		DestWordPtr += Count;
	}

	/*
	 * The last phase is to write a partial last buffer.
	 */
	if (BytesLeft) {

		/*
		 * Send command to write buffer.Write number of words to
		 * be written (always the maximum).
		 */
		WRITE_FLASH_16(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);

		WRITE_FLASH_16(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer
		 */
		Index = 0;
		while (Count--) {
			/*
			 * Full word.
			 */
			if (BytesLeft > 1) {
				WRITE_FLASH_16(&DestWordPtr[Index],
							SrcWordPtr[Index]);
				BytesLeft -= 2;
			}

			/* End of SrcWords */
			else if (BytesLeft == 0) {
				WRITE_FLASH_16(&DestWordPtr[Index], 0xFFFF);
			}
			/* Partial word */
			else {	/* BytesLeft == 1 */

				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0xFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0x00FF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_16(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR16(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Program the device(s) using the faster write to buffer mechanism. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on
* @param	DestPtr is the physical destination address in flash memory
*		space
* @param	SrcPtr is the source data
* @param	Bytes is the number of bytes to program
*
* @return
*
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific. Use XFlash_DeviceControl() to
*		  retrieve specific error conditions. When this error is
*		  returned, it is possible that the target address range was
*		  only partially programmed.
*		- XFLASH_ALIGNMENT_ERROR if the source and destination pointers
*		  are not aligned to a 16-bit word.
* @note
*
* This algorithm programs only full buffers at a time and must take care
* of the following situations:
*	- Partial first buffer programming with pre and/or post padding
*	  required.
*	- Multiple full buffer programming.
*	- Partial last buffer programming with post padding.
* <br><br>
* When padding, 0xFF is written to each byte to be padded. This in effect does
* nothing to the current contents of the flash memory and saves us from having
* to merge real flash data with user data on partial buffer writes.
* <br><br>
* If hardware is failing, then this routine could get stuck in an endless loop.
*
******************************************************************************/
static int WriteBufferIntelFlashDevice(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	XFlashVendorData_Intel *DevDataPtr;
	u16 *SrcWordPtr = (u16*)SrcPtr;
	u16 *DestWordPtr = (u16*)DestPtr;
	u16 StatusReg;
	u16 ReadyMask;
	u32 BaseAddress;
	u32 BytesLeft = Bytes;
	u32 PartialBytes;
	u32 Count;
	int Status = XST_SUCCESS;
	u32 Index;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;
	ReadyMask = DevDataPtr->SR_WsmReady.Mask16;

	/*
	 * Make sure DestPtr and SrcPtr are aligned to a 16-bit word.
	 */
	if (((int) SrcWordPtr & 1) || ((int) DestWordPtr & 1)) {
		return (XFLASH_ALIGNMENT_ERROR);
	}

	/*
	 * Determine if a partial first buffer must be programmed.
	 */
	PartialBytes = (u32) DestWordPtr &
		InstancePtr->Properties.ProgCap.WriteBufferAlignmentMask;

	/*
	 * This write cycle programs the first partial write buffer.
	 */
	if (PartialBytes) {
		/*
		 * Backup DestWord to the beginning of a buffer alignment area
		 * Count is the number of write cycles left after pre-filling
		 * the write buffer with 0xFFFF.
		 */
		DestWordPtr = (u16*)((u32) DestWordPtr - PartialBytes);
		Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 1;

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available. Write number of words to be written (always the
		 * maximum).
		 */
		WRITE_FLASH_16(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);

		StatusReg = READ_FLASH_16(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_16(DestWordPtr);

		}

		WRITE_FLASH_16(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Write 0xFFFF padding until we get to the start of the
		 * original DestWord.
		 */
		while (PartialBytes > 1) {
			WRITE_FLASH_16(DestWordPtr++, 0xFFFF);
			PartialBytes -= 2;
			Count--;
		}

		/*
		 * Write the remainder of this write buffer.
		 */
		Index = 0;
		while (Count--) {
			/* Full word */
			if (BytesLeft > 1) {
				WRITE_FLASH_16(&DestWordPtr[Index],
						SrcWordPtr[Index]);
				BytesLeft -= 2;
			}

			/* End of SrcWords */
			else if (BytesLeft == 0) {
				WRITE_FLASH_16(&DestWordPtr[Index], 0xFFFF);
			}

			/* Partial word */
			else {	/* BytesLeft == 1 */
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0xFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0x00FF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_16(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR16(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination to next buffer
		 */
		SrcWordPtr += Index;
		DestWordPtr += Index;
	}

	/*
	 * At this point DestWordPtr and SrcWordPtr are aligned to a write
	 * buffer boundary. The next batch of writes utilize write cycles full
	 * of SrcData.
	 */
	Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 1;
	while (BytesLeft > InstancePtr->Properties.ProgCap.WriteBufferSize) {

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available. Write number of words to be written (always the
		 * maximum).
		 */
		WRITE_FLASH_16(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);
		StatusReg = READ_FLASH_16(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_16(DestWordPtr);
		}
		WRITE_FLASH_16(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer
		 */
		for (Index = 0; Index < Count; Index++) {
			WRITE_FLASH_16(&DestWordPtr[Index], SrcWordPtr[Index]);
			BytesLeft -= 2;
		}

		/*
		 * Send confirmation and wait for status
		 */
		WRITE_FLASH_16(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR16(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination.
		 */
		SrcWordPtr += Count;
		DestWordPtr += Count;
	}

	/*
	 * The last phase is to write a partial last buffer.
	 */
	if (BytesLeft) {

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available. Write number of words to be written (always the
		 * maximum).
		 */
		WRITE_FLASH_16(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);

		StatusReg = READ_FLASH_16(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_16(DestWordPtr);
		}

		WRITE_FLASH_16(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer
		 */
		Index = 0;
		while (Count--) {
			/*
			 * Full word.
			 */
			if (BytesLeft > 1) {
				WRITE_FLASH_16(&DestWordPtr[Index],
							SrcWordPtr[Index]);
				BytesLeft -= 2;
			}

			/* End of SrcWords */
			else if (BytesLeft == 0) {
				WRITE_FLASH_16(&DestWordPtr[Index], 0xFFFF);
			}
			/* Partial word */
			else {	/* BytesLeft == 1 */

				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0xFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_16(&DestWordPtr[Index],
					       0x00FF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_16(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR16(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Program the device(s) using the faster write to buffer mechanism. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on
* @param	DestPtr is the physical destination address in flash memory
*		space
* @param	SrcPtr is the source data
* @param	Bytes is the number of bytes to program
*
* @return
*
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific. Use XFlash_DeviceControl() to
*		  retrieve specific error conditions. When this error is
*		  returned, it is possible that the target address range was
*		  only partially programmed.
*		- XFLASH_ALIGNMENT_ERROR if the source and destination pointers
*		  are not aligned to a 16-bit word.
* @note
*
******************************************************************************/
static int WriteBuffer16(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	u32 Status;
	if (InstancePtr->Properties.PartID.DeviceID == 0x01) {
		Status = WriteBufferStrataFlashDevice(InstancePtr, DestPtr,
							SrcPtr, Bytes);
	} else {
		Status = WriteBufferIntelFlashDevice(InstancePtr, DestPtr,
							SrcPtr, Bytes);
	}
	return Status;
}

/*****************************************************************************/
/**
*
* Programs the device(s) using the faster write to buffer mechanism. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on
* @param	DestPtr is the physical destination address in flash memory
*		space
* @param	SrcPtr is the source data
* @param	Bytes is the number of bytes to program
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific. Use XFlash_DeviceControl() to
*		  retrieve specific error conditions. When this error is
*		  returned, it is possible that the target address range was
*		  only partially programmed.
*		- XFLASH_ALIGNMENT_ERROR if the source and destination pointers
*		  are not aligned to a 16-bit word.
*
* @note
*
* This algorithm programs only full buffers at a time and must take care
* of the following situations:
*	- Partial first buffer programming with pre and/or post padding
*	  required.
*	- Multiple full buffer programming.
*	- Partial last buffer programming with post padding.
* <br><br>
* When padding, 0xFF is written to each byte to be padded. This in effect does
* nothing to the current contents of the flash memory and saves us from having
* to merge real flash data with user data on partial buffer writes.
* <br><br>
* If hardware is failing, then this routine could get stuck in an endless loop.
*
******************************************************************************/
static int WriteBuffer32(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	XFlashVendorData_Intel *DevDataPtr;
	u32 *SrcWordPtr = (u32*)SrcPtr;
	u32 *DestWordPtr = (u32*)DestPtr;
	u32 StatusReg;
	u32 ReadyMask;
	u32 BaseAddress;
	u32 BytesLeft = Bytes;
	u32 PartialBytes;
	u32 Count;
	int Status = XST_SUCCESS;
	u32 Index;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;
	ReadyMask = DevDataPtr->SR_WsmReady.Mask32;

	/*
	 * Make sure DestPtr and SrcPtr are aligned to a 32-bit word.
	 */
	if (((int) SrcWordPtr & 3) || ((int) DestWordPtr & 3)) {
		return (XFLASH_ALIGNMENT_ERROR);
	}

	/*
	 * Determine if a partial first buffer must be programmed.
	 */
	PartialBytes = (u32) DestWordPtr &
		InstancePtr->Properties.ProgCap.WriteBufferAlignmentMask;

	/*
	 * This write cycle programs the first partial write buffer.
	 */
	if (PartialBytes) {

		/*
		 * Backup DestWord to the beginning of a buffer alignment area
		 * Count is the number of write cycles left after pre-filling
		 * the write buffer with 0xFFFF.
		 */
		DestWordPtr = (u32*)((u32) DestWordPtr - PartialBytes);
		Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 2;

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available.
		 * Write number of words to be written (always the maximum).
		 */
		WRITE_FLASH_32(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);
		StatusReg = READ_FLASH_32(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_32(DestWordPtr);
		}
		WRITE_FLASH_32(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Write 0xFFFFFFFF padding until we get to the start of the
		 * user data.
		 */
		while (PartialBytes > 3) {
			WRITE_FLASH_32(DestWordPtr++, 0xFFFFFFFF);
			PartialBytes -= 4;
			Count--;
		}

		/*
		 * Write the remainder of this write buffer.
		 */
		Index = 0;
		while (Count--) {
			/* Full word */
			if (BytesLeft > 3) {
				WRITE_FLASH_32(&DestWordPtr[Index],
							SrcWordPtr[Index]);
				BytesLeft -= 4;
			}

			/* End of SrcWords */
			else if (BytesLeft == 0) {
				WRITE_FLASH_32(&DestWordPtr[Index], 0xFFFFFFFF);
			}

			/* Partial word */
			else if (BytesLeft == 1) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFFFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x00FFFFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			else if (BytesLeft == 2) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFF0000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x0000FFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 2;
			}
			else {	/* BytesLeft == 3 */
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFF000000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x000000FF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 3;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_32(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR32(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination to last access position.
		 */
		SrcWordPtr += Index;
		DestWordPtr += Index;
	}

	/*
	 * At this point DestPtr and SrcPtr are aligned to a write buffer
	 * boundary.
	 * The next batch of writes utilize a write cycle full of SrcData.
	 */
	Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 2;
	while (BytesLeft >= InstancePtr->Properties.ProgCap.WriteBufferSize) {

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available.
		 * Write number of words to be written (always the maximum).
		 */
		WRITE_FLASH_32(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);
		StatusReg = READ_FLASH_32(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_32(DestWordPtr);
		}

		WRITE_FLASH_32(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer.
		 */
		for (Index = 0; Index < Count; Index++) {
			WRITE_FLASH_32(&DestWordPtr[Index], SrcWordPtr[Index]);
			BytesLeft -= 4;
		}

		/*
		 * Send confirmation and wait for status.
		 */
		WRITE_FLASH_32(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR32(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination.
		 */
		SrcWordPtr += Count;
		DestWordPtr += Count;
	}

	/*
	 * The last phase is to write a partial last write buffer.
	 */
	if (BytesLeft) {
		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available.
		 * Write number of words to be written (always the maximum).
		 */
		WRITE_FLASH_32(DestWordPtr,
			InstancePtr->Command.WriteBufferCommand);
		StatusReg = READ_FLASH_32(DestWordPtr);
		while ((StatusReg & ReadyMask) != ReadyMask) {
			StatusReg = READ_FLASH_32(DestWordPtr);
		}
		WRITE_FLASH_32(DestWordPtr, DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer.
		 */
		Index = 0;
		while (Count--) {
			/*
			 * Full word.
			 */
			if (BytesLeft > 3) {
				WRITE_FLASH_32(&DestWordPtr[Index],
							SrcWordPtr[Index]);
				BytesLeft -= 4;
			}

			/* End of SrcWords */
			else if (BytesLeft == 0) {
				WRITE_FLASH_32(&DestWordPtr[Index], 0xFFFFFFFF);
			}

			/* Partial word */
			else if (BytesLeft == 1) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFFFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x00FFFFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			else if (BytesLeft == 2) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFF0000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x0000FFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 2;
			}
			else {	/* BytesLeft == 3 */

				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFF000000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x000000FF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 3;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_32(DestWordPtr, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR32(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Programs the device(s) using the faster write to buffer mechanism. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on.
* @param	DestPtr is the physical destination address in flash memory
*		space.
* @param	SrcPtr is the source data.
* @param	Bytes is the number of bytes to program.
*
* @return
*
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific. Use XFlash_DeviceControl() to
*		  retrieve specific error conditions. When this error is
*		  returned, it is possible that the target address range was
*		  only partially programmed.
*		- XFLASH_ALIGNMENT_ERROR if the source and destination pointers
*		  are not aligned to a 16-bit word.
*
* @note
*
* This algorithm programs only full buffers at a time and must take care
* of the following situations:
*	- Partial first buffer programming with pre and/or post padding
*	  required.
*	- Multiple full buffer programming.
*	- Partial last buffer programming with post padding.
* <br><br>
* When padding, 0xFF is written to each byte to be padded. This in effect does
* nothing to the current contents of the flash memory and saves us from having
* to merge real flash data with user data on partial buffer writes.
* <br><br>
* If hardware is failing, then this routine could get stuck in an endless loop.
*
******************************************************************************/
static int WriteBuffer64(XFlash *InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	XFlashVendorData_Intel *DevDataPtr;
	u32 *SrcWordPtr = (u32*)SrcPtr;
	u32 *DestWordPtr = (u32*)DestPtr;
	Xuint64 StatusReg;
	u32 BaseAddress;
	u32 ReadyMask;
	u32 BytesLeft = Bytes;
	u32 PartialBytes;
	u32 Count;
	int Status = XST_SUCCESS;
	u32 Index;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;
	ReadyMask = DevDataPtr->SR_WsmReady.Mask32;

	XUINT64_MSW(StatusReg) = 0;
	XUINT64_LSW(StatusReg) = 0;

	/*
	 * Make sure DestPtr and SrcPtr are aligned to a 32-bit word.
	 */
	if (((int) SrcWordPtr & 3) || ((int) DestWordPtr & 3)) {
		return (XFLASH_ALIGNMENT_ERROR);
	}

	/*
	 * Determine if a partial first buffer must be programmed.
	 */
	PartialBytes = (u32) DestWordPtr &
		InstancePtr->Properties.ProgCap.WriteBufferAlignmentMask;

	/*
	 * This write cycle programs the first partial write buffer.
	 */
	if (PartialBytes) {
		/*
		 * Backup DestWord to the beginning of a buffer alignment area
		 * Count is the number of write cycles left after pre-filling
		 * the write buffer with 0xFFFF.
		 */
		DestWordPtr = (u32*)((u32) DestWordPtr - PartialBytes);
		Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 2;

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available.
		 * Write number of words to be written (always the maximum)
		 */
		WRITE_FLASH_64x2(DestWordPtr,
				InstancePtr->Command.WriteBufferCommand,
				InstancePtr->Command.WriteBufferCommand);
		READ_FLASH_64(DestWordPtr, StatusReg);
		while (((XUINT64_MSW(StatusReg) & ReadyMask) != ReadyMask) ||
		       ((XUINT64_LSW(StatusReg) & ReadyMask) != ReadyMask)) {
			READ_FLASH_64(DestWordPtr, StatusReg);
		}
		WRITE_FLASH_64x2(DestWordPtr, DevDataPtr->WriteBufferWordCount,
				 DevDataPtr->WriteBufferWordCount);

		/*
		 * Write 0xFFFFFFFF padding until we get to the start of the
		 * user data
		 */
		while (PartialBytes > 3) {
			WRITE_FLASH_32(DestWordPtr++, 0xFFFFFFFF);
			PartialBytes -= 4;
			Count--;
		}

		/*
		 * Write the remainder of this write buffer
		 */
		Index = 0;
		while (Count--) {
			/* Full word */
			if (BytesLeft > 3) {
				WRITE_FLASH_32(&DestWordPtr[Index],
				SrcWordPtr[Index]);
				BytesLeft -= 4;
			}

			/* End of SrcWords */
			else if (BytesLeft == 0) {
				WRITE_FLASH_32(&DestWordPtr[Index], 0xFFFFFFFF);
			}

			/* Partial word */
			else if (BytesLeft == 1) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFFFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x00FFFFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			else if (BytesLeft == 2) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFF0000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x0000FFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 2;
			}
			else {	/* BytesLeft == 3 */

				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFF000000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x000000FF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 3;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_64x2((u32) DestWordPtr & ~7,
				 XFL_INTEL_CMD_CONFIRM, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR64(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination to last access position
		 */
		SrcWordPtr += Index;
		DestWordPtr += Index;
	}

	/*
	 * At this point DestPtr and SrcPtr are aligned to a write buffer
	 * boundary.
	 * The next batch of writes utilize a write cycle full of SrcData
	 */
	Count = InstancePtr->Properties.ProgCap.WriteBufferSize >> 2;
	while (BytesLeft >= InstancePtr->Properties.ProgCap.WriteBufferSize) {

		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available.
		 * Write number of words to be written (always the maximum)
		 */
		WRITE_FLASH_64x2(DestWordPtr,
				InstancePtr->Command.WriteBufferCommand,
				InstancePtr->Command.WriteBufferCommand);

		READ_FLASH_64(DestWordPtr, StatusReg);
		while (((XUINT64_MSW(StatusReg) & ReadyMask) != ReadyMask) ||
			((XUINT64_LSW(StatusReg) & ReadyMask) != ReadyMask)) {
			READ_FLASH_64(DestWordPtr, StatusReg);
		}
		WRITE_FLASH_64x2(DestWordPtr, DevDataPtr->WriteBufferWordCount,
				 DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer.
		 */
		for (Index = 0; Index < Count; Index++) {
			WRITE_FLASH_32(&DestWordPtr[Index], SrcWordPtr[Index]);
			BytesLeft -= 4;
		}

		/*
		 * Send confirmation and wait for status.
		 */
		WRITE_FLASH_64x2((u32) DestWordPtr & ~7,
				 XFL_INTEL_CMD_CONFIRM, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR64(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}

		/*
		 * Increment source and destination.
		 */
		SrcWordPtr += Count;
		DestWordPtr += Count;
	}

	/*
	 * The last phase is to write a partial last write buffer.
	 */
	if (BytesLeft) {
		/*
		 * Send command to write buffer. Wait for buffer to become
		 * available.
		 * Write number of words to be written (always the maximum).
		 */
		WRITE_FLASH_64x2(DestWordPtr,
				InstancePtr->Command.WriteBufferCommand,
				InstancePtr->Command.WriteBufferCommand);

		while (((XUINT64_MSW(StatusReg) & ReadyMask) != ReadyMask) ||
		       ((XUINT64_LSW(StatusReg) & ReadyMask) != ReadyMask)) {
			READ_FLASH_64(DestWordPtr, StatusReg);
		}
		WRITE_FLASH_64x2(DestWordPtr, DevDataPtr->WriteBufferWordCount,
				 DevDataPtr->WriteBufferWordCount);

		/*
		 * Fill the buffer.
		 */
		Index = 0;
		while (Count--) {
			/*
			 * Full word.
			 */
			if (BytesLeft > 3) {
				WRITE_FLASH_32(&DestWordPtr[Index],
							SrcWordPtr[Index]);
				BytesLeft -= 4;
			}

			/* End of SrcWords. */
			else if (BytesLeft == 0) {
				WRITE_FLASH_32(&DestWordPtr[Index], 0xFFFFFFFF);
			}

			/* Partial word. */
			else if (BytesLeft == 1) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFFFF00 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x00FFFFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft--;
			}
			else if (BytesLeft == 2) {
				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFFFF0000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x0000FFFF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 2;
			}
			else {	/* BytesLeft == 3. */

				#ifdef XPAR_AXI_EMC
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0xFF000000 | SrcWordPtr[Index]);
				#else
				WRITE_FLASH_32(&DestWordPtr[Index],
					       0x000000FF | SrcWordPtr[Index]);
				#endif
				BytesLeft -= 3;
			}
			Index++;
		}

		/*
		 * Buffer write completed. Send confirmation command and wait
		 * for completion.
		 */
		WRITE_FLASH_64x2((u32) DestWordPtr & ~7,
				 XFL_INTEL_CMD_CONFIRM, XFL_INTEL_CMD_CONFIRM);
		Status = PollSR64(InstancePtr,
				  ((u32)DestWordPtr) - BaseAddress);
		if (Status != XFLASH_READY) {
			return (Status);
		}
	}

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
*
* Reads and interprets part identification data.
*
* @param	InstancePtr is the instance to work on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void GetPartID(XFlash *InstancePtr)
{
	XFlashVendorData_Intel *DevDataPtr;
	u32 CmdAddress;
	void *Ptr;
	u8 Interleave;
	u8 Mode;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	CmdAddress = InstancePtr->Geometry.BaseAddress;
	Interleave = (InstancePtr->Geometry.MemoryLayout &
		      XFL_LAYOUT_CFI_INTERL_MASK) >> 24;
	Mode = (InstancePtr->Geometry.MemoryLayout &
		      XFL_LAYOUT_PART_MODE_MASK) >> 8;

	/*
	 * Send Read id codes command.
	 */
	DevDataPtr->SendCmd(CmdAddress, 8,
			    XFL_INTEL_CMD_READ_ID_CODES);

	/*
	 * Retrieve Manufacturer ID located at word offset 0.
	 */
	XFL_CFI_POSITION_PTR(Ptr, CmdAddress, Interleave, 0);
	InstancePtr->Properties.PartID.ManufacturerID =
		XFlashCFI_Read8((u8*)Ptr, Interleave, Mode);

	/*
	 * Retrieve Device Code located at word offset 1.
	 */
	XFL_CFI_ADVANCE_PTR8(Ptr, Interleave);
	InstancePtr->Properties.PartID.DeviceID =
		XFlashCFI_Read8((u8*)Ptr, Interleave, Mode);

	/*
	 * Place device(s) back into read-array mode.
	 */
	(void) XFlashIntel_Reset(InstancePtr);
}

/*****************************************************************************/
/**
*
* Sets the RYBY control signal.
*
* @param	InstancePtr is the instance to work on.
* @param	Mode is the mode to set the RYBY signal to.
*
* @return
*
* 		- XST_SUCCESS if the mode was successfully
*		  changed.
* 		- XFLASH_NOT_SUPPORTED if the mode given is
*		  unknown/unsupported.
*
* @note		None.
*
******************************************************************************/
static int SetRYBY(XFlash *InstancePtr, u32 Mode)
{
	u32 Cmd;
	XFlashVendorData_Intel *DevDataPtr;

	/*
	 * Determine which command code to use.
	 */
	switch (Mode) {
	case XFL_INTEL_RYBY_PULSE_OFF:
		Cmd = XFL_INTEL_CONFIG_RYBY_LEVEL;
		break;

	case XFL_INTEL_RYBY_PULSE_ON_ERASE:
		Cmd = XFL_INTEL_CONFIG_RYBY_PULSE_ERASE;
		break;

	case XFL_INTEL_RYBY_PULSE_ON_PROG:
		Cmd = XFL_INTEL_CONFIG_RYBY_PULSE_WRITE;
		break;

	case XFL_INTEL_RYBY_PULSE_ON_ERASE_PROG:
		Cmd = XFL_INTEL_CONFIG_RYBY_PULSE_ALL;
		break;

	default:
		return (XFLASH_NOT_SUPPORTED);
	}

	/*
	 * Send the command to the devices.
	 */
	DevDataPtr = GET_PARTDATA(InstancePtr);
	DevDataPtr->SendCmdSeq(InstancePtr->Geometry.BaseAddress, 0,
			       XFL_INTEL_CMD_CONFIG, Cmd);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Sends commands to erase blocks.
*
* @param	InstancePtr is the pointer to xflash object to work on.
* @param	Region is the region which the first block appears.
* @param	Block is the starting block to erase.
* @param	MaxBlocks is the number of consecutive blocks to erase.
*
* @return	The number of blocks enqueued to the part's erase buffer.
*		Region and Block parameters are incremented the number of blocks
*		queued.
*
* @note		Limitation: Only support enqueuing one block at a time.
*
******************************************************************************/
static u16 EnqueueEraseBlocks(XFlash *InstancePtr, u16 *Region,
			      u16 *Block, u16 MaxBlocks)
{
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Intel *DevDataPtr;
	u32 BlockAddress;

	/*
	 * If for some reason the maximum number of blocks to enqueue is
	 * zero, then don't do anything.
	 */
	if (MaxBlocks == 0) {
		return (0);
	}

	GeomPtr = &InstancePtr->Geometry;
	DevDataPtr = GET_PARTDATA(InstancePtr);

	/*
	 * Get the physical address to write the command to and send command.
	 */
	(void) XFlashGeometry_ToAbsolute(GeomPtr, *Region, *Block, 0,
					 &BlockAddress);
	DevDataPtr->SendCmdSeq(GeomPtr->BaseAddress, BlockAddress,
				XFL_INTEL_CMD_BLOCK_ERASE,
				XFL_INTEL_CMD_CONFIRM);

	/*
	 * Increment Region/Block.
	 */
	XFL_GEOMETRY_INCREMENT(GeomPtr, *Region, *Block);

	/*
	 * Return the number of blocks enqueued.
	 */
	return (1);
}

#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */
