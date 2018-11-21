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
* @file xmetile_strm.c
* @{
*
* This file contains routines for the Stream switch master and slave ports.
* These are applicable for both the ME and Shim tiles.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Hyun    10/03/2018  Added the event port select function
* 1.3  Hyun    10/10/2018  Use the mask write API
* </pre>
*
******************************************************************************/
#include "xmegbl_defs.h"
#include "xmegbl.h"
#include "xmetile_strm.h"

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XMETILE_STRM_MODULE_CORE		0x0
#define XMETILE_STRM_MODULE_PL			0x1

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to configure the selected master port of the stream switch
* in the corresponding tile as per the parameters.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Master - Master port ID value.
* @param	Enable - Enable/Disable the master port
*		(1-Enable,0-Disable).

* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).

* @param	Config - Config value to be used for circuit/packet sw.
*		Applicable only when Enable==1.
*		Bit encoding when PktEnable==1: 7-Drop header on packet,
*		6:3-Mask, 2:0-Arbiter
*		Bit encoding when PktEnable==0: 7:5-Rsvd, 4:0-Slave port ID to
*		which the master port need to connect to
*		Use the macro "xmetile_strm.c::XMETILE_STRSW_MPORT_CFGPKT()" to
*		frame the 8-bit Config.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_StrmConfigMstr(XMeGbl_Tile *TileInstPtr, u8 Master, u8 Enable,
						u8 PktEnable, u8 Config)
{
	u64 RegAddr;
	u32 RegVal = 0U;
        u8 DropHdr = 0U;
        XMeGbl_RegStrmMstr *RegPtr;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

       	if(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE) {
                RegPtr = &TileStrmMstr[Master];
        } else {
                RegPtr = &ShimStrmMstr[Master];
        }

	/* Get the address of Master port config reg */
	RegAddr = TileInstPtr->TileAddr + RegPtr->RegOff;

	if(Enable == XME_ENABLE) {
                /* Extract the drop header field */
                DropHdr = XMe_GetField(Config, RegPtr->DrpHdr.Lsb,
                                                RegPtr->DrpHdr.Mask);
		/* Frame the 32-bit reg value */
		RegVal = XMe_SetField(Enable, RegPtr->MstrEn.Lsb,
					RegPtr->MstrEn.Mask) |
			XMe_SetField(PktEnable, RegPtr->PktEn.Lsb,
					RegPtr->PktEn.Mask) |
                        XMe_SetField(DropHdr, RegPtr->DrpHdr.Lsb,
					RegPtr->DrpHdr.Mask) |
			XMe_SetField(Config, RegPtr->Config.Lsb,
					RegPtr->Config.Mask);
	}
	XMeGbl_Write32(RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to configure the selected slave port of the stream switch
* in the corresponding tile.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Slave - Slave port ID value.
* @param	Enable - Enable/Disable the slave port (1-Enable,0-Disable).
* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_StrmConfigSlv(XMeGbl_Tile *TileInstPtr, u8 Slave, u8 Enable,
								u8 PktEnable)
{
	u64 RegAddr;
	u32 RegVal = 0U;
        XMeGbl_RegStrmSlv *RegPtr;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

       	if(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE) {
                RegPtr = &TileStrmSlv[Slave];
        } else {
                RegPtr = &ShimStrmSlv[Slave];
        }

	/* Get the address of Slave port config reg */
	RegAddr = TileInstPtr->TileAddr + RegPtr->RegOff;

	if(Enable == XME_ENABLE) {
		/* Frame the 32-bit reg value */
		RegVal = XMe_SetField(Enable, RegPtr->SlvEn.Lsb,
						RegPtr->SlvEn.Mask) |
			XMe_SetField(PktEnable, RegPtr->PktEn.Lsb,
						RegPtr->PktEn.Mask);
	}
	XMeGbl_Write32(RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to configure the selected slot of the slave port in the
* stream switch of the corresponding tile.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Slave - Slave port ID value.
* @param	Slot - Slave slot ID value, ranging from 0-3.
* @param	Enable - Enable/Disable the slave slot (1-Enable,0-Disable).

* @param	RegVal - Config value to be used for the slot. Applicable only
*		when Enable==1, else set to 0.
*		Bit encoding : 31:21-Rsvd, 28:24-Slot ID, 23:21-Rsvd,
*			20:16-ID mask, 15:6-Rsvd, 5:4-Master select/msel,
*			3-Rsvd, 2:0-Arbiter to use.
*		Use the macro "xmetile_strm.c::XMETILE_STRSW_SLVSLOT_CFG()" to
*		frame the 32-bit RegVal.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_StrmConfigSlvSlot(XMeGbl_Tile *TileInstPtr, u8 Slave, u8 Slot,
							u8 Enable, u32 RegVal)
{
	u64 RegAddr;
        XMeGbl_RegStrmSlot *RegPtr;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

       	if(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE) {
                RegPtr = &(TileStrmSlot[(XMETILE_STRSW_SPORT_NUMSLOTS * Slave) +
                                                                 Slot]);
        } else {
                RegPtr = &(ShimStrmSlot[(XMETILE_STRSW_SPORT_NUMSLOTS * Slave) +
                                                                 Slot]);
        }

	/* Get the address of Slave slot config reg */
	RegAddr = TileInstPtr->TileAddr + RegPtr->RegOff;

	if(Enable == XME_ENABLE) {
		RegVal|=(XMe_SetField(Enable, RegPtr->En.Lsb, RegPtr->En.Mask));
	} else {
		RegVal = 0U;
	}

	XMeGbl_Write32(RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to connect the selected master port to the specified slave
* port of the stream switch.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Slave - slave port ID value.
* @param	Master - Master port ID value.
* @param	SlvEnable - Enable/Disable the slave port (1-Enable,0-Disable).
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_StrmConnectCct(XMeGbl_Tile *TileInstPtr, u8 Slave, u8 Master,
								u8 SlvEnable)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

	/*
	 * Enable the master port in circuit switched mode and specify the
	 * slave port it is connected to
	 */	
	XMeTile_StrmConfigMstr(TileInstPtr, Master, XME_ENABLE,
							XME_DISABLE, Slave);

	if(SlvEnable == XME_ENABLE) {
		/* Enable the slave port in circuit switched mode */
		XMeTile_StrmConfigSlv(TileInstPtr, Slave, SlvEnable,
								XME_DISABLE);
	}
}

/*****************************************************************************/
/**
*
* This API sets up the mux configuraiton for Shim.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Port: Should be one of XMETILE_SHIM_STRM_MUX_SOUTH2,
* XMETILE_SHIM_STRM_MUX_SOUTH3, XMETILE_SHIM_STRM_MUX_SOUTH6,
* or XMETILE_SHIM_STRM_MUX_SOUTH7
* @param	Input: Should be one of XMETILE_SHIM_STRM_MUX_PL,
* XMETILE_SHIM_STRM_MUX_DMA, or XMETILE_SHIM_STRM_MUX_NOC.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_ShimStrmMuxConfig(XMeGbl_Tile *TileInstPtr, u32 Port, u32 Input)
{
	u32 FldVal;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC);

	FldVal = Input << ShimStrmMuxCfg.Port[Port].Lsb;
	XMeGbl_MaskWrite32(TileInstPtr->TileAddr + ShimStrmMuxCfg.CtrlOff,
			ShimStrmMuxCfg.Port[Port].Mask, FldVal);
}

/*****************************************************************************/
/**
*
* This API sets up the mux configuraiton for Shim DMA.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Port: Should be one of XMETILE_SHIM_STRM_DEM_SOUTH2,
* XMETILE_SHIM_STRM_DEM_SOUTH3, XMETILE_SHIM_STRM_DEM_SOUTH4,
* or XMETILE_SHIM_STRM_DEM_SOUTH5
* @param	Output: Should be one of XMETILE_SHIM_STRM_DEM_PL,
* XMETILE_SHIM_STRM_DEM_DMA, or XMETILE_SHIM_STRM_DEM_NOC.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_ShimStrmDemuxConfig(XMeGbl_Tile *TileInstPtr, u32 Port, u32 Output)
{
	u32 FldVal;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC);

	FldVal = Output << ShimStrmDemCfg.Port[Port].Lsb;
	XMeGbl_MaskWrite32(TileInstPtr->TileAddr + ShimStrmDemCfg.CtrlOff,
			ShimStrmDemCfg.Port[Port].Mask, FldVal);
}

/*****************************************************************************/
/**
*
* This API sets up the event port in stream switch
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Port: Port number. 0 to 7.
* @param	Master: 1 for master. 0 for slave.
* @param	Id: Port ID for event generation
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_StrmEventPortSelect(XMeGbl_Tile *TileInstPtr, u8 Port, u8 Master,
		u8 Id)
{
	u32 FldVal;
	u32 FldMask;
	u8 ModId;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(Port < 8);
	XMe_AssertNonvoid(Master < 2);

	if (TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE) {
		ModId = XMETILE_STRM_MODULE_CORE;
	} else {
		ModId = XMETILE_STRM_MODULE_PL;
	}

	FldMask = TileStrmEvtPort[ModId].Port[Port].Mask;
	FldVal = Id << TileStrmEvtPort[ModId].Port[Port].Lsb;
	FldMask |= TileStrmEvtPort[ModId].MstrSlv[Port].Mask;
	FldVal |= Master << TileStrmEvtPort[ModId].MstrSlv[Port].Lsb;
	XMeGbl_MaskWrite32(TileInstPtr->TileAddr + TileStrmEvtPort[ModId].RegOff[Port / 4],
			FldMask, FldVal);
}

/** @} */

