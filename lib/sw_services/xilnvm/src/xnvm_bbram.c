/*******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_bbram.c
*
* This file contains NVM library BBRAM functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xnvm_bbram.h"
#include "xnvm_bbram_hw.h"


/*************************** Constant Definitions *****************************/

/* Polynomial used for CRC calculation */
#define REVERSE_POLYNOMIAL	(0x82F63B78U)

/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
static u32 XNvm_BbramEnablePgmMode();
static inline void XNvm_BbramDisablePgmMode();
static u32 XNvm_BbramValidateAesKeyCrc(const u32* Key);
static u32 XNvm_BbramRowAesCrcCalc(u32 PrevCRC, u32 Data, u32 Addr);
static inline u32 XNvm_BbramAesCrcCalc(const u32 *Key);

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * Validates CRC of the key stored in BBRAM with CRC of input key.
 *
 * @param   Key - Pointer to key which is to be matched with key stored in BBRAM
 * @param   KeyLen - XNVM_128_BITS_AES_KEY_LEN_IN_BYTES for 128-bit AES key
 *                   XNVM_256_BITS_AES_KEY_LEN_IN_BYTES for 256-bit AES key
 *
 * @return
 *          XST_SUCCESS	- Key is written to BBRAM
 *          XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT - Timeout during enabling
 *                  programming mode
 *          XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT - Timeout during disabling
 *                  programming mode
 *          XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT - CRC validation check timed
 *                  out.
 *          XNVM_BBRAM_ERROR_AES_CRC_MISMATCH - CRC mismatch
 *          XST_FAILURE - Unexpected error
 *
 * @note    None.
 ******************************************************************************/
u32 XNvm_BbramWriteAesKey(const u8* Key, u16 KeyLen)
{
	u32 Status = XST_FAILURE;
	u32 BbramAesKey[XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS];
	const u32 *AesKey = (const u32 *) Key;
	u32 BbramKeyAddr;
	u32 KeyLenInWords;
	u8 Idx;

	if ((KeyLen != XNVM_128_BITS_AES_KEY_LEN_IN_BYTES) &&
	    (KeyLen != XNVM_256_BITS_AES_KEY_LEN_IN_BYTES)) {
		Status = XST_INVALID_PARAM;
	}

	Status = XNvm_BbramEnablePgmMode();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	KeyLenInWords = KeyLen / sizeof(u32);
	BbramKeyAddr = XNVM_BBRAM_0_REG;
	for (Idx = 0U; Idx < XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS; Idx++) {
		if (Idx < KeyLenInWords) {
			BbramAesKey[Idx] = AesKey[Idx];
		}
		else {
			/* Fill zeros for empty BBRAM key area for 128-bit
			   AES key */
			BbramAesKey[Idx] = 0x00U;
		}
		XNvm_BbramWriteReg(BbramKeyAddr, BbramAesKey[Idx]);
		BbramKeyAddr += sizeof(u32);
	}

	Status = XNvm_BbramValidateAesKeyCrc(BbramAesKey);

	XNvm_BbramDisablePgmMode();
END:
	return Status;
}

/******************************************************************************/
/**
 * Locks the User data written in BBRAM i.e. Make User data written in BBRAM
 * as read only.
 *
 * @param   None
 *
 * @return
 *          XST_SUCCESS	- Locked BBRAM User data for write
 *          XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE - User data locked for write
 *          XST_FAILURE - Unexpected error
 *
 * @note    None.
 ******************************************************************************/
u32 XNvm_BbramLockUsrDataWrite()
{
	u32 Status = XST_FAILURE;
	u32 LockStatus = 0U;

	XNvm_BbramWriteReg(XNVM_BBRAM_MSW_LOCK_REG, XNVM_BBRAM_MSW_LOCK);
	LockStatus = XNvm_BbramReadReg(XNVM_BBRAM_MSW_LOCK_REG);
	if((LockStatus & XNVM_BBRAM_MSW_LOCK) != 0) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE;
	}

	return Status;
}

/******************************************************************************/
/**
 * Writes user provided 32-bit data to BBRAM.
 *
 * @param   UsrData - 32-bit user data to be written to BBRAM
 *
 * @return
 *          XST_SUCCESS	- User data written to BBRAM
 *          XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED - User data locked for write
 *          XST_FAILURE - Unexpected error
 *
 * @note    None.
 ******************************************************************************/
u32 XNvm_BbramWriteUsrData(u32 UsrData)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus;

	LockStatus = XNvm_BbramReadReg(XNVM_BBRAM_MSW_LOCK_REG);

	if((LockStatus & XNVM_BBRAM_MSW_LOCK) != 0) {
		Status = XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED;
	}
	else {
		XNvm_BbramWriteReg(XNVM_BBRAM_8_REG, UsrData);
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * Reads 32-bit user data from BBRAM.
 *
 * @param   None
 *
 * @return  32-bit user data stored in BBRAM
 *
 * @note    None.
 ******************************************************************************/
u32 XNvm_BbramReadUsrData()
{
	return XNvm_BbramReadReg(XNVM_BBRAM_8_REG);
}

/******************************************************************************/
/**
 * Zeroize the BBRAM.
 *
 * @param   None
 *
 * @return
 *          XST_SUCCESS	- Zeroization of BBRAM done
 *          XNVM_BBRAM_ERROR_ZEROIZE_TIMEOUT - Timed out during BBRAM
 *                  zeroization
 *          XNVM_BBRAM_ERROR_ZEROIZE_VERIFY - Failed verification after BBRAM
 *                  zeroization
 *          XST_FAILURE - Unexpected error
 *
 * @note    None.
 ******************************************************************************/
u32 XNvm_BbramZeroize()
{
	u32 Status = XST_FAILURE;

	/* Initiate zeroization */
	XNvm_BbramWriteReg(XNVM_BBRAM_CTRL_REG, XNVM_BBRAM_CTRL_START_ZEROIZE);
	Status = Xil_WaitForEvent(XNVM_BBRAM_BASE_ADDR + XNVM_BBRAM_STATUS_REG,
	                         XNVM_BBRAM_STATUS_ZEROIZED,
	                         XNVM_BBRAM_STATUS_ZEROIZED,
	                         XNVM_BBRAM_ZEROIZE_TIMEOUT_VAL);
	if (Status != XST_SUCCESS) {
		Status = XNVM_BBRAM_ERROR_ZEROIZE_TIMEOUT;
	}

	return Status;
}

/******************************************************************************/
/**
 * Enabling  the BBRAM controller in programming mode.
 *
 * @param   None
 *
 * @return
 *          XST_SUCCESS	- Enabled programming mode
 *          XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT - On failure
 *          XST_FAILURE - Unexpected error
 *
 * @note    None.
 ******************************************************************************/
static u32 XNvm_BbramEnablePgmMode()
{
	u32 Status = XST_FAILURE;

	XNvm_BbramWriteReg(XNVM_BBRAM_PGM_MODE_REG,
	                   XNVM_EFUSE_PGM_MODE_PASSCODE);
	Status = Xil_WaitForEvent(XNVM_BBRAM_BASE_ADDR + XNVM_BBRAM_STATUS_REG,
	                         XNVM_BBRAM_STATUS_PGM_MODE_DONE,
	                         XNVM_BBRAM_STATUS_PGM_MODE_DONE,
	                         XNVM_BBRAM_PGM_MODE_TIMEOUT_VAL);
	if (Status != XST_SUCCESS) {
		Status = XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT;
	}

	return Status;
}

/******************************************************************************/
/**
 * Disables the programming mode of BBRAM controller.
 *
 * @param   None
 *
 * @return
 *          XST_SUCCESS	- Disabled programming mode
 *          XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT - On failure
 *          XST_FAILURE - Unexpected error
 *
 * @note    None.
 ******************************************************************************/
static inline void XNvm_BbramDisablePgmMode()
{
	XNvm_BbramWriteReg(XNVM_BBRAM_PGM_MODE_REG, 0x00U);
}

/******************************************************************************/
/**
 * Validates CRC of the key stored in BBRAM with CRC of input key.
 *
 * @param   Key  Pointer to key which is to be matched with key stored in BBRAM
 *
 * @return
 *          XST_SUCCESS	- CRC matched
 *          XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT - CRC validation check timed
 *                  out.
 *          XNVM_BBRAM_ERROR_AES_CRC_MISMATCH - CRC mismatch
 *          XST_FAILURE - Unexpected error
 *
 * @note    None.
 ******************************************************************************/
static u32 XNvm_BbramValidateAesKeyCrc(const u32* Key)
{
	u32 Crc;
	u32 Status = XST_FAILURE;
	u32 BbramStatus;

	Crc = XNvm_BbramAesCrcCalc(Key);
	XNvm_BbramWriteReg(XNVM_BBRAM_AES_CRC_REG, Crc);
	Status = Xil_WaitForEvent(XNVM_BBRAM_BASE_ADDR + XNVM_BBRAM_STATUS_REG,
	                         XNVM_BBRAM_STATUS_AES_CRC_DONE,
	                         XNVM_BBRAM_STATUS_AES_CRC_DONE,
	                         XNVM_BBRAM_AES_CRC_DONE_TIMEOUT_VAL);
	if (Status != XST_SUCCESS) {
		Status = XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT;
		goto END;
	}

	BbramStatus = XNvm_BbramReadReg(XNVM_BBRAM_STATUS_REG);

	if((BbramStatus & XNVM_BBRAM_STATUS_AES_CRC_PASS) != 0) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_BBRAM_ERROR_AES_CRC_MISMATCH;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * Calculates CRC value for each row of AES key.
 *
 * @param   PrevCRC - Holds the prev row's CRC
 * @param   Data    - Holds the present row's key
 * @param   Addr    - Stores the current row number
 *
 * @return  Crc of current row
 *
 * @note    None.
 *
 ******************************************************************************/
static u32 XNvm_BbramRowAesCrcCalc(u32 PrevCRC, u32 Data, u32 Addr)
{
	u32 Crc = PrevCRC;
	u32 Value = Data;
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
 * @param   Key - Pointer to the key for which CRC has to be calculated
 *
 * @return  CRC of AES key
 *
 * @note    None
 *
 ******************************************************************************/
static inline u32 XNvm_BbramAesCrcCalc(const u32 *Key)
{
	u32 Crc = 0U;
	u8 Idx;

	for (Idx = 0U; Idx < XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS ; Idx++) {
		Crc = XNvm_BbramRowAesCrcCalc(Crc,
		               Key[XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS - Idx - 1U],
		               XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS - Idx);
	}

	return Crc;
}
