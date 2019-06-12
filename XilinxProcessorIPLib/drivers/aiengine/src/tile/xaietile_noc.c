/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/******************************************************************************/
/**
 * @file xaietile_noc.c
 * @{
 *
 * This file contains routines for NoC module
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who	Date		Changes
 * ----- ------  -------- -----------------------------------------------------
 * 1.0	Nishad	01/29/2019	Initial creation
 * </pre>
 *
 ******************************************************************************/
/****************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaietile_noc.h"

/****************************** Constant Definitions **************************/
/****************************** Macro Definitions *****************************/
#define XAIE_NPI_INTR_COUNT		4U

/*************************** Variable Definitions *****************************/
/*************************** Function Definitions *****************************/
/******************************************************************************/
/**
 *
 * This API returns the status of 2nd level interrupt. Each bit in the status
 * value corresponds to a particular type of interrupt from a specific 1st level
 * section
 *
 * @param	TileInstPtr - Pointer to the Tile instance. Should be shim NoC
 *				tile
 *
 * @return	Current 16-bit status value
 *
 * @note	None
 *
 ******************************************************************************/
u16 XAieTile_NoCIntcL2StatusGet(XAieGbl_Tile *TileInstPtr)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
						XAIEGBL_NOC_INTCON2NDLEVSTA);

	return XAie_GetField(RegVal, XAIEGBL_NOC_INTCON2NDLEVSTA_STAA_LSB,
					XAIEGBL_NOC_INTCON2NDLEVSTA_STAA_MASK);
}

/******************************************************************************/
/**
 *
 * This API clears the status of interrupt at the 2nd level interrupt controller
 * level. As each bit in the status corresponds to the type of interrupt from a
 * particular 1st level section, supply a set bit in the Mask value at the same
 * location to clear the status
 *
 * @param	TileInstPtr - Pointer to the Tile instance. Should be shim NoC
 *				tile
 * @param	Mask - Set bit corresponding to the status to be cleared
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XAieTile_NoCIntcL2StatusClear(XAieGbl_Tile *TileInstPtr, u16 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_NOC_INTCON2NDLEVSTA,
									Mask);
}

/******************************************************************************/
/**
 *
 * This API returns the current mask value for 2nd level interrupt controller.
 * Masking/unmasking of interrupts from 1st level sections can be done using
 * XAieTile_NoCIntcL2Enable() and XAieTile_NoCIntcL2Disable() APIs
 *
 * @param	TileInstPtr - Pointer to the Tile instance. Should be shim NoC
 *				tile
 *
 * @return	Current 16-bit mask value
 *
 * @note	None
 *
 ******************************************************************************/
u16 XAieTile_NoCIntcL2Mask(XAieGbl_Tile *TileInstPtr)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
						XAIEGBL_NOC_INTCON2NDLEVMSK);

	return XAie_GetField(RegVal, XAIEGBL_NOC_INTCON2NDLEVMSK_IRQMSKA_LSB,
				XAIEGBL_NOC_INTCON2NDLEVMSK_IRQMSKA_MASK);
}

/******************************************************************************/
/**
 *
 * This API configures the 2nd level interrupt controller to be triggered by a
 * groups of interrupts defined at the 1st level section
 *
 * @param	TileInstPtr - Pointer to the Tile instance. Should be shim NoC
 *				tile
 * @param	InterruptBitMask - Group of events to trigger 2nd level
 *					controller
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XAieTile_NoCIntcL2Enable(XAieGbl_Tile *TileInstPtr, u16 InterruptBitMask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_NOC_INTCON2NDLEVENA,
							InterruptBitMask);
}

/******************************************************************************/
/**
 *
 * This API configures the 2nd level interrupt controller to ignore groups
 * of interrupts defined at the 1st level section
 *
 * @param	TileInstPtr - Pointer to the Tile instance. Should be shim NoC
 *				tile
 * @param	InterruptBitMask - Group of events to ignore at 2nd level
 *					controller
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XAieTile_NoCIntcL2Disable(XAieGbl_Tile *TileInstPtr, u16 InterruptBitMask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_NOC_INTCON2NDLEVDIS,
							InterruptBitMask);
}

/******************************************************************************/
/**
 *
 * This API returns the NoC interrupt line number to which the 2nd level
 * interrupt controller is driving the 1st level interrupts to
 *
 * @param	TileInstPtr - Pointer to the Tile instance. Should be shim NoC
 *				tile.
 *
 * @return	NoC interrupt line number
 *
 * @note	None
 *
 ******************************************************************************/
u8 XAieTile_NoCIntcL2IntrGet(XAieGbl_Tile *TileInstPtr)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
						XAIEGBL_NOC_INTCON2NDLEVINT);

	return XAie_GetField(RegVal, XAIEGBL_NOC_INTCON2NDLEVINT_NOCINT_LSB,
				XAIEGBL_NOC_INTCON2NDLEVINT_NOCINT_MASK);
}

/******************************************************************************/
/**
 *
 * This API configures the 2nd level interrupt controller to drive one of the
 * the four NoC interrupt line when an active event occurs
 *
 * @param	TileInstPtr - Pointer to the Tile instance. Should be shim NoC
 *				tile
 * @param	NoCInterrupt - NoC interrupt line number
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XAieTile_NoCIntcL2IntrSet(XAieGbl_Tile *TileInstPtr, u8 NoCInterrupt)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);
	XAie_AssertNonvoid(NoCInterrupt < XAIE_NPI_INTR_COUNT);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_NOC_INTCON2NDLEVINT,
								NoCInterrupt);
}
