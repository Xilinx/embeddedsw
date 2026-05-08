/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/ecc_keypair/xsecure_plat_elliptic_ipihandler.c
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
* 5.7   tvp  04/30/26 Added XSecure_EllipticPrivateKeyGen IPI handler
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
#include "xsecure_elliptic.h"
#include "xsecure_error.h"
#include "xplmi_plat.h"
#include "xil_sutil.h"

/************************** Constant Definitions *****************************/
#define XSECURE_ECC_SEED_BUF_LEN	(64U)  /**< ECC key generation seed
						 buffer size in bytes */
#define XSECURE_ECC_SEED_LEN		(48U)  /**< ECC key generation seed and
						 personalization string length */

/************************** Function Prototypes *****************************/

static int XSecure_GenSharedSecret(u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_EllipticPrivateKeyGen(u32 SubsystemId, u32 SrcAddrLow,
		u32 SrcAddrHigh, u32 ParamsSize);

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param	Cmd	is a pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS - If the handler execution is successful
 *		 - XST_INVALID_PARAM - If any parameter is invalid.
 *		 - XST_FAILURE - If there is a failure
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
	case XSECURE_API(XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN):
		Status = XSecure_EllipticPrivateKeyGen(Cmd->SubsystemId,
				Pload[0U], Pload[1U], Pload[2U]);
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
 * @param	SrcAddrLow	Lower 32 bit address of the EcdhParams structure from client
 * @param	SrcAddrHigh	Upper 32 bit address of the EcdhParams structure from client
 *
 * @return
 *		 - XST_SUCCESS - On Success
 *		 - XST_FAILURE - On failure
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

/*****************************************************************************/
/**
 * @brief	This function performs Elliptic private key generation.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	SrcAddrLow	Lower 32 bit address of the
 *				XSecure_EccPrivateKeyParams structure.
 * @param	SrcAddrHigh	Higher 32 bit address of the
 *				XSecure_EccPrivateKeyParams structure.
 * @param	ParamsSize	Size in bytes of the XSecure_EccPrivateKeyParams
 *				structure as reported by the client.
 *
 * @return
 *		 - XST_SUCCESS  On Success.
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  If ParamsSize is invalid.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_EllipticPrivateKeyGen(u32 SubsystemId, u32 SrcAddrLow,
		u32 SrcAddrHigh, u32 ParamsSize)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;
	XSecure_ElliptcPrivateKeyGen EccPvtKeyGen;
	XSecure_EccPrivateKeyParams Params;
	u8 Seed[XSECURE_ECC_SEED_BUF_LEN];
	u8 PersString[XSECURE_ECC_SEED_LEN];
	u8 PrivKeyBuf[XSECURE_ECC_P384_SIZE_IN_BYTES];
	u64 ParamAddr = (((u64)SrcAddrHigh << XSECURE_ADDR_HIGH_SHIFT) |
			 (u64)SrcAddrLow);

	/**
	 * - Validate the size passed by the client matches the expected
	 *   parameter structure size and is 8-byte aligned
	 */
	if ((ParamsSize != (u32)sizeof(XSecure_EccPrivateKeyParams)) ||
	    ((ParamsSize & XSECURE_IPI_PARAMS_SIZE_ALIGN_MASK) != 0U)) {
		Status = XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ParamAddr,
				sizeof(XSecure_EccPrivateKeyParams), Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);

	Xil_MemCpyFrom64To32Addr((u32)(UINTPTR)&Params, ParamAddr,
				 (u32)sizeof(XSecure_EccPrivateKeyParams));

	/**
	 * - Validate that the private key length reported by the client matches
	 *   the only currently supported ECC P-384 key size
	 */
	if (Params.PrivKeyLen != XSECURE_ECC_P384_SIZE_IN_BYTES) {
		Status = XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Validate output address for the generated private key using the
	 *   client-provided private key length
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Params.PrivKeyAddr,
				Params.PrivKeyLen, Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);

	Status = XSecure_GetRandomNum(Seed, XSECURE_ECC_SEED_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_GetRandomNum(PersString, XSECURE_ECC_SEED_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EccPvtKeyGen.SeedAddr = (u32)(UINTPTR)Seed;
	EccPvtKeyGen.SeedLength = XSECURE_ECC_SEED_LEN;
	EccPvtKeyGen.PerStringAddr = (u32)(UINTPTR)PersString;
	EccPvtKeyGen.KeyOutPutAddr = (u32)(UINTPTR)PrivKeyBuf;
	Status = XSecure_EllipticPrvtKeyGenerate(XSECURE_ECC_NIST_P384,
						 &EccPvtKeyGen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Copy the generated private key from the local buffer to the
	 *   client-supplied 64-bit destination address
	 */
	Xil_MemCpyFrom32To64Addr(Params.PrivKeyAddr, (u32)(UINTPTR)PrivKeyBuf,
				 Params.PrivKeyLen);

END:
	/**
	 * - Securely zeroize Seed and PersString to prevent key-generation
	 *   entropy from lingering on stack
	 */
	ClrStatus = Xil_SecureZeroize(Seed, sizeof(Seed));
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}

	ClrStatus = Xil_SecureZeroize(PersString, sizeof(PersString));
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}

	/**
	 * - Securely zeroize the local private key buffer to prevent the
	 *   generated key from lingering on stack after the operation
	 */
	ClrStatus = Xil_SecureZeroize(PrivKeyBuf, sizeof(PrivKeyBuf));
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}

	return Status;
}
#endif /* PLM_ECDSA_EXCLUDE */
/** @} */
