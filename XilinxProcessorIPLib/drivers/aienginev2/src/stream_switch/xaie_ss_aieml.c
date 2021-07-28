/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss_aieml.c
* @{
*
* This file contains internal api implementations for AIE-ML stream switch.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who         Date        Changes
* ----- ---------   ----------  -----------------------------------------------
* 1.0   Siddharth   12/09/2020  Initial creation
* </pre>
*
******************************************************************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"

#ifdef XAIE_FEATURE_SS_ENABLE

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This api verifies if a stream switch connection exists within the ports.
* Numerous conditions exist to determine if the port combination is valid.
* The method assumes both ports exist, and the relevant tile is an AIE-Tile.
*
* @param	Slave: The type of the slave port.
* @param	SlvPortNum: The number of the slave port.
* @param	Master: The type of the master port.
* @param	MstrPortNum: The number of the master port.
*
* @return	XAIE_OK if a stream switch connection is possible.
*		XAIE_ERR_STREAM_PORT if the connection isn't possible.
*
* @note		Internal API for AIE-ML. This API shouldn't be called directly.
*		It is invoked using a function pointer within the Stream
*		Module data structure.
*
*****************************************************************************/
AieRC _XAieMl_AieTile_StrmSwCheckPortValidity(StrmSwPortType Slave,
		u8 SlvPortNum, StrmSwPortType Master, u8 MstrPortNum)
{
	AieRC RC = XAIE_OK;

	switch(Slave) {
	case TRACE:
		if(Master == FIFO || Master == SOUTH || (Master == DMA &&
			MstrPortNum == 0U)) {
			break;
		}
		RC = XAIE_ERR_STREAM_PORT;
		break;
	case CORE:
		if(Master == CORE) {
			RC = XAIE_ERR_STREAM_PORT;
		}
		break;
	case DMA:
		if((Master == DMA && SlvPortNum == MstrPortNum) ||
			Master != DMA){
			break;
		}
		RC = XAIE_ERR_STREAM_PORT;
		break;
	case CTRL:
		if(Master == DMA || Master == CTRL) {
			RC = XAIE_ERR_STREAM_PORT;
		}
		break;
	case FIFO:
		break;
	case SOUTH:
	case WEST:
	case NORTH:
	case EAST:
		if(Slave == Master && SlvPortNum != MstrPortNum) {
			RC = XAIE_ERR_STREAM_PORT;
		}
		break;
	default:
		RC = XAIE_ERR_STREAM_PORT;
		break;
	}

	return RC;
}

/**
*
* This api verifies if a stream switch connection exists within the ports.
* Less valid ports exist in the MemTile, but certain combinations exist.
* The method assumes both ports exist, and the relevant tile is an MEM-Tile.
*
* @param	Slave: The type of the slave port.
* @param	SlvPortNum: The number of the slave port.
* @param	Master: The type of the master port.
* @param	MstrPortNum: The number of the master port.
*
* @return	XAIE_OK if a stream switch connection is possible.
* 		XAIE_ERR_STREAM_PORT if the connection isn't possible.
*
* @note		Internal API for AIE-ML. This API shouldn't be called directly.
* 		It is invoked using a function pointer within the Stream
* 		Module data structure.
*
*****************************************************************************/
AieRC _XAieMl_MemTile_StrmSwCheckPortValidity(StrmSwPortType Slave,
		u8 SlvPortNum, StrmSwPortType Master, u8 MstrPortNum)
{
	AieRC RC = XAIE_OK;

	switch(Slave) {
	case TRACE:
		if(Master == SOUTH || (Master == DMA && MstrPortNum == 5U)) {
			break;
		}
		RC = XAIE_ERR_STREAM_PORT;
		break;
	case DMA:
		if((Master == DMA && SlvPortNum == MstrPortNum) ||
				Master != DMA){
			break;
		}
		RC = XAIE_ERR_STREAM_PORT;
		break;
	case CTRL:
		if((Master == DMA && MstrPortNum != 5U) || Master == CTRL) {
			RC = XAIE_ERR_STREAM_PORT;
		}
		break;
	case SOUTH:
	case NORTH:
		if((Master == SOUTH || Master == NORTH) &&
			SlvPortNum != MstrPortNum) {
			RC = XAIE_ERR_STREAM_PORT;
		}
		break;
	default:
		RC = XAIE_ERR_STREAM_PORT;
		break;
	}

	return RC;
}

/**
*
* This api verifies if a stream switch connection exists within the ports.
* Less types of stream switch port exist within the SHIM tile.
* The method assumes both ports exist, and the relevant tile is a SHIM-Tile.
*
* @param	Slave: The type of the slave port.
* @param	SlvPortNum: The number of the slave port.
* @param	Master: The type of the master port.
* @param	MstrPortNum: The number of the master port.
*
* @return	XAIE_OK if a stream switch connection is possible.
* 		XAIE_ERR_STREAM_PORT if the connection isn't possible.
*
* @note		Internal API for AIE-ML. This API shouldn't be called directly.
* 		It is invoked using a function pointer within the Stream
* 		Module data structure.
*
*****************************************************************************/

AieRC _XAieMl_ShimTile_StrmSwCheckPortValidity(StrmSwPortType Slave,
		u8 SlvPortNum, StrmSwPortType Master, u8 MstrPortNum)
{
	AieRC RC = XAIE_OK;

	switch(Slave) {
	case TRACE:
		if(Master == FIFO || Master == SOUTH) {
			break;
		}
		RC = XAIE_ERR_STREAM_PORT;
		break;
	case CTRL:
		if(Master == CTRL) {
			RC = XAIE_ERR_STREAM_PORT;
		}
		break;
	case FIFO:
	case SOUTH:
		break;
	case WEST:
	case NORTH:
	case EAST:
		if(Slave == Master && SlvPortNum != MstrPortNum) {
			RC = XAIE_ERR_STREAM_PORT;
		}
		break;
	default:
		RC = XAIE_ERR_STREAM_PORT;
		break;
	}

	return RC;
}

#endif /* XAIE_FEATURE_SS_ENABLE */
