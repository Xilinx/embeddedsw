/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_strm.h
* @{
*
*  Header file for stream switch configuration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/06/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Hyun    10/03/2018  Added the event port select function
* 1.3  Hyun    10/08/2018  Added the offset for shim trace slave port
* 1.4  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.5  Wendy   16/05/2019  Wrap pointers parameters with () in macro
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_STRM_H
#define XAIETILE_STRM_H

/***************************** Include Files *********************************/
#include "xaiegbl_reginit.h"

/**************************** Variable Declarations ***************************/
extern XAieGbl_RegStrmMstr TileStrmMstr[];
extern XAieGbl_RegStrmSlv TileStrmSlv[];
extern XAieGbl_RegStrmSlot TileStrmSlot[];
extern XAieGbl_RegStrmMstr ShimStrmMstr[];
extern XAieGbl_RegStrmSlv ShimStrmSlv[];
extern XAieGbl_RegStrmSlot ShimStrmSlot[];
extern XAieGbl_RegShimMuxCfg ShimStrmMuxCfg;
extern XAieGbl_RegShimDemCfg ShimStrmDemCfg;
extern XAieGbl_RegStrmEvtPort TileStrmEvtPort[];

/***************************** Constant Definitions **************************/
/* Tile Stream switch master port ID offsets */
#define XAIETILE_TILESTRSW_MPORT_CORE_OFF	0U
#define XAIETILE_TILESTRSW_MPORT_DMA_OFF		2U
#define XAIETILE_TILESTRSW_MPORT_CTRL_OFF	4U
#define XAIETILE_TILESTRSW_MPORT_FIFO_OFF	5U
#define XAIETILE_TILESTRSW_MPORT_SOUTH_OFF	7U
#define XAIETILE_TILESTRSW_MPORT_WEST_OFF	11U
#define XAIETILE_TILESTRSW_MPORT_NORTH_OFF	15U
#define XAIETILE_TILESTRSW_MPORT_EAST_OFF	21U

/* Tile Stream switch slave port ID offsets */
#define XAIETILE_TILESTRSW_SPORT_CORE_OFF	0U
#define XAIETILE_TILESTRSW_SPORT_DMA_OFF		2U
#define XAIETILE_TILESTRSW_SPORT_CTRL_OFF	4U
#define XAIETILE_TILESTRSW_SPORT_FIFO_OFF	5U
#define XAIETILE_TILESTRSW_SPORT_SOUTH_OFF	7U
#define XAIETILE_TILESTRSW_SPORT_WEST_OFF	13U
#define XAIETILE_TILESTRSW_SPORT_NORTH_OFF	17U
#define XAIETILE_TILESTRSW_SPORT_EAST_OFF	21U	
#define XAIETILE_TILESTRSW_SPORT_TRACE_OFF	25U

/* Shim Stream switch master port ID offsets */
#define XAIETILE_SHIMSTRSW_MPORT_CTRL_OFF	0U
#define XAIETILE_SHIMSTRSW_MPORT_FIFO_OFF	1U
#define XAIETILE_SHIMSTRSW_MPORT_SOUTH_OFF	3U
#define XAIETILE_SHIMSTRSW_MPORT_WEST_OFF	9U
#define XAIETILE_SHIMSTRSW_MPORT_NORTH_OFF	13U
#define XAIETILE_SHIMSTRSW_MPORT_EAST_OFF	19U

/* Shim Stream switch slave port ID offsets */
#define XAIETILE_SHIMSTRSW_SPORT_CTRL_OFF	0U
#define XAIETILE_SHIMSTRSW_SPORT_FIFO_OFF	1U
#define XAIETILE_SHIMSTRSW_SPORT_SOUTH_OFF	3U
#define XAIETILE_SHIMSTRSW_SPORT_WEST_OFF	11U
#define XAIETILE_SHIMSTRSW_SPORT_NORTH_OFF	15U
#define XAIETILE_SHIMSTRSW_SPORT_EAST_OFF	19U
#define XAIETILE_SHIMSTRSW_SPORT_TRACE_OFF	23U

/* Slave port slot IDs */
#define XAIETILE_STRSW_SPORT_NUMSLOTS            4U
#define XAIETILE_STRSW_SPORT_SLOT0		0U
#define XAIETILE_STRSW_SPORT_SLOT1		1U
#define XAIETILE_STRSW_SPORT_SLOT2		2U
#define XAIETILE_STRSW_SPORT_SLOT3		3U

/* Address multipliers for master,slave and slot config registers */
#define XAIETILE_STRSW_MPORT_ADDRMUL		4U
#define XAIETILE_STRSW_SPORT_ADDRMUL		4U
#define XAIETILE_STRSW_SLVSLOT_SLVADDRMUL	16U
#define XAIETILE_STRSW_SLVSLOT_SLOTADDRMUL	4U

/* Register bit field offsets */
#define XAIETILE_STRSW_MPORT_PKTMSEL_SHIFT	3U
#define XAIETILE_STRSW_MPORT_PKTARB_SHIFT	0U

/* Mux streams */
#define XAIETILE_SHIM_STRM_MUX_SOUTH2		0U
#define XAIETILE_SHIM_STRM_MUX_SOUTH3		1U
#define XAIETILE_SHIM_STRM_MUX_SOUTH6		2U
#define XAIETILE_SHIM_STRM_MUX_SOUTH7		3U

/* Mux inputs */
#define XAIETILE_SHIM_STRM_MUX_PL		0x0U
#define XAIETILE_SHIM_STRM_MUX_DMA		0x1U
#define XAIETILE_SHIM_STRM_MUX_NOC		0x2U

/* Demux streams */
#define XAIETILE_SHIM_STRM_DEM_SOUTH2		0U
#define XAIETILE_SHIM_STRM_DEM_SOUTH3		1U
#define XAIETILE_SHIM_STRM_DEM_SOUTH4		2U
#define XAIETILE_SHIM_STRM_DEM_SOUTH5		3U

/* Demux outputs */
#define XAIETILE_SHIM_STRM_DEM_PL		0x0U
#define XAIETILE_SHIM_STRM_DEM_DMA		0x1U
#define XAIETILE_SHIM_STRM_DEM_NOC		0x2U

/***************************** Macro Definitions *****************************/
/*****************************************************************************/
/**
*
* Macro to frame the configuration word for the Master port.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Master - Master port ID value.
* @param	DropHdr - Drop header on packet.
* @param	Msk - Mask to be used on packet ID.
* @param	Arbiter - Arbiter to use when packet matches.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_CFGPKT(TileInstPtr, Master, DropHdr, Msk, Arbiter)  \
({                                                                              \
        XAieGbl_RegStrmMstr *TmpPtr;                                             \
        TmpPtr = ((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?            \
                        &TileStrmMstr[Master]:&ShimStrmMstr[Master];            \
        (XAie_SetField(DropHdr, TmpPtr->DrpHdr.Lsb, TmpPtr->DrpHdr.Mask) |      \
	((u32)Msk << XAIETILE_STRSW_MPORT_PKTMSEL_SHIFT) |                       \
	((u32)Arbiter << XAIETILE_STRSW_MPORT_PKTARB_SHIFT));                    \
})

/*****************************************************************************/
/**
*
* Macro to frame the configuration word for the slave port slot register.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Slave - Slave port ID value.
* @param	SlotIdx - Slave slot ID value, ranging from 0-3.
* @param	SlotId - Slot ID value.
* @param	SlotMask - Slot mask value.
* @param	SlotEnable - Slot enable (1-Enable,0-Disable).
* @param	SlotMsel - master select.
* @param	SlotArbiter - Arbiter to use.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SLVSLOT_CFG(TileInstPtr, Slave, SlotIdx, SlotId,          \
                                SlotMask, SlotEnable, SlotMsel, SlotArbiter) 	\
({                                                                              \
        XAieGbl_RegStrmSlot *TmpPtr;                                            \
        TmpPtr = (((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?          \
                &TileStrmSlot[XAIETILE_STRSW_SPORT_NUMSLOTS*Slave + SlotIdx]:    \
                &ShimStrmSlot[XAIETILE_STRSW_SPORT_NUMSLOTS*Slave + SlotIdx]);   \
        (XAie_SetField(SlotId, TmpPtr->Id.Lsb, TmpPtr->Id.Mask) |      	\
        XAie_SetField(SlotMask, TmpPtr->Mask.Lsb, TmpPtr->Mask.Mask) |         \
        XAie_SetField(SlotEnable, TmpPtr->En.Lsb, TmpPtr->En.Mask) |           \
        XAie_SetField(SlotMsel, TmpPtr->Msel.Lsb, TmpPtr->Msel.Mask) |         \
	XAie_SetField(SlotArbiter, TmpPtr->Arb.Lsb, TmpPtr->Arb.Mask));        \
})

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-Core.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_CORE(TileInstPtr, Idx)				\
		(XAIETILE_TILESTRSW_SPORT_CORE_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-DMA.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_DMA(TileInstPtr, Idx)				\
		(XAIETILE_TILESTRSW_SPORT_DMA_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-Ctrl.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_CTRL(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_SPORT_CTRL_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_SPORT_CTRL_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-FIFO.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_FIFO(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_SPORT_FIFO_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_SPORT_FIFO_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-South.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_SOUTH(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_SPORT_SOUTH_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_SPORT_SOUTH_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-West.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_WEST(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_SPORT_WEST_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_SPORT_WEST_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-North.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_NORTH(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_SPORT_NORTH_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_SPORT_NORTH_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-East.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_EAST(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_SPORT_EAST_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_SPORT_EAST_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch slave port-Trace.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_SPORT_TRACE(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_SPORT_TRACE_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_SPORT_TRACE_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-Core.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_CORE(TileInstPtr, Idx)				\
		(XAIETILE_TILESTRSW_MPORT_CORE_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-DMA.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_DMA(TileInstPtr, Idx)				\
		(XAIETILE_TILESTRSW_MPORT_DMA_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-Ctrl.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_CTRL(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_MPORT_CTRL_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_MPORT_CTRL_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-FIFO.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_FIFO(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_MPORT_FIFO_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_MPORT_FIFO_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-South.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_SOUTH(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_MPORT_SOUTH_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_MPORT_SOUTH_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-West.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_WEST(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_MPORT_WEST_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_MPORT_WEST_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-North.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_NORTH(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_MPORT_NORTH_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_MPORT_NORTH_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-East.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_EAST(TileInstPtr, Idx)				\
		((TileInstPtr)->TileType == XAIEGBL_TILE_TYPE_AIETILE)?		\
		(XAIETILE_TILESTRSW_MPORT_EAST_OFF + Idx):			\
		(XAIETILE_SHIMSTRSW_MPORT_EAST_OFF + Idx)

/*****************************************************************************/
/**
*
* Macro to compute the ID value of stream switch master port-Trace.
*
* @param	TileInstPtr - Tile instance pointer.
* @param	Idx - Index value.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XAIETILE_STRSW_MPORT_TRACE(TileInstPtr, Idx)				\
		(XAIETILE_TILESTRSW_MPORT_TRACE_OFF + Idx)

/************************** Function Prototypes  *****************************/
void XAieTile_StrmConnectCct(XAieGbl_Tile *TileInstPtr, u8 Slave, u8 Master, u8 SlvEnable);
void XAieTile_StrmConfigMstr(XAieGbl_Tile *TileInstPtr, u8 Master, u8 Enable, u8 PktEnable, u8 Config);
void XAieTile_StrmConfigSlv(XAieGbl_Tile *TileInstPtr, u8 Slave, u8 Enable, u8 PktEnable);
void XAieTile_StrmConfigSlvSlot(XAieGbl_Tile *TileInstPtr, u8 Slave, u8 Slot, u8 Enable, u32 RegVal);
void XAieTile_ShimStrmMuxConfig(XAieGbl_Tile *TileInstPtr, u32 Port, u32 Input);
void XAieTile_ShimStrmDemuxConfig(XAieGbl_Tile *TileInstPtr, u32 Port, u32 Output);
void XAieTile_StrmEventPortSelect(XAieGbl_Tile *TileInstPtr, u8 Port, u8 Master, u8 Id);

#endif		/* end of protection macro */
/** @} */

