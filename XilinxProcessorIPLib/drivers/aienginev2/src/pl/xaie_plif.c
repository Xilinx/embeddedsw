/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_plif.c
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/28/2019  Initial creation
* 1.1   Tejus   03/16/2020  Implementation of apis for Mux/Demux configuration
* 1.2   Tejus   03/20/2020  Make internal functions static
* 1.3   Tejus   04/13/2020  Remove range apis and change to single tile apis
* 1.4   Tejus   06/10/2020  Switch to new io backend apis.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_plif.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/
#define XAIE_PLIF_WIDTH_64SHIFT 6U
#define XAIE_MUX_DEMUX_CONFIG_TYPE_PL	0x0
#define XAIE_MUX_DEMUX_CONFIG_TYPE_DMA	0x1
#define XAIE_MUX_DEMUX_CONFIG_TYPE_NOC	0x2

#define XAIE_STREAM_SOUTH_PORT_2	2U
#define XAIE_STREAM_SOUTH_PORT_3	3U
#define XAIE_STREAM_SOUTH_PORT_4	4U
#define XAIE_STREAM_SOUTH_PORT_5	5U
#define XAIE_STREAM_SOUTH_PORT_6	6U
#define XAIE_STREAM_SOUTH_PORT_7	7U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API sets the bypass register for a range of AIE tile on the PL2ME
* interface.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
* @param	Enable: XAIE_DISABLE for disable, XAIE_ENABLE for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
static AieRC _XAie_PlIfBliBypassConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, u8 Enable)
{
	u8 TileType;
	u64 RegAddr;
	u32 FldVal;
	u32 Mask;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	/*
	 * Ports 3 and 7 BLI Bypass is enabled in the hardware by default.
	 * Check and return error if the portnum is invalid.
	 */
	if((PortNum > PlIfMod->MaxByPassPortNum) || (PortNum == 3U) ||
			(PortNum == 7U)) {
		XAIE_ERROR("Invalid Port Number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Port number 4-6 are mapped to bits 3-5 */
	if(PortNum > 3U)
		PortNum--;

	Mask = PlIfMod->DownSzrByPass[PortNum].Mask;
	FldVal = XAie_SetField(Enable, PlIfMod->DownSzrByPass[PortNum].Lsb,
			Mask);

	/* Compute register address */
	RegAddr = PlIfMod->DownSzrByPassOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	XAie_MaskWrite32(DevInst, RegAddr, Mask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the Downsizer Enable register for stream ports connecting
* PL2AIE.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Enable: XAIE_DISABLE for disable, XAIE_ENABLE for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
static AieRC _XAie_PlIfDownSzrPortEnableReg(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 PortNum, u8 Enable)
{
	u8 TileType;
	u64 RegAddr;
	u32 FldVal;
	u32 Mask;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	if((PortNum > PlIfMod->NumDownSzrPorts)) {
		XAIE_ERROR("Invalid Port Number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Enable or Disable stream port in PL2ME downsizer enable register */
	Mask = PlIfMod->DownSzrEn[PortNum].Mask;
	FldVal = XAie_SetField(Enable, PlIfMod->DownSzrEn[PortNum].Lsb, Mask);

	/* Compute register address */
	RegAddr = PlIfMod->DownSzrEnOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	XAie_MaskWrite32(DevInst, RegAddr, Mask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided. This is an internal API
* only and can be used to Enable or Disable AIE->PL interface.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		When the width is 128 bits, port number can be any one of the
*		valid port numbers. Ex: For 4_5 combo, port number can be 4 or
*		5. Internal API only.
*
******************************************************************************/
static AieRC _XAie_AieToPlIntfConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width, u8 Enable)
{
	u8 TileType;
	u8 Idx;
	u32 FldVal;
	u32 FldMask;
	u32 RegOff;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Check Width for validity */
	if((Width != PLIF_WIDTH_32) && (Width != PLIF_WIDTH_64) &&
			(Width != PLIF_WIDTH_128)) {
		XAIE_ERROR("Invalid Width\n");
		return XAIE_INVALID_PLIF_WIDTH;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	/* Setup field mask and field value for aie to pl interface */
	if(PortNum >= PlIfMod->NumDownSzrPorts) {
		XAIE_ERROR("Invalid stream port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if(Width == PLIF_WIDTH_128) {
		/*
		 * Get the register field to configure. Valid port
		 * numbers are 0-5. Divide the port number by 2 to get
		 * the right index. Two 64 Bit ports are combined to
		 * get a 128 Bit port.
		 */
		Idx = PortNum / 2U;
		FldMask = PlIfMod->UpSzr128Bit[Idx].Mask;
		FldVal = XAie_SetField(Enable,
				PlIfMod->UpSzr128Bit[Idx].Lsb,
				FldMask);
	} else {
		FldMask = PlIfMod->UpSzr32_64Bit[PortNum].Mask;
		/*
		 * Field Value has to be set to 1 for 64 Bit interface
		 * and 0 for 32 Bit interface
		 */
		FldVal = XAie_SetField(Width >> XAIE_PLIF_WIDTH_64SHIFT,
				PlIfMod->UpSzr32_64Bit[PortNum].Lsb,
				FldMask);
	}

	RegOff = PlIfMod->UpSzrOff;

	RegAddr = RegOff + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Mask write to the upsizer register */
	XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the stream width for PL->AIE interfaces. The downsizer
* register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register. The api configures the interface for a range
* of AIE Tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to enable stream ports in downsizer enable register is not
*		required. When configuring for 128 bit width, the user has to
*		provide one valid port. The api enables the other port by
*		default. Internal API only.
*
******************************************************************************/
static AieRC _XAie_PlToAieIntfConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width, u8 Enable)
{
	u8 TileType;
	u8 Idx;
	u32 FldVal;
	u32 FldMask;
	u32 DwnSzrEnMask;
	u32 DwnSzrEnVal;
	u32 RegOff;
	u64 RegAddr;
	u64 DwnSzrEnRegAddr;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Check Width for validity */
	if((Width != PLIF_WIDTH_32) && (Width != PLIF_WIDTH_64) &&
			(Width != PLIF_WIDTH_128)) {
		XAIE_ERROR("Invalid Width\n");
		return XAIE_INVALID_PLIF_WIDTH;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	/* Setup field mask and field value for pl to aie interface */
	if(PortNum >= PlIfMod->NumDownSzrPorts) {
		XAIE_ERROR("Invalid stream port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if(Width == PLIF_WIDTH_128) {
		/*
		 * Get the register field to configure. Valid port
		 * numbers are 0-5. Divide the port number by 2 to get
		 * the right index. Two 64 Bit ports are combined to
		 * get a 128 Bit port.
		 */
		Idx = PortNum / 2U;
		FldMask = PlIfMod->DownSzr128Bit[Idx].Mask;
		FldVal = XAie_SetField(Enable,
				PlIfMod->DownSzr128Bit[Idx].Lsb,
				FldMask);
	} else {
		FldMask = PlIfMod->DownSzr32_64Bit[PortNum].Mask;
		/*
		 * Field Value has to be set to 1 for 64 Bit interface
		 * and 0 for 32 Bit interface. Width is shifted to move 64(2^6)
		 * to LSB. When width is 32, the shift results in 0.
		 */
		FldVal = XAie_SetField(Width >> XAIE_PLIF_WIDTH_64SHIFT,
				PlIfMod->DownSzr32_64Bit[PortNum].Lsb,
				FldMask);
	}

	/*
	 * For PL to AIE interfaces, once the downsizer register is configured,
	 * bits corresponding to the stream ports have to enabled in the
	 * downsizer enable register.
	 */
	DwnSzrEnMask = PlIfMod->DownSzrEn[PortNum].Mask;
	DwnSzrEnVal = XAie_SetField(Enable,
			PlIfMod->DownSzrEn[PortNum].Lsb, DwnSzrEnMask);

	/* If width is 128 bits, enable both ports */
	if(Width == PLIF_WIDTH_128) {
		PortNum = (PortNum % 2U) ? (PortNum - 1U) : (PortNum + 1U);

		DwnSzrEnMask |= PlIfMod->DownSzrEn[PortNum].Mask;
		DwnSzrEnVal |= XAie_SetField(Enable,
				PlIfMod->DownSzrEn[PortNum].Lsb,
				PlIfMod->DownSzrEn[PortNum].Mask);
	}

	RegOff = PlIfMod->DownSzrOff;

	RegAddr = RegOff + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	/* Mask write to the downsizer register */
	XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);

	/* Mast write to downsizer enable register */
	DwnSzrEnRegAddr = PlIfMod->DownSzrEnOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	XAie_MaskWrite32(DevInst, DwnSzrEnRegAddr, DwnSzrEnMask, DwnSzrEnVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API Enables the stream port with width for PL->AIE interfaces. The
* downsizer register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to enable stream ports in downsizer enable register is not
*		required.
*
******************************************************************************/
AieRC XAie_PlToAieIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_PlToAieIntfConfig(DevInst, Loc, PortNum, Width,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables the stream port with width for PL->AIE interfaces. The
* downsizer register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to disable stream ports in downsizer enable register is not
*		required.
*
******************************************************************************/
AieRC XAie_PlToAieIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_PlToAieIntfConfig(DevInst, Loc, PortNum, Width,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API Enables the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided.
*
* @param	DevInst: Device Instance
* @param	Loc: Coodinate of AIE Tile
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AieToPlIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_AieToPlIntfConfig(DevInst, Loc, PortNum, Width,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided.
*
* @param	DevInst: Device Instance
* @param	Loc: Coodinate of AIE Tile
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AieToPlIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_AieToPlIntfConfig(DevInst, Loc, PortNum, Width,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API Enables a stream port in the Downsizer Enable register for connecting
* PL2AIE. This is for a single tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfDownSzrEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_PlIfDownSzrPortEnableReg(DevInst, Loc, PortNum,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables a stream port in the Downsizer Enable register for
* connecting PL2AIE. This is for a single tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfDownSzrDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_PlIfDownSzrPortEnableReg(DevInst, Loc, PortNum,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API Enables the BLI bypass for a port number at the PL2ME interface, for
* a single AIE tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfBliBypassEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_PlIfBliBypassConfig(DevInst, Loc, PortNum, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables the BLI bypass for a port number at the PL2ME interface, for
* a single AIE tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfBliBypassDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_PlIfBliBypassConfig(DevInst, Loc, PortNum, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API configures the Mux registers in the AIE Shim NoC tiles. The input
* stream switch ports for incoming data from PL, NoC or DMA can be enabled using
* this API.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 6, 7)
* @param	InputConnectionType: XAIE_MUX_DEMUX_CONFIG_TYPE_PL,
*		XAIE_MUX_DEMUX_CONFIG_TYPE_DMA or XAIE_MUX_DEMUX_CONFIG_TYPE_NOC
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API Only.
*
******************************************************************************/
static AieRC _XAie_ConfigShimNocMux(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, u8 InputConnectionType)
{
	u8 TileType;
	u32 FldVal;
	u32 FldMask;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if((PortNum != XAIE_STREAM_SOUTH_PORT_2) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_6) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_7)) {
		XAIE_ERROR("Invalid port number for Mux\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Map the port numbers to 0, 1, 2, 3 */
	if(PortNum > 3U) {
		PortNum -= 4U;
	} else {
		PortNum -= 2U;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	FldVal = InputConnectionType << PlIfMod->ShimNocMux[PortNum].Lsb;
	FldMask = PlIfMod->ShimNocMux[PortNum].Mask;

	RegAddr = PlIfMod->ShimNocMuxOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Mask write to the Mux register */
	XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the DeMux registers in the AIE Shim NoC tiles. The output
* stream switch ports for outgoing data to PL, NoC or DMA can be enabled using
* this API.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 4, 5)
* @param	OutputConnectionType: XAIE_MUX_DEMUX_CONFIG_TYPE_PL,
*		XAIE_MUX_DEMUX_CONFIG_TYPE_DMA or XAIE_MUX_DEMUX_CONFIG_TYPE_NOC
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API Only.
*
******************************************************************************/
static AieRC _XAie_ConfigShimNocDeMux(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, u8 OutputConnectionType)
{
	u8 TileType;
	u32 FldVal;
	u32 FldMask;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if((PortNum != XAIE_STREAM_SOUTH_PORT_2) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_4) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_5)) {
		XAIE_ERROR("Invalid port number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Map the port numbers to 0, 1, 2, 3 */
	PortNum -= 2U;

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	FldVal = OutputConnectionType << PlIfMod->ShimNocDeMux[PortNum].Lsb;
	FldMask = PlIfMod->ShimNocDeMux[PortNum].Mask;

	RegAddr = PlIfMod->ShimNocDeMuxOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Mask write to the Mux register */
	XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables the Shim DMA to input stream switch portconnection in the Mux
* configuration register for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (3, 7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableShimDmaToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	if((PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_7)) {
		XAIE_ERROR("Invalid port number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	return _XAie_ConfigShimNocMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_DMA);
}

/*****************************************************************************/
/**
*
* This API enables the stream switch port to Shim DMA connection (for data
* going outside of AIE) in the DeMux configuration register for a given AIE
* tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableAieToShimDmaStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	if((PortNum != XAIE_STREAM_SOUTH_PORT_2) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_3)) {
		return XAIE_ERR_STREAM_PORT;
	}

	return _XAie_ConfigShimNocDeMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_DMA);
}

/*****************************************************************************/
/**
*
* This API enables the NoC to input stream switch portconnection in the Mux
* configuration register for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 6, 7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableNoCToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_ConfigShimNocMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_NOC);
}

/*****************************************************************************/
/**
*
* This API enables the stream switch port to NoC connection (for data
* going outside of AIE) in the DeMux configuration register for a given AIE
* tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param        PortNum: Stream Port Number (2, 3, 4, 5)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableAieToNoCStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_ConfigShimNocDeMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_NOC);
}

/*****************************************************************************/
/**
*
* This API enables the PL to input stream switch portconnection in the Mux
* configuration register for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 6, 7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		After a device reset, AIE<->PL connections are enabled by
*		default. This API has to be called only if AIE<->SHIMDMA or
*		AIE<->NOC connections have been enabled after a device reset.
*
******************************************************************************/
AieRC XAie_EnablePlToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_ConfigShimNocMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_PL);
}

/*****************************************************************************/
/**
*
* This API enables the stream switch port to PL connection (for data
* going outside of AIE) in the DeMux configuration register for a given AIE
* tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param        PortNum: Stream Port Number (2, 3, 4, 5)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		After a device reset, AIE<->PL connections are enabled by
*		default. This API has to be called only if AIE<->SHIMDMA or
*		AIE<->NOC connections have been enabled after a device reset.
*
******************************************************************************/
AieRC XAie_EnableAieToPlStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	return _XAie_ConfigShimNocDeMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_PL);
}

/** @} */
