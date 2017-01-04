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
* @file xilflash_cfi.c
*
* The file implements the functions for retrieval and translation of CFI data
* from a compliant flash device. CFI contains data that defines part geometry,
* write/erase timing, and programming data.
*
* Data is retrieved using macros defined in this xflash_cfi.h file. The
* macros simplify data extraction because they have been written to take into
* account the layout of parts on the data bus. To the library, CFI data appears
* as if it were always being read from a single 8-bit part (XFL_LAYOUT_X8_X8_X1)
* Otherwise, the retrieval code would have to contend with all the formats
* illustrated below. The illustration shows how the first three bytes of the CFI
* query data "QRY" appear in flash memory space for various part layouts.
* <pre>
*
*			Byte Offset (Big-Endian)
*			0123456789ABCDEF
*			----------------
*			XFL_LAYOUT_X16_X16_X1	Q R Y
*			XFL_LAYOUT_X16_X16_X2	Q Q R R Y Y
* </pre>
*
* Where the holes between Q, R, and Y are NULL (all bits 0)
*
* @note
*
* This code is intended to be RTOS and processor independent.
* It works with physical addresses only. Any needs for dynamic memory
* management, threads, mutual exclusion, virtual memory, or cache control
* management must be satisfied by the layer above this library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  10/25/07 First release
* 1.00a mta  10/25/07 Updated to flash library
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 1.02a ksu  08/06/09 Added code to read the platform flash bank information
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros
* 2.02a sdm  06/30/10 Updated to support AXI EMC with Little Endian Processor
* 3.00a sdm  03/03/11 Removed static parameters in mld and updated code to
*		      determine these parameters from the CFI data.
* 3.01a srt  03/02/12 Added support for Micron G18 Flash device to fix
*		      CRs 648372, 648282.
*		      Added DATA_SYNC to fix the CR 644750.
* 3.02a srt  05/30/12 Changed Implementation for Micron G18 Flash, which
*		      fixes the CR 662317.
*		      CR 662317 Description - Xilinx Platform Flash on ML605
*		      fails to work.
* 3.04a srt  02/18/13 Fixed CR 700553.
* </pre>
*
*
****************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "include/xilflash.h"
#include "include/xilflash_cfi.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Retrieves the standard CFI data from the part(s), interpret the data, and
* update the provided geometry and properties structures.
*
* Extended CFI data is part specific and ignored here. This data must be read
* by the specific flash device family library.
*
* @param	InstancePtr is the pointer to the XFlash instance.
* @param	BusWidth is the total width of the flash memory, in bytes.
*
* @return
*		- XST_SUCCESS if successful.
*		- XFLASH_CFI_QUERY_ERROR if an error occurred interpreting
*		  the data.
*		- XFLASH_PART_NOT_SUPPORTED if invalid Layout parameter.
*
* @note		None.
*
******************************************************************************/
int XFlashCFI_ReadCommon(XFlash *InstancePtr, u8 BusWidth)
{
	void *DataPtr;
	u32 BaseAddress;
	int Status = XST_SUCCESS;
	u8 Data8;
	u8 Mode;
	u8 DataQRY[3];
	u16 Data16;
	u16 ExtendedQueryTblOffset;
	u32 SizeMultiplier;
	u32 CurrentAbsoluteOffset;
	u16 CurrentAbsoluteBlock;
	u32 Index;
	u32 Interleave;
	u32 CfiQryAddr;
	u32 Layout;
	u32 Data32;
	Xuint64 Data64;
	u16 NumBanks, Bank;
	u16 NumEraseRegions;
	u8 TypesEraseBlock;
	u16 NumBlockInBank;
	u32 SizeBlockInBank;
	XFlashGeometry *GeomPtr;

	if(InstancePtr == NULL) {
		return XST_FAILURE;
	}

	BaseAddress = InstancePtr->Geometry.BaseAddress;
	CfiQryAddr = 0x10;

	switch (BusWidth) {
		case 1:
			/* Check for one 16 bit flash in x8 mode */
			WRITE_FLASH_8(BaseAddress, 0xFF);
			WRITE_FLASH_8(BaseAddress + 0xAA, 0x98);
			DATA_SYNC;
			Data8 = READ_FLASH_8(BaseAddress + (CfiQryAddr << 1));
			if (Data8 == 0x51) {
				Layout = XFL_LAYOUT_X16_X8_X1;
			} else {
				Layout = XFLASH_PART_NOT_SUPPORTED;
			}

		break;

		case 2:
			/* Check for one 16 bit flash in x16 mode */
			CfiQryAddr <<= 1;
			WRITE_FLASH_16(BaseAddress, 0xFF);
			WRITE_FLASH_16(BaseAddress + 0xAA, 0x98);
			DATA_SYNC;
			Data16 = READ_FLASH_16(BaseAddress + CfiQryAddr);
			if (Data16 == 0x51) {
				Layout = XFL_LAYOUT_X16_X16_X1;
			} else {
				Layout = XFLASH_PART_NOT_SUPPORTED;
			}

		break;

		case 4:
			/* Check for two 16 bit flash devices in x32 mode */
			CfiQryAddr <<= 2;
			WRITE_FLASH_32(BaseAddress, 0x00FF00FF);
			WRITE_FLASH_32(BaseAddress + 0xAA, 0x00980098);
			DATA_SYNC;
			Data32 = READ_FLASH_32(BaseAddress + CfiQryAddr);
			if (Data32 == 0x00510051) {
				Layout = XFL_LAYOUT_X16_X16_X2;
			} else {
				Layout = XFLASH_PART_NOT_SUPPORTED;
			}

		break;

		case 8:
		/* Check for four 16 bit flash devices in x64 mode */
			CfiQryAddr <<= 3;
			WRITE_FLASH_64x2(BaseAddress + 0xAA,
					 0x00FF00FF, 0x00FF00FF);
			WRITE_FLASH_64x2(BaseAddress + 0xAA,
					 0x00980098, 0x00980098);
			DATA_SYNC;
			READ_FLASH_64(BaseAddress + CfiQryAddr, Data64);
			if ((XUINT64_MSW(Data64) == 0x00510051) &&
			    (XUINT64_LSW(Data64) == 0x00510051)) {
				Layout = XFL_LAYOUT_X16_X16_X4;
			} else {
				Layout = XFLASH_PART_NOT_SUPPORTED;
			}

		break;

		default:
		Layout = XFLASH_PART_NOT_SUPPORTED;
	}

	if (Layout == XFLASH_PART_NOT_SUPPORTED) {
		return (XFLASH_PART_NOT_SUPPORTED);
	}

	InstancePtr->Geometry.MemoryLayout = Layout;

	/*
	 * To stay consistent when retrieving the CFI data for all part layouts
	 * we use the XFL_CFI_READ macros supplying the correct interleave based
	 * on the layout.
	 *
	 * The size of a block for an instance is the block size reported
	 * by the device multiplied by the number of devices (SizeMultiplier).
	 */
	Interleave = (InstancePtr->Geometry.MemoryLayout &
			XFL_LAYOUT_CFI_INTERL_MASK) >> 24;
	SizeMultiplier = (InstancePtr->Geometry.MemoryLayout &
			  XFL_LAYOUT_NUM_PARTS_MASK);
	Mode = (InstancePtr->Geometry.MemoryLayout &
		XFL_LAYOUT_PART_MODE_MASK) >> 8;

	/*
	 * Begin reading the data. Each datum is documented in comments with
	 * its offset range.
	 */

	/*
	 * 10h-12h : Contains the "QRY" string. Must be present.
	 */
	XFL_CFI_POSITION_PTR(DataPtr, BaseAddress, Interleave, 0x10);

	DataQRY[0] = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
	XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);
	DataQRY[1] = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
	XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);
	DataQRY[2] = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);

	if ((DataQRY[0] != 'Q') || (DataQRY[1] != 'R') || (DataQRY[2] != 'Y')) {
		Status = XFLASH_CFI_QUERY_ERROR;
	}
	else {
		/*
		 * 13h-14h : Primary vender command set.
		 */
		XFL_CFI_POSITION_PTR(DataPtr, BaseAddress, Interleave, 0x13);
		InstancePtr->Properties.PartID.CommandSet =
			XFlashCFI_Read16((u8*)DataPtr, Interleave, Mode);
		InstancePtr->CommandSet =
				InstancePtr->Properties.PartID.CommandSet;

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
		/* Support for Micron G18. This flash is partially compatible
		   with Intel CFI command set and it has a different geometry
		   from the other Intel Flash Devices */
		if (InstancePtr->CommandSet == XFL_CMDSET_INTEL_G18) {
			InstancePtr->Command.WriteBufferCommand =
					XFL_INTEL_G18_CMD_WRITE_BUFFER;
			InstancePtr->Command.ProgramCommand =
					XFL_INTEL_G18_CMD_PROGRAM;
		}
		else {
			InstancePtr->Command.WriteBufferCommand =
					XFL_INTEL_CMD_WRITE_BUFFER;
			InstancePtr->Command.ProgramCommand =
					XFL_INTEL_CMD_PROGRAM;
		}
#endif

		/*
		 * 15h-16h : Address for Primary Algorithm extended Query table.
		 */
		XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);
		ExtendedQueryTblOffset =
			XFlashCFI_Read16((u8*)DataPtr, Interleave, Mode);

		/*
		 * 17h-1Ah : Vendor data to be interpreted by part (ignored
		 * here).
		 * 1Bh-1Eh : Voltage requirements (ignored by this library).
		 *
		 * Interpret the timing requirements starting here.
		 *	1Fh : Typical timeout for single byte/word program cycle
		 * 		(2^N Us).
		 */
		XFL_CFI_POSITION_PTR(DataPtr, BaseAddress, Interleave, 0x1F);
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		if (Data8 != 0) {
			InstancePtr->Properties.TimeTypical.WriteSingle_Us =
								1 << Data8;
		}

		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 20h:Typical timeout for max buffer program cycle (2^N Us)
		 * = 0 if not supported.
		 */

		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		if (Data8 != 0) {
			InstancePtr->Properties.TimeTypical.WriteBuffer_Us =
								1 << Data8;
		}

		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 21h : Typical timeout for single block erase (2^N Ms).
		 */
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		if (Data8 != 0) {
			InstancePtr->Properties.TimeTypical.EraseBlock_Ms =
								1 << Data8;
		}

		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 22h : Typical timeout for full chip erase (2^N Ms)
		 * = 0 if not supported.
		 */
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		if (Data8 != 0) {
			InstancePtr->Properties.TimeTypical.EraseChip_Ms =
								1 << Data8;
		}

		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 23h : Maximum timeout for single byte/word program cycle
		 *	(2^N * typical time).
		 */
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		InstancePtr->Properties.TimeMax.WriteSingle_Us =
			InstancePtr->Properties.TimeTypical.WriteSingle_Us *
								(1 << Data8);
		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 24h : Maximum timeout for max buffer program cycle
		 *	(2^N * typical time)
		 *	= 0 if not supported.
		 */
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		InstancePtr->Properties.TimeMax.WriteBuffer_Us =
			InstancePtr->Properties.TimeTypical.WriteBuffer_Us *
			(1 << Data8);
		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 25h : Maximum timeout for single block erase
		 *	(2^N * typical time).
		 */
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		InstancePtr->Properties.TimeMax.EraseBlock_Ms =
			InstancePtr->Properties.TimeTypical.EraseBlock_Ms *
								(1 << Data8);
		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 26h : Maximum timeout for full chip erase
		 *	(2^N * typical time)
		 *	= 0 if not supported.
		 */
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		InstancePtr->Properties.TimeMax.EraseChip_Ms =
			InstancePtr->Properties.TimeTypical.EraseChip_Ms *
								(1 << Data8);
		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 27h : Device size in bytes
		 *	= 2^N bytes * (Number of parts).
		 */
		Data8 = XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		InstancePtr->Geometry.DeviceSize = (1 << Data8) *
								SizeMultiplier;
		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 28h-29h : Device interface description (ignored).
		 */
		XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);

		/*
		 * 2Ah-2Bh : Maximum number of bytes in write buffer
		 *		= 2^N bytes * (Number of parts).
		 */
		Data16 = XFlashCFI_Read16((u8*)DataPtr, Interleave, Mode);
		if (Data16 != 0) {
			InstancePtr->Properties.ProgCap.WriteBufferSize =
						(1 << Data16) * SizeMultiplier;
		}

		XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);

		/*
		 * 2Ch : Number of erase regions.
		 * Make sure there are not too many to contain in the instance.
		 * This will ensure the for loop below doesn't go out of bounds
		 * on the Geometry.EraseRegion array.
		 */
		InstancePtr->Geometry.NumEraseRegions = XFlashCFI_Read8(
								(u8*)DataPtr,
								Interleave,
								Mode);
		if ((InstancePtr->CommandSet == XFL_CMDSET_AMD_STANDARD) ||
		    (InstancePtr->CommandSet == XFL_CMDSET_AMD_EXTENDED)) {
			if (InstancePtr->Geometry.NumEraseRegions >
			    XFL_AMD_MAX_ERASE_REGIONS) {
				return (XFLASH_TOO_MANY_REGIONS);
			}
		} else {
			if (InstancePtr->Geometry.NumEraseRegions >
			    XFL_INTEL_MAX_ERASE_REGIONS) {
				return (XFLASH_TOO_MANY_REGIONS);
			}
		}

		XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

		/*
		 * 2Dh-30h : Erase region #1 definition
		 * 31h-34h : Erase region #2 definition
		 * 35h-39h : Erase region #3 definition,  etc.
		 */
		CurrentAbsoluteOffset = 0;
		CurrentAbsoluteBlock = 0;
		InstancePtr->Geometry.NumBlocks = 0;
		for (Index = 0; Index < InstancePtr->Geometry.NumEraseRegions;
		     Index++) {

			/*
			 * Offset 0-1 : Number of blocks in the region
			 *		= N + 1.
			 */
			Data16 = XFlashCFI_Read16((u8*)DataPtr, Interleave,
							Mode);
			InstancePtr->Geometry.EraseRegion[Index].Number = Data16
									+ 1;
			InstancePtr->Geometry.NumBlocks +=
				InstancePtr->Geometry.EraseRegion[Index].Number;
			XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);

			/*
			 * Offset 2-3 : Size of erase blocks in the region
			 * = N * 256 * (Number of parts).
			 */
			Data16 = XFlashCFI_Read16((u8*)DataPtr, Interleave,
						Mode);
			InstancePtr->Geometry.EraseRegion[Index].Size =
						Data16 * 256 * SizeMultiplier;
			XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);

			/*
			 * Calculate the part offset where this region begins.
			 */
			InstancePtr->Geometry.EraseRegion[Index].AbsoluteOffset
							= CurrentAbsoluteOffset;
			InstancePtr->Geometry.EraseRegion[Index].AbsoluteBlock =
							CurrentAbsoluteBlock;

			/*
			 * Increment absolute counters.
			 */
			CurrentAbsoluteOffset +=
			(InstancePtr->Geometry.EraseRegion[Index].Size *
			InstancePtr->Geometry.EraseRegion[Index].Number);
			CurrentAbsoluteBlock +=
				InstancePtr->Geometry.EraseRegion[Index].Number;
		}

		/*
		 * Set the absolute offsets for NumEraseRegions+1. This is not a
		 * real region, but marks one unit past the part's addressable
		 * region. For example, if the device(s) are a total of 1000
		 * bytes in size with a total of 10 blocks then 1000 and 10 are
		 * written to the Absolute parameters. The Size & Number are
		 * left as zero.
		 */
		InstancePtr->Geometry.EraseRegion[Index].AbsoluteOffset =
							CurrentAbsoluteOffset;
		InstancePtr->Geometry.EraseRegion[Index].AbsoluteBlock =
							CurrentAbsoluteBlock;

		/*
		 * This ends the query. The following summarizes what attributes
		 * of the InstancePtr were initialized:
		 *
		 *	Properties.PartID
		 *	- CommandSet defined.
		 *	- ManufacturerID is defined by the part's Initialize
		 *	  function.
		 *	- DeviceID is defined by the part's Initialize function.
		 *
		 *	Properties.TimeTypical
		 *	Completely defined.
		 *
		 *	Properties.TimeMax
		 *	Completely defined.
		 *
		 *	Properties.ProgCap
		 *	- WriteBufferAlignment must be defined by the device.
		 *	  It defaults to 0 here.
		 *	- EraseQueueSize must be defined by the device. It
		 *	  defaults to 1 here.
		 *
		 * Geometry
		 *	Completely defined.
		 *
		 */
		InstancePtr->Properties.ProgCap.EraseQueueSize = 1;

		/*
		 * Some of AMD flash have different geometry based on
		 * type of boot mode. Read boot mode to identify
		 * location of boot block and parameter blocks.
		 */
		if (InstancePtr->CommandSet == 0x02) {
			/*
			 * Extended Query Table Offset + 0x0F: Boot mode.
			 */
			XFL_CFI_POSITION_PTR(DataPtr, BaseAddress, Interleave,
					     (ExtendedQueryTblOffset + 0x0F));
			InstancePtr->Geometry.BootMode =
				XFlashCFI_Read8((u8*)DataPtr, Interleave, Mode);
		}

		/*
		 * The platform flash (Intel) have multiple banks in same erase
		 * region. Read number of identical banks in each erase region,
		 * number of erase blocks and size of blocks in each bank. For
		 * platfrom flash, library treats each bank as seperate region.
		 */
		if (((InstancePtr->CommandSet == XFL_CMDSET_INTEL_STANDARD) ||
		    (InstancePtr->CommandSet == XFL_CMDSET_INTEL_EXTENDED)) &&
		    (InstancePtr->IsPlatformFlash == 1)) {

			Index = 0;
			NumEraseRegions = InstancePtr->Geometry.NumEraseRegions;
			InstancePtr->Geometry.NumEraseRegions = 0;
			CurrentAbsoluteOffset = 0;
			CurrentAbsoluteBlock = 0;
			Bank = 0;
			GeomPtr = &InstancePtr->Geometry;
			while (Index < NumEraseRegions) {
				/*
				 * Extended Query Table Offset + 0x24/0x32:
				 * Number of banks in  region.
				 */
				XFL_CFI_POSITION_PTR(DataPtr, BaseAddress,
					Interleave, (ExtendedQueryTblOffset +
					0x24 + (Index * 0x0E)));
				NumBanks = XFlashCFI_Read16((u8*)DataPtr,
						Interleave, Mode);
				/*
				 * Ignore the information about multiple
				 * operation in bank and region as it is not
				 * supported by the library
				 */
				XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);
				XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);
				XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

				/*
				 * Extended Query Table Offset + 0x29/0x37:
				 * Types of erase block in the bank
				 */
				TypesEraseBlock =
					XFlashCFI_Read8((u8*)DataPtr,
						Interleave, Mode);

				XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);
				while (TypesEraseBlock--) {
					/*
					 * Number of erase block in bank
					 */
					NumBlockInBank =
						XFlashCFI_Read16((u8*)DataPtr,
							Interleave, Mode);
					NumBlockInBank += 1;
					XFL_CFI_ADVANCE_PTR16(DataPtr,
						Interleave);
					/*
					 * Size of each erase block in bank
					 */
					SizeBlockInBank =
						XFlashCFI_Read16((u8*)DataPtr,
							Interleave, Mode);
					SizeBlockInBank *= 256;
					/*
					 * Update flash instance structure
					 */
					GeomPtr->NumEraseRegions += NumBanks;
					while (Bank < GeomPtr->NumEraseRegions){
						GeomPtr->EraseRegion[Bank].
							Number = NumBlockInBank;
						GeomPtr->EraseRegion[Bank].Size
							= SizeBlockInBank;
						GeomPtr->EraseRegion[Bank].
							AbsoluteOffset =
							CurrentAbsoluteOffset;
						GeomPtr->EraseRegion[Bank].
							AbsoluteBlock =
							CurrentAbsoluteBlock;

						CurrentAbsoluteOffset +=
							(GeomPtr->EraseRegion
							[Bank].Size *
							GeomPtr->EraseRegion
							[Bank].Number);
						CurrentAbsoluteBlock +=
							GeomPtr->EraseRegion
							[Bank].Number;
						Bank++;
					}
					XFL_CFI_ADVANCE_PTR16(DataPtr,
						Interleave);
					XFL_CFI_ADVANCE_PTR16(DataPtr,
						Interleave);
					XFL_CFI_ADVANCE_PTR16(DataPtr,
						Interleave);
				}
				Index++;
			}
			GeomPtr->EraseRegion[Bank].AbsoluteOffset =
				CurrentAbsoluteOffset;
			GeomPtr->EraseRegion[Bank].AbsoluteBlock =
				CurrentAbsoluteBlock;
		}

		/*
		 * The Micron G18 flash (Intel) have multiple banks in same erase
		 * region. Read number of identical banks in each erase region,
		 * number of erase blocks and size of blocks in each bank. For
		 * Micron G18 flash, library treats each bank as seperate region.
		 */
		if (InstancePtr->CommandSet == XFL_CMDSET_INTEL_G18) {
			Index = 0;
			NumEraseRegions = InstancePtr->Geometry.NumEraseRegions;
			InstancePtr->Geometry.NumEraseRegions = 0;
			CurrentAbsoluteOffset = 0;
			CurrentAbsoluteBlock = 0;
			Bank = 0;
			GeomPtr = &InstancePtr->Geometry;

			while (Index < NumEraseRegions) {
				/*
				 * Extended Query Table Offset + 0x22:
				 * Number of banks in  region.
				 */
				XFL_CFI_POSITION_PTR(DataPtr, BaseAddress,
					Interleave, (ExtendedQueryTblOffset +
					0x25));
				NumBanks = XFlashCFI_Read8((u8*)DataPtr,
						Interleave, Mode);

				XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);
				XFL_CFI_ADVANCE_PTR16(DataPtr, Interleave);
				XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);

				/*
				 * Extended Query Table Offset + 0x2A:
				 * Types of erase block in the bank
				 */
				TypesEraseBlock =
					XFlashCFI_Read8((u8*)DataPtr,
						Interleave, Mode);

				XFL_CFI_ADVANCE_PTR8(DataPtr, Interleave);
				while (TypesEraseBlock--) {
					/*
					 * Number of erase block in bank
					 */
					NumBlockInBank =
						XFlashCFI_Read16((u8*)DataPtr,
							Interleave, Mode);
					NumBlockInBank += 1;
					XFL_CFI_ADVANCE_PTR16(DataPtr,
						Interleave);
					/*
					 * Size of each erase block in bank
					 */
					SizeBlockInBank =
						XFlashCFI_Read16((u8*)DataPtr,
							Interleave, Mode);
					SizeBlockInBank *= 256;

					/*
					 * Update flash instance structure
					 */
					GeomPtr->NumEraseRegions += NumBanks;
					while (Bank < GeomPtr->NumEraseRegions){
						GeomPtr->EraseRegion[Bank].
							Number = NumBlockInBank;
						GeomPtr->EraseRegion[Bank].Size
							= SizeBlockInBank;
						GeomPtr->EraseRegion[Bank].
							AbsoluteOffset =
							CurrentAbsoluteOffset;
						GeomPtr->EraseRegion[Bank].
							AbsoluteBlock =
							CurrentAbsoluteBlock;

						CurrentAbsoluteOffset +=
							(GeomPtr->EraseRegion
							[Bank].Size *
							GeomPtr->EraseRegion
							[Bank].Number);
						CurrentAbsoluteBlock +=
							GeomPtr->EraseRegion
							[Bank].Number;
						Bank++;
					}
				}
				Index++;
			}
			GeomPtr->EraseRegion[Bank].AbsoluteOffset =
				CurrentAbsoluteOffset;
			GeomPtr->EraseRegion[Bank].AbsoluteBlock =
				CurrentAbsoluteBlock;
		}
	}

	switch (InstancePtr->Geometry.MemoryLayout) {
		case XFL_LAYOUT_X16_X8_X1:
			WRITE_FLASH_8(BaseAddress + 0xAA, 0xFF);
			break;

		case XFL_LAYOUT_X16_X16_X1:
			WRITE_FLASH_16(BaseAddress + 0xAA, 0xFF);
		break;

		case XFL_LAYOUT_X16_X16_X2:
			WRITE_FLASH_32(BaseAddress + 0xAA, 0x00FF00FF);
		break;

		case XFL_LAYOUT_X16_X16_X4:
			WRITE_FLASH_64x2(BaseAddress + 0xAA,
					 0x00FF00FF, 0x00FF00FF);
		break;
	}

	return (Status);
}

#ifdef XPAR_AXI_EMC
/*****************************************************************************/
/**
*
* Reads 8-bits of data from the CFI data location into a local variable.
*
* @param	Ptr is the pointer to read. Can be a pointer to any type.
* @param	Interleave is the byte interleaving (based on part layout).
* @param	Mode is the mode of operation (based on part layout).
*
* @return	The byte at Ptr adjusted for the interleave factor.
*
*****************************************************************************/
int XFlashCFI_Read8(u8 *Ptr, u8 Interleave, u8 Mode)
{
	(void) Interleave;
	(void) Mode;
	return (READ_FLASH_8((u32)Ptr));
}

/*****************************************************************************/
/**
*
* Reads 16-bits of data from the CFI data location into a local variable.
*
* @param	Ptr is the pointer to read. Can be a pointer to any type.
* @param	Interleave is the byte interleaving (based on part layout).
* @param	Mode is the mode of operation (based on part layout).
*
* @return	The 16-bit value at Ptr adjusted for the interleave factor.
*
*****************************************************************************/
int XFlashCFI_Read16(u8 *Ptr, u8 Interleave, u8 Mode)
{
	(void) Mode;
	int Data = 0;

	(Data) = (u8)READ_FLASH_8((u8*)(Ptr) + Interleave);
	(Data) <<= 8;
	(Data) |= (u8)READ_FLASH_8((u8*)(Ptr));

	return Data;
}

#else /* XPAR_XPS_MCH_EMC */
/*****************************************************************************/
/**
*
* Reads 8-bits of data from the CFI data location into a local variable.
*
* @param	Ptr is the pointer to read. Can be a pointer to any type.
* @param	Interleave is the byte interleaving (based on part layout).
* @param	Mode is the mode of operation (based on part layout).
*
* @return	The byte at Ptr adjusted for the interleave factor.
*
*****************************************************************************/
int XFlashCFI_Read8(u8 *Ptr, u8 Interleave, u8 Mode)
{
	if (Mode == (u8)1) {
		Interleave = 1;
	}

	return (READ_FLASH_8((u32)Ptr + Interleave - 1));
}

/*****************************************************************************/
/**
*
* Reads 16-bits of data from the CFI data location into a local variable.
*
* @param	Ptr is the pointer to read. Can be a pointer to any type.
* @param	Interleave is the byte interleaving (based on part layout).
* @param	Mode is the mode of operation (based on part layout).
*
* @return	The 16-bit value at Ptr adjusted for the interleave factor.
*
*****************************************************************************/
int XFlashCFI_Read16(u8 *Ptr, u8 Interleave, u8 Mode)
{
	int Data = 0;

	if (Mode == (u8)1) {
		(Data) = (u8)READ_FLASH_8((u8*)(Ptr) + Interleave);
		(Data) <<= 8;
		(Data) |= (u8)READ_FLASH_8((u8*)(Ptr));
	}
	else if (Mode == (u8)2) {
		(Data) = (u16)READ_FLASH_8((u8*)(Ptr) + ((Interleave) * 2) - 1);
		(Data) <<= 8;
		(Data) |= (u16)READ_FLASH_8((u8*)(Ptr) + (Interleave) - 1);
	}

	return Data;
}
#endif /* XPAR_AXI_EMC */
