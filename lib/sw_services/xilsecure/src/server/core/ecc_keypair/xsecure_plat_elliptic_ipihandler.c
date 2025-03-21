/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_elliptic_ipihandler.c
*
* This file contains the Xilsecure versalnet IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 521   har  06/20/2023 Initial release
*       dd   10/11/23 MISRA-C violation Rule 8.13 fixed
* 5.4   yog  04/29/24 Fixed doxygen grouping and doxygen warnings.
*       mb   07/31/24 Added the check to validate Payload for NULL pointer
*       yog  03/18/25 Updated XSecure_GenSharedSecret() API
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_plat_elliptic_ipihandler.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_defs.h"
#include "xsecure_plat_defs.h"
#include "xsecure_ellipticplat.h"
#include "xplmi_dma.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/

static int XSecure_GenSharedSecret(u32 SrcAddrLow, u32 SrcAddrHigh);

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param	Cmd	is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the handler execution is successful
 *		 - XST_INVALID_PARAM  If any parameter is invalid.
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_PlatEllipticIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if (Cmd == NULL || Cmd->Payload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
		 Status = XSecure_GenSharedSecret(Pload[0U], Pload[1U]);
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
 * @brief	This function handler extracts the payload params with respect
 * 		to XSECURE_API_GEN_SHARED_SECRET IPI command and calls
 * 		XSecure_EcdhGetSecret server API to generate the shared secret
 * 		using ECDH.
 *
 * @param	SrcAddrLow	Lower 32 bit address of the EcdhParams struture from client
 * @param	SrcAddrHigh	Upper 32 bit address of the EcdhParams struture from client
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_GenSharedSecret(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	int Status = XST_FAILURE;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_EcdhParams EcdhParams;
	u64 PrvtKeyAddr = 0U;
	u64 PubKeyAddr = 0U;
	u64 SharedSecretAddr = 0U;

	Status = XPlmi_MemCpy64((UINTPTR)&EcdhParams, SrcAddr, sizeof(EcdhParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PrvtKeyAddr = ((u64)EcdhParams.PrivKeyAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)EcdhParams.PrivKeyAddrLow;
	PubKeyAddr = ((u64)EcdhParams.PubKeyAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)EcdhParams.PubKeyAddrLow;
	SharedSecretAddr = ((u64)EcdhParams.SharedSecretAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)EcdhParams.SharedSecretAddrLow;

	Status = XSecure_EcdhGetSecret((XSecure_EllipticCrvTyp)EcdhParams.CurveType, PrvtKeyAddr, PubKeyAddr,
		SharedSecretAddr);

END:
	return Status;
}
#endif /* PLM_ECDSA_EXCLUDE */
/** @} */
