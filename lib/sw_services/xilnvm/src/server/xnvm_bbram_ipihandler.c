/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_bbram_ipihandler.c
* @addtogroup xnvm_apis XilNvm Versal APIs
* @{
* @cond xnvm_internal
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
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xnvm_bbram.h"
#include "xnvm_bbram_ipihandler.h"
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
int XNvm_BbramIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_BBRAM_WRITE_AES_KEY):
		Status = XNvm_BbramKeyWrite(Pload[0], Pload[1], Pload[2]);
		break;
	case XNVM_API(XNVM_BBRAM_ZEROIZE):
		Status = XNvm_BbramClear();
		break;
	case XNVM_API(XNVM_BBRAM_WRITE_USER_DATA):
		Status = XNvm_BbramUsrDataWrite(Pload[0]);
		break;
	case XNVM_API(XNVM_BBRAM_READ_USER_DATA):
		Status = XNvm_BbramUsrDataRead(Pload[0], Pload[1]);
		break;
	case XNVM_API(XNVM_BBRAM_LOCK_WRITE_USER_DATA):
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
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u64 Addr = ((u64)KeyAddrHigh << 32U) | (u64)KeyAddrLow;
	u8 Key[XNVM_BBRAM_AES_KEY_SIZE];

	Status = XPlmi_MemSet((u64)(UINTPTR)&Key, 0U,
			(XNVM_BBRAM_AES_KEY_SIZE / XNVM_WORD_LEN));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)Key, Size / XNVM_WORD_LEN,
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_BbramWriteAesKey(Key, Size);

END:
	StatusTmp = XNvm_ZeroizeAndVerify((u8 *)Key, Size);
	if (Status == XST_SUCCESS) {
		Status = StatusTmp;
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
