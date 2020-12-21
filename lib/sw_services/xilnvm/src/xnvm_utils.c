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
* 2.0	kal  02/28/2020 Added utility APIs XNvm_ValidateHash, XNvm_AesCrcCalc
*                       XNvm_ConvertBytesToBits and XNvm_ConvertBitsToBytes
*       kal  04/11/2020 Renamed conversion APIs to XNvm_ConvertHexToByteArray
*       		and XNvm_ConvertByteArrayToHex
*       kal  05/04/2020 Moved few utility functions to application and removed
*       		usage of conversion APIs as the same functionality is
*       		achieved by bit-wise operators.
* 2.1	am   08/19/2020 Resolved MISRA C violations.
* 	kal  09/03/2020 Fixed Security CoE review comments
*	am   10/13/2020 Resolved MISRA C violations
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xnvm_utils.h"
#include "xstatus.h"
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
 * @brief	Validate the input string contains valid AES key.
 *
 * @param   Key - Pointer to AES key.
 *
 * @return - XST_SUCCESS - On valid input AES key string.
 *		   - XST_INVALID_PARAM - On invalid length of the input string.
 *		   - XST_FAILURE	- On non hexadecimal character in string
 *
 ******************************************************************************/
int XNvm_ValidateAesKey(const char *Key)
{
	int Status = XST_INVALID_PARAM;
	u32 Len;

	if(NULL == Key) {
		goto END;
	}

	Len = Xil_Strnlen(Key, XNVM_MAX_AES_KEY_LEN_IN_CHARS + 1U);

	if ((Len != XNVM_256_BITS_AES_KEY_LEN_IN_CHARS) &&
		(Len != XNVM_128_BITS_AES_KEY_LEN_IN_CHARS)) {
		goto END;
	}

	Status = (int)Xil_ValidateHexStr(Key);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function calculates CRC of AES key.
 *
 * @param	Key - Pointer to the key for which CRC has to be calculated.
 *
 * @return	CRC of AES key.
 *
 ******************************************************************************/
u32 XNvm_AesCrcCalc(const u32 *Key)
{
	u32 Crc = 0U;
	u32 Value;
	u8 Idx;
	u8 BitNo;
	volatile u32 Temp1Crc;
	volatile u32 Temp2Crc;

	for (Idx = 0U; Idx < XNVM_AES_KEY_SIZE_IN_WORDS; Idx++) {
		/* Process each bits of 32-bit Value */
		Value = Key[XNVM_AES_KEY_SIZE_IN_WORDS - Idx - 1U];
		for (BitNo = 0U; BitNo < 32U; BitNo++) {
			Temp1Crc = Crc >> 1U;
			Temp2Crc = Temp1Crc ^ REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & 0x1U) != 0U) {
				Crc = Temp2Crc;
			}
			else {
				Crc = Temp1Crc;
			}
			Value = Value >> 1U;
		}

		/* Get 5-bit from Address */
		Value = XNVM_AES_KEY_SIZE_IN_WORDS - (u32)Idx;
		for (BitNo = 0U; BitNo < 5U; BitNo++) {
			Temp1Crc = Crc >> 1U;
			Temp2Crc = Temp1Crc ^ REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & 0x1U) != 0U) {
				Crc = Temp2Crc;
			}
			else {
				Crc = Temp1Crc;
			}
			Value = Value >> 1U;
		}
	}

	return Crc;
}
