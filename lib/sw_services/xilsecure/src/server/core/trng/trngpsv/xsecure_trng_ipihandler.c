/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/trng/trngpsv/xsecure_trng_ipihandler.c
*
* This file contains the xilsecure TRNG IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  11/14/25 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_trng_server_apis XilSecure TRNG Server APIs
* @{
*/
/****************************************** Include Files *****************************************/
#include "xplmi_dma.h"
#include "xsecure_defs.h"
#include "xsecure_trng.h"
#include "xsecure_trng_ipihandler.h"
#include "xil_sutil.h"
#include "xsecure_init.h"

/*************************************** Constant Definitions *************************************/

static int XSecure_TrngGenerateRandNum(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size);

/*************************************** Function Definitions *************************************/

/**************************************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID.
 *
 * @param 	Cmd	Pointer to the command structure.
 *
 * @return
 *		 - XST_SUCCESS If the handler execution is successful.
 *		 - XST_INVALID_PARAM If any input parameter is invalid.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
int XSecure_TrngIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
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

/**************************************************************************************************/
/**
 * @brief	This function handler extracts the payload params with respect to
 * 		XSECURE_API_TRNG_GENERATE IPI command and calls XSecure_GetRandomNum server API to
 * 		generate random number.
 *
 * @param	SrcAddrLow	Lower 32 bit address of the random data buffer address.
 * @param	SrcAddrHigh	Higher 32 bit address of the random data buffer address.
 * @param	Size		Number of random bytes needs to be generated.
 *
 * @return
 *		 - XST_SUCCESS If the generate is successful.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
static int XSecure_TrngGenerateRandNum(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u64 RandAddr = ((u64)SrcAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)SrcAddrLow;
	u8 RandBuf[XTRNGPSV_SEC_STRENGTH_BYTES];

	Status = Xil_SMemSet(&RandBuf, sizeof(RandBuf), 0U, sizeof(RandBuf));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_GetRandomNum, RandBuf,
				XTRNGPSV_SEC_STRENGTH_BYTES);

	Status = XPlmi_MemCpy64(RandAddr, (u64)(UINTPTR)RandBuf, Size);

END:
	return Status;
}
/** @} */
