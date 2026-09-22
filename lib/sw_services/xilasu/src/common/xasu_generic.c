/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_generic.c
 *
 * This file contains the generic function definition which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ----  -------- ----------------------------------------------------------------------------
 * 1.0   yog   11/24/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_generic_apis Generic APIs
 * @{
*/

/***************************** Include Files *****************************************************/
#include "xasu_def.h"
#include "xasu_generic.h"
#include "xstatus.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function checks whether the buffer has a non-zero value or not.
 *
 * @param	Buffer	Pointer to the buffer whose value needs to be checked.
 * @param	Length	Length of the buffer in bytes.
 *
 * @return
 *	-	XST_SUCCESS, if buffer has non-zero value.
 *	-	XST_FAILURE, if buffer has all zeroes as values.
 *
 *************************************************************************************************/
s32 XAsu_IsBufferNonZero(u8 *Buffer, u32 Length)
{
	s32 Status = XST_FAILURE;
	volatile u32 Index;

	if ((Buffer == NULL) || (Length == 0U)) {
		Status = XST_FAILURE;
	} else {
		for (Index = 0U; Index < Length ; Index++) {
			if (Buffer[Index] != 0U) {
				Status = XST_SUCCESS;
				break;
			}
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether two non-empty address ranges overlap or either range
 *		wraps.
 *
 * @param	FirstAddr	Start address of the first range.
 * @param	FirstLen	Length of the first range.
 * @param	SecondAddr	Start address of the second range.
 * @param	SecondLen	Length of the second range.
 *
 * @return
 *	- XASU_TRUE, if either range wraps or the ranges overlap.
 *	- XASU_FALSE, if either range is empty or the ranges are representable and disjoint.
 *
 *************************************************************************************************/
u8 XAsu_IsRangeOverlap(u64 FirstAddr, u32 FirstLen, u64 SecondAddr, u32 SecondLen)
{
	u64 FirstEnd = FirstAddr + (u64)FirstLen;
	u64 SecondEnd = SecondAddr + (u64)SecondLen;
	u8 Overlap = XASU_FALSE;

	if ((FirstLen != 0U) && (SecondLen != 0U) &&
	    ((FirstEnd < FirstAddr) || (SecondEnd < SecondAddr) ||
	     ((FirstAddr < SecondEnd) && (SecondAddr < FirstEnd)))) {
		Overlap = XASU_TRUE;
	}

	return Overlap;
}

/** @} */
