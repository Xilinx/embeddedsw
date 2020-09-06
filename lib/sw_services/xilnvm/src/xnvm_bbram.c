/*******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
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
* 2.0   kal  11/14/2019 Added error check when BBRAM keylen is not 128 or 256.
* 2.1	am   08/19/2020 Resolved MISRA C violations.
*	kal  09/03/2020 Fixed Security CoE review comments
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
static int XNvm_BbramEnablePgmMode(void);
static inline int XNvm_BbramDisablePgmMode(void);
static int XNvm_BbramValidateAesKeyCrc(const u32* Key);

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief	Validates CRC of the key stored in BBRAM with CRC of input key.
 *
 * @param   Key - Pointer to key which is to be matched with key stored in BBRAM
 * @param   KeyLen - XNVM_256_BITS_AES_KEY_LEN_IN_BYTES for 256-bit AES key
 *
 * @return - XST_SUCCESS -Key is written to BBRAM.
 * 	   - XST_INVALID_PARAM -Invalid parameter passed.
 *         - XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT -Timeout during enabling
 *							programming mode.
 *         - XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT -Timeout during disabling
 *							programming mode.
 *         - XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT -CRC validation check
 *							timed out.
 *         - XNVM_BBRAM_ERROR_AES_CRC_MISMATCH	-CRC mismatch.
 *
 ******************************************************************************/
int XNvm_BbramWriteAesKey(const u8* Key, u16 KeyLen)
{
	u32 Status = XST_FAILURE;
	const u32 *AesKey = NULL;
	u32 BbramKeyAddr;
	u8 Idx;

	if ((KeyLen != XNVM_256_BITS_AES_KEY_LEN_IN_BYTES) ||
		(Key == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	AesKey = (const u32 *) Key;

	/*
	 * As per hardware design, zeroization is must between two BBRAM
	 * AES CRC Check requests
	 */
	Status = XNvm_BbramZeroize();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_BbramEnablePgmMode();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	BbramKeyAddr = XNVM_BBRAM_0_REG;
	for (Idx = 0U; Idx < XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS; Idx++) {

		XNvm_BbramWriteReg(BbramKeyAddr, AesKey[Idx]);
		BbramKeyAddr += sizeof(u32);
	}

	Status = XNvm_BbramValidateAesKeyCrc(AesKey);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_BbramDisablePgmMode();
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Locks the User data written in BBRAM i.e. Make User data written
 * 			in BBRAM as read only.
 *
 * @return - XST_SUCCESS - Locked BBRAM User data for write.
 *         - XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE - User data locked for write.
 *
 ******************************************************************************/
int XNvm_BbramLockUsrDataWrite(void)
{
	int Status = XST_FAILURE;
	u32 LockStatus = 0U;

	XNvm_BbramWriteReg(XNVM_BBRAM_MSW_LOCK_REG, XNVM_BBRAM_MSW_LOCK);
	LockStatus = XNvm_BbramReadReg(XNVM_BBRAM_MSW_LOCK_REG);
	if((LockStatus & XNVM_BBRAM_MSW_LOCK) == XNVM_BBRAM_MSW_LOCK) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Writes user provided 32-bit data to BBRAM.
 *
 * @param   UsrData - 32-bit user data to be written to BBRAM
 *
 * @return - XST_SUCCESS - User data written to BBRAM
 *         - XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED - User data locked for
 *							write.
 *
 ******************************************************************************/
int XNvm_BbramWriteUsrData(u32 UsrData)
{
	int Status = XST_FAILURE;
	u32 LockStatus;
	u32 ReadReg;

	LockStatus = XNvm_BbramReadReg(XNVM_BBRAM_MSW_LOCK_REG);

	if((LockStatus & XNVM_BBRAM_MSW_LOCK) == XNVM_BBRAM_MSW_LOCK) {
		Status = XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED;
	}
	else {
		XNvm_BbramWriteReg(XNVM_BBRAM_8_REG, UsrData);
		ReadReg = XNvm_BbramReadReg(XNVM_BBRAM_8_REG);
		if (ReadReg == UsrData) {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Reads 32-bit user data from BBRAM.
 *
 * @param   None
 *
 * @return  32-bit user data stored in BBRAM
 *
 * @note    None.
 ******************************************************************************/
inline u32 XNvm_BbramReadUsrData(void)
{
	return XNvm_BbramReadReg(XNVM_BBRAM_8_REG);
}

/******************************************************************************/
/**
 * @brief	Zeroize the BBRAM.
 *
 * @param   None
 *
 * @return - XST_SUCCESS - Zeroization of BBRAM done.
 *         - XNVM_BBRAM_ERROR_ZEROIZE_TIMEOUT - Timed out during BBRAM
 *						zeroization
 *
 ******************************************************************************/
int XNvm_BbramZeroize(void)
{
	int Status = XST_FAILURE;

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
 * @brief	Enabling  the BBRAM controller in programming mode.
 *
 * @return - XST_SUCCESS - Enabled programming mode.
 *         - XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT - On failure.
 *
 ******************************************************************************/
static int XNvm_BbramEnablePgmMode(void)
{
	int Status = XST_FAILURE;

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
 * @brief	Disables the programming mode of BBRAM controller.
 *
 *
 ******************************************************************************/
static inline int XNvm_BbramDisablePgmMode(void)
{
	u32 Status = XST_FAILURE;
	u32 ReadReg = XNvm_BbramReadReg(XNVM_BBRAM_PGM_MODE_REG);

	XNvm_BbramWriteReg(XNVM_BBRAM_PGM_MODE_REG, 0x00U);
	ReadReg = XNvm_BbramReadReg(XNVM_BBRAM_PGM_MODE_REG);
	if (ReadReg == 0x00U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validates CRC of the key stored in BBRAM with CRC of input key.
 *
 * @param   Key - Pointer to key which is to be matched with key
 *				  stored in BBRAM.
 *
 * @return - XST_SUCCESS - CRC matched
 *         - XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT - CRC validation check timed
 *               					out.
 *         - XNVM_BBRAM_ERROR_AES_CRC_MISMATCH     - CRC mismatch
 *
 ******************************************************************************/
static int XNvm_BbramValidateAesKeyCrc(const u32* Key)
{
	int Status = XST_FAILURE;
	u32 Crc;
	u32 BbramStatus;

	Crc = XNvm_AesCrcCalc(Key);
	XNvm_BbramWriteReg(XNVM_BBRAM_AES_CRC_REG, Crc);
	Status = Xil_WaitForEvent(XNVM_BBRAM_BASE_ADDR + XNVM_BBRAM_STATUS_REG,
	                         XNVM_BBRAM_STATUS_AES_CRC_DONE,
	                         XNVM_BBRAM_STATUS_AES_CRC_DONE,
	                         XNVM_BBRAM_AES_CRC_DONE_TIMEOUT_VAL);
	if (Status != XST_SUCCESS) {
		Status = XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT;
		goto END;
	}

	Status = XST_FAILURE;

	BbramStatus = XNvm_BbramReadReg(XNVM_BBRAM_STATUS_REG);

	if((BbramStatus & XNVM_BBRAM_STATUS_AES_CRC_PASS)
			== XNVM_BBRAM_STATUS_AES_CRC_PASS) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_BBRAM_ERROR_AES_CRC_MISMATCH;
	}

END:
	return Status;
}
