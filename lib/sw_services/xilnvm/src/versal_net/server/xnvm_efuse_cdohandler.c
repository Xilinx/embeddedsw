/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_efuse_cdohandler.c
* @addtogroup xnvm_apis XilNvm Versal_Net eFuse APIs
* @{
* @cond xnvm_internal
* This file contains the Versal_Net XilNvm EFUSE CDO Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   12/07/2022 Initial release
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
#include "xnvm_efuse.h"
#include "xnvm_efuse_cdohandler.h"
#include "xnvm_defs.h"
#include "xnvm_utils.h"

/************************** Constant Definitions *****************************/
#define XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK 	(0x0000FFFFU)

/************************** Function Prototypes *****************************/
static int XNvm_EfuseWriteAesKeyFromCdoPload(u32 *Pload);
static int XNvm_EfuseWritePpkHashFromCdoPload(u32 *Pload);
static int XNvm_EfuseWriteIvFromCdoPload(u32 *Pload);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseCdoHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEY_FROM_PLOAD):
		Status = XNvm_EfuseWriteAesKeyFromCdoPload(Pload);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH_FROM_PLOAD):
		Status = XNvm_EfuseWritePpkHashFromCdoPload(Pload);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV_FROM_PLOAD):
		Status = XNvm_EfuseWriteIvFromCdoPload(Pload);
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
 * @brief       This function programs Efuse Aes key from the CDO
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteAesKeyFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	XNvm_AesKeyType KeyType = (XNvm_AesKeyType)(Pload[0U] &
					XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK);

	Status = XNvm_EfuseWriteAesKey(KeyType, (XNvm_AesKey *)&Pload[1U]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Efuse PpkHash from the CDO
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePpkHashFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	XNvm_PpkType HashType = (XNvm_PpkType)(Pload[0U] &
					XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK);

	Status = XNvm_EfuseWritePpkHash(HashType, (XNvm_PpkHash *)&Pload[1U]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Efuse IV from the CDO
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteIvFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	XNvm_IvType IvType = (XNvm_IvType)Pload[0U];

	Status = XNvm_EfuseWriteIv(IvType, (XNvm_Iv *)&Pload[1U]);

	return Status;
}

#endif
