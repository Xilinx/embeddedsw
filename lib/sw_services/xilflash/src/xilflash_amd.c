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
* @file xilflash_amd.c
*
* This file implements the AMD CFI Version of the XFlash Library.
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
*
* - This library and the underlying AMD flash memory does not allow re-
*	programming while code is executing from the same memory.
* - If hardware is flakey or fails, then this library could hang a thread of
*	execution.
* - This library is only tested on M29DW323DT device in 8 bit and 16 bit mode of
*	operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a ksu  04/10/08 First release.
* 1.02a ksu  06/16/09 Added Reset Bank function
*		      Added bank(s) reset operation at the top of the read
*		      function
*		      Fixed memory corruption issue in 16 bit read operation
* 2.01a ktn  03/31/10 Updated to support uniform sector WP modes.
* 2.02a sdm  07/07/10 Updated XFlashAmd_Initialize() to NOT change the erase
*		      region information of a top boot device, when the number
*		      of erase regions is not more than 1.
* 4.1	nsk  06/06/12 Updated Spansion WriteBuffer programming.
*		      (CR 781697).
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilflash.h"

#ifdef XPAR_XFL_DEVICE_FAMILY_AMD
#include "include/xilflash_amd.h"
#include "include/xilflash_cfi.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*
 * Define AMD specific data to be part of the instance. This structure will
 * overlay the XFlash_PartData structure attribute of the base class.
 */
typedef struct XFlashVendorData_AmdTag {
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
	 *		devices.
	 * PollSR - Poll the status register of the devices until the device
	 *		is ready.
	 */
	void (*SendCmd) (u32 BaseAddr, u32 Offset, u32 Cmd);
	void (*SendCmdSeq) (u32 BaseAddr, u32 Offset1, u32 Offset2, u32 Cmd1,
					u32 Cmd2);
	void (*WriteFlash) (u32 BaseAddr, u32 Offset, u32 Cmd);
	int (*WriteBuffer) (XFlash * InstancePtr, void *DestPtr,
			    void *SrcPtr, u32 Bytes);

	int (*GetStatus) (u32 BaseAddr, u32 BlockAddr);
	int (*PollSR) (u32 BaseAddr, u32 BlockAddr);
} XFlashVendorData_Amd;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************
* GET_PARTDATA - Type safe way to convert the base component VendorData type
*		to the family specific VendorData_Amd type.
*
* Macro signature:
*
*	XFlashVendorData_Amd *GET_PARTDATA(XFlash_PartData *BaseComponent)
*****************************************************************************/
#define GET_PARTDATA(BaseComponent) \
	((XFlashVendorData_Amd*) ((u32)(&(BaseComponent->VendorData))))


/************************** Function Prototypes ******************************/

static void SendCmd8(u32 BaseAddr, u32 Offset, u32 Cmd);
static void SendCmd16(u32 BaseAddr, u32 Offset, u32 Cmd);
static void SendCmd32(u32 BaseAddr, u32 Offset, u32 Cmd);

static void SendCmdSeq8(u32 BaseAddr, u32 Offset1, u32 Offset2, u32 Cmd1,
			u32 Cmd2);
static void SendCmdSeq16(u32 BaseAddr, u32 Offset1, u32 Offset2, u32 Cmd1,
			u32 Cmd2);
static void SendCmdSeq32(u32 BaseAddr, u32 Offset1, u32 Offset2, u32 Cmd1,
			u32 Cmd2);

static void WriteFlash8(u32 BaseAddr, u32 Offset, u32 Cmd);
static void WriteFlash16(u32 BaseAddr, u32 Offset, u32 Cmd);
static void WriteFlash32(u32 BaseAddr, u32 Offset, u32 Cmd);

static int WriteBuffer8(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);
static int WriteBuffer16(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);
static int WriteBuffer32(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);

static int GetStatus8(u32 BaseAddr, u32 BlockAddr);
static int GetStatus16(u32 BaseAddr, u32 BlockAddr);
static int GetStatus32(u32 BaseAddr, u32 BlockAddr);

static int PollSR8(u32 BaseAddr, u32 BlockAddr);
static int PollSR16(u32 BaseAddr, u32 BlockAddr);
static int PollSR32(u32 BaseAddr, u32 BlockAddr);

static void GetPartID(XFlash * InstancePtr);
static u16 EnqueueEraseBlocks(XFlash * InstancePtr, u16 *Region,
			      u16 *Block, u16 MaxBlocks);
static int EraseResume (XFlash * InstancePtr, u32 EraseAddrOff);
static int EraseSuspend (XFlash * InstancePtr, u32 EraseAddrOff);
static void EnterExtendedBlockMode(XFlash * InstancePtr);
static void ExitExtendedBlockMode(XFlash * InstancePtr);
static int CheckBlockProtection(XFlash * InstancePtr, u32 Offset);
static void  FlashPause(u32 MicroSeconds);
static int XFlashAmd_ResetBank(XFlash * InstancePtr, u32 Offset, u32 Bytes);
extern int XFlashGeometry_ToBlock(XFlashGeometry * InstancePtr,
				u32 AbsoluteOffset,
				u16 *Region, u16 *Block, u32 *BlockOffset);
extern int XFlashGeometry_ToAbsolute(XFlashGeometry * InstancePtr,
				u16 Region,
				u16 Block,
				u32 BlockOffset, u32 *AbsoluteOffsetPtr);
int WriteSingleBuffer(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes);
static int WriteBufferAmd(XFlash * InstancePtr, void *DestPtr,
                         void *SrcPtr, u32 Bytes);
static int WriteBufferSpansion(XFlash * InstancePtr, void *DestPtr,
                         void *SrcPtr, u32 Bytes);
void AmdDevice_is_Ready(XFlash * InstancePtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Check the device is ready or not.
*
* @param	InstancePtr is a pointer to the XFlash instance.
*
* @return
* 		This API waits until the device ready.
*
* @note		None.
*
******************************************************************************/
void AmdDevice_is_Ready(XFlash * InstancePtr)
{
	u32 FlashStatus;
	XFlashGeometry *GeomPtr;
	GeomPtr = &InstancePtr->Geometry;
	XFlashVendorData_Amd *DevDataPtr;

	DevDataPtr = GET_PARTDATA(InstancePtr);

	while (1) {
		/* Send Status Register Read Command. */
		DevDataPtr->SendCmd(GeomPtr->BaseAddress,XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_STATUS_REG_READ);
		FlashStatus = DevDataPtr->GetStatus(GeomPtr->BaseAddress,0);
                if (FlashStatus & XFL_AMD_DEVICE_READY_MASK)
			break;
	}

}
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
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XFlashAmd_Initialize(XFlash * InstancePtr)
{
	u8 Index;
	u8 BusWidth;
	u32 Layout;
	u32 PartMode;
	u32 ParamBlocks;
	u32 ParamBlockSize;
	u32 CurrentAbsoluteOffset;
	u32 CurrentAbsoluteBlock;
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	/*
	 * Grab layout and get Vendor specific part information.
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
	 */
	switch (Layout) {
		case XFL_LAYOUT_X16_X8_X1:
			DevDataPtr->SendCmd = SendCmd8;
			DevDataPtr->SendCmdSeq = SendCmdSeq8;
			DevDataPtr->WriteFlash = WriteFlash8;
			DevDataPtr->PollSR = PollSR8;
			DevDataPtr->WriteBuffer = WriteBuffer8;
			DevDataPtr->GetStatus = GetStatus8;
			break;

		case XFL_LAYOUT_X16_X16_X1:
			DevDataPtr->SendCmd = SendCmd16;
			DevDataPtr->SendCmdSeq = SendCmdSeq16;
			DevDataPtr->WriteFlash = WriteFlash16;
			DevDataPtr->PollSR = PollSR16;
			DevDataPtr->WriteBuffer = WriteBuffer16;
			DevDataPtr->GetStatus = GetStatus16;
			break;

		case XFL_LAYOUT_X16_X16_X2:
			DevDataPtr->SendCmd = SendCmd32;
			DevDataPtr->SendCmdSeq = SendCmdSeq32;
			DevDataPtr->WriteFlash = WriteFlash32;
			DevDataPtr->PollSR = PollSR32;
			DevDataPtr->WriteBuffer = WriteBuffer32;
			DevDataPtr->GetStatus = GetStatus32;
			break;

		default:
			return (XFLASH_PART_NOT_SUPPORTED);
	}

	/*
	 * Get part ID and Reset the part.
	 */
	GetPartID(InstancePtr);

	/*
	 * Setup bank information as per the boot block location.
	 */

	 /*
	  * Get device operational mode.
	  */
	 PartMode = (Layout & XFL_LAYOUT_PART_MODE_MASK);
	if (PartMode == XFL_LAYOUT_PART_MODE_8) {
		BusWidth = 1;
	}
	else if (PartMode == XFL_LAYOUT_PART_MODE_16) {
		BusWidth = 2;
	}
	else {
		return (XFLASH_PART_NOT_SUPPORTED);
	}

	InstancePtr->Geometry.DeviceSize = InstancePtr->Geometry.DeviceSize /
						BusWidth;
	for (Index = 0; Index < InstancePtr->Geometry.NumEraseRegions;
		Index++) {
		InstancePtr->Geometry.EraseRegion[Index].Size =
		InstancePtr->Geometry.EraseRegion[Index].Size / BusWidth;
	}
	/*
	 * If device is top boot then change erase region information.
	 */
	if (((InstancePtr->Geometry.BootMode == XFL_AMD_TOP_BOOT) ||
	    (InstancePtr->Geometry.BootMode == XFL_AMD_TOP_WP_UNIFORM)) &&
	    (InstancePtr->Geometry.NumEraseRegions > 1)) {
		/*
		 * Move boot block region to top.
		 */
		ParamBlocks = InstancePtr->Geometry.EraseRegion[0].Number;
		ParamBlockSize = InstancePtr->Geometry.EraseRegion[0].Size;

		InstancePtr->Geometry.EraseRegion[0].Number =
			InstancePtr->Geometry.EraseRegion[1].Number;
		InstancePtr->Geometry.EraseRegion[0].Size =
			InstancePtr->Geometry.EraseRegion[1].Size ;

		InstancePtr->Geometry.EraseRegion[1].Number = ParamBlocks ;
		InstancePtr->Geometry.EraseRegion[1].Size = ParamBlockSize;

		/*
		 * Calculate Erase region offset and block address.
		 */
		CurrentAbsoluteOffset = 0;
		CurrentAbsoluteBlock = 0;
		for (Index = 0;
			Index < InstancePtr->Geometry.NumEraseRegions;
			Index++) {
			/*
			 * Calculate part offset.
			 */
			InstancePtr->Geometry.EraseRegion[Index].AbsoluteOffset
					= CurrentAbsoluteOffset;
			InstancePtr->Geometry.EraseRegion[Index].AbsoluteBlock
					= CurrentAbsoluteBlock;

			/*
			 * Increment absolute counters.
			 */
			CurrentAbsoluteOffset +=
			(InstancePtr->Geometry.EraseRegion[Index].Size *
			InstancePtr->Geometry.EraseRegion[Index].Number);
			CurrentAbsoluteBlock +=
			InstancePtr->Geometry.EraseRegion[Index].Number;
		}
	}
	else if (BusWidth != 1) {
		/*
		 * Calculate Erase region offset and block address.
		 */
		CurrentAbsoluteOffset = 0;
		CurrentAbsoluteBlock = 0;
		for (Index = 0;
			Index < InstancePtr->Geometry.NumEraseRegions;
			Index++) {
			/*
			 * Calculate part offset.
			 */
			InstancePtr->Geometry.EraseRegion[Index].AbsoluteOffset
					= CurrentAbsoluteOffset;
			InstancePtr->Geometry.EraseRegion[Index].AbsoluteBlock
					= CurrentAbsoluteBlock;

			/*
			 * Increment absolute counters.
			 */
			CurrentAbsoluteOffset +=
			(InstancePtr->Geometry.EraseRegion[Index].Size *
			InstancePtr->Geometry.EraseRegion[Index].Number);
			CurrentAbsoluteBlock +=
			InstancePtr->Geometry.EraseRegion[Index].Number;
		}
	}


	(void) XFlashAmd_Reset(InstancePtr);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* The routine reads the data from the AMD flash device and copies it into
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
*		- XFLASH_PART_NOT_SUPPORTED if the command set algorithm or
*		  Layout is not supported by any specific flash device family
*		  compiled into the system.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XFlashAmd_Read(XFlash *InstancePtr, u32 Offset, u32 Bytes, void *DestPtr)
{
	u8 *Dest8BitPtr;
	u8 *Src8BitPtr;
	u16 *Dest16BitPtr;
	u16 *Src16BitPtr;
	u32  PartMode;
	u32 Index;
	u32 Startoffset;
	u32 EndOffset;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Amd *DevDataPtr;
	volatile u16 FlashStatus;

	/* Verify inputs are valid. */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	if(DestPtr == NULL) {
		return (XST_FAILURE);
	}

	if(Bytes == 0) {
		return (XST_FAILURE);
	}

	GeomPtr = &InstancePtr->Geometry;
	DevDataPtr = GET_PARTDATA(InstancePtr);

	if (InstancePtr->Geometry.MemoryLayout == XFL_LAYOUT_X16_X16_X1) {
		Startoffset = Offset >> 1;
		EndOffset = Offset + Bytes - 1;
		EndOffset = EndOffset >> 1;
	}
	else {
		Startoffset = Offset;
	}
	PartMode = (InstancePtr->Geometry.MemoryLayout &
			XFL_LAYOUT_PART_MODE_MASK);

	/*
	 * Check to make sure start address is within the device.
	 */
	if (!XFL_GEOMETRY_IS_ABSOLUTE_VALID(&InstancePtr->Geometry,
		Startoffset)) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	if (XFlashAmd_ResetBank(InstancePtr, Startoffset, Bytes)
		!= XST_SUCCESS) {
		return (XST_FAILURE);
	}

	/*
	 * Perform copy to the user buffer from the flash buffer.
	 */
	if (PartMode == XFL_LAYOUT_PART_MODE_8) {
		/*
		 * Perform copy to the user buffer from the buffer.
		 */
		Src8BitPtr = (u8*) (((volatile u8*)InstancePtr->Geometry.
				     BaseAddress) + Offset);
		Dest8BitPtr = (u8*) DestPtr;

		for (Index = 0; Index < Bytes; Index++) {
			Dest8BitPtr[Index] = Src8BitPtr[Index];
		}

	}
	else if (PartMode == XFL_LAYOUT_PART_MODE_16) {

		/* Wait until device is ready. */
		AmdDevice_is_Ready(InstancePtr);

		/* Send Status Register Clear Command. */
		DevDataPtr->SendCmd(GeomPtr->BaseAddress,XFL_AMD_CMD1_ADDR,
						XFL_AMD_CMD_STATUS_REG_CLEAR);
		/* Perform copy to the user buffer from the buffer. */
		Src16BitPtr = (u16*) (((volatile u16*) InstancePtr->Geometry.
				       BaseAddress) + Startoffset);
		Dest16BitPtr = (u16*) DestPtr;
		for (Index = 0; Index < (Bytes/2); Index++) {
			Dest16BitPtr[Index] = Src16BitPtr[Index];
		}
	} else {
		return (XFLASH_PART_NOT_SUPPORTED);
	}

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
*
* Programs the AMD flash device with data stored in the user buffer.
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
*		- XFLASH_ADDRESS_ERROR if the destination address range is
*		  not completely within the addressable areas of the device(s).
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XFlashAmd_Write(XFlash * InstancePtr, u32 Offset, u32 Bytes, void *SrcPtr)
{
	int Status;
	u32 StartOffset;
	u32 EndOffset;
	XFlashVendorData_Amd *DevDataPtr;

	/* Verify inputs are valid. */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	if(SrcPtr == NULL) {
		return (XST_FAILURE);
	}

	/* Nothing specified to be programmed. */
	if (Bytes == 0) {
		return (XST_SUCCESS);
	}

	if (InstancePtr->Geometry.MemoryLayout == XFL_LAYOUT_X16_X16_X1){
		StartOffset = Offset >> 1;
		EndOffset = Offset + Bytes -1;
		EndOffset = EndOffset >> 1;
	}
	else {
		StartOffset = Offset ;
		EndOffset = Offset + Bytes -1;
	}

	/* Verify the address range is within the part. */
	if (!XFL_GEOMETRY_IS_ABSOLUTE_VALID(&InstancePtr->Geometry, StartOffset)
		 || !XFL_GEOMETRY_IS_ABSOLUTE_VALID(&InstancePtr->Geometry,
			EndOffset)) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/* Call the proper write buffer function. */
	DevDataPtr = GET_PARTDATA(InstancePtr);
	Status = DevDataPtr->WriteBuffer(InstancePtr, (void *)Offset, SrcPtr,
					(Bytes));

	/* Reset the bank(s) so that it returns to the read mode. */
	(void) XFlashAmd_ResetBank(InstancePtr, Offset, Bytes);

	return (Status);
}

/*****************************************************************************/
/**
*
* Erases the specified address range in the AMD Flash device.
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
*		- XST_FAILURE if failed.
*
* @note		Application has to check block protection status of all the
*		which needs to be erased before calling this API. If any block
* 		is protected then this API will return error.
*
******************************************************************************/
int XFlashAmd_Erase(XFlash * InstancePtr, u32 Offset, u32 Bytes)
{
	u16 StartRegion;
	u16 EndRegion;
	u16 StartBlock;
	u16 EndBlock;
	u16 BlocksLeft;
	u16 BlocksQueued;
	u32 Dummy;
	u32 StartOffset;
	u32 EndOffset;
	int Status;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Amd *DevDataPtr;

	/* Verify inputs are valid. */
	if (InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	GeomPtr = &InstancePtr->Geometry;

	/* Handle case when zero bytes is provided. */
	if (Bytes == 0) {
		return (XST_SUCCESS);
	}

	if (InstancePtr->Geometry.MemoryLayout == XFL_LAYOUT_X16_X16_X1) {
		StartOffset = Offset >> 1;
		EndOffset = Offset + Bytes - 1;
		EndOffset = EndOffset >> 1;
	}
	else {
		StartOffset = Offset;
		EndOffset = Offset + Bytes - 1;
	}
	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, StartOffset, &StartRegion,
							&StartBlock, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Convert the ending address to block coordinates. This also verifies
	 * the ending address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, EndOffset,
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
		Status = DevDataPtr->PollSR(GeomPtr->BaseAddress, StartOffset);
		if (Status != XFLASH_READY) {
			(void) XFlashAmd_ResetBank(InstancePtr, StartOffset,
				Bytes);
			return (Status);
		}
	}

	/* Reset the bank(s) so that it returns to the read mode. */
	(void) XFlashAmd_ResetBank(InstancePtr, StartOffset, Bytes);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Locks the blocks in the specified range of the AMD flash device.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset into the device(s) address space from which
*		to begin block locking.
* @param	Bytes indicates the number of bytes to Lock in the Block
*		starting from Offset.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XFLASH_ADDRESS_ERROR if the destination address range is
*		  not completely within the addressable areas of the device(s).
*		- XST_FAILURE if failed.
*
* @note		This API should be called after applying VID (voltage) to the RP
*		pin of flash device. This API is not tested.
*
******************************************************************************/
int XFlashAmd_Lock(XFlash * InstancePtr, u32 Offset, u32 Bytes)
{
	u8 Mode;
	u8 AddrMul;
	volatile u16 StatusReg;
	u16 Region;
	u16 Block;
	u16 NumAttempt = 0;
	u32 Dummy;
	u32 BaseAddress;
	u32 BlockAddress;
	u32 GroupAddress;
	int Status;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);

	GeomPtr = &InstancePtr->Geometry;
	BaseAddress = GeomPtr->BaseAddress;

	Mode = (InstancePtr->Geometry.MemoryLayout &
			XFL_LAYOUT_PART_MODE_MASK) >> 8;

	if (Mode == 2) {
		AddrMul = 1;
	}
	else if (Mode == 1) {
		AddrMul = 2;
	}
	else {
		return (XST_FAILURE);
	}

	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset, &Region, &Block,
						&Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Get the physical address to write the command to and send command.
	 */
	(void) XFlashGeometry_ToAbsolute(GeomPtr, Region, Block, 0,
			 &BlockAddress);

	GroupAddress = (BlockAddress | (XFL_AMD_PROT_STATUS_OFFSET * AddrMul));

	 /*
	  * Set-up Phase.
	  */
	DevDataPtr->WriteFlash(BaseAddress,
				GroupAddress, XFL_AMD_CMD_GROUP_PROTECT1);

	Status = XST_FAILURE;
	do {
		/*
		 * Protect Phase.
		 */
		DevDataPtr->WriteFlash(BaseAddress,
				GroupAddress, XFL_AMD_CMD_GROUP_PROTECT1);

		FlashPause(100);

		/*
		 * Verify Phase.
		 */
		DevDataPtr->WriteFlash(BaseAddress,
				GroupAddress, XFL_AMD_CMD_GROUP_PROTECT2);

		FlashPause(4);

		StatusReg = DevDataPtr->GetStatus(InstancePtr->Geometry.
					BaseAddress, GroupAddress);
		/*
		 * Check Protection completed.
		 */
		if(StatusReg == XFL_AMD_GROUP_PROTECTED) {
			Status = XST_SUCCESS;
		        break;
		}
	}
	while(++NumAttempt < 25);

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	(void) XFlashAmd_ResetBank(InstancePtr, Offset, Bytes);

	return (Status);
}

/*****************************************************************************/
/**
*
* Unlocks the all blocks in the AMD flash device.
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
*		- XST_FAILURE if failed.
*
* @note		This API should be called after applying VID (voltage) to the RP
*		pin of flash device. This API is not tested.
*
******************************************************************************/
int XFlashAmd_Unlock(XFlash * InstancePtr, u32 Offset, u32 Bytes)
{
	u8 Mode;
	u8 AddrMul;
	u8 TopBoot = 0;
	u8 MakeUnprotectStep = 0;
	u8 NoOfBlockInGroup;
	volatile u16 StatusReg;
	u16 Region = 0;
	u16 NumAttempt = 0;
	u16 Block = 0;
	u32 BaseAddress;
	u32 Index;
	u32 GroupAddress;
	int Status = XST_FAILURE;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);
	GeomPtr = &InstancePtr->Geometry;
	BaseAddress = GeomPtr->BaseAddress;

	if (InstancePtr->Geometry.BootMode == XFL_AMD_TOP_BOOT) {
		TopBoot = 1;
	}

	Mode = (InstancePtr->Geometry.MemoryLayout &
		XFL_LAYOUT_PART_MODE_MASK) >> 8;

	if (Mode == 2) {
		AddrMul = 1;
	}
	else if (Mode == 1) {
		AddrMul = 2;
	}
	else {
		return (XST_FAILURE);
	}


	/*
	 * Protect all groups of block in the device.
	 */
	for (Block = 0; Block < GeomPtr->NumBlocks; Block++) {
		/*
		 * Get the physical address.
		 */
		(void) XFlashGeometry_ToAbsolute(GeomPtr, Region, Block, 0,
						 &GroupAddress);

		if (XFlashAmd_Lock(InstancePtr, GroupAddress, 0) !=
			XST_SUCCESS ) {
			return XST_FAILURE;
		}

		if (TopBoot) {
			if ((Block == 0) || (Block >= 63)) {
				NoOfBlockInGroup = 1;
			}
			else if ((Block == 1) || (Block == 60)) {
				NoOfBlockInGroup = 3;
			}
			else {
				NoOfBlockInGroup = 4;
			}
		}
		else {
			if ((Block <= 7) || (Block == 70)) {
				NoOfBlockInGroup = 1;
			}
			else if ((Block == 8) || (Block == 67)) {
				NoOfBlockInGroup = 3;
			}
			else {
				NoOfBlockInGroup = 4;
			}
		}

		for (Index = 0; Index < NoOfBlockInGroup; Index++) {
			/*
			 * Increment Region/Block.
			 */
			XFL_GEOMETRY_INCREMENT(GeomPtr, Region, Block);
			Block++;
		}
	}

	/*
	 * Setup Phase.
	 */
	GroupAddress = (0x0000 | (AddrMul * XFL_AMD_CHIP_UNPROTECT_ADDR));

	DevDataPtr->WriteFlash(BaseAddress,
				GroupAddress, XFL_AMD_CMD_GROUP_PROTECT1);

	Block = 0;
	Region = 0;
	MakeUnprotectStep = 0;
	while (NumAttempt < 1000) {
		(void) XFlashGeometry_ToAbsolute(GeomPtr, Region, Block, 0,
				 &GroupAddress);

		GroupAddress = (GroupAddress |
				(AddrMul * XFL_AMD_CHIP_UNPROTECT_ADDR));

		if (MakeUnprotectStep == 0) {
			/*
			 * Unprotect phase.
			 */
			 DevDataPtr->WriteFlash(InstancePtr->Geometry.
					BaseAddress,GroupAddress,
					XFL_AMD_CMD_GROUP_PROTECT1);

			 FlashPause(10000);
		}

		/*
		 * Verify Phase.
		 */
		DevDataPtr->WriteFlash(InstancePtr->Geometry.
				BaseAddress,GroupAddress,
				XFL_AMD_CMD_GROUP_PROTECT2);

		FlashPause(4);

		StatusReg = DevDataPtr->GetStatus(InstancePtr->Geometry.
					BaseAddress, GroupAddress);

		if (StatusReg == XFL_AMD_GROUP_UNPROTECTED) {
			if (Block == ((GeomPtr->NumBlocks) - 1)) {
				Status = XST_SUCCESS;
				break;
			}
			else {
				MakeUnprotectStep = 1;
				if (TopBoot) {
					if ((Block == 0) || (Block >= 63)) {
						NoOfBlockInGroup = 1;
					}
					else if ((Block == 1) || (Block == 60))
					{
						NoOfBlockInGroup = 3;
					}
					else {
						NoOfBlockInGroup = 4;
					}
				}
				else {
					if ((Block <= 7) || (Block == 70)) {
						NoOfBlockInGroup = 1;
					}
					else if ((Block == 8) || (Block == 67))
					{
						NoOfBlockInGroup = 3;
					}
					else {
						NoOfBlockInGroup = 4;
					}
				}

				for (Index = 0; Index < NoOfBlockInGroup;
						Index++) {
					/*
					 * Increment Region/Block.
					 */
					XFL_GEOMETRY_INCREMENT(GeomPtr, Region,
					Block);
					Block++;
				}
			}
		}
		else {
			MakeUnprotectStep = 0;
			NumAttempt++;
		}
	}

	/*
	 * Reset the bank(s) so that it returns to the read mode.
	 */
	(void) XFlashAmd_ResetBank(InstancePtr, Offset, Bytes);

	return (Status);
}

/*****************************************************************************/
/**
*
* The function can be used to erase the whole flash chip. Each Block is erased
* in turn. The function only returns when all of the Blocks have been erased or
* have generated an error..
*
* @param	InstancePtr is the pointer to the XFlash instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		Application has to check block protection status of all the
*		blocks before calling this API. If any block is protected then
*		this API will return error.
*
******************************************************************************/
int XFlashAmd_EraseChip(XFlash * InstancePtr)
{
	int Status = XST_SUCCESS;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);

	GeomPtr = &InstancePtr->Geometry;

	/*
	 * Send Chip Erase command.
	 */
	DevDataPtr->SendCmdSeq(GeomPtr->BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
			XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);
	DevDataPtr->SendCmd(GeomPtr->BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD_ERASE1);
	DevDataPtr->SendCmdSeq(GeomPtr->BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
			XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->SendCmd(GeomPtr->BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD_ERASE_CHIP);


	/*
	 * Wait until Program/Erase Controller starts.
	 */
	Status = DevDataPtr->PollSR(GeomPtr->BaseAddress, 0);
	(void) XFlashAmd_Reset(InstancePtr);
	if (Status != XFLASH_READY) {
		return (Status);
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function places the flash in the Read Array mode. In this mode the flash
* can be read as normal memory. All of the other functions leave the flash in
* the Read Array mode so this is not strictly necessary. It is provided for
* completeness and in case of problems.
*
* @param	None.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed to reset device.
*
* @note		None.
*
******************************************************************************/

int XFlashAmd_Reset(XFlash * InstancePtr)
{
	int Status;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Send reset for all region/bank(s) in the device.
	 */
	Status = XFlashAmd_ResetBank(InstancePtr, 0,
				InstancePtr->Geometry.DeviceSize);
	return Status;
}

/*****************************************************************************/
/**
*
* This function places the flash region in the Read Array mode. In this mode the
* flash can be read as normal memory. All of the other functions leave the flash
* in the Read Array mode so this is not strictly necessary. It is provided for
* completeness and in case of problems.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the bank/region address.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed to reset device.
*
* @note		None.
*
******************************************************************************/

static int XFlashAmd_ResetBank(XFlash * InstancePtr, u32 Offset, u32 Bytes)
{
	XFlashVendorData_Amd *DevDataPtr;
	u16 Region, Block;
	u32 Dummy, Status;
	XFlashGeometry *GeomPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	/*
	 * Handle case when zero bytes is provided.
	 */
	if (Bytes == 0) {
		return XST_SUCCESS;
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);
	GeomPtr = &InstancePtr->Geometry;

	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset, &Region,
							&Block, &Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	while ((InstancePtr->Geometry.EraseRegion[Region].AbsoluteOffset <=
		(Offset + Bytes - 1)) &&
		(Region < InstancePtr->Geometry.NumEraseRegions)) {
		/*
		 * Send the Command Reset. Use the max write
		 * width to notify parts of all layouts.
		 */
		DevDataPtr->WriteFlash(InstancePtr->Geometry.BaseAddress,
					InstancePtr->Geometry.EraseRegion
					[Region].AbsoluteOffset,
					XFL_AMD_CMD_RESET);
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
* Performs the AMD device specific control functions.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Command is the device specific command to issue.
* @param	Parameters specifies the arguments passed to the device control
*		function.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_NOT_SUPPORTED if the command is not
* 		  recognized/supported by the device(s).
*
* @note		None.
*
******************************************************************************/
int XFlashAmd_DeviceControl(XFlash * InstancePtr, u32 Command,
						DeviceCtrlParam *Parameters)
{
	u32 BlockAddr;
	u32 Offset;
	int Status = XST_SUCCESS;
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);

	switch (Command) {
		case XFL_DEVCTL_GET_GEOMETRY:
			Parameters->GeometryParam.GeometryPtr =
							&InstancePtr->Geometry;
			Status = (XST_SUCCESS);
			break;

		case XFL_DEVCTL_GET_PROPERTIES:
			Parameters->PropertiesParam.PropertiesPtr =
						&InstancePtr->Properties;
			Status = (XST_SUCCESS);
			break;

		case XFL_DEVCTL_ERASE_RESUME:
			BlockAddr = (*((u32*)Parameters));
			Status = EraseResume (InstancePtr, BlockAddr);
			break;

		case XFL_DEVCTL_ERASE_SUSPEND:
			BlockAddr = (*((u32*)Parameters));
			Status = EraseSuspend (InstancePtr, BlockAddr);
			break;

		case XFL_DEVCTL_ENTER_EXT_MODE:
			EnterExtendedBlockMode(InstancePtr);
			Status = (XST_SUCCESS);
			break;

		case XFL_DEVCTL_EXIT_EXT_MODE:
			ExitExtendedBlockMode(InstancePtr);
			Status = (XST_SUCCESS);
			break;

		case XFL_DEVCTL_PROTECTION_STATUS:
			Offset = (*((u32*)Parameters));
			Status = CheckBlockProtection(InstancePtr, Offset);
			break;

		case XFL_DEVCTL_CHIP_ERASE:
			Status = XFlashAmd_EraseChip(InstancePtr);
			break;

		default:
			Status = (XFLASH_NOT_SUPPORTED);
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* Writes a command using a 8-bit bus cycle.
*
* @param	BaseAddr is the base address of device.
* @param	Offset is the offset into the device(s) address space on which
*		command is required to be written.
* @param	Cmd is the command/data to write at BaseAddress + Offset.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmd8(u32 BaseAddr, u32 Offset, u32 Cmd)
{
	Offset = ((2*Offset) + (!(Offset & 0x1)));
	WRITE_FLASH_8(((volatile u8*)BaseAddr) + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command using a 16-bit bus cycle.
*
* @param	BaseAddr is the base address of device.
* @param	Offset is the offset into the device(s) address space on which
*		command is required to be written.
* @param	Cmd is the command/data to write at BaseAddress + Offset.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmd16(u32 BaseAddr, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_16(((volatile u16*)BaseAddr) + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command using a 32-bit bus cycle.
*
* @param	BaseAddr is the base address of device.
* @param	Offset is the offset into the device(s) address space on which
*		command is required to be written.
* @param	Cmd is the command/data to write at BaseAddress + Offset.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmd32(u32 BaseAddr, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_32(((volatile u32*)BaseAddr) + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command sequence using 8-bit bus cycles.
*
* @param	BaseAddr is the base address of device.
* @param	Offset1 is the first offset address into the device(s) address
*		space on which first command (Cmd1) is required to be written.
* @param	Offset2 is the second offset address into the device(s) address
*		space on which second command (Cmd2) is required to be written.
* @param	Cmd1 is the first command to write at BaseAddress + Offset1.
* @param	Cmd2 is the second command to write at BaseAddress + Offset2.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmdSeq8(u32 BaseAddr, u32 Offset1, u32 Offset2, u32 Cmd1,
				u32 Cmd2)
{
	Offset1 = ((2*Offset1) + (!(Offset1 & 0x1)));
	Offset2 = ((2*Offset2) + (!(Offset2 & 0x1)));
	WRITE_FLASH_8(((volatile u8*)BaseAddr) + Offset1, Cmd1);
	WRITE_FLASH_8(((volatile u8*)BaseAddr) + Offset2, Cmd2);
}

/*****************************************************************************/
/**
*
* Writes a command sequence using 16-bit bus cycles.
*
* @param	BaseAddr is the base address of device.
* @param	Offset1 is the first offset address into the device(s) address
*		space on which first command (Cmd1) is required to be written.
* @param	Offset2 is the second offset address into the device(s) address
*		space on which second command (Cmd2) is required to be written.
* @param	Cmd1 is the first command to write at BaseAddress + Offset1.
* @param	Cmd2 is the second command to write at BaseAddress + Offset2.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmdSeq16(u32 BaseAddr, u32 Offset1, u32 Offset2, u32 Cmd1,
				u32 Cmd2)
{
	WRITE_FLASH_16(((volatile u16*)BaseAddr) + Offset1, Cmd1);
	WRITE_FLASH_16(((volatile u16*)BaseAddr) + Offset2, Cmd2);
}

/*****************************************************************************/
/**
*
* Writes a command sequence using 32-bit bus cycles.
*
* @param	BaseAddr is the base address of device.
* @param	Offset1 is the first offset address into the device(s) address
*		space on which first command (Cmd1) is required to be written.
* @param	Offset2 is the second offset address into the device(s) address
*		space on which second command (Cmd2) is required to be written.
* @param	Cmd1 is the first command to write at BaseAddress + Offset1.
* @param	Cmd2 is the second command to write at BaseAddress + Offset2.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendCmdSeq32(u32 BaseAddr, u32 Offset1, u32 Offset2, u32 Cmd1,
				u32 Cmd2)
{
	WRITE_FLASH_32(((volatile u32*)BaseAddr) + Offset1, Cmd1);
	WRITE_FLASH_32(((volatile u32*)BaseAddr) + Offset2, Cmd2);
}

/*****************************************************************************/
/**
*
* Writes a command/data using a 8-bit bus cycle.
*
* @param	BaseAddr is the base address of device.
* @param	Offset is the offset into the device(s) address space on which
*		command is required to be written.
* @param	Cmd is the command/data to write at BaseAddress + Offset.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WriteFlash8(u32 BaseAddr, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_8(((volatile u8*)BaseAddr) + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command/data using a 16-bit bus cycle.
*
* @param	BaseAddr is the base address of device.
* @param	Offset is the offset into the device(s) address space on which
*		command is required to be written.
* @param	Cmd is the command/data to write at BaseAddress + Offset.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WriteFlash16(u32 BaseAddr, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_16(((volatile u16*)BaseAddr) + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* Writes a command/data using a 32-bit bus cycle.
*
* @param	BaseAddr is the base address of device.
* @param	Offset is the offset into the device(s) address space on which
*		command is required to be written.
* @param	Cmd is the command/data to write at BaseAddress + Offset.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WriteFlash32(u32 BaseAddr, u32 Offset, u32 Cmd)
{
	WRITE_FLASH_32(((volatile u32*)BaseAddr) + Offset, Cmd);
}

/*****************************************************************************/
/**
*
* This function is used to program the memory in signal bank. It does not erase
* the flash first and will fail if the block(s) are not erased first. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on.
* @param	DestPtr is the physical destination address in flash memory
*		space.
* @param	SrcPtr is the source data.
* @param	Bytes is the number of bytes to program.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific.
*
* @note		None.
*
******************************************************************************/
static int WriteBuffer8(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	u8 *SourcePtr = (u8*)SrcPtr;
	u8 *DestinationPtr = (u8*)DestPtr;
	u32 BaseAddress;
	u32 Index = 0;
	int Status = XST_SUCCESS;
	XFlashVendorData_Amd *DevDataPtr;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;

	/*
	 * Send the Unlock Bypass command.
	 */
	DevDataPtr->SendCmdSeq(BaseAddress,
				XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
				XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->SendCmd(BaseAddress,
				XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_UNLOCK_BYPASS);

	Index = 0;
	while (Bytes != 0) {
		DevDataPtr->WriteFlash(BaseAddress,
			0, XFL_AMD_CMD_PROGRAM);

		DevDataPtr->WriteFlash(BaseAddress,
			(u32)DestinationPtr, SourcePtr[Index]);

		Status = DevDataPtr->PollSR(BaseAddress,
						(u32)DestinationPtr);
		if (Status != XFLASH_READY) {
			(void) XFlashAmd_ResetBank(InstancePtr, (u32)DestPtr,
					Bytes);
			return (Status);
		}
		DestinationPtr++;
		Index++;
		Bytes -= 1;
	}

	/*
	 * Unlock Bypass Reset.
	 */
	DevDataPtr->WriteFlash(BaseAddress, 0,
		XFL_AMD_CMD_UNLOCK_BYPASS_RESET1);
	DevDataPtr->WriteFlash(BaseAddress,0,
		XFL_AMD_CMD_UNLOCK_BYPASS_RESET2);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function is used to program the memory in signal bank. It does not erase
* the flash first and will fail if the block(s) are not erased first. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on.
* @param	DestPtr is the physical destination address in flash memory
*		space.
* @param	SrcPtr is the source data.
* @param	Bytes is the number of bytes to program.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific.
*
* @note		None.
*
******************************************************************************/
static int WriteBuffer16(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	u32 Status;
	u16 *SrcWordPtr = (u16*)SrcPtr;
	u16 *DestWordPtr = (u16*)DestPtr;

	/*
	 * Make sure DestPtr and SrcPtr are aligned to a 16-bit word.
	 */
	if (((int) SrcWordPtr & 1) || ((int) DestWordPtr & 1)) {
		return (XFLASH_ALIGNMENT_ERROR);
	}

	if (InstancePtr->Properties.PartID.ManufacturerID == 0x01)
		Status = WriteBufferSpansion(InstancePtr,DestPtr,SrcPtr,Bytes);
	else
		Status = WriteBufferAmd(InstancePtr,DestPtr,SrcPtr,Bytes);

	return Status;
}


/*****************************************************************************/
/**
*
* This function is used to program Spansion devices. It does not erase
* the flash first and will fail if the block(s) are not erased first. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on.
* @param	DestPtr is the physical destination address in flash memory
*		space.
* @param	SrcPtr is the source data.
* @param	Bytes is the number of bytes to program.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific.
*
* @note		None.
*
******************************************************************************/
static int WriteBufferSpansion(XFlash * InstancePtr, void *DestPtr,
                         void *SrcPtr, u32 Bytes)
{
       u32 DestinationPtr = (u32)DestPtr;
       u32 BaseAddress= InstancePtr->Geometry.BaseAddress;
       u16* Tempsrcptr = (u16 *)SrcPtr;
       u32 Index = 0;
       int Status = XST_SUCCESS;
       XFlashVendorData_Amd *DevDataPtr = GET_PARTDATA(InstancePtr);
       u32 BufferSize = InstancePtr->Properties.ProgCap.WriteBufferSize;

	while (Bytes != 0)
	{
		/* Bytes to write should not exceed the buffer size. */
		if (Bytes > BufferSize)
		{
			Status = WriteSingleBuffer(InstancePtr, DestPtr,
						Tempsrcptr, BufferSize);
			Bytes = Bytes - BufferSize;
			DestPtr = DestPtr + BufferSize;
			Tempsrcptr = Tempsrcptr + BufferSize/2;
		}
		else
		{
			Status = WriteSingleBuffer(InstancePtr, DestPtr,
						Tempsrcptr, Bytes);
			Bytes = 0;
		}
		if (Status != XST_SUCCESS)
		{
			return Status;
		}
	}
	return Status;
}


/*****************************************************************************/
/**
*
* This function is used to program Amd devices. It does not erase
* the flash first and will fail if the block(s) are not erased first. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on.
* @param	DestPtr is the physical destination address in flash memory
*		space.
* @param	SrcPtr is the source data.
* @param	Bytes is the number of bytes to program.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific.
*
* @note		None.
*
******************************************************************************/
static int WriteBufferAmd(XFlash * InstancePtr, void *DestPtr,
                         void *SrcPtr, u32 Bytes)
{
	u16 *SourcePtr = (u16*)SrcPtr;
	u32 DestinationPtr = (u32)DestPtr;
	u32 BaseAddress = InstancePtr->Geometry.BaseAddress;
	u32 Index = 0;
	int Status = XST_SUCCESS;
	XFlashVendorData_Amd *DevDataPtr = GET_PARTDATA(InstancePtr);

	/* Send the Unlock Bypass command. */
	DevDataPtr->SendCmdSeq(BaseAddress,
				XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
				XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->SendCmd(BaseAddress,
				XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_UNLOCK_BYPASS);

	Index = 0;
	while (Bytes != 0) {
		DevDataPtr->WriteFlash(BaseAddress,
			0, XFL_AMD_CMD_PROGRAM);

		DevDataPtr->WriteFlash(BaseAddress,
			(u32)DestinationPtr, SourcePtr[Index]);

		Status = DevDataPtr->PollSR(BaseAddress,
						(u32)DestinationPtr);
		if (Status != XFLASH_READY) {
			(void) XFlashAmd_ResetBank(InstancePtr, (u32)DestPtr,
					Bytes);
			return (Status);
		}
		DestinationPtr++;
		Index++;
		Bytes -= 2;
	}

	/* Unlock Bypass Reset. */
	DevDataPtr->WriteFlash(BaseAddress, 0,
		XFL_AMD_CMD_UNLOCK_BYPASS_RESET1);
	DevDataPtr->WriteFlash(BaseAddress, 0,
		XFL_AMD_CMD_UNLOCK_BYPASS_RESET2);

	return (XST_SUCCESS);
}
/*****************************************************************************/
/**
*
* This function is used to program Buffer sized data. It does not erase
* the flash first and will fail if the block(s) are not erased first. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on.
* @param	DestPtr is the physical destination address in flash memory
*		space.
* @param	SrcPtr is the source data.
* @param	Bytes is the number of bytes to program.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific.
*
* @note		None.
*
******************************************************************************/
int WriteSingleBuffer(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	u16 *SourcePtr = (u16*)SrcPtr;
	u32 DestinationPtr = (u32)DestPtr;
	u32 BaseAddress;
	u32 Index = 0;
	int Status = XST_SUCCESS;
	XFlashVendorData_Amd *DevDataPtr;
	u32 SectorAddress;
	u32 WordCount = Bytes/2;
	u16 StartRegion;
	u16 EndRegion;
	u16 StartBlock;
	u16 EndBlock;
	u32 Dummy;
	u32 BlockAddress;
	volatile u16 FlashStatus;
	XFlashGeometry *GeomPtr;

	GeomPtr = &InstancePtr->Geometry;
	Status = XFlashGeometry_ToBlock(GeomPtr, (u32)DestPtr/2, &StartRegion,
					&StartBlock, &Dummy);
	(void) XFlashGeometry_ToAbsolute(GeomPtr, StartRegion, StartBlock, 0,
						 &BlockAddress);
	SectorAddress = BlockAddress *2;
	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;
	DestinationPtr = DestinationPtr/2;

	/* Wait until device is ready. */
	AmdDevice_is_Ready(InstancePtr);

	/* Send Status Register Clear Command. */
	DevDataPtr->SendCmd(BaseAddress,XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_STATUS_REG_CLEAR);
	/* Send two Unlock cycles Commands. */
	DevDataPtr->SendCmdSeq(BaseAddress,
				XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
				XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);
	/* Send Write to Buffer Command. */
	DevDataPtr->SendCmd(BaseAddress,(SectorAddress/2),
				XFL_AMD_CMD_WRITE_BUFFER);
	/* Issue Number of words to Write at sector address. */
	DevDataPtr->SendCmd(BaseAddress,(SectorAddress/2),
				(WordCount -  1));
	Index = 0;

	/* Write Data to Buffer. */
	while (WordCount != 0) {
		DevDataPtr->WriteFlash(BaseAddress,
				(u32)DestinationPtr, SourcePtr[Index]);
		DestinationPtr++;
		Index++;
		WordCount -= 1;
	}

	/* Send Write buffer program confirm command. */
	DevDataPtr->WriteFlash(BaseAddress,(SectorAddress/2),
				XFL_AMD_CMD_PROGRAM_BUFFER);
	Status = DevDataPtr->PollSR(BaseAddress,(u32)(DestinationPtr * 2));
	if (Status != XFLASH_READY) {
		(void) XFlashAmd_ResetBank(InstancePtr, (u32)DestPtr,
					Bytes);
		return Status;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to program the memory in sin gal bank. It does not erase
* the flash first and will fail if the block(s) are not erased first. The
* device(s) are programmed in parallel.
*
* @param	InstancePtr is the instance to work on.
* @param	DestPtr is the physical destination address in flash memory
*		space.
* @param	SrcPtr is the source data.
* @param	Bytes is the number of bytes to program.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_ERROR if a write error occurred. This error is
*		  usually device specific.
*		- XFLASH_NOT_SUPPORTED if the feature is not
* 		  supported by the device(s)/library.
*
* @note		None.
*
******************************************************************************/
static int WriteBuffer32(XFlash * InstancePtr, void *DestPtr,
			 void *SrcPtr, u32 Bytes)
{
	u32 *SourcePtr = (u32*)SrcPtr;
	u32 *DestinationPtr = (u32*)DestPtr;
	u32 BaseAddress;
	u32 Index = 0;
	int Status = XST_SUCCESS;
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Make sure DestPtr and SrcPtr are aligned to a 16-bit word.
	 */
	if (((int) SourcePtr & 3) || ((int) DestinationPtr & 3)) {
		return (XFLASH_ALIGNMENT_ERROR);
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);
	BaseAddress = InstancePtr->Geometry.BaseAddress;

	/*
	 * Send the Unlock Bypass command .
	 */
	DevDataPtr->SendCmdSeq(BaseAddress,
				XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
				XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->SendCmd(BaseAddress,
				XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_UNLOCK_BYPASS);
	Index = 0;
	while (Bytes != 0) {
		DevDataPtr->WriteFlash(BaseAddress,
			0, XFL_AMD_CMD_PROGRAM);

		DevDataPtr->WriteFlash(BaseAddress,
			(u32)DestinationPtr, SourcePtr[Index]);

		Status = DevDataPtr->PollSR(BaseAddress,
						(u32)DestinationPtr);
		if (Status != XFLASH_READY) {
			(void) XFlashAmd_ResetBank(InstancePtr, (u32)DestPtr,
					Bytes);
			return (Status);
		}
		DestinationPtr++;
		Index++;
		Bytes -= 4;
	}

	/*
	 * Unlock Bypass Reset.
	 */
	DevDataPtr->WriteFlash(BaseAddress, 0,
		XFL_AMD_CMD_UNLOCK_BYPASS_RESET1);
	DevDataPtr->WriteFlash(BaseAddress,0,
		XFL_AMD_CMD_UNLOCK_BYPASS_RESET2);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* Retrieves and interprets status register data from part arrays that occupy a
* 8-bit bus.
*
* @param	BaseAddr contains base address of the part.
* @param	Offset is the offset into the device(s) address space from which
*		to read status information.
*
* @return
*		- Status register contents.
*
* @note		None.
*
******************************************************************************/
static int GetStatus8(u32 BaseAddr, u32 Offset)
{
	return (READ_FLASH_8(((volatile u8*)BaseAddr) + Offset));
}


/*****************************************************************************/
/**
*
* Retrieves and interprets status register data from part arrays that occupy a
* 16-bit bus.
*
* @param	BaseAddr contains base address of the part.
* @param	Offset is the offset into the device(s) address space from which
*		to read status information.
*
* @return
*		- Status register contents.
*
* @note		None.
*
******************************************************************************/
static int GetStatus16(u32 BaseAddr, u32 Offset)
{
	return (READ_FLASH_16(((volatile u16*)BaseAddr) + Offset));
}

/*****************************************************************************/
/**
*
* Retrieves and interprets status register data from part arrays that occupy a
* 32-bit bus.
*
* @param	BaseAddr contains base address of the part.
* @param	Offset is the offset into the device(s) address space from which
*		to read status information.
*
* @return
*		- Status register contents.
*		- XFLASH_NOT_SUPPORTED if the feature is not
* 		  supported by the device(s)/library.
*
* @note		None.
*
******************************************************************************/
static int GetStatus32(u32 BaseAddr, u32 Offset)
{
	return (READ_FLASH_32(((volatile u32*)BaseAddr) + Offset));
}

/*****************************************************************************/
/**
*
* Polls the status register until the erase or program operation is completed.
* The device(s) are polled by repeatedly reading the status register.
*
* @param	BaseAddr contains base address of the part.
* @param	BlockAddr contains address of block on which erase or program
*		operation is performed.
*
* @return	- XFLASH_READY if erase operation successful is ready.
*		- XFLASH_ERROR if error occurs.
*
* @note		It is assumed that the status register is currently visible.
*
******************************************************************************/
static int PollSR8(u32 BaseAddr, u32 BlockAddr)
{
	u8 StatusReg1;
	u8 StatusReg2;

	while(TRUE) {
		/*
		 * Read DQ5 and DQ6 (into word).
		 */
		StatusReg1 = READ_FLASH_8(((volatile u8*) BaseAddr) +
				 BlockAddr);

		/*
		 * Read DQ6 (into another word).
		 */
		StatusReg2 = READ_FLASH_8(((volatile u8*) BaseAddr) +
				BlockAddr);

		/*
		 * If DQ6 did not toggle between two reads then return
		 * Flash_Success.
		 */
		if((StatusReg1 & XFL_AMD_SR_ERASE_COMPL_MASK) ==
			(StatusReg2 & XFL_AMD_SR_ERASE_COMPL_MASK)) {
			/*
			 * DQ6 == NO Toggle.
			 */
			return (XFLASH_READY);
		}

		/*
		 * If DQ5 is zero then operation is not yet complete.
		 */
		if((StatusReg2 & XFL_AMD_SR_ERASE_ERROR_MASK) !=
			XFL_AMD_SR_ERASE_ERROR_MASK) {
			continue;
		}

		/*
		 * Else (DQ5 == 1), read DQ6 twice.
		 */
		StatusReg1 = READ_FLASH_8(((volatile u8*) BaseAddr) +
				BlockAddr);
		StatusReg2 = READ_FLASH_8(((volatile u8*) BaseAddr) +
				BlockAddr);

		/*
		 * If DQ6 did not toggle between two reads then return
		 * Flash_Success.
		 */
		if((StatusReg1 & XFL_AMD_SR_ERASE_COMPL_MASK) ==
			(StatusReg2 & XFL_AMD_SR_ERASE_COMPL_MASK)) {
			/*
			 * DQ6 == NO Toggle.
			 */
			return (XFLASH_READY);
		}
		else {
			/*
			 * Else return Flash_ToggleFail; DQ6 == Toggle here
			 * means fail.
			 */
		        return (XFLASH_ERROR);
		}
	}
}

/*****************************************************************************/
/**
*
* Polls the status register until the erase or program operation is completed.
* The device(s) are polled by repeatedly reading the status register.
*
* @param	BaseAddr contains base address of the part.
* @param	BlockAddr contains address of block on which erase or program
*		operation is performed.
*
* @return	- XFLASH_READY if erase operation successful is ready.
*		- XFLASH_ERROR if error occurs.
*
* @note		It is assumed that the status register is currently visible.
*
******************************************************************************/
static int PollSR16(u32 BaseAddr, u32 BlockAddr)
{
	volatile u16 StatusReg1;
	volatile u16 StatusReg2;

	while(TRUE) {
		/* Read DQ5 and DQ6 (into word). */
		StatusReg1 = READ_FLASH_16(((volatile u16*)BaseAddr) +
				BlockAddr);

		/* Read DQ6 (into another word). */
		StatusReg2 = READ_FLASH_16(((volatile u16*)BaseAddr) +
				BlockAddr);

		/*
		 * If DQ6 did not toggle between two reads then return
		 * Flash_Success.
		 */
		if((StatusReg1 & XFL_AMD_SR_ERASE_COMPL_MASK) ==
			(StatusReg2 & XFL_AMD_SR_ERASE_COMPL_MASK)) {
			/* DQ6 == NO Toggle. */
			return (XFLASH_READY);
		}

		/* If DQ5 is zero then operation is not yet complete. */
		if((StatusReg2 & XFL_AMD_SR_ERASE_ERROR_MASK) !=
			XFL_AMD_SR_ERASE_ERROR_MASK) {
			continue;
		}

		/* Else (DQ5 == 1), read DQ6 twice. */
		StatusReg1 = READ_FLASH_16(((volatile u16*)BaseAddr) +
				BlockAddr);
		StatusReg2 = READ_FLASH_16(((volatile u16*)BaseAddr) +
				BlockAddr);

		/*
		 * If DQ6 did not toggle between two reads then return
		 * Flash_Success.
		 */
		if((StatusReg1 & XFL_AMD_SR_ERASE_COMPL_MASK) ==
			(StatusReg2 & XFL_AMD_SR_ERASE_COMPL_MASK)) {
			/* DQ6 == NO Toggle. */
			return (XFLASH_READY);
		}
		else {
			/*
			 * Else return Flash_ToggleFail; DQ6 == Toggle here
			 * means fail.
			 */
		        return (XFLASH_ERROR);
		}
	}
}

/*****************************************************************************/
/**
*
* Polls the status register until the erase or program operation is completed.
* The device(s) are polled by repeatedly reading the status register.
*
* @param	BaseAddr contains base address of the part.
* @param	BlockAddr contains address of block on which erase or program
*		operation is performed.
*
* @return	- XFLASH_READY if erase operation successful is ready.
*		- XFLASH_ERROR if error occurs.
*		- XFLASH_NOT_SUPPORTED if the feature is not
* 		  supported by the device(s)/library.
*
* @note		It is assumed that the status register is currently visible.
*
******************************************************************************/
static int PollSR32(u32 BaseAddr , u32 BlockAddr)
{
	u32 StatusReg1;
	u32 StatusReg2;

	while(TRUE) {
		/*
		 * Read DQ5 and DQ6 (into word).
		 */
		StatusReg1 = READ_FLASH_32(((volatile u32*)BaseAddr) +
				BlockAddr);

		/*
		 * Read DQ6 (into another word).
		 */
		StatusReg2 = READ_FLASH_32(((volatile u32*)BaseAddr) +
				BlockAddr);

		/*
		 * If DQ6 did not toggle between two reads then return
		 * Flash_Success.
		 */
		if((StatusReg1 & XFL_AMD_SR_ERASE_COMPL_MASK) ==
			(StatusReg2 & XFL_AMD_SR_ERASE_COMPL_MASK)) {
			/*
			 * DQ6 == NO Toggle.
			 */
			return (XFLASH_READY);
		}

		/*
		 * If DQ5 is zero then operation is not yet complete.
		 */
		if((StatusReg2 & XFL_AMD_SR_ERASE_ERROR_MASK) !=
			XFL_AMD_SR_ERASE_ERROR_MASK) {
			continue;
		}

		/*
		 * Else (DQ5 == 1), read DQ6 twice.
		 */
		StatusReg1 = READ_FLASH_32(((volatile u32*)BaseAddr) +
				BlockAddr);
		StatusReg2 = READ_FLASH_32(((volatile u32*)BaseAddr) +
				BlockAddr);

		/*
		 * If DQ6 did not toggle between two reads then return
		 * Flash_Success.
		 */
		if((StatusReg1 & XFL_AMD_SR_ERASE_COMPL_MASK) ==
			(StatusReg2 & XFL_AMD_SR_ERASE_COMPL_MASK)) {
			/*
			 * DQ6 == NO Toggle.
			 */
			return (XFLASH_READY);
		}
		else {
			/*
			 * Else return Flash_ToggleFail; DQ6 == Toggle here
			 * means fail.
			 */
			return (XFLASH_ERROR);
		}
	}
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
static void GetPartID(XFlash * InstancePtr)
{
	void *Ptr;
	u8 Mode;
	u8 Interleave;
	u32 CmdAddress;
	XFlashVendorData_Amd *DevDataPtr;

	DevDataPtr = GET_PARTDATA(InstancePtr);
	CmdAddress = InstancePtr->Geometry.BaseAddress;
	Interleave = (InstancePtr->Geometry.MemoryLayout &
		      XFL_LAYOUT_CFI_INTERL_MASK) >> 24;
	Mode = (InstancePtr->Geometry.MemoryLayout &
		      XFL_LAYOUT_PART_MODE_MASK) >> 8;

	/*
	 * Send Read id codes command.
	 */
	DevDataPtr->SendCmdSeq(CmdAddress,
				XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
				XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->SendCmd(CmdAddress,
				XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_AUTO_SELECT);

	/*
	 * Retrieve Manufacturer ID located at word offset 0.
	 */
	XFL_CFI_POSITION_PTR(Ptr, CmdAddress, Interleave,
				XFL_AMD_MANUFECTURE_ID_OFFSET);
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
	(void) XFlashAmd_Reset(InstancePtr);
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
static u16 EnqueueEraseBlocks(XFlash * InstancePtr, u16 *Region,
			      u16 *Block, u16 MaxBlocks)
{
	u32 BlockAddress;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Amd *DevDataPtr;
	volatile u16 FlashStatus;

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

	/* Send Status register clear command. */
	DevDataPtr->SendCmd(GeomPtr->BaseAddress,XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_STATUS_REG_CLEAR);

	/* Write Block Erase command. */
	DevDataPtr->SendCmdSeq(GeomPtr->BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
			XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);
	DevDataPtr->SendCmd(GeomPtr->BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD_ERASE1);
	DevDataPtr->SendCmdSeq(GeomPtr->BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
			XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->WriteFlash(GeomPtr->BaseAddress, BlockAddress,
			XFL_AMD_CMD_ERASE_BLOCK);

	/* Increment Region/Block. */
	XFL_GEOMETRY_INCREMENT(GeomPtr, *Region, *Block);

	/* Return the number of blocks enqueued. */
	return (1);
}

/*****************************************************************************/
/**
*
* This function resumes a suspended operation on a bank of the AMD flash
* device.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	EraseAddrOff holds the address inside the bank where the
*		operation must be resumed.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		This API can not be used in application, because Erase API is
*		blocking function in this library. If user wants to use this
*		feature than Flash library Erase function should be changed such
*		Erase API will be non blocking function. In case of non blocking
*		Erase API, application must check status of erase operation in
*		the application.
*
******************************************************************************/
static int EraseResume(XFlash * InstancePtr, u32 EraseAddrOff)
{
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);

	/*
	 * Send the erase resume command.
	 */
	DevDataPtr->WriteFlash(InstancePtr->Geometry.BaseAddress,
				EraseAddrOff, XFL_AMD_CMD_ERASE_RESUME);
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function suspends an erase operation on a bank of the AMD flash device.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	EraseAddrOff holds the address inside the bank where the
*		operation must be suspended.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		This API can not be used in application, because Erase API is
*		blocking function in this library. If user wants to use this
*		feature than Flash library Erase function should be changed such
*		Erase API will be non blocking function. In case of non blocking
*		Erase API, application must check status of erase operation in
*		the application.
*
******************************************************************************/
static int EraseSuspend(XFlash * InstancePtr, u32 EraseAddrOff)
{
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	DevDataPtr = GET_PARTDATA(InstancePtr);

	/*
	 * Send the erase suspend command.
	 */
	DevDataPtr->WriteFlash(InstancePtr->Geometry.BaseAddress,
				EraseAddrOff, XFL_AMD_CMD_ERASE_SUSPEND);

	/*
	 * Wait until Toggle Stops.
	 */
	(void) DevDataPtr->PollSR(InstancePtr->Geometry.BaseAddress,
				  EraseAddrOff);

	/*
	 * Return to Read mode.
	 */
	(void) XFlashAmd_Reset(InstancePtr);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function will make the device to enter into Extended Block Mode, where
* the Extended Block can be read and written, using the Boot block addresses.
*
* @param	InstancePtr is the pointer to the XFlash instance.
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
static void EnterExtendedBlockMode(XFlash * InstancePtr)
{
	XFlashVendorData_Amd *DevDataPtr;

	DevDataPtr = GET_PARTDATA(InstancePtr);

	/*
	 * Issue Enter Extended Block Command.
	 */
	DevDataPtr->SendCmdSeq(InstancePtr->Geometry.BaseAddress,
				XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
				XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->SendCmd(InstancePtr->Geometry.BaseAddress,
				XFL_AMD_CMD1_ADDR,
				XFL_AMD_CMD_ENTER_EXT_MODE);
}

/*****************************************************************************/
/**
*
* This command returns the device to Read mode from Extended Block Mode.
*
* @param	InstancePtr is the pointer to the XFlash instance.
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
static void ExitExtendedBlockMode(XFlash * InstancePtr)
{
	XFlashVendorData_Amd *DevDataPtr;

	DevDataPtr = GET_PARTDATA(InstancePtr);

	/*
	 * Issue Exit Extended Block Command.
	 */
	DevDataPtr->SendCmdSeq(InstancePtr->Geometry.BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
			XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->SendCmd(InstancePtr->Geometry.BaseAddress,
			XFL_AMD_CMD1_ADDR, XFL_AMD_CMD_EXIT_EXT_MODE);

	DevDataPtr->WriteFlash(InstancePtr->Geometry.BaseAddress,
				0, 0);
}

/*****************************************************************************/
/**
*
* This function reads the protection status of a block group.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	Offset is the offset into the device(s) address space from where
* 		to read protection status.
*
* @return
*		- XST_SUCCESS if block is not protected.
*		- XFLASH_BLOCK_PROTECTED if block is protected.
*		- XFLASH_ADDRESS_ERROR if the destination address range is
*		  not completely within the addressable areas of the device(s).
*		- XST_FAILURE if block protection is unclear.
*
* @note		None.
*
******************************************************************************/
static int CheckBlockProtection(XFlash * InstancePtr, u32 Offset)
{
	u8 Mode;
	u8 AddrMul;
	u16 Region;
	u16 Block;
	u32 Dummy;
	u32 BlockAddress;
	u32 CmdAddress;
	u32 ProtStatus;
	int Status;
	XFlashGeometry *GeomPtr;
	XFlashVendorData_Amd *DevDataPtr;

	/*
	 * Verify inputs are valid.
	 */
	if(InstancePtr == NULL) {
		return (XST_FAILURE);
	}

	GeomPtr = &InstancePtr->Geometry;

	DevDataPtr = GET_PARTDATA(InstancePtr);

	Mode = (InstancePtr->Geometry.MemoryLayout &
			XFL_LAYOUT_PART_MODE_MASK) >> 8;
	if (Mode == 2) {
		AddrMul = 1;
	}
	else if (Mode == 1) {
		AddrMul = 2;
	}
	else {
		return (XST_FAILURE);
	}

	/*
	 * Convert the starting address to block coordinates. This also verifies
	 * the starting address is within the instance's address space.
	 */
	Status = XFlashGeometry_ToBlock(GeomPtr, Offset, &Region, &Block,
						&Dummy);
	if (Status != XST_SUCCESS) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Get the physical address to write the command to and send command.
	 */
	(void) XFlashGeometry_ToAbsolute(GeomPtr, Region, Block, 0,
				 &BlockAddress);

	CmdAddress = BlockAddress | ((XFL_AMD_CMD1_ADDR * AddrMul) + !(
					XFL_AMD_CMD1_ADDR & 0x1));

	/*
	 * Send the command.
	 */
	DevDataPtr->SendCmdSeq(GeomPtr->BaseAddress,
				XFL_AMD_CMD1_ADDR, XFL_AMD_CMD2_ADDR,
				XFL_AMD_CMD1_DATA, XFL_AMD_CMD2_DATA);

	DevDataPtr->WriteFlash(InstancePtr->Geometry.BaseAddress,
				CmdAddress, XFL_AMD_CMD_AUTO_SELECT);

	/*
	 * Retrieve Protection Status located at word offset 2.
	 */
	CmdAddress = BlockAddress + (XFL_AMD_PROT_STATUS_OFFSET * AddrMul);

	ProtStatus = DevDataPtr->GetStatus(GeomPtr->BaseAddress, CmdAddress);
	if(ProtStatus == XFL_AMD_GROUP_UNPROTECTED) {
		Status = XST_SUCCESS;
	}
	else if(ProtStatus == XFL_AMD_GROUP_PROTECTED) {
		Status = XFLASH_BLOCK_PROTECTED;
	}
	else {
		Status = XST_FAILURE;
	}

	/*
	 * Return to Read mode.
	 */
	(void) XFlashAmd_Reset(InstancePtr);

	return (Status);
}

/*****************************************************************************/
/**
*
* This routine returns after MicroSeconds have elapsed. It is used in several
* parts of the code to generate a pause required for correct operation of the
* flash part.
*
* @param	MicroSeconds is the length of the pause in microseconds.
*
* @return
* 		None
*
* @note		None.
*
******************************************************************************/
static void  FlashPause(u32 MicroSeconds)
{
	static u32 Counter;

	/*
	 * Compute the count size.
	 */
	Counter = MicroSeconds * XFL_COUNT_FOR_A_MICROSECOND ;

	/*
	 * Count to the required size.
	 */
	while(Counter > 0) {
		/*
		 * Count down.
		 */
		Counter--;
	}
}

#endif /* XPAR_XFL_DEVICE_FAMILY_AMD */
