/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_util.c
 *
 * This file contains the code for util APIs in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   03/29/24 Initial release
 *       am   06/26/24 Added XAsufw_CryptoCoreReleaseReset(), XAsufw_CryptoCoreSetReset() and
 *                     XAsufw_RCMW() function.
 *  	 ss   07/11/24 Added XAsufw_ChangeEndiannessAndCpy function.
 *       ss   08/20/24 Updated description for XAsufw_ChangeEndianness() function.
 *       am   09/13/24 Fixed pointer conversion error for cpp compiler.
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       ss   10/05/24 Added XAsufw_IsBufferNonZero function.
 * 1.1   ma   02/03/25 Updated TempVar in XAsufw_ChangeEndianness with volatile and zeroize at
 *                     the end.
 *       vns  02/06/25 Removed XAsufw_ChangeEndiannessAndCpy() function which is not in use
 *       vns  02/12/25 Removed XAsufw_RCMW() API
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_util.h"
#include "xasufw_status.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function will Read, Modify and Write to a register.
 *
 * @param	Addr	Address of the register.
 * @param	Mask	Bits to be modified.
 * @param	Value	Value to be written to the register.
 *
 *************************************************************************************************/
void XAsufw_RMW(u32 Addr, u32 Mask, u32 Value)
{
	u32 Val;

	/** Read the value from the register. */
	Val = XAsufw_ReadReg(Addr);

	/**
	 * Reset designated bits in a register value to zero, and replace them with the
	 * given value.
	 */
	Val = (Val & (~Mask)) | (Mask & Value);

	/** Update the value to the register. */
	XAsufw_WriteReg(Addr, Val);
}

/*****************************************************************************/
/**
 * @brief	This function takes the hardware core out of reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	Offset		Offset of the reset register.
 *
 *****************************************************************************/
void XAsufw_CryptoCoreReleaseReset(u32 BaseAddress, u32 Offset)
{
	XAsufw_WriteReg((BaseAddress + Offset), XASUFW_RESET_SET);
	XAsufw_WriteReg((BaseAddress + Offset), XASUFW_RESET_UNSET);
}

/*****************************************************************************/
/**
 * @brief	This function places the hardware core into the reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	Offset		Offset of the reset register.
 *
 *****************************************************************************/
void XAsufw_CryptoCoreSetReset(u32 BaseAddress, u32 Offset)
{
	XAsufw_WriteReg((BaseAddress + Offset), XASUFW_RESET_SET);
}

/*************************************************************************************************/
/**
 * @brief	This function changes the endianness of data and stores it in the same buffer.
 *
 * @param	Buffer	Pointer to the buffer whose endianness needs to be changed.
 * @param	Length	Length of the buffer in bytes.
 *
 * @return
 *	-	XASUFW_SUCCESS, when endianness change and copy is success.
 *	-	XASUFW_INVALID_PARAM, if input parameters are invalid.
 *
 * @note
 * 	- Supports only for even lengths.
 *
 *************************************************************************************************/
s32 XAsufw_ChangeEndianness(u8 *Buffer, u32 Length)
{
	s32 Status = XASUFW_FAILURE;
	volatile u32 Index;
	volatile u8 TempVar;

	if ((Buffer == NULL) || (Length == 0U) || ((Length % 2U) != 0U)) {
		Status = XASUFW_INVALID_PARAM;
	} else {
		for (Index = 0U; Index < (Length / 2U); Index++) {
			TempVar = Buffer[Index];
			Buffer[Index] = Buffer[Length - Index - 1U];
			Buffer[Length - Index - 1U] = TempVar;
		}
		if (Index == (Length / 2U)) {
			Status = XASUFW_SUCCESS;
		}
	}

	TempVar = 0U;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether the buffer has a non zero value or not.
 *
 * @param	Buffer	Pointer to the buffer whose value needs to be checked.
 * @param	Length	Length of the buffer in bytes.
 *
 * @return
 *	-	XASUFW_SUCCESS, when buffer has non zero value.
 *	-	XASUFW_INVALID_PARAM, if input parameters are invalid.
 *	-	XASUFW_FAILURE, when buffer has all zeroes as values.
 *
 *************************************************************************************************/
s32 XAsufw_IsBufferNonZero(u8 *Buffer, u32 Length)
{
	s32 Status = XASUFW_FAILURE;
	volatile u32 Index;

	if ((Buffer == NULL) || (Length == 0U)) {
		Status = XASUFW_INVALID_PARAM;
	} else {
		for (Index = 0U; Index < Length ; Index++) {
			if (Buffer[Index] != 0U) {
				Status = XASUFW_SUCCESS;
				break;
			}
		}
	}

	return Status;
}
/** @} */
