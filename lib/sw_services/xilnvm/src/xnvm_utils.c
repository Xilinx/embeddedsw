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
 *          XST_SUCCESS	- On valid input AES key sring
 *          XST_INVALID_PARAM - On invalid length of the input string
 *          XST_FAILURE	- On non hexadecimal character in string
 *
 * @note    None
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
 * Validate the input string contains valid PPK hash
 *
 * @param	Hash - Pointer to PPK hash
 *
 * @param	Len  - Length of the input string
 *
 * @return
 *	- XST_SUCCESS	- On valid input Ppk Hash sring
 *	- XST_INVALID_PARAM - On invalid length of the input string
 *	- XST_FAILURE	- On non hexadecimal character in string
 *
 ******************************************************************************/
u32 XNvm_ValidateHash(const char *Hash, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;
	u32 i;

	if (NULL == Hash) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (Len == 0U) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (strlen(Hash) != Len) {
		goto END;
	}

	for(i = 0U; i < strlen(Hash); i++) {
		if(Xil_IsValidHexChar(Hash[i]) != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END :
	return Status;
}

/****************************************************************************/
/**
 * Convert the Bits to Bytes in Little Endian format.
 *	Ex: 0x5C -> {0, 0, 1, 1, 1, 0, 1, 0}
 *
 * @param	Bits 	Input Buffer in Hex.
 * @param	Bytes	Output buffer where each bit of given Hex is converted
 * 			to equivalent byte.
 * @param	Len 	Output number of Byte arrays
 * @return
 *	- XST_SUCCESS - On successfull conversion from bits to byte array
 *	- XST_INVALID_PARAM - On invalid params.
 *
 ****************************************************************************/
u32 XNvm_ConvertHexToByteArray(const u8 *Bits, u8 *Bytes, u32 Len)
{
	u32 Status = XST_INVALID_PARAM;
	u8 Data;
	u32 Index;
	u32 BitIndex = 0U;
	u32 ByteIndex = 0U;
	u32 BytLen;

	if (Bits == NULL) {
		goto END;
	}
	if (Bytes == NULL) {
		goto END;
	}
	if ((Len % 8) != 0U) {
		goto END;
	}

	BytLen = Len;
	/**
	* Make sure the bytes array is 0'ed first.
	*/
	for(Index = 0U; Index < BytLen; Index++) {
		Bytes[Index] = 0U;
	}

	while(BytLen != 0U) {
		/**
		* Convert 8 Bit One Byte to 1 Bit 8 Bytes
		*/
		for(Index = 0U; Index < 8U; Index++) {
			/**
			 * Convert from LSB -> MSB - Little Endian
			 */
			Data = (Bits[BitIndex] >> Index) & 0x1U;
			Bytes[ByteIndex] = Data;
			ByteIndex++;
			BytLen--;
		}
		BitIndex++;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * Convert the Bytes Array to Hex value in little endian format
 * 0th byte is LSB, 7th byte is MSB
 *	Ex: {0, 0, 1, 1, 1, 0, 1, 0} -> 0x5C
 *
 * @param	Bytes	Input Buffer.
 * @param	Bits	Output buffer.
 * @param	Len	Output buffer Len in bits.
 *
 * @return	XST_SUCCESS - On successfull conversion from byte array to Hex
 *		XST_INVALID_PARAM - On invalid params.
 ****************************************************************************/
u32 XNvm_ConvertByteArrayToHex(const u8 *Bytes, u8 *Bits , u32 Len)
{
	u32 Status = XST_INVALID_PARAM;
	u8 Tmp;
	u32 Index;
	u32 BitIndex = 0U;
	u32 ByteIndex = 0U;
	u32 BytLen = Len;

	if (Bits == NULL) {
		goto END;
	}
	if (Bytes == NULL) {
		goto END;
	}

	/*
	 * Make sure the bits array is 0 first.
	 */
	for(Index = 0U;
	Index < (((BytLen % 8U) != 0U) ? ((BytLen / 8U) + 1U) : (BytLen / 8U));
	Index++) {
		Bits[Index] = 0U;
	}

	while(BytLen != 0U) {
		/**
		 * Convert 1 Bit 8 Bytes to 8 Bit 1 Byte
		 */
		for(Index = 0U; Index < 8U; Index++) {
			/**
			 * Store from LSB -> MSB - Little Endian
			 */
			Tmp = (Bytes[ByteIndex]) & 0x1U;
			Bits[BitIndex] |= (Tmp << Index);
			ByteIndex++;
			BytLen--;
		}
		BitIndex++;
	}

	Status = XST_SUCCESS;
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

/******************************************************************************/
/**
 * Validate the input string contains valid User Fuse String
 *
 * @param   UserFuseStr - Pointer to User Fuse String
 *
 * @return
 *          XST_SUCCESS	- On valid input UserFuse string
 *          XST_INVALID_PARAM - On invalid length of the input string
 *
 ******************************************************************************/
u32 XNvm_ValidateUserFuseStr(const char *UserFuseStr)
{
	u32 Status = XST_INVALID_PARAM;

	if(NULL == UserFuseStr) {
		goto END;
	}

	if (strlen(UserFuseStr) % 8 != 0x00U) {
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * Validate the input string contains valid IV String
 *
 * @param   IvStr - Pointer to Iv String
 *
 * @return
 *          XST_SUCCESS	- On valid input IV string
 *          XST_INVALID_PARAM - On invalid length of the input string
 *
 ******************************************************************************/
u32 XNvm_ValidateIvString(const char *IvStr)
{
	u32 Status = XST_INVALID_PARAM;

        if(NULL == IvStr) {
                goto END;
        }

        if (strlen(IvStr) != 24U) {
                goto END;
        }

        Status = XST_SUCCESS;
END:
        return Status;

}
