/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.1  Jubaer  03/07/2019  Add Shim Reset Enable
* 1.2  Hyun    06/27/2019  Add XAieTile_PlReadTimer()
* 1.3  Wendy   02/25/2020  Add get/set 1st level irq event
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

extern XAieGbl_RegShimReset ShimReset;
extern XAieGbl_RegTimer TimerReg[];
extern XAieGbl_1stIrqCntr Shim_1stIrqCntr;

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
	u64 RegAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.MaskOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	RegVal = XAieGbl_Read32(RegAddr);

	return XAie_GetField(RegVal, Shim_1stIrqCntr.IrqsMask.Lsb,
			     Shim_1stIrqCntr.IrqsMask.Mask);
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
	u64 RegAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.EnableOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	XAieGbl_MaskWrite32(RegAddr, Shim_1stIrqCntr.IrqsMask.Mask, Mask);
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
	u64 RegAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.DisableOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	XAieGbl_MaskWrite32(RegAddr, Shim_1stIrqCntr.IrqsMask.Mask, Mask);
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
	u64 RegAddr;
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.StatusOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	RegVal = XAieGbl_Read32(RegAddr);
	return XAie_GetField(RegVal, Shim_1stIrqCntr.IrqsMask.Lsb,
			     Shim_1stIrqCntr.IrqsMask.Mask);
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
	u64 RegAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.StatusOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	XAieGbl_MaskWrite32(RegAddr, Shim_1stIrqCntr.IrqsMask.Mask, Status);
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
	u64 RegAddr;
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.IrqNoOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	RegVal = XAieGbl_Read32(RegAddr);
	return XAie_GetField(RegVal, Shim_1stIrqCntr.IrqNoFld.Lsb,
			     Shim_1stIrqCntr.IrqNoFld.Mask);
}

/*****************************************************************************/
/**
*
* This API is used to set which broadcast event signal the interrupt shall be
* driven to.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	irqNum - Irq number to set
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
	u64 RegAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);
	XAie_AssertNonvoid(irqNum <= 15);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.IrqNoOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	XAieGbl_MaskWrite32(RegAddr, Shim_1stIrqCntr.IrqNoFld.Mask, irqNum);
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
	u64 RegAddr;
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.BlockNorthValueOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	RegVal = XAieGbl_Read32(RegAddr);
	return XAie_GetField(RegVal,
			     Shim_1stIrqCntr.BcEvents.Lsb,
			     Shim_1stIrqCntr.BcEvents.Mask);
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
	u64 RegAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.BlockNorthSetOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	XAieGbl_MaskWrite32(RegAddr, Shim_1stIrqCntr.BcEvents.Mask, Mask);
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
	u64 RegAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.BlockNorthClearOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	XAieGbl_MaskWrite32(RegAddr, Shim_1stIrqCntr.BcEvents.Mask, Mask);
}

/*****************************************************************************/
/**
*
* This gets 1st level interrupt event
*
* @param	TileInstPtr - Pointer to the Shim tile instance.
* @param	IrqEvent - Irq event (16, 17, 18, 19)
* @param	SwitchAB - Flag to indicate if it's the A or B block.
*
* @return	Event set to generate the specified Irq event.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlIntcL1IrqEventGet(XAieGbl_Tile *TileInstPtr, u8 IrqEvent,
				 u8 SwitchAB)
{
	u64 RegAddr;
	u32 RegVal;
	u8 IrqEventOff;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);
	XAie_AssertNonvoid(IrqEvent >= XAIETILE_PL_INTERN_EVENT16 &&
			   IrqEvent <= XAIETILE_PL_INTERN_EVENT19);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.IrqEventOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	RegVal = XAieGbl_Read32(RegAddr);
	IrqEventOff = IrqEvent - XAIETILE_PL_INTERN_EVENT16;
	return XAie_GetField(RegVal,
			     Shim_1stIrqCntr.IrqEventRegFld[IrqEventOff].Lsb,
			     Shim_1stIrqCntr.IrqEventRegFld[IrqEventOff].Mask);
}

/*****************************************************************************/
/**
*
* This sets 1st level interrupt event
*
* @param	TileInstPtr - Pointer to the Shim tile instance.
* @param	SwitchAB - Flag to indicate if it's the A or B block.
* @param	IrqEvent - Irq event (16, 17, 18, 19)
* @param	Event - Event to generate the specified Irq event
*
* @return	Success.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlIntcL1IrqEventSet(XAieGbl_Tile *TileInstPtr, u8 IrqEvent,
				  u8 Event, u8 SwitchAB)
{
	u64 RegAddr;
	u8 IrqEventOff;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SwitchAB == XAIETILE_PL_BLOCK_SWITCHA ||
			   SwitchAB == XAIETILE_PL_BLOCK_SWITCHB);
	XAie_AssertNonvoid(IrqEvent >= XAIETILE_PL_INTERN_EVENT16 &&
			   IrqEvent <= XAIETILE_PL_INTERN_EVENT19);

	RegAddr = TileInstPtr->TileAddr + Shim_1stIrqCntr.IrqEventOff;
	RegAddr += SwitchAB * Shim_1stIrqCntr.SwitchOff;
	IrqEventOff = IrqEvent - XAIETILE_PL_INTERN_EVENT16;
	XAieGbl_MaskWrite32(RegAddr,
			    Shim_1stIrqCntr.IrqEventRegFld[IrqEventOff].Mask,
			    Event);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API enable / disable the shim reset to given time
*
* @param	TileInstPtr - Pointer to the Shim tile instance.
* @param	Reset - 1 for enable. 0 for disable.
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_PlShimResetEnable(XAieGbl_Tile *TileInstPtr, u8 Reset)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Reset <= 1);

	XAieGbl_Write32(TileInstPtr->TileAddr + ShimReset.RegOff, Reset &
			XAIEGBL_PL_AIESHIRSTENA_RST_MASK);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API returns the current value of the PL module 64-bit timer.
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	64-bit timer value.
*
* @note		None.
*
*******************************************************************************/
u64 XAieTile_PlReadTimer(XAieGbl_Tile *TileInstPtr)
{
	u32 CurValHigh;
	u32 CurValLow;
	u64 CurVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	CurValLow = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_PL].LowOff);
	CurValHigh = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_PL].HighOff);
	CurVal = ((u64)CurValHigh << 0x20U) | CurValLow;

	return CurVal;
}

/** @} */
