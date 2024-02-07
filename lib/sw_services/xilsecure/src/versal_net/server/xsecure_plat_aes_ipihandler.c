/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_aes_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal Net IPI Handler APIs
* @{
* @cond xsecure_internal
* This file contains the Xilsecure Versal Net IPI Handler APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 5.3   har  02/05/2024 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_plat_aes_ipihandler.h"
#include "xsecure_aes_ipihandler.h"
#include "xsecure_defs.h"
#include "xplmi_dma.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XSecure_AesPerformOperationAndZeroizeKey(u32 AesParamsAddrLow, u32 AesParamsAddrHigh, u32 KeyAddrLow, u32 KeyAddrHigh);

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the handler execution is successful
 *		 - ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XSecure_PlatAesIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY):
		/**   - @ref XSecure_AesPerformOperationAndZeroizeKey */
		Status = XSecure_AesPerformOperationAndZeroizeKey(Pload[0], Pload[1], Pload[2], Pload[3]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs below operation:
 *			- Write key into the key source provided by the user
 *			- Encrypt/decrypt a single block of data using the provided key
 *			- Zeroize the key once the AES operation is done
 *
 * @param	AesParamsAddrLow - Lower 32 bit address of the XSecure_AesDataBlockParams structure.
 * @param	AesParamsAddrHigh - Upper 32 bit address of the XSecure_AesDataBlockParams structure.
 * @param	KeyAddrLow - Lower 32 bit address of the buffer which stores the key
 * @param	KeyAddrHigh - Upper 32 bit address of the buffer which stores the key
 *
 * @return
 *      -       XST_SUCCESS - In case of success
 *      -       Error Code in case of failure
 *
 ******************************************************************************/
static int XSecure_AesPerformOperationAndZeroizeKey(u32 AesParamsAddrLow, u32 AesParamsAddrHigh, u32 KeyAddrLow, u32 KeyAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AesParamsAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)AesParamsAddrLow;
	XSecure_AesDataBlockParams AesParams;

	Status =  XPlmi_MemCpy64((u64)(UINTPTR)&AesParams, Addr, sizeof(AesParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END_KEY_CLR, Status, XSecure_AesKeyWrite, (u8)AesParams.KeySize,
		(u8)AesParams.KeySrc, KeyAddrLow, KeyAddrHigh);

	Status = XST_FAILURE;
	Status = XSecure_AesPerformOperation(AesParamsAddrLow, AesParamsAddrHigh);

END_KEY_CLR:
	Status = XST_FAILURE;
	Status = XSecure_AesKeyZeroize(AesParams.KeySrc);

END:
	return Status;
}