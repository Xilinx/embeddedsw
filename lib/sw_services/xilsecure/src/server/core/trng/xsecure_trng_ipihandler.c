/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trng_ipihandler.c
*
* This file contains the xilsecure TRNG IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   kpt  05/15/2022 Initial release
*       kpt  07/24/2022 Moved XSecure_TrngKat to xsecure_kat_plat.c
* 5.2   yog  08/07/2023 Added a single function call using XSecure_GetRandomNum API
*                       to generate random number
*       am   08/23/2023 Replaced XPlmi_DmaXfr with XPlmi_MemCpy64
*       yog  09/04/2023 Replaced error code XSECURE_TRNG_INVALID_BUF_SIZE with
*                       XTRNGPSX_INVALID_BUF_SIZE
*       dd   10/11/23 MISRA-C violation Rule 8.13 fixed
*	ss   04/05/24 Fixed doxygen warnings
* 5.4   yog  04/29/24 Fixed doxygen grouping and doxygen warnings.
*       mb   07/31/2024 Added the check to validate Payload for NULL pointer
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_trng_server_apis Xilsecure TRNG Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_defs.h"
#include "xsecure_trng.h"
#include "xsecure_trng_ipihandler.h"
#include "xil_sutil.h"
#include "xsecure_init.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XSecure_TrngGenerateRandNum(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size);

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd	is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the handler execution is successful
 *		 - XST_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_TrngIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if (Cmd == NULL || Cmd->Payload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_TRNG_GENERATE):
		Status = XSecure_TrngGenerateRandNum(Pload[0], Pload[1], Pload[2]);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler extracts the payload params with respect
 * 		to XSECURE_API_TRNG_GENERATE IPI command and calls
 * 		XSecure_GetRandomNum server API to generate random number.
 *
 * @param	SrcAddrLow	Lower 32 bit address of the random
 * 				data buffer address.
 * @param	SrcAddrHigh	Higher 32 bit address of the random
 * 				data buffer address.
 * @param	Size		Number of random bytes needs to be generated.
 *
 * @return
 *		 - XST_SUCCESS  If the generate is successful
 *		 - XTRNGPSX_INVALID_BUF_SIZE  If input size is invalid.
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_TrngGenerateRandNum(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u64 RandAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u8 RandBuf[XTRNGPSX_SEC_STRENGTH_IN_BYTES] = {0U};

	if (Size > XTRNGPSX_SEC_STRENGTH_IN_BYTES) {
		Status = (int)XTRNGPSX_INVALID_BUF_SIZE;
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_GetRandomNum, RandBuf, XTRNGPSX_SEC_STRENGTH_IN_BYTES);

	Status = XPlmi_MemCpy64(RandAddr, (u64)(UINTPTR)RandBuf, Size);

END:
	return Status;
}
/** @} */
