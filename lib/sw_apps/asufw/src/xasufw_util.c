/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/*************************************************************************************************/
/**
 * @brief	This function will read, check and conditionally modify an address.
 *
 * @param	Addr	Address of the register.
 * @param	Mask	Bits to be modified.
 * @param	Value	Value to be written to the register.
 *
 *************************************************************************************************/
void XAsufw_RCMW(u32 Addr, u32 Mask, u32 Value)
{
	u32 Val;

	/** Read the value from the register */
	Val = XAsufw_ReadReg(Addr);

	/** Check if the mask bits are already set. */
	if ((Val & Mask) != (Mask & Value)) {
		/**
		 * Reset designated bits in a register value to zero, and replace them with the
		 * given value.
		 */
		Val = (Val & (~Mask)) | (Mask & Value);

		/** Update the value to the register. */
		XAsufw_WriteReg(Addr, Val);
	}
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
 * @brief	This function changes the endianness of source data and copies it into
 * 		destination buffer.
 *
 * @param	Dest		Pointer to the destination address.
 * @param	DestSize	Size of the destination buffer in bytes.
 * @param	Src		Pointer to the source address.
 * @param	SrcSize		Size of the source buffer in bytes.
 * @param	CopyLen		Number of bytes to be copied.
 *
 * @return
 * 		- XASUFW_SUCCESS on success
 * 		- XASUFW_FAILURE on failure
 * 		- XASUFW_INVALID_PARAM Invalid inputs
 *
 *************************************************************************************************/
/*TODO: Will remove this API when sending client patch */
s32 XAsufw_ChangeEndiannessAndCpy(void *Dest, const u32 DestSize, const void *Src,
				  const u32 SrcSize,
				  const u32 CopyLen)
{
	s32 Status = XASUFW_FAILURE;
	volatile u32 Index;
	const u8 *Src8 = (const u8 *)Src;
	const u8 *Dst8 = (const u8 *)Dest;
	u8 *DestTemp = (u8 *)Dest;
	const u8 *SrcTemp = (const u8 *)Src;

	if ((Dest == NULL) || (Src == NULL)) {
		Status =  XASUFW_INVALID_PARAM;
	} else if ((CopyLen == 0U) || (DestSize < CopyLen) || (SrcSize < CopyLen) ||
		   ((CopyLen % 2U) != 0) ) {
		Status =  XASUFW_INVALID_PARAM;
	}
	/* Return error for overlap string */
	else if ((Src8 < Dst8) && (&Src8[CopyLen - 1U] >= Dst8)) {
		Status =  XASUFW_INVALID_PARAM;
	} else if ((Dst8 < Src8) && (&Dst8[CopyLen - 1U] >= Src8)) {
		Status =  XASUFW_INVALID_PARAM;
	} else {
		for (Index = 0U; Index < (CopyLen / 2U); Index++) {
			DestTemp[CopyLen - Index - 1U] = SrcTemp[Index];
			DestTemp[Index] = SrcTemp[CopyLen - Index - 1U];
		}
		if (Index == (CopyLen / 2U)) {
			Status = XASUFW_SUCCESS;
		}
	}

	return Status;
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
	u8 TempVar;

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

	return Status;
}
/** @} */
