/******************************************************************************
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xio.h
* @addtogroup cpu Overview
* @{
* @details
*
* This file contains the interface for the XIo component, which encapsulates
* the Input/Output functions for processors that do not require any special
* I/O handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rpm  11/07/03 Added InSwap/OutSwap routines for endian conversion
* 1.00a xd   11/04/04 Improved support for doxygen
* 1.01a ecm  02/24/06 CR225908 corrected the extra curly braces in macros
*                     and bumped version to 1.01.a.
* 1.11a mta  03/21/07 Updated to new coding style.
* 1.11b va   04/17/08 Updated Tcl for better CORE_CLOCK_FREQ_HZ definition
* 1.11a sdm  03/12/09 Updated Tcl to define correct value for CORE_CLOCK_FREQ_HZ
*                     (CR  #502010)
* 1.13a sdm  03/12/09 Updated the Tcl to pull appropriate libraries for Little
*                     Endian Microblaze
* 2.0   adk  19/12/13 Updated as per the New Tcl API's
* 2.1   bss  04/14/14 Updated tcl to copy libgloss.a and libgcc.a libraries
* 2.1   bss  04/29/14 Updated to copy libgloss.a if exists otherwise libxil.a
*			CR#794205
* 2.2   bss  08/04/14 Updated driver tcl to add protection macros for
*		      xparameters.h (CR#802257).
* 2.3	sk	 12/15/14 Updated mdd file to delete ï¿½ffunction-sections &
*					  -fdata-sections flags from extra compiler flags CR#838648
*					  Changed default os to latest version in mdd file.
* 2.4   nsk  11/05/15 Updated generate and post_generate procs in driver tcl
*                     not to generate cpu macros, when microblaze is connected
*                     as one of the streaming slaves to itself. CR#876604
* 2.11  mus  02/26/20 Updated as per 2020.1 Vitis toolchain directory structure
* 2.13  mus  05/23/21 Added -fno-tree-loop-distribute-patterns to prevent for loops
*                     to memset conversions. It fixes CR#1090083.
* 2.14  mus  08/23/21 Removed fsl.h, it is duplicate of standalone/src/microblaze/fsl.h.
*                     It fixes CR#1108558.
* 2.17  adk  30/09/22 Updated tcl to copy proper libraries when fpu is enabled
*		      (C_USE_FPU > 0) and when processor is configured with
*		      MUL64 support(C_USE_HW_MUL = 2).
*
* </pre>
*
* @note
*
* This file may contain architecture-dependent items (memory-mapped or
* non-memory-mapped I/O).
*
******************************************************************************/

#ifndef XIO_H			/* prevent circular inclusions */
#define XIO_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
 * Typedef for an I/O address.  Typically correlates to the width of the
 * address bus.
 */
typedef u32 XIo_Address;

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * The following macros allow optimized I/O operations for memory mapped I/O.
 * It should be noted that macros cannot be used if synchronization of the I/O
 * operation is needed as it will likely break some code.
 */

/*****************************************************************************/
/**
*
* Performs an input operation for an 8-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	InputPtr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
#define XIo_In8(InputPtr)  (*(volatile u8  *)(InputPtr))

/*****************************************************************************/
/**
*
* Performs an input operation for a 16-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	InputPtr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
#define XIo_In16(InputPtr) (*(volatile u16 *)(InputPtr))

/*****************************************************************************/
/**
*
* Performs an input operation for a 32-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	InputPtr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
#define XIo_In32(InputPtr)  (*(volatile u32 *)(InputPtr))


/*****************************************************************************/
/**
*
* Performs an output operation for an 8-bit memory location by writing the
* specified value to the the specified address.
*
* @param	OutputPtr contains the address to perform the output operation
*		at.
* @param	Value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
#define XIo_Out8(OutputPtr, Value)  \
	(*(volatile u8  *)((OutputPtr)) = (Value))

/*****************************************************************************/
/**
*
* Performs an output operation for a 16-bit memory location by writing the
* specified value to the the specified address.
*
* @param	OutputPtr contains the address to perform the output operation
*		at.
* @param	Value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
#define XIo_Out16(OutputPtr, Value) \
	(*(volatile u16 *)((OutputPtr)) = (Value))

/*****************************************************************************/
/**
*
* Performs an output operation for a 32-bit memory location by writing the
* specified value to the the specified address.
*
* @param	OutputPtr contains the address to perform the output operation
*		at.
* @param	Value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
#define XIo_Out32(OutputPtr, Value) \
	(*(volatile u32 *)((OutputPtr)) = (Value))


/* The following macros allow the software to be transportable across
 * processors which use big or little endian memory models.
 *
 * Defined first is a no-op endian conversion macro. This macro is not to
 * be used directly by software. Instead, the XIo_To/FromLittleEndianXX and
 * XIo_To/FromBigEndianXX macros below are to be used to allow the endian
 * conversion to only be performed when necessary
 */
#define XIo_EndianNoop(Source, DestPtr)		(*DestPtr = Source)

#ifdef XLITTLE_ENDIAN

#define XIo_ToLittleEndian16			XIo_EndianNoop
#define XIo_ToLittleEndian32			XIo_EndianNoop
#define XIo_FromLittleEndian16			XIo_EndianNoop
#define XIo_FromLittleEndian32			XIo_EndianNoop

#define XIo_ToBigEndian16(Source, DestPtr) XIo_EndianSwap16(Source, DestPtr)
#define XIo_ToBigEndian32(Source, DestPtr) XIo_EndianSwap32(Source, DestPtr)
#define XIo_FromBigEndian16			XIo_ToBigEndian16
#define XIo_FromBigEndian32			XIo_ToBigEndian32

#else

#define XIo_ToLittleEndian16(Source, DestPtr) XIo_EndianSwap16(Source, DestPtr)
#define XIo_ToLittleEndian32(Source, DestPtr) XIo_EndianSwap32(Source, DestPtr)
#define XIo_FromLittleEndian16			XIo_ToLittleEndian16
#define XIo_FromLittleEndian32			XIo_ToLittleEndian32

#define XIo_ToBigEndian16			XIo_EndianNoop
#define XIo_ToBigEndian32			XIo_EndianNoop
#define XIo_FromBigEndian16			XIo_EndianNoop
#define XIo_FromBigEndian32			XIo_EndianNoop

#endif

/************************** Function Prototypes ******************************/

/* The following functions allow the software to be transportable across
 * processors which use big or little endian memory models. These functions
 * should not be directly called, but the macros XIo_To/FromLittleEndianXX and
 * XIo_To/FromBigEndianXX should be used to allow the endian conversion to only
 * be performed when necessary.
 */
void XIo_EndianSwap16(u16 Source, u16 *DestPtr);
void XIo_EndianSwap32(u32 Source, u32 *DestPtr);

/* The following functions handle IO addresses where data must be swapped
 * They cannot be implemented as macros
 */
u16 XIo_InSwap16(XIo_Address InAddress);
u32 XIo_InSwap32(XIo_Address InAddress);
void XIo_OutSwap16(XIo_Address OutAddress, u16 Value);
void XIo_OutSwap32(XIo_Address OutAddress, u32 Value);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
