/*******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc.  All rights reserved.
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
* 2.3	kal  01/27/2021 Added XNvm_ZeroizeAndVerify API
* 2.4   kal  07/13/2021 Fixed doxygen warnings
* 2.5   kpt  12/07/2021 Replace memset with Xil_SMemSet
* 3.0   kal  08/01/2022 Reset Status to XST_FAILURE before for loop in
* 			XNvm_ZeroizeAndVerify
*       kpt  08/03/2022 Added volatile keyword to avoid compiler optimization
*                       of loop redundancy check
*       dc   08/29/2022 Changed u8 to u32 type
* 3.1   skg  10/28/2022 Added In body comments for APIs
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
#include "xnvm_temp.h"

/*************************** Constant Definitions *****************************/

/**< Polynomial used for CRC calculation */
#define REVERSE_POLYNOMIAL	(0x82F63B78U)


/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/
/******************************************************************************/
/**
 * @brief	This function reads the given register.
 *
 * @param	BaseAddress is the eFuse controller base address.
 * @param	RegOffset is the register offset from the base address.
 *
 * @return	The 32-bit value of the register.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadReg(u32 BaseAddress, u32 RegOffset)
{
	return Xil_In32((UINTPTR)(BaseAddress + RegOffset));
}

/******************************************************************************/
/**
 * @brief	This function writes the value into the given register.
 *
 * @param	BaseAddress is the eFuse controller base address.
 * @param	RegOffset is the register offset from the base address.
 * @param	Data is the 32-bit value to be written to the register.
 *
 * @return	None
 *
 ******************************************************************************/
void XNvm_EfuseWriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32((UINTPTR)(BaseAddress + RegOffset), Data);
}

/******************************************************************************/
/**
 * @brief	This function locks the eFUSE Controller to prevent accidental
 * 		writes to eFUSE controller registers.
 *
 * @return	- XST_SUCCESS - eFUSE controller locked.
 *		- XNVM_EFUSE_ERR_LOCK - Failed to lock eFUSE controller
 *					                register access.
 *
 ******************************************************************************/
int XNvm_EfuseLockController(void)
{
	int Status = XST_FAILURE;
	volatile u32 LockStatus = ~XNVM_EFUSE_CTRL_WR_LOCKED;

    /**
	 *  Write lock Passcode in efuse control at offset of WR_LOCK_REG
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_WR_LOCK_REG_OFFSET,
			~XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	/**
     *  Read the WR_LOCK_REG if above write was successful. Return XNVM_EFUSE_ERR_LOCK if not success
     */
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(XNVM_EFUSE_CTRL_WR_LOCKED == LockStatus) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_LOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function unlocks the eFUSE Controller for writing
 *		to its registers.
 *
 * @return	XST_SUCCESS - eFUSE controller locked.
 *		XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 *							register access.
 *
 ******************************************************************************/
int XNvm_EfuseUnlockController(void)
{
	int Status = XST_FAILURE;
	volatile u32 LockStatus = ~XNVM_EFUSE_CTRL_WR_UNLOCKED;

     /**
	 *  Write unlock Passcode in efuse control at offset of WR_LOCK_REG
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_WR_LOCK_REG_OFFSET,
				XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	/**
     *  Read the WR_LOCK_REG if above write was successful. Return XNVM_EFUSE_ERR_UNLOCK if not success
     */
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(XNVM_EFUSE_CTRL_WR_UNLOCKED == LockStatus) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_UNLOCK;
	}

	return Status;
}


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
	u32 Idx;
	u32 BitNo;
	volatile u32 Temp1Crc;
	volatile u32 Temp2Crc;

	for (Idx = 0U; Idx < XNVM_AES_KEY_SIZE_IN_WORDS; Idx++) {
		/**
         *	Process each bits of 32-bit Value
		 */
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

		/**
         *	Get 5-bit from Address
		 */
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

    /**
	 *  Return CRC value upon success
	 */
	return Crc;
}

/*****************************************************************************/
/**
 * @brief	This function is used to zeroize the memory
 *
 * @param	DataPtr Pointer to the memory which need to be zeroized.
 * @param	Length	Length of the data in bytes.
 *
 * @return
 *		- XST_SUCCESS: If Zeroization is successful.
 *		- XST_FAILURE: If Zeroization is not successful.
 ********************************************************************************/
int XNvm_ZeroizeAndVerify(u8 *DataPtr, const u32 Length)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;

	/**
     *	Clear the decrypted data
	 */
	Status = Xil_SMemSet(DataPtr, Length, 0, Length);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
     *	Read it back to verify
	 */
	Status = XST_FAILURE;
	for (Index = 0U; Index < Length; Index++) {
		if (DataPtr[Index] != 0x00U) {
			goto END;
		}
	}
	if (Index == Length) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs the CRC check of AES key/User0 key/User1 key
 *
 * @param	CrcRegOffSet - Register offset of respective CRC register
 * @param	CrcDoneMask - Respective CRC done mask in status register
 * @param	CrcPassMask - Respective CRC pass mask in status register
 * @param	Crc - A 32 bit CRC value of an expected AES key.
 *
 * @return	- XST_SUCCESS - On successful CRC check.
 *		- XNVM_EFUSE_ERR_CRC_VERIFICATION - If AES boot key integrity
 *							check is failed.
 *		- XST_FAILURE - If AES boot key integrity check
 *							has not finished.
 *
 * @note	For Calculating the CRC of the AES key use the
 *		XNvm_AesCrcCalc() function.
 *
 ******************************************************************************/
int XNvm_EfuseCheckAesKeyCrc(u32 CrcRegOffSet, u32 CrcDoneMask, u32 CrcPassMask, u32 Crc)
{
	int Status = XST_FAILURE;
	int LockStatus = XST_FAILURE;
	u32 ReadReg;
	u32 IsUnlocked = FALSE;

    /**
     *  Read the WR_LOCK_REG. Unlock the controller if read as locked
     */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(XNVM_EFUSE_CTRL_WR_LOCKED == ReadReg) {
		Status = XNvm_EfuseUnlockController();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		IsUnlocked = TRUE;
	}

	/**
	 *  Write the crc to crcregoffset of eFuse_ctrl register
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR, CrcRegOffSet, Crc);

    /**
	 *  Wait for crcdone
	 */
	Status = (int)Xil_WaitForEvent((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR + XNVM_EFUSE_STATUS_REG_OFFSET),
				CrcDoneMask, CrcDoneMask, XNVM_POLL_TIMEOUT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

    /**
	 *  Read efuse status register. If Crc is not done return XST_FAILURE
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_STATUS_REG_OFFSET);

	if ((ReadReg & CrcDoneMask) != CrcDoneMask) {
		Status = XST_FAILURE;
	}
	/**
	 *  Return XNVM_EFUSE_ERR_CRC_VERIFICATION if Crc is not Pass. Return XST_SUCCESS upon crc pass and done
	 */
	else if ((ReadReg & CrcPassMask) != CrcPassMask) {
		Status = (int)XNVM_EFUSE_ERR_CRC_VERIFICATION;
	}
	else {
		Status = XST_SUCCESS;
	}
END:
    /**
	 *  Lock efuse controller
	 */
	if (IsUnlocked == TRUE) {
		LockStatus = XNvm_EfuseLockController();
		if (XST_SUCCESS == Status) {
			Status = LockStatus;
		}
	}
	return Status;
}
