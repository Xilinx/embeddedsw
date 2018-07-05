/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdppsu_edid.c
 *
 * This file contains functions related to accessing the Extended Display
 * Identification Data (EDID) of a specified sink using the XDpPsu driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  27/01/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xil_assert.h"
#include "xdppsu.h"
#include "xstatus.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function retrieves an immediately connected RX device's Extended Display
 * Identification Data (EDID) structure.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Edid is a pointer to the Edid buffer to save to.
 *
 * @return
 *		- XST_SUCCESS if the I2C transactions to read the EDID were
 *		  successful.
 *		- XST_ERROR_COUNT_MAX if the EDID read request timed out.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_GetEdid(XDpPsu *InstancePtr, u8 *Edid)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Edid != NULL);

	/* Retrieve the base EDID block = EDID block #0. */
	return XDpPsu_GetEdidBlock(InstancePtr, Edid, 0);
}

/******************************************************************************/
/**
 * Retrieve an immediately connected RX device's Extended Display Identification
 * Data (EDID) block given the block number. A block number of 0 represents the
 * base EDID and subsequent block numbers represent EDID extension blocks.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Data is a pointer to the data buffer to save the block data to.
 * @param	BlockNum is the EDID block number to retrieve.
 *
 * @return
 *		- XST_SUCCESS if the block read has successfully completed with
 *		  no errors.
 *		- XST_ERROR_COUNT_MAX if a time out occurred while attempting to
 *		  read the requested block.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_GetEdidBlock(XDpPsu *InstancePtr, u8 *Data, u8 BlockNum)
{
	u16 Offset;

	/* Calculate the I2C offset for the specified EDID block. */
	Offset = BlockNum * XDPPSU_EDID_BLOCK_SIZE;

	/* Issue the I2C read for the specified EDID block. */
	return XDpPsu_IicRead(InstancePtr, XDPPSU_EDID_ADDR, Offset,
						XDPPSU_EDID_BLOCK_SIZE, Data);
}

/******************************************************************************/
/**
 * Given a section tag, search for and retrieve the appropriate section data
 * block that is part of the specified DisplayID structure.
 *
 * @param	DisplayIdRaw is a pointer to the DisplayID data.
 * @param	SectionTag is the tag to search for that represents the desired
 *		section data block.
 * @param	DataBlockPtr will be set by this function to point to the
 *		appropriate section data block that is part of the DisplayIdRaw.
 *
 * @return
 *		- XST_SUCCESS if the section data block with the specified tag
 *		  was found.
 *		- XST_FAILURE otherwise.
 *
 * @note	The DataBlockPtr argument is modified to point to the entry
 *		in DisplayIdRaw that represents the beginning of the desired
		section data block.
 *
*******************************************************************************/
u32 XDpPsu_GetDispIdDataBlock(u8 *DisplayIdRaw, u8 SectionTag, u8 **DataBlockPtr)
{
	u8 Index;
	u8 DispIdSize = DisplayIdRaw[XDPPSU_DISPID_SIZE];
	u8 *DataBlock;

	/* Search for a section data block matching the specified tag. */
	for (Index = XDPPSU_DISPID_PAYLOAD_START; Index < DispIdSize; Index++) {
		DataBlock = &DisplayIdRaw[Index];

		/* Check if the tag mataches the current section data block. */
		if (DataBlock[XDPPSU_DISPID_DB_SEC_TAG] == SectionTag) {
			*DataBlockPtr = DataBlock;
			return XST_SUCCESS;
		}

		if (DataBlock[XDPPSU_DISPID_DB_SEC_SIZE] == 0) {
			/* End of valid section data blocks. */
			break;
		}
		else {
			/* Increment the search index to skip the remaining
			 * bytes of the current section data block. */
			Index += (XDPPSU_DISPID_DB_SEC_SIZE +
					DataBlock[XDPPSU_DISPID_DB_SEC_SIZE]);
		}
	}

	/* The entire DisplayID was searched or the search ended due to data
	 * no longer containing a valid section data block. No section data
	 * block with the specified tag was found. */
	return XST_FAILURE;
}
