/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XFLASH_CFI_H
#define XFLASH_CFI_H

/*****************************************************************************/
/**
*
* @file xilflash_cfi.h
* @brief Declares the methods that extract and interpret CFI data.
*
* This file contains methods used to extract and interpret Common Flash
* Interface (CFI) from a flash memory part that supports the CFI query command.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  10/25/07 First release
* 1.00a mta  10/25/07 Updated to flash library
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros
* 4.14  vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xilflash.h"
#include "xilflash_properties.h"
#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
#include "xilflash_intel.h"
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */
#ifdef XPAR_XFL_DEVICE_FAMILY_AMD
#include "xilflash_amd.h"
#endif /* XPAR_XFL_DEVICE_FAMILY_AMD */
/************************** Constant Definitions *****************************/

/**
 * @name Layout constants
 *
 * LAYOUT constants are used to parse the XFL_LAYOUT_X* constants.
 * @{
 */
#define XFL_LAYOUT_NUM_PARTS_MASK	0x000000FF /**< Number of parts mask. */
#define XFL_LAYOUT_PART_MODE_MASK	0x0000FF00 /**< Part mode mask. */
#define XFL_LAYOUT_PART_WIDTH_MASK	0x00FF0000 /**< Part width mask. */
#define XFL_LAYOUT_CFI_INTERL_MASK	0xFF000000 /**< CFI interleave mask. */

#define XFL_LAYOUT_NUM_PARTS_1		0x00000001 /**< One part. */
#define XFL_LAYOUT_NUM_PARTS_2		0x00000002 /**< Two parts. */
#define XFL_LAYOUT_NUM_PARTS_4		0x00000004 /**< Four parts. */
#define XFL_LAYOUT_PART_MODE_8		0x00000100 /**< 8-bit part mode. */
/** 16-bit part mode. */
#define XFL_LAYOUT_PART_MODE_16		0x00000200
/** 8-bit part width. */
#define XFL_LAYOUT_PART_WIDTH_8		0x00010000
#define XFL_LAYOUT_PART_WIDTH_16	0x00020000 /**< 16-bit part width. */
/** CFI interleave of 1. */
#define XFL_LAYOUT_CFI_INTERL_1		0x01000000
/** CFI interleave of 2. */
#define XFL_LAYOUT_CFI_INTERL_2		0x02000000
/** CFI interleave of 4. */
#define XFL_LAYOUT_CFI_INTERL_4		0x04000000
/** CFI interleave of 8. */
#define XFL_LAYOUT_CFI_INTERL_8		0x08000000
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
*
* @brief Moves the CFI data pointer to the address of a CFI byte offset.
*
* Moves the CFI data pointer to a physical address that corresponds to a
* specific CFI byte offset.
*
* @param[out] Ptr Pointer to modify. Can be of any type.
* @param[in]  BaseAddr Base address of flash part.
* @param[in]  Interleave Byte interleaving (based on part layout).
* @param[in]  ByteAddr Byte offset within CFI data to read.
*
* @return     The Ptr argument is set to point at the CFI byte specified by the
*             ByteAddr parameter.
*
* @note       C-style signature:
*             void *XFL_CFI_POSITION_PTR(void *Ptr, u32 BaseAddr,
*                                        u8 Interleave, u32 ByteAddr)
*
*****************************************************************************/
#define XFL_CFI_POSITION_PTR(Ptr, BaseAddr, Interleave, ByteAddr) 	\
	(								  	\
			Ptr = (void*)((u32)BaseAddr + ((u32)Interleave *	  	\
					(u32)ByteAddr))	  	\
	)

/*****************************************************************************/
/**
*
* @brief Advances the CFI pointer to the next byte.
*
* @param[in,out] Ptr Pointer to advance. Can be a pointer to any type.
* @param[in]     Interleave Byte interleaving (based on part layout).
*
* @return        Adjusted Ptr.
*
* @note          C-style signature:
*                void *XFL_CFI_ADVANCE_PTR8(void *Ptr, u8 Interleave)
*
*****************************************************************************/
#define XFL_CFI_ADVANCE_PTR8(Ptr, Interleave) \
	(Ptr = (void*)((u32)Ptr + (Interleave)))

/*****************************************************************************/
/**
*
* @brief Advances the CFI pointer to the next 16-bit quantity.
*
* @param[in,out] Ptr Pointer to advance. Can be a pointer to any type.
* @param[in]     Interleave Byte interleaving (based on part layout).
*
* @return        Adjusted Ptr.
*
* @note          C-style signature:
*                void *XFL_CFI_ADVANCE_PTR16(void *Ptr, u8 Interleave)
*
*****************************************************************************/
#define XFL_CFI_ADVANCE_PTR16(Ptr, Interleave) \
	(Ptr = (void*)((u32)Ptr + ((Interleave) << 1)))


/************************** Function Prototypes ******************************/

int XFlashCFI_ReadCommon(XFlash *InstancePtr, u8 BusWidth);
int XFlashCFI_Read8(u8 *Ptr, u8 Interleave, u8 Mode);
int XFlashCFI_Read16(u8 *Ptr, u8 Interleave, u8 Mode);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
