/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_elliptic_ipihandler.c
* @addtogroup xsecure_apis XilSecure versal net platform handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure versalnet IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 521   har  06/20/2023 Initial release
*       dd   10/11/23 MISRA-C violation Rule 8.13 fixed
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_plat_elliptic_ipihandler.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_defs.h"
#include "xsecure_plat_defs.h"
#include "xsecure_ellipticplat.h"
#include "xplmi_dma.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/

static int XSecure_GenSharedSecret(u32 CrvType, u32 PrvtKeyAddrLow, u32 PrvtKeyAddrHigh, u32 PubKeyAddrLow,
	u32 PubKeyAddrHigh, u32 SharedSecretAddrLow, u32 SharedSecretAddrHigh);

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
int XSecure_PlatEllipticIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
		 Status = XSecure_GenSharedSecret(Pload[0U], Pload[1U], Pload[2U], Pload[3U],
			Pload[4U], Pload[5U], Pload[6U]);
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
 * @brief	This function handler calls XSecure_EcdhGetSecret server API to
 * 		generate the shared secret using ECDH.
 *
 * @param	CrvType - Type of elliptic curve
 * @param	PrvtKeyAddrLow - Lower 32 bit address of the private key buffer
 * @param	PrvtKeyAddrHigh - Upper 32 bit address of the private key buffer
 * @param	PubKeyAddrLow - Lower 32 bit address of the public key buffer
 * @param	PubKeyAddrHigh - Upper 32 bit address of the public key buffer
 * @param	SharedSecretAddrLow - Lower 32 bit address of the Shared Secret buffer
 * @param	SharedSecretAddrHigh - Upper 32 bit address of the Shared Secret buffer
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - Errorcode  On failure

 ******************************************************************************/
static int XSecure_GenSharedSecret(u32 CrvType, u32 PrvtKeyAddrLow, u32 PrvtKeyAddrHigh, u32 PubKeyAddrLow,
	u32 PubKeyAddrHigh, u32 SharedSecretAddrLow, u32 SharedSecretAddrHigh)
{
	int Status = XST_FAILURE;
	u64 PrvtKeyAddr = ((u64)PrvtKeyAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)PrvtKeyAddrLow;
	u64 PubKeyAddr = ((u64)PubKeyAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)PubKeyAddrLow;
	u64 SharedSecretAddr = ((u64)SharedSecretAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)SharedSecretAddrLow;

	Status = XSecure_EcdhGetSecret((XSecure_EllipticCrvTyp)CrvType, PrvtKeyAddr, PubKeyAddr,
		SharedSecretAddr);

	return Status;
}
#endif /* PLM_ECDSA_EXCLUDE */