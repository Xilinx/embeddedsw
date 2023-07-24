/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_edid.c
 * @addtogroup dp Overview
 * @{
 *
 * This file contains functions related to accessing the Extended Display
 * Identification Data (EDID) of a specified sink using the XDp driver operating
 * in TX mode.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"

/**************************** Function Definitions ****************************/

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function retrieves an immediately connected RX device's Extended Display
 * Identification Data (EDID) structure.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxGetEdid(XDp *InstancePtr, u8 *Edid)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(Edid != NULL);

	/* Retrieve the base EDID block = EDID block #0. */
	Status = XDp_TxGetEdidBlock(InstancePtr, Edid, 0);

	return Status;
}

/******************************************************************************/
/**
 * This function retrieves a remote RX device's Extended Display Identification
 * Data (EDID) structure.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
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
u32 XDp_TxGetRemoteEdid(XDp *InstancePtr, u8 LinkCountTotal,
						u8 *RelativeAddress, u8 *Edid)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(Edid != NULL);

	/* Retrieve the base EDID block = EDID block #0. */
	Status = XDp_TxGetRemoteEdidBlock(InstancePtr, Edid, 0, LinkCountTotal,
							RelativeAddress);

	return Status;
}

/******************************************************************************/
/**
 * Retrieve an immediately connected RX device's Extended Display Identification
 * Data (EDID) block given the block number. A block number of 0 represents the
 * base EDID and subsequent block numbers represent EDID extension blocks.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxGetEdidBlock(XDp *InstancePtr, u8 *Data, u8 BlockNum)
{
	u32 Status;
	u16 Offset;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(Data != NULL);

	/* Calculate the I2C offset for the specified EDID block. */
	Offset = BlockNum * XDP_EDID_BLOCK_SIZE;

	/* Issue the I2C read for the specified EDID block. */
	Status = XDp_TxIicRead(InstancePtr, XDP_EDID_ADDR, Offset,
						XDP_EDID_BLOCK_SIZE, Data);

	return Status;
}

/******************************************************************************/
/**
 * Retrieve a downstream DisplayPort device's Extended Display Identification
 * Data (EDID) block given the block number. A block number of 0 represents the
 * base EDID and subsequent block numbers represent EDID extension blocks.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Data is a pointer to the data buffer to save the block data to.
 * @param	BlockNum is the EDID block number to retrieve.
 * @param	LinkCountTotal is the total DisplayPort links connecting the
 *		DisplayPort TX to the targeted downstream device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the targeted DisplayPort device.
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
u32 XDp_TxGetRemoteEdidBlock(XDp *InstancePtr, u8 *Data, u8 BlockNum,
					u8 LinkCountTotal, u8 *RelativeAddress)
{
	u32 Status;
	u16 Offset;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(Data != NULL);

	/* Calculate the I2C offset for the specified EDID block. */
	Offset = BlockNum * XDP_EDID_BLOCK_SIZE;

	/* Issue the I2C read for the specified EDID block. */
	Status = XDp_TxRemoteIicRead(InstancePtr, LinkCountTotal,
		RelativeAddress, XDP_EDID_ADDR, Offset, XDP_EDID_BLOCK_SIZE,
									Data);

	return Status;
}

/******************************************************************************/
/**
 * Search for and retrieve a downstream DisplayPort device's Extended Display
 * Identification Data (EDID) extension block of type DisplayID.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Data is a pointer to the data buffer to save the DisplayID to.
 * @param	LinkCountTotal is the total DisplayPort links connecting the
 *		DisplayPort TX to the targeted downstream device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the targeted DisplayPort device.
 *
 * @return
 *		- XST_SUCCESS a DisplayID extension block was found.
 *		- XST_ERROR_COUNT_MAX if a time out occurred while attempting to
 *		  read an extension block.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE if no DisplayID extension block was found or some
 *		  error occurred in the search.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxGetRemoteEdidDispIdExt(XDp *InstancePtr, u8 *Data,
					u8 LinkCountTotal, u8 *RelativeAddress)
{
	u32 Status;
	u8 NumExt;
	u8 ExtIndex;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(Data != NULL);

	/* Get the base EDID block. */
	Status = XDp_TxGetRemoteEdid(InstancePtr, LinkCountTotal,
							RelativeAddress, Data);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	NumExt = Data[XDP_EDID_EXT_BLOCK_COUNT];
	for (ExtIndex = 0; ExtIndex < NumExt; ExtIndex++) {
		/* Get an EDID extension block. */
		Status = XDp_TxGetRemoteEdidBlock(InstancePtr, Data,
				ExtIndex + 1, LinkCountTotal, RelativeAddress);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		if (XDp_TxIsEdidExtBlockDispId(Data)) {
			/* The current extension block is of type DisplayID. */
			return XST_SUCCESS;
		}
	}

	/* All extension blocks have been searched; no DisplayID extension block
	 * exists in sink's EDID structure. */
	return XST_FAILURE;
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
 *		section data block.
 *
*******************************************************************************/
u32 XDp_TxGetDispIdDataBlock(u8 *DisplayIdRaw, u8 SectionTag, u8 **DataBlockPtr)
{
	u8 Index;
	u8 DispIdSize = DisplayIdRaw[XDP_TX_DISPID_SIZE];
	u8 *DataBlock;

	/* Verify arguments. */
	Xil_AssertNonvoid(DisplayIdRaw != NULL);

	/* Search for a section data block matching the specified tag. */
	for (Index = XDP_TX_DISPID_PAYLOAD_START; Index < DispIdSize; Index++) {
		DataBlock = &DisplayIdRaw[Index];

		/* Check if the tag matches the current section data block. */
		if (DataBlock[XDP_TX_DISPID_DB_SEC_TAG] == SectionTag) {
			*DataBlockPtr = DataBlock;
			return XST_SUCCESS;
		}

		if (DataBlock[XDP_TX_DISPID_DB_SEC_SIZE] == 0) {
			/* End of valid section data blocks. */
			break;
		}
		else {
			/* Increment the search index to skip the remaining
			 * bytes of the current section data block. */
			Index += (XDP_TX_DISPID_DB_SEC_SIZE +
					DataBlock[XDP_TX_DISPID_DB_SEC_SIZE]);
		}
	}

	/* The entire DisplayID was searched or the search ended due to data
	 * no longer containing a valid section data block. No section data
	 * block with the specified tag was found. */
	return XST_FAILURE;
}

/******************************************************************************/
/**
 * Search for and retrieve a downstream DisplayPort device's Tiled Display
 * Topology (TDT) section data block that is part of the downstream device's
 * DisplayID structure. The DisplayID structure is part of the Extended Display
 * Identification Data (EDID) in the form of an extension block.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	EdidExt is a pointer to the data area that will be filled by the
 *		retrieved DisplayID extension block.
 * @param	LinkCountTotal is the total DisplayPort links connecting the
 *		DisplayPort TX to the targeted downstream device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the targeted DisplayPort device.
 * @param	DataBlockPtr will be set by this function to point to the TDT
 *		data block that is part of the EdidExt extension block.
 *
 * @return
 *		- XST_SUCCESS a DisplayID extension block was found.
 *		- XST_ERROR_COUNT_MAX if a time out occurred while attempting to
 *		  read an extension block.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE if no DisplayID extension block was found or some
 *		  error occurred in the search.
 *
 * @note	The EdidExt will be filled with the DisplayID EDID extension
 *		block and the DataBlockPtr argument is modified to point to the
 *		EdidExt entry representing the TDT section data block.
 *
*******************************************************************************/
u32 XDp_TxGetRemoteTiledDisplayDb(XDp *InstancePtr, u8 *EdidExt,
		u8 LinkCountTotal, u8 *RelativeAddress, u8 **DataBlockPtr)
{
	u32 Status;
	u8 *EdidExtDispId;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(EdidExt != NULL);

	/* Obtain a DisplayID EDID extension block. */
	Status = XDp_TxGetRemoteEdidDispIdExt(InstancePtr, EdidExt,
					LinkCountTotal, RelativeAddress);
	if (Status != XST_SUCCESS) {
		/* The sink does not have a DisplayID EDID extension block. */
		return Status;
	}

	/* The first byte of the extension block is the tag. */
	EdidExtDispId = &EdidExt[0x01];

	/* Obtain the tiled display topology block data from the DisplayId EDID
	 * extension block. */
	Status = XDp_TxGetDispIdDataBlock(EdidExtDispId, XDP_TX_DISPID_TDT_TAG,
								DataBlockPtr);
	if (Status != XST_SUCCESS) {
		/* The sink does not possess a DisplayID EDID data block with
		 * the specified tag. */
		return Status;
	}

	return XST_SUCCESS;
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */
/** @} */
