/******************************************************************************
* Copyright (c) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilflash_cfi.h
*
* This file  contains methods used to extract and interpret Common Flash
* Interface(CFI) from a flash memory part that supports the CFI query command.
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
* </pre>
*
***************************************************************************/

#ifndef XFLASH_CFI_H		/* prevent circular inclusions */
#define XFLASH_CFI_H		/* by using protection macros */

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
 * LAYOUT constants are used to parse the XFL_LAYOUT_X* constants.
 */
#define XFL_LAYOUT_NUM_PARTS_MASK	0x000000FF
#define XFL_LAYOUT_PART_MODE_MASK	0x0000FF00
#define XFL_LAYOUT_PART_WIDTH_MASK	0x00FF0000
#define XFL_LAYOUT_CFI_INTERL_MASK	0xFF000000

#define XFL_LAYOUT_NUM_PARTS_1		0x00000001
#define XFL_LAYOUT_NUM_PARTS_2		0x00000002
#define XFL_LAYOUT_NUM_PARTS_4		0x00000004
#define XFL_LAYOUT_PART_MODE_8		0x00000100
#define XFL_LAYOUT_PART_MODE_16		0x00000200
#define XFL_LAYOUT_PART_WIDTH_8		0x00010000
#define XFL_LAYOUT_PART_WIDTH_16	0x00020000
#define XFL_LAYOUT_CFI_INTERL_1		0x01000000
#define XFL_LAYOUT_CFI_INTERL_2		0x02000000
#define XFL_LAYOUT_CFI_INTERL_4		0x04000000
#define XFL_LAYOUT_CFI_INTERL_8		0x08000000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
*
* Moves the CFI data pointer to a physical address that corresponds to a
* specific CFI byte offset.
*
* @param	Ptr is the pointer to modify. Can be of any type.
* @param	BaseAddr is the base address of flash part.
* @param	Interleave is the byte interleaving (based on part layout).
* @param	ByteAddr is the byte offset within CFI data to read.
*
* @return	The Ptr argument is set to point at the CFI byte specified
*		by the ByteAddr parameter.
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
* Advances the CFI pointer to the next byte.
*
* @param	Ptr is the pointer to advance. Can be a pointer to any type.
* @param	Interleave is the byte interleaving (based on part layout).
*
* @return	Adjusted Ptr.
*
*****************************************************************************/
#define XFL_CFI_ADVANCE_PTR8(Ptr, Interleave) \
	(Ptr = (void*)((u32)Ptr + (Interleave)))

/*****************************************************************************/
/**
*
* Advances the CFI pointer to the next 16-bit quantity.
*
* @param	Ptr is the pointer to advance. Can be a pointer to any type.
* @param	Interleave is the byte interleaving (based on part layout).
*
* @return	Adjusted Ptr.
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
