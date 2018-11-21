/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* @file xmetile_strm.h
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
* </pre>
*
******************************************************************************/
#ifndef XMETILE_STRM_H
#define XMETILE_STRM_H

/***************************** Include Files *********************************/
#include "xmegbl_reginit.h"

/**************************** Variable Declarations ***************************/
extern XMeGbl_RegStrmMstr TileStrmMstr[];
extern XMeGbl_RegStrmSlv TileStrmSlv[];
extern XMeGbl_RegStrmSlot TileStrmSlot[];
extern XMeGbl_RegStrmMstr ShimStrmMstr[];
extern XMeGbl_RegStrmSlv ShimStrmSlv[];
extern XMeGbl_RegStrmSlot ShimStrmSlot[];
extern XMeGbl_RegShimMuxCfg ShimStrmMuxCfg;
extern XMeGbl_RegShimDemCfg ShimStrmDemCfg;
extern XMeGbl_RegStrmEvtPort TileStrmEvtPort[];

/***************************** Constant Definitions **************************/
/* Tile Stream switch master port ID offsets */
#define XMETILE_TILESTRSW_MPORT_CORE_OFF	0U
#define XMETILE_TILESTRSW_MPORT_DMA_OFF		2U
#define XMETILE_TILESTRSW_MPORT_CTRL_OFF	4U
#define XMETILE_TILESTRSW_MPORT_FIFO_OFF	5U
#define XMETILE_TILESTRSW_MPORT_SOUTH_OFF	7U
#define XMETILE_TILESTRSW_MPORT_WEST_OFF	11U
#define XMETILE_TILESTRSW_MPORT_NORTH_OFF	15U
#define XMETILE_TILESTRSW_MPORT_EAST_OFF	21U

/* Tile Stream switch slave port ID offsets */
#define XMETILE_TILESTRSW_SPORT_CORE_OFF	0U
#define XMETILE_TILESTRSW_SPORT_DMA_OFF		2U
#define XMETILE_TILESTRSW_SPORT_CTRL_OFF	4U
#define XMETILE_TILESTRSW_SPORT_FIFO_OFF	5U
#define XMETILE_TILESTRSW_SPORT_SOUTH_OFF	7U
#define XMETILE_TILESTRSW_SPORT_WEST_OFF	13U
#define XMETILE_TILESTRSW_SPORT_NORTH_OFF	17U
#define XMETILE_TILESTRSW_SPORT_EAST_OFF	21U	
#define XMETILE_TILESTRSW_SPORT_TRACE_OFF	25U

/* Shim Stream switch master port ID offsets */
#define XMETILE_SHIMSTRSW_MPORT_CTRL_OFF	0U
#define XMETILE_SHIMSTRSW_MPORT_FIFO_OFF	1U
#define XMETILE_SHIMSTRSW_MPORT_SOUTH_OFF	3U
#define XMETILE_SHIMSTRSW_MPORT_WEST_OFF	9U
#define XMETILE_SHIMSTRSW_MPORT_NORTH_OFF	13U
#define XMETILE_SHIMSTRSW_MPORT_EAST_OFF	19U

/* Shim Stream switch slave port ID offsets */
#define XMETILE_SHIMSTRSW_SPORT_CTRL_OFF	0U
#define XMETILE_SHIMSTRSW_SPORT_FIFO_OFF	1U
#define XMETILE_SHIMSTRSW_SPORT_SOUTH_OFF	3U
#define XMETILE_SHIMSTRSW_SPORT_WEST_OFF	11U
#define XMETILE_SHIMSTRSW_SPORT_NORTH_OFF	15U
#define XMETILE_SHIMSTRSW_SPORT_EAST_OFF	19U
#define XMETILE_SHIMSTRSW_SPORT_TRACE_OFF	23U

/* Slave port slot IDs */
#define XMETILE_STRSW_SPORT_NUMSLOTS            4U
#define XMETILE_STRSW_SPORT_SLOT0		0U
#define XMETILE_STRSW_SPORT_SLOT1		1U
#define XMETILE_STRSW_SPORT_SLOT2		2U
#define XMETILE_STRSW_SPORT_SLOT3		3U

/* Address multipliers for master,slave and slot config registers */
#define XMETILE_STRSW_MPORT_ADDRMUL		4U
#define XMETILE_STRSW_SPORT_ADDRMUL		4U
#define XMETILE_STRSW_SLVSLOT_SLVADDRMUL	16U
#define XMETILE_STRSW_SLVSLOT_SLOTADDRMUL	4U

/* Register bit field offsets */
#define XMETILE_STRSW_MPORT_PKTMSEL_SHIFT	3U
#define XMETILE_STRSW_MPORT_PKTARB_SHIFT	0U

/* Mux streams */
#define XMETILE_SHIM_STRM_MUX_SOUTH2		0U
#define XMETILE_SHIM_STRM_MUX_SOUTH3		1U
#define XMETILE_SHIM_STRM_MUX_SOUTH6		2U
#define XMETILE_SHIM_STRM_MUX_SOUTH7		3U

/* Mux inputs */
#define XMETILE_SHIM_STRM_MUX_PL		0x0U
#define XMETILE_SHIM_STRM_MUX_DMA		0x1U
#define XMETILE_SHIM_STRM_MUX_NOC		0x2U

/* Demux streams */
#define XMETILE_SHIM_STRM_DEM_SOUTH2		0U
#define XMETILE_SHIM_STRM_DEM_SOUTH3		1U
#define XMETILE_SHIM_STRM_DEM_SOUTH4		2U
#define XMETILE_SHIM_STRM_DEM_SOUTH5		3U

/* Demux outputs */
#define XMETILE_SHIM_STRM_DEM_PL		0x0U
#define XMETILE_SHIM_STRM_DEM_DMA		0x1U
#define XMETILE_SHIM_STRM_DEM_NOC		0x2U

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
#define XMETILE_STRSW_MPORT_CFGPKT(TileInstPtr, Master, DropHdr, Msk, Arbiter)  \
({                                                                              \
        XMeGbl_RegStrmMstr *TmpPtr;                                             \
        TmpPtr = (TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?            \
                        &TileStrmMstr[Master]:&ShimStrmMstr[Master];            \
        (XMe_SetField(DropHdr, TmpPtr->DrpHdr.Lsb, TmpPtr->DrpHdr.Mask) |      \
	((u32)Msk << XMETILE_STRSW_MPORT_PKTMSEL_SHIFT) |                       \
	((u32)Arbiter << XMETILE_STRSW_MPORT_PKTARB_SHIFT));                    \
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
#define XMETILE_STRSW_SLVSLOT_CFG(TileInstPtr, Slave, SlotIdx, SlotId,          \
                                SlotMask, SlotEnable, SlotMsel, SlotArbiter) 	\
({                                                                              \
        XMeGbl_RegStrmSlot *TmpPtr;                                            \
        TmpPtr = ((TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?          \
                &TileStrmSlot[XMETILE_STRSW_SPORT_NUMSLOTS*Slave + SlotIdx]:    \
                &ShimStrmSlot[XMETILE_STRSW_SPORT_NUMSLOTS*Slave + SlotIdx]);   \
        (XMe_SetField(SlotId, TmpPtr->Id.Lsb, TmpPtr->Id.Mask) |      	\
        XMe_SetField(SlotMask, TmpPtr->Mask.Lsb, TmpPtr->Mask.Mask) |         \
        XMe_SetField(SlotEnable, TmpPtr->En.Lsb, TmpPtr->En.Mask) |           \
        XMe_SetField(SlotMsel, TmpPtr->Msel.Lsb, TmpPtr->Msel.Mask) |         \
	XMe_SetField(SlotArbiter, TmpPtr->Arb.Lsb, TmpPtr->Arb.Mask));        \
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
#define XMETILE_STRSW_SPORT_CORE(TileInstPtr, Idx)				\
		(XMETILE_TILESTRSW_SPORT_CORE_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_DMA(TileInstPtr, Idx)				\
		(XMETILE_TILESTRSW_SPORT_DMA_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_CTRL(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_SPORT_CTRL_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_SPORT_CTRL_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_FIFO(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_SPORT_FIFO_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_SPORT_FIFO_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_SOUTH(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_SPORT_SOUTH_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_SPORT_SOUTH_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_WEST(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_SPORT_WEST_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_SPORT_WEST_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_NORTH(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_SPORT_NORTH_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_SPORT_NORTH_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_EAST(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_SPORT_EAST_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_SPORT_EAST_OFF + Idx)

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
#define XMETILE_STRSW_SPORT_TRACE(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_SPORT_TRACE_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_SPORT_TRACE_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_CORE(TileInstPtr, Idx)				\
		(XMETILE_TILESTRSW_MPORT_CORE_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_DMA(TileInstPtr, Idx)				\
		(XMETILE_TILESTRSW_MPORT_DMA_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_CTRL(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_MPORT_CTRL_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_MPORT_CTRL_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_FIFO(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_MPORT_FIFO_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_MPORT_FIFO_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_SOUTH(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_MPORT_SOUTH_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_MPORT_SOUTH_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_WEST(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_MPORT_WEST_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_MPORT_WEST_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_NORTH(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_MPORT_NORTH_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_MPORT_NORTH_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_EAST(TileInstPtr, Idx)				\
		(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE)?		\
		(XMETILE_TILESTRSW_MPORT_EAST_OFF + Idx):			\
		(XMETILE_SHIMSTRSW_MPORT_EAST_OFF + Idx)

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
#define XMETILE_STRSW_MPORT_TRACE(TileInstPtr, Idx)				\
		(XMETILE_TILESTRSW_MPORT_TRACE_OFF + Idx)

/************************** Function Prototypes  *****************************/
void XMeTile_StrmConnectCct(XMeGbl_Tile *TileInstPtr, u8 Slave, u8 Master, u8 SlvEnable);
void XMeTile_StrmConfigMstr(XMeGbl_Tile *TileInstPtr, u8 Master, u8 Enable, u8 PktEnable, u8 Config);
void XMeTile_StrmConfigSlv(XMeGbl_Tile *TileInstPtr, u8 Slave, u8 Enable, u8 PktEnable);
void XMeTile_StrmConfigSlvSlot(XMeGbl_Tile *TileInstPtr, u8 Slave, u8 Slot, u8 Enable, u32 RegVal);
void XMeTile_ShimStrmMuxConfig(XMeGbl_Tile *TileInstPtr, u32 Port, u32 Input);
void XMeTile_ShimStrmDemuxConfig(XMeGbl_Tile *TileInstPtr, u32 Port, u32 Output);
void XMeTile_StrmEventPortSelect(XMeGbl_Tile *TileInstPtr, u8 Port, u8 Master, u8 Id);

#endif		/* end of protection macro */
/** @} */

