/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss.c
* @{
*
* This file contains routines for AIE stream switch
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   10/21/2019  Optimize stream switch data structures
* 1.2   Tejus	01/04/2020  Cleanup error messages
* 1.3   Tejus   03/20/2020  Make internal function static
* 1.4   Tejus   03/21/2020  Fix slave port configuration bug
* 1.5   Tejus   03/21/2020  Add stream switch packet switch mode apis
* 1.6   Tejus   04/13/2020  Remove range apis and change to single tile apis
* 1.7   Nishad  06/19/2020  Move XAIE_PACKETID_MAX to xaiegbl.h
* 1.8   Tejus   06/10/2020  Switch to new io backend apis.
* 1.9   Nishad  07/01/2020  Move _XAie_GetSlaveIdx() helper API common
*			    directory.
* 2.0   Nishad  09/15/2020  Add check to validate XAie_StrmSwPktHeader value in
*			    _XAie_StrmPktSwMstrPortConfig()
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_ss.h"

#ifdef XAIE_FEATURE_SS_ENABLE

/************************** Constant Definitions *****************************/
#define XAIE_SS_MASTER_PORT_ARBITOR_LSB		0U
#define XAIE_SS_MASTER_PORT_ARBITOR_MASK	0x7
#define XAIE_SS_MASTER_PORT_MSELEN_LSB		0x3
#define XAIE_SS_MASTER_PORT_MSELEN_MASK		0x78

#define XAIE_SS_ARBITOR_MAX			0x7
#define XAIE_SS_MSEL_MAX			0x3
#define XAIE_SS_MASK				0x1F
#define XAIE_SS_MSELEN_MAX			0xF

#define XAIE_SS_DETERMINISTIC_MERGE_MAX_PKT_CNT (64 - 1) /* 6 bits */

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to get the register offset and value required to configure
* the selected slave port of the stream switch in the corresponding tile.
*
* @param	PortPtr - Pointer to the internal port data structure.
* @param	PortNum - Port Number.
* @param	Enable - Enable/Disable the slave port (1-Enable,0-Disable).
* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).
* @param	RegVal - pointer to store the register value.
* @param	RegOff - pointer to store the regster offset.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		Internal API. When PortType is TRACE and there are more than one
*		TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StrmConfigSlv(const XAie_StrmMod *StrmMod,
		StrmSwPortType PortType, u8 PortNum, u8 Enable, u8 PktEnable,
		u32 *RegVal, u32 *RegOff)
{
	*RegVal = 0U;
	const XAie_StrmPort  *PortPtr;

	/* Get the slave port pointer from stream module */
	PortPtr = &StrmMod->SlvConfig[PortType];

	if((PortPtr->NumPorts == 0) || (PortNum >= PortPtr->NumPorts)) {
		XAIE_ERROR("Invalid Slave Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	*RegOff = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;

	if(Enable != XAIE_ENABLE)
		return XAIE_OK;

	/* Frame the 32-bit reg value */
	*RegVal = XAie_SetField(Enable, StrmMod->SlvEn.Lsb,
			StrmMod->SlvEn.Mask) |
		XAie_SetField(PktEnable,
				StrmMod->SlvPktEn.Lsb, StrmMod->SlvPktEn.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to get the register offset and value required to configure
* the selected master port of the stream switch in the corresponding tile.
*
* @param	PortPtr - Pointer to the internal port data structure.
* @param	PortNum - Port Number.
* @param	Enable - Enable/Disable the slave port (1-Enable,0-Disable).
* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).
* @param	RegVal - pointer to store the register value.
* @param	RegOff - pointer to store the regster offset.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		Internal API.
*
*******************************************************************************/
static AieRC _StrmConfigMstr(const XAie_StrmMod *StrmMod,
		StrmSwPortType PortType, u8 PortNum, u8 Enable, u8 PktEnable,
		u8 Config, u32 *RegVal, u32 *RegOff)
{

	u8 DropHdr;
	*RegVal = 0U;
	const XAie_StrmPort *PortPtr;

	PortPtr = &StrmMod->MstrConfig[PortType];

	if((PortPtr->NumPorts == 0) || (PortNum >= PortPtr->NumPorts)) {
		XAIE_ERROR("Invalid Stream Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	*RegOff = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	if(Enable != XAIE_ENABLE)
		return XAIE_OK;

	/* Extract the drop header field */
	DropHdr = XAie_GetField(Config, StrmMod->DrpHdr.Lsb,
			StrmMod->DrpHdr.Mask);

	/* Frame 32-bit reg value */
	*RegVal = XAie_SetField(Enable, StrmMod->MstrEn.Lsb,
			StrmMod->MstrEn.Mask) |
		XAie_SetField(PktEnable, StrmMod->MstrPktEn.Lsb,
				StrmMod->MstrPktEn.Mask) |
		XAie_SetField(DropHdr, StrmMod->DrpHdr.Lsb,
				StrmMod->DrpHdr.Mask) |
		XAie_SetField(Config, StrmMod->Config.Lsb,
				StrmMod->Config.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to connect the selected master port to the specified slave
* port of the stream switch switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave - Slave port type.
* @param	SlvPortNum- Slave port number.
* @param	Master - Master port type.
* @param	MstrPortNum- Master port number.
* @param	SlvEnable - Enable/Disable the slave port (1-Enable,0-Disable).
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API. When PortType is TRACE and there are more than one
*		TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StreamSwitchConfigureCct(XAie_DevInst *DevInst,
		XAie_LocType Loc, StrmSwPortType Slave, u8 SlvPortNum,
		StrmSwPortType Master, u8 MstrPortNum, u8 Enable)
{
	AieRC RC;
	u64 MstrAddr;
	u64 SlvAddr;
	u32 MstrOff;
	u32 MstrVal;
	u32 SlvOff;
	u32 SlvVal;
	u8 SlaveIdx;
	u8 TileType;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if((Slave >= SS_PORT_TYPE_MAX) || (Master >= SS_PORT_TYPE_MAX)) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	RC = StrmMod->PortVerify(Slave, SlvPortNum, Master, MstrPortNum);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Slave port(Type: %d, Number: %d) can't connect to Master port(Type: %d, Number: %d) on the AIE tile.\n",
				Slave, SlvPortNum, Master, MstrPortNum);
                return RC;
        }

	RC = _XAie_GetSlaveIdx(StrmMod, Slave, SlvPortNum, &SlaveIdx);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to compute Slave Index\n");
		return RC;
	}

	/* Compute the register value and register address for the master port*/
	RC = _StrmConfigMstr(StrmMod, Master, MstrPortNum, Enable, XAIE_DISABLE,
			SlaveIdx, &MstrVal, &MstrOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Master config error\n");
		return RC;
	}

	/* Compute the register value and register address for slave port */
	RC = _XAie_StrmConfigSlv(StrmMod, Slave, SlvPortNum, Enable,
			XAIE_DISABLE, &SlvVal, &SlvOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Slave config error\n");
		return RC;
	}

	/* Compute absolute address and write to register */
	MstrAddr = MstrOff + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	SlvAddr = SlvOff + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	RC = XAie_Write32(DevInst, MstrAddr, MstrVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	return XAie_Write32(DevInst, SlvAddr, SlvVal);
}

/*****************************************************************************/
/**
*
* This API is used to enable the connection between the selected master port
* to the specified slave port of the stream switch switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmConnCctEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	return _XAie_StreamSwitchConfigureCct(DevInst, Loc, Slave, SlvPortNum,
			Master, MstrPortNum, XAIE_ENABLE);

}

/*****************************************************************************/
/**
*
* This API is used to disable the connection between the selected master port
* to the specified slave port of the stream switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmConnCctDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	return _XAie_StreamSwitchConfigureCct(DevInst, Loc, Slave, SlvPortNum,
			Master, MstrPortNum, XAIE_DISABLE);

}

/*****************************************************************************/
/**
*
* This API is used to configure the slave port of a stream switch.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	PktEn: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable packet switch
*		mode.
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable slave port.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. When PortType is TRACE and there are more than
*		one TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT
*		and PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StrmSlavePortConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 EnPkt, u8 Enable)
{
	AieRC RC;
	u64 Addr;
	u32 RegOff;
	u32 RegVal = 0U;
	u8 TileType;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if((Slave >= SS_PORT_TYPE_MAX)) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	/* Compute the register value and register address for slave port */
	RC = _XAie_StrmConfigSlv(StrmMod, Slave, SlvPortNum, EnPkt,
			Enable, &RegVal, &RegOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Slave config error\n");
		return RC;
	}

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	return XAie_Write32(DevInst, Addr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to Enable the slave port of a stream switch in packet switch
* mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlavePortEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum)
{
	return _XAie_StrmSlavePortConfig(DevInst, Loc, Slave, SlvPortNum,
			XAIE_ENABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to Disable the slave port of a stream switch in packet switch
* mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlavePortDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum)
{
	return _XAie_StrmSlavePortConfig(DevInst, Loc, Slave, SlvPortNum,
			XAIE_DISABLE, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to configure the register fields of Master ports for
* configuration in packet switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
* @param	DropHeader: Enable or disable the drop header bit
* @param	Arbitor: Arbitor to use for this packet switch connection
* @param	MselEn: MselEn field in the Master port register field
* @param	PktEn: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable packet switch
*		mode.
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable master port.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. When Enable is XAIE_DISABLE, the API configures
*		Master port register to reset value.
*
*
*******************************************************************************/
static AieRC _XAie_StrmPktSwMstrPortConfig(XAie_DevInst *DevInst,
		XAie_LocType Loc, StrmSwPortType Master, u8 MstrPortNum,
		XAie_StrmSwPktHeader DropHeader, u8 Arbitor, u8 MSelEn,
		u8 PktEn, u8 Enable)
{
	AieRC RC;
	u64 Addr;
	u32 RegOff;
	u32 RegVal;
	u8 TileType;
	const XAie_StrmMod *StrmMod;
	u32 Config = 0U;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(DropHeader > XAIE_SS_PKT_DROP_HEADER) {
		XAIE_ERROR("Invalid stream switch packet drop header value\n");
		return XAIE_INVALID_ARGS;
	}

	if((Arbitor > XAIE_SS_ARBITOR_MAX) || (MSelEn > XAIE_SS_MSELEN_MAX)) {
		XAIE_ERROR("Invalid Arbitor or MSel Enable\n");
		return XAIE_INVALID_ARGS;
	}

	if((Master >= SS_PORT_TYPE_MAX)) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	/* Construct Config and Drop header register fields */
	if(Enable == XAIE_ENABLE) {
		Config = XAie_SetField(DropHeader, StrmMod->DrpHdr.Lsb,
				StrmMod->DrpHdr.Mask) |
			XAie_SetField(Arbitor, XAIE_SS_MASTER_PORT_ARBITOR_LSB,
					XAIE_SS_MASTER_PORT_ARBITOR_MASK) |
			XAie_SetField(MSelEn, XAIE_SS_MASTER_PORT_MSELEN_LSB,
					XAIE_SS_MASTER_PORT_MSELEN_MASK);
	}

	/* Compute the register value and register address for the master port*/
	RC = _StrmConfigMstr(StrmMod, Master, MstrPortNum, Enable, PktEn,
			Config, &RegVal, &RegOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Master config error\n");
		return RC;
	}

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	return XAie_Write32(DevInst, Addr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to Enable the Master ports with configuration for packet
* switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
* @param	DropHeader: Enable or disable the drop header bit
* @param	Arbitor: Arbitor to use for this packet switch connection
* @param	MSelEn: MselEn field in the Master port register field
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*
*******************************************************************************/
AieRC XAie_StrmPktSwMstrPortEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Master, u8 MstrPortNum,
		XAie_StrmSwPktHeader DropHeader, u8 Arbitor, u8 MSelEn)
{
	return _XAie_StrmPktSwMstrPortConfig(DevInst, Loc, Master, MstrPortNum,
			DropHeader, Arbitor, MSelEn, XAIE_ENABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to configure the register fields of Master ports for
* configuration in packet switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Configures Master port register to reset value.
*
*
*******************************************************************************/
AieRC XAie_StrmPktSwMstrPortDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Master, u8 MstrPortNum)
{
	return _XAie_StrmPktSwMstrPortConfig(DevInst, Loc, Master, MstrPortNum,
			XAIE_SS_PKT_DONOT_DROP_HEADER, 0U, 0U, XAIE_DISABLE,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to configure the stream switch slave slot configuration
* registers. This API should be used in combination with other APIs to
* first configure the master and slave ports in packet switch mode. Disabling
* the slave port slot writes reset values to the registers.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type
* @param	SlvPortNum: Slave port number
* @param	SlotNum: Slot number for the slave port
* @param	Pkt: Packet with initialized packet id and packet type
* @param	Mask: Mask field in the slot register
* @param	Msel: Msel register field in the slave slot register
* @param	Arbitor: Arbitor to use for this packet switch connection
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to Enable or disable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. When PortType is TRACE and there are more than
*		one TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT
*		and PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StrmSlaveSlotConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum,
		XAie_Packet Pkt, u8 Mask, u8 MSel, u8 Arbitor, u8 Enable)
{
	u8 TileType;
	u64 RegAddr;
	u32 RegVal = 0U;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if((Arbitor > XAIE_SS_ARBITOR_MAX) || (MSel > XAIE_SS_MSEL_MAX) ||
			(Mask & ~XAIE_SS_MASK) ||
			(Pkt.PktId > XAIE_PACKET_ID_MAX)) {
		XAIE_ERROR("Invalid Arbitor, MSel, PktId or Mask\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;
	if((Slave >= SS_PORT_TYPE_MAX) || (SlotNum >= StrmMod->NumSlaveSlots) ||
			(SlvPortNum >= StrmMod->SlvConfig[Slave].NumPorts)) {
		XAIE_ERROR("Invalid Slave port and slot arguments\n");
		return XAIE_ERR_STREAM_PORT;
	}

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		StrmMod->SlvSlotConfig[Slave].PortBaseAddr +
		SlvPortNum * StrmMod->SlotOffsetPerPort +
		SlotNum * StrmMod->SlotOffset;

	if(Enable == XAIE_ENABLE) {
		RegVal = XAie_SetField(Pkt.PktId, StrmMod->SlotPktId.Lsb,
				StrmMod->SlotPktId.Mask) |
			XAie_SetField(Mask, StrmMod->SlotMask.Lsb,
					StrmMod->SlotMask.Mask) |
			XAie_SetField(XAIE_ENABLE, StrmMod->SlotEn.Lsb,
					StrmMod->SlotEn.Mask) |
			XAie_SetField(MSel, StrmMod->SlotMsel.Lsb,
					StrmMod->SlotMsel.Mask) |
			XAie_SetField(Arbitor, StrmMod->SlotArbitor.Lsb,
					StrmMod->SlotArbitor.Mask);
	}

	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to configure the stream switch slave slot configuration
* registers. This API should be used in combination with other APIs to
* first configure the master and slave ports in packet switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type
* @param	SlvPortNum: Slave port number
* @param	SlotNum: Slot number for the slave port
* @param	Pkt: Packet with initialized packet id and packet type
* @param	Mask: Mask field in the slot register
* @param	MSel: Msel register field in the slave slot register
* @param	Arbitor: Arbitor to use for this packet switch connection
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlaveSlotEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum,
		XAie_Packet Pkt, u8 Mask, u8 MSel, u8 Arbitor)
{
	return _XAie_StrmSlaveSlotConfig(DevInst, Loc, Slave, SlvPortNum,
			SlotNum, Pkt, Mask, MSel, Arbitor, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to disable the stream switch slave port slots. The API
* disables the slot and writes reset values to all other fields.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type
* @param	SlvPortNum: Slave port number
* @param	SlotNum: Slot number for the slave port
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlaveSlotDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum)
{
	XAie_Packet Pkt = XAie_PacketInit(0U, 0U);
	return _XAie_StrmSlaveSlotConfig(DevInst, Loc, Slave, SlvPortNum,
			SlotNum, Pkt, 0U, 0U, 0U, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to get the physical port id of the stream switch for a given
* tile location, logical port type and port number.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Port: XAIE_STRMSW_SLAVE/MASTER for Slave or Master ports
* @param	PortType: Logical port type of the stream switch
* @param	PortNum: Logical port number
* @param	PhyPortId: Pointer to store the physical port id.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmSwLogicalToPhysicalPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_StrmPortIntf Port, StrmSwPortType PortType, u8 PortNum,
		u8 *PhyPortId)
{
	u8 TileType;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) || (PhyPortId == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if((PortType >= SS_PORT_TYPE_MAX) || (Port > XAIE_STRMSW_MASTER)) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	if(Port == XAIE_STRMSW_SLAVE) {
		return _XAie_GetSlaveIdx(StrmMod, PortType, PortNum, PhyPortId);
	} else {
		return _XAie_GetMstrIdx(StrmMod, PortType, PortNum, PhyPortId);
	}
}

/*****************************************************************************/
/**
*
* This API is used to get logical port id and port number for a given tile
* location and physical port id.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Port: XAIE_STRMSW_SLAVE/MASTER for Slave or Master ports
* @param	PhyPortId: Physical port id
* @param	PortType: Pointer to store the logical port type of the stream
*		switch
* @param	PortNum: Pointer to store the logical port number
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmSwPhysicalToLogicalPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_StrmPortIntf Port, u8 PhyPortId, StrmSwPortType *PortType,
		u8 *PortNum)
{
	u8 TileType, MaxPhyPorts;
	const XAie_StrmSwPortMap *PortMap;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) || (PortType == XAIE_NULL) ||
			(PortNum == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(Port > XAIE_STRMSW_MASTER) {
		XAIE_ERROR("Invalid Stream Switch port interface\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	if(Port == XAIE_STRMSW_SLAVE) {
		PortMap = StrmMod->SlavePortMap;
		MaxPhyPorts = StrmMod->MaxSlavePhyPortId;
	} else {
		PortMap = StrmMod->MasterPortMap;
		MaxPhyPorts = StrmMod->MaxMasterPhyPortId;
	}

	if(PhyPortId > MaxPhyPorts) {
		XAIE_ERROR("Invalid physical port id\n");
		return XAIE_ERR_STREAM_PORT;
	}

	*PortType = PortMap[PhyPortId].PortType;
	*PortNum = PortMap[PhyPortId].PortNum;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to configure the stream switch module for deterministic
* merge of packets from its ports.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
* @param	Slave: Slave port type.
* @param	PortNum: Slave port number.
* @param	PktCount: Number of packets to merge from Slave and PortNum.
* @param	Position: Position of the packets arriving from Slave & PortNum.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmSwDeterministicMergeConfig(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor, StrmSwPortType Slave, u8 PortNum,
		u8 PktCount, u8 Position)
{
	AieRC RC;
	u8 TileType, SlvIdx;
	u32 RegVal, Mask;
	u64 RegAddr;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;
	if(StrmMod->DetMergeFeature == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Deterministic merge feature is not available\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if((Slave >= SS_PORT_TYPE_MAX) ||
			(PortNum >= StrmMod->SlvConfig[Slave].NumPorts)) {
		XAIE_ERROR("Invalid stream port type and port number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if((Arbitor >= StrmMod->DetMerge->NumArbitors) ||
			(Position >= StrmMod->DetMerge->NumPositions) ||
			(PktCount > XAIE_SS_DETERMINISTIC_MERGE_MAX_PKT_CNT)) {
		XAIE_ERROR("Invalid Arbitor/Position or PktCount\n");
		return XAIE_INVALID_ARGS;
	}

	RC = _XAie_GetSlaveIdx(StrmMod, Slave, PortNum, &SlvIdx);
	if(RC != XAIE_OK) {
		return RC;
	}

	RegAddr = StrmMod->DetMerge->ConfigBase +
		StrmMod->DetMerge->ArbConfigOffset * Arbitor +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	if(Position > 1U) {
		RegAddr += 0x4U;
	}

	if((Position % 2U) == 0U) {
		RegVal = XAie_SetField(SlvIdx, StrmMod->DetMerge->SlvId0.Lsb,
				StrmMod->DetMerge->SlvId0.Mask) |
			XAie_SetField(PktCount, StrmMod->DetMerge->PktCount0.Lsb,
					StrmMod->DetMerge->PktCount0.Mask);
		Mask = StrmMod->DetMerge->SlvId0.Mask |
			StrmMod->DetMerge->PktCount0.Mask;
	} else {
		RegVal = XAie_SetField(SlvIdx, StrmMod->DetMerge->SlvId1.Lsb,
				StrmMod->DetMerge->SlvId1.Mask) |
			XAie_SetField(PktCount, StrmMod->DetMerge->PktCount1.Lsb,
					StrmMod->DetMerge->PktCount1.Mask);
		Mask = StrmMod->DetMerge->SlvId1.Mask |
			StrmMod->DetMerge->PktCount1.Mask;
	}

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to enable/disable the deterministic merge feature of stream
* switch modules.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
* @param	Enable: XAIE_ENABLE to enable. XAIE_DISABLE to disable.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_StrmSwDeterministicMergeCtrl(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor, u8 Enable)
{
	u8 TileType;
	u32 RegVal;
	u64 RegAddr;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;
	if(StrmMod->DetMergeFeature == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Deterministic merge feature is not available\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(Arbitor >= StrmMod->DetMerge->NumArbitors) {
		XAIE_ERROR("Invalid Arbitor number\n");
		return XAIE_INVALID_ARGS;
	}

	RegAddr = StrmMod->DetMerge->EnableBase +
		StrmMod->DetMerge->ArbConfigOffset * Arbitor +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	RegVal = XAie_SetField(Enable, StrmMod->DetMerge->Enable.Lsb,
			StrmMod->DetMerge->Enable.Mask);

	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to enable the deterministic merge feature of stream switch
* modules.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmSwDeterministicMergeEnable(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor)
{
	return _XAie_StrmSwDeterministicMergeCtrl(DevInst, Loc, Arbitor,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to disable the deterministic merge feature of stream switch
* modules.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmSwDeterministicMergeDisable(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor)
{
	return _XAie_StrmSwDeterministicMergeCtrl(DevInst, Loc, Arbitor,
			XAIE_DISABLE);
}

#endif /* XAIE_FEATURE_SS_ENABLE */
/** @} */
