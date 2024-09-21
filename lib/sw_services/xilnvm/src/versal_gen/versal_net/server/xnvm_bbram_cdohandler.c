/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_bbram_cdohandler.c
*
* This file contains the Versal_Net XilNvm BBRAM CDO Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 3.0   kal  07/12/2022 Initial release
* 3.1   skg  10/23/2022 Added In body comments for APIs
*
* </pre>
*
* @note
*
******************************************************************************/

/**
 * @addtogroup xnvm_bbram_server_apis XilNvm BBRAM Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xnvm_bbram.h"
#include "xnvm_bbram_cdohandler.h"
#include "xnvm_bbram_common_cdohandler.h"
#include "xnvm_defs.h"
#include "xnvm_utils.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XNvm_BbramWriteKeyFromCdoPload(u32 *Pload);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramCdoHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	if (NULL == Pload) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

        /**
	 *  Call the respective handler based on API ID.
	 */
	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_AES_KEY_FROM_PLOAD):
		Status = XNvm_BbramWriteKeyFromCdoPload(Pload);
		break;
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_AES_KEY):
	case XNVM_API(XNVM_API_ID_BBRAM_ZEROIZE):
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_USER_DATA):
	case XNVM_API(XNVM_API_ID_BBRAM_READ_USER_DATA):
	case XNVM_API(XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA):
#ifdef VERSAL_AIEPG2
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_CFG_LMT_PARAMS):
#endif
		Status = XNvm_BbramCommonCdoHandler(Cmd);
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
 * @brief       This function programs BBRAM Aes key from the CDO
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return
 * 		- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_BbramWriteKeyFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	u32 KeySize = Pload[0U] * XNVM_WORD_LEN;

	Status = XNvm_BbramWriteAesKey((u8 *)&Pload[1U], KeySize);

	return Status;
}

#endif
