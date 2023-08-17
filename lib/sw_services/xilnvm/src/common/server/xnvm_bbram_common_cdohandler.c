/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_bbram_common_cdohandler.c
*
* This file contains the XilNvm BBRAM IPI Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal   07/05/2021 Initial release
* 2.4   bsv   09/09/2021 Added PLM_NVM macro
* 2.5   kpt   12/06/2021 Avoid using DMA while writing 32bit user data
*                        in XNvm_BbramUsrDataRead
*       kpt   01/19/2022 Added redundancy for XNvm_ZeroizeAndVerify in
*                        XNvm_BbramKeyWrite
*       am    02/28/2022 Fixed MISRA C violation rule 4.5
* 3.0   dc    08/29/2022 Removed initialization
* 3.1   skg   10/23/2022 Added In body comments for APIs
*       kal   03/08/2023 Make status as volatile in XNvm_BbramKeyWrite
* 3.2   kpt   07/31/2023 Assign key clear status only when status is XST_SUCCESS
*	kpt   08/17/2023 Remove oring the Status with error code in XNvm_BbramKeyWrite
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xnvm_bbram.h"
#include "xnvm_bbram_common_cdohandler.h"
#include "xnvm_defs.h"
#include "xnvm_init.h"
#include "xnvm_utils.h"
#include "xplmi_dma.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/

static int XNvm_BbramKeyWrite(u32 Size, u32 KeyAddrLow, u32 KeyAddrHigh);
static int XNvm_BbramClear(void);
static int XNvm_BbramUsrDataWrite(u32 UsrData);
static int XNvm_BbramUsrDataRead(u32 DstAddrLow, u32 DstAddrHigh);
static int XNvm_BbramLockUsrData(void);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramCommonCdoHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

    /**
	 *  Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

   /**
    *  Calls the respected API handler according to API ID
	*/
	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_AES_KEY):
		Status = XNvm_BbramKeyWrite(Pload[0], Pload[1], Pload[2]);
		break;
	case XNVM_API(XNVM_API_ID_BBRAM_ZEROIZE):
		Status = XNvm_BbramClear();
		break;
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_USER_DATA):
		Status = XNvm_BbramUsrDataWrite(Pload[0]);
		break;
	case XNVM_API(XNVM_API_ID_BBRAM_READ_USER_DATA):
		Status = XNvm_BbramUsrDataRead(Pload[0], Pload[1]);
		break;
	case XNVM_API(XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA):
		Status = XNvm_BbramLockUsrData();
		break;
	default:
		XNvm_Printf(XNVM_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs BBRAM AES key.
 *
 * @param 	Size		Size of the input data in bytes to be
 *				updated
 * 		KeyAddrLow	Lower 32 bit address of the key
 *		KeyAddrHigh	Higher 32 bit address of the key
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_BbramKeyWrite(u32 Size, u32 KeyAddrLow, u32 KeyAddrHigh)
{
	volatile int Status = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	u64 Addr = ((u64)KeyAddrHigh << 32U) | (u64)KeyAddrLow;
	u8 Key[XNVM_BBRAM_AES_KEY_SIZE];

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)Key, Size / XNVM_WORD_LEN,
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		Status = XNVM_BBRAM_ERROR_IN_DMA_XFER;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_BbramWriteAesKey(Key, Size);

END:
	ClearStatus = XNvm_ZeroizeAndVerify((u8 *)Key, Size);
	ClearStatusTmp = XNvm_ZeroizeAndVerify((u8 *)Key, Size);
	if (Status == XST_SUCCESS) {
		Status |= (ClearStatus | ClearStatusTmp);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function zeroizes BBRAM.
 *
 * @return	- XST_SUCCESS - If the zeroize is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_BbramClear(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_BbramZeroize();

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs BBRAM user data.
 *
 * @param 	UsrData		User data to be written to BBRAM
 *
 * @return	- XST_SUCCESS - If the write is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_BbramUsrDataWrite(u32 UsrData)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_BbramWriteUsrData(UsrData);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads BBRAM user data.
 *
 * @param	DstAddrLow	Lower 32 bit address of the destination
 * 				address to store the user data
 *		DstAddrHigh	Higher 32 bit address of the destination
 *				address to store the user data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_BbramUsrDataRead(u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	u32 UsrData;

	UsrData = XNvm_BbramReadUsrData();

	XPlmi_Out64(Addr, UsrData);

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function locks BBRAM user data.
 *
 * @return	- XST_SUCCESS - If the locking is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_BbramLockUsrData(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_BbramLockUsrDataWrite();

	return Status;
}

#endif
