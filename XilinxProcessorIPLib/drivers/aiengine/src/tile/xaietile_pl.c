/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* @file xaietile_pl.c
* @{
*
* This file contains APIs for PL module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who       Date        Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Jubaer  01/29/2019  Initial creation
* </pre>
*
******************************************************************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_params.h"
#include "xaiegbl_reginit.h"
#include "xaietile_pl.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to read the current value of the 1st level interrupt
* controller mask.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SwitchAB - Flag to indicate if it's the A or B block. Should
* 		be one of XAIETILE_EVENT_BLOCK_SWITCHA
*		or XAIETILE_EVENT_BLOCK_SWITCHB.
*
* @return	Current value of the mask
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlIntcL1Mask(XAieGbl_Tile *TileInstPtr, u8 SwitchAB)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVMSKA);
	} else {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVMSKB);
	}

	return XAie_GetField(RegVal, XAIEGBL_PL_INTCON1STLEVMSKA_IRQMSKA_LSB,
			     XAIEGBL_PL_INTCON1STLEVMSKAMSK);
}

/*****************************************************************************/
/**
*
* This API is used to enable individual interrupts by setting bits in the mask
* value.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Mask - Mask with bits to set
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntcL1Enable(XAieGbl_Tile *TileInstPtr, u32 Mask, u8 SwitchAB)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVENAA, Mask);
	} else {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVENAB, Mask);
	}
}

/*****************************************************************************/
/**
*
* This API is used to disable individual interrupts by clearing bits in the
* mask value.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Mask - Mask with bits to clear
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntcL1Disable(XAieGbl_Tile *TileInstPtr, u32 Mask,
			      u8 SwitchAB)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVDISA, Mask);
	} else {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVDISB, Mask);
	}
}

/*****************************************************************************/
/**
*
* This API is used to read the 1st level status register of interrupt
* controller.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	Status - unsigned 32 bits indicating the Status of the
*		1st level interrupt controller.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlIntcL1StatusGet(XAieGbl_Tile *TileInstPtr, u8 SwitchAB)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVSTAA);
	} else {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVSTAB);
	}

	return XAie_GetField(RegVal, XAIEGBL_PL_INTCON1STLEVMSKA_IRQMSKA_LSB,
			     XAIEGBL_PL_INTCON1STLEVSTAAMSK);
}

/*****************************************************************************/
/**
*
* This API is used to clear status register of 1st level interrupt.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Status - 32-bit register to clear status register.
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntcL1StatusClr(XAieGbl_Tile *TileInstPtr, u32 Status,
				u8 SwitchAB)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVSTAA, Status);
	} else {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVSTAB, Status);
	}
}

/*****************************************************************************/
/**
*
* This API is used to read which broadcast event signal the interrupt shall
* be driven to.
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	1st level broadcast number (4 bits)
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlIntcL1IrqNoGet(XAieGbl_Tile *TileInstPtr, u8 SwitchAB)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVIRQNOA);
	} else {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVIRQNOB);
	}

	return XAie_GetField(RegVal, XAIEGBL_PL_INTCON1STLEVIRQNOA_STAA_LSB,
			     XAIEGBL_PL_INTCON1STLEVIRQNOAMSK);
}

/*****************************************************************************/
/**
*
* This API is used to set which broadcast event signal the interrupt shall be
* driven to.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	irqNum - Mask with bits to set
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntcL1IrqNoSet(XAieGbl_Tile *TileInstPtr, u32 irqNum,
			       u8 SwitchAB)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVIRQNOA, irqNum);
	} else {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVIRQNOB, irqNum);
	}
}

/*****************************************************************************/
/**
*
* This API is used to read the interrupt controller 1st level north block value
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	Return current value of block.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlIntcL1BlockNorthVal(XAieGbl_Tile *TileInstPtr, u8 SwitchAB)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVBLKNORINAVAL);
	} else {
		RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
					XAIEGBL_PL_INTCON1STLEVBLKNORINBVAL);
	}

	return XAie_GetField(RegVal,
			     XAIEGBL_PL_INTCON1STLEVBLKNORINAVAL_VAL_LSB,
			     XAIEGBL_PL_INTCON1STLEVBLKNORINAVALMSK);
}

/*****************************************************************************/
/**
*
* This API is used to set the north block of 1st level interrupt controller.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Mask - Mask with bits to set.
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntcL1BlockNorthSet(XAieGbl_Tile *TileInstPtr, u32 Mask,
				  u8 SwitchAB)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVBLKNORINASET, Mask);
	} else {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVBLKNORINBSET, Mask);
	}
}

/*****************************************************************************/
/**
*
* This API is used to clear the north block of 1st level interrupt controller.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Mask - Mask with bits to clear.
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntcL1BlockNorthClr(XAieGbl_Tile *TileInstPtr, u32 Mask,
				  u8 SwitchAB)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVBLKNORINACLR, Mask);
	} else {
		XAieGbl_Write32(TileInstPtr->TileAddr +
				XAIEGBL_PL_INTCON1STLEVBLKNORINBCLR, Mask);
	}
}

/** @} */
