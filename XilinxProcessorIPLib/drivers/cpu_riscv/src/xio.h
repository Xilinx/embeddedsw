/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xio.h
* @addtogroup cpu_riscv Overview
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
* 1.0   sa   11/22/19 Initial version
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

#define XIo_ToLittleEndian16			XIo_EndianNoop
#define XIo_ToLittleEndian32			XIo_EndianNoop
#define XIo_FromLittleEndian16			XIo_EndianNoop
#define XIo_FromLittleEndian32			XIo_EndianNoop

#define XIo_ToBigEndian16(Source, DestPtr) XIo_EndianSwap16(Source, DestPtr)
#define XIo_ToBigEndian32(Source, DestPtr) XIo_EndianSwap32(Source, DestPtr)
#define XIo_FromBigEndian16			XIo_ToBigEndian16
#define XIo_FromBigEndian32			XIo_ToBigEndian32

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
