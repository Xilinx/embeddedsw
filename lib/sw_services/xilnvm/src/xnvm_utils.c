/*******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_utils.c
*
* This file contains NVM library utility functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
*	har  09/24/2019 Fixed MISRA-C violations
* 2.0	kal  03/08/2020 Added Utility APIs
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xnvm_utils.h"
#include "xil_util.h"

/*************************** Constant Definitions *****************************/

/* Polynomial used for CRC calculation */
#define REVERSE_POLYNOMIAL	(0x82F63B78U)


/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/


/******************************************************************************/
/**
 * Validate the input string contains valid AES key
 *
 * @param   Key - Pointer to AES key
 *
 * @return
 *	- XST_SUCCESS	- On valid input AES key sring
 *	- XST_INVALID_PARAM - On invalid length of the input string
 *	- XST_FAILURE	- On non hexadecimal character in string
 *
 ******************************************************************************/
u32 XNvm_ValidateAesKey(const char *Key)
{
	u32 Status = XST_INVALID_PARAM;
	u32 Len;

	if(NULL == Key) {
		goto END;
	}

	Len = Xil_Strnlen(Key, XNVM_MAX_AES_KEY_LEN_IN_CHARS + 1U);

	if ((Len != XNVM_256_BITS_AES_KEY_LEN_IN_CHARS) &&
		(Len != XNVM_128_BITS_AES_KEY_LEN_IN_CHARS)) {
		goto END;
	}

	Status = Xil_ValidateHexStr(Key);
END:
	return Status;
}

/******************************************************************************/
/**
 * Calculates CRC value for each row of AES key.
 *
 * @param	PrevCRC	Holds the prev row's CRC
 * @param	Data	Holds the present row's key
 * @param	Addr	 Stores the current row number
 *
 * @return	Crc of current row
 *
 ******************************************************************************/
static u32 XNvm_RowAesCrcCalc(u32 PrevCRC, u32 *Data, u32 Addr)
{
	u32 Crc = PrevCRC;
	u32 Value = *(u32 *)Data;
	u32 Row = Addr;
	u32 Idx;

	/* Process each bits of 32-bit Value */
	for (Idx = 0U; Idx < 32U; Idx++) {
		if ((((Value & 0x1U) ^ Crc) & 0x1U) != 0U) {
			Crc = ((Crc >> 1U) ^ REVERSE_POLYNOMIAL);
		}
		else {
			Crc = Crc >> 1U;
		}
		Value = Value >> 1U;
	}

	/* Get 5-bit from Address */
	for (Idx = 0U; Idx < 5U; Idx++) {
		if ((((Row & 0x1U) ^ Crc) & 0x1U) != 0U) {
			Crc = ((Crc >> 1U) ^ REVERSE_POLYNOMIAL);
		}
		else {
			Crc = Crc >> 1U;
		}
		Row = Row >> 1U;
	}

	return Crc;
}
/******************************************************************************/
/**
 * This function calculates CRC of AES key
 *
 * @param	Key	Pointer to the key for which CRC has to be calculated
 *
 * @return	CRC of AES key
 *
 ******************************************************************************/
u32 XNvm_AesCrcCalc(u32 *Key)
{
	u32 Crc = 0U;
	u8 Idx;

	for (Idx = 0U; Idx < XNVM_AES_KEY_SIZE_IN_WORDS ; Idx++) {
		Crc = XNvm_RowAesCrcCalc(Crc,
				&Key[XNVM_AES_KEY_SIZE_IN_WORDS - Idx - 1U],
				XNVM_AES_KEY_SIZE_IN_WORDS - Idx);
	}

	return Crc;
}

