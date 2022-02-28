/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_bbramclient.c
*
* This file contains the implementation of the client interface functions for
* BBRAM programming.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/21 Initial release
* 1.1   am   02/28/22 Fixed MISRA C violation rule 4.5
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xnvm_bbramclient.h"
#include "xnvm_defs.h"
#include "xnvm_ipi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to program BBRAM AES key
 *
 * @param	KeyAddr		Address of the key buffer where the key to
 * 				be programmed is stored
 *
 * @param	KeyLen		Size of the Aes key
 *
 * @return	- XST_SUCCESS - If the BBRAM programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramWriteAesKey(const u64 KeyAddr, const u32 KeyLen)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload3((u32)XNVM_API_ID_BBRAM_WRITE_AES_KEY,
			KeyLen, (u32)KeyAddr, (u32)(KeyAddr >> 32U));
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "BBRAM programming Failed \r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to zeroize the BBRAM
 *
 * @return	- XST_SUCCESS - If the BBRAM zeroize is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramZeroize(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload0((u32)XNVM_API_ID_BBRAM_ZEROIZE);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to write the user data into
 * 		BBRAM user data registers
 *
 * @param	UsrData		User data to be written to BBRAM
 *
 * @return	- XST_SUCCESS - If the BBRAM user data write successful
 *		- XST_FAILURE - If there is a failure
 *
 *****************************************************************************/
int XNvm_BbramWriteUsrData(const u32 UsrData)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload1((u32)XNVM_API_ID_BBRAM_WRITE_USER_DATA,
			UsrData);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read the BBRAM user data
 *
 * @param	OutDataAddr	Address of the output buffer to store the
 * 				BBRAM user data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramReadUsrData(const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_API_ID_BBRAM_READ_USER_DATA,
			(u32)OutDataAddr, (u32)(OutDataAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to lock the user data written
 * 		to BBRAM
 *
 * @return	- XST_SUCCESS - If the Locking is successful
 *		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramLockUsrDataWrite(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload0((u32)XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA);

	return Status;
}
