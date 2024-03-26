/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_cmd.c
*
* This file contains the xilsecure IPI handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/2021 Initial release
*       har  05/18/2021 Added support for secure access control for Xilsecure
*                       IPI calls
* 4.6   har  07/14/2021 Fixed doxygen warnings
*       kal  08/16/2021 Fixed magic number usage comment and fixed bug in
*                       XSecure_FeaturesCmd API
*       rb   08/11/2021 Fix compilation warnings
* 5.0   kpt  07/24/2022 Added XSecure_KatIpiHandler and support to go into
*                       secure lockdown when KAT fails
* 5.1   skg  10/17/2022 Added Null to invalid command handler of secure module
* 5.2   bm   06/23/2023 Added access permissions for IPI commands
*       bm   07/05/2023 Added crypto check in features command
*       vns  07/07/2023 Added separate IPI commands for Crypto Status and KAT status updates
*       kpt  07/10/2023 Added support for key wrap and unwrap
*       ng   07/13/2023 Added support for system device tree flow
*       dd   10/11/2023 MISRA-C violation Rule 10.4 fixed
*       dd   10/11/2023 MISRA-C violation Rule 8.13 fixed
* 5.3   har  02/06/2024 Added support for AES operation and zeroize key
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xsecure_error.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xsecure_aes_ipihandler.h"
#include "xsecure_defs.h"
#include "xsecure_elliptic_ipihandler.h"
#include "xsecure_rsa_ipihandler.h"
#include "xsecure_sha_ipihandler.h"
#include "xsecure_trng_ipihandler.h"
#include "xsecure_plat_elliptic_ipihandler.h"
#include "xsecure_plat_kat_ipihandler.h"
#include "xsecure_plat_ipihandler.h"
#include "xsecure_cmd.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_cryptochk.h"
#include "xsecure_plat_aes_ipihandler.h"

#ifdef SDT
#include "xsecure_config.h"
#endif

/************************** Function Prototypes ******************************/

/************************** Constant Definitions *****************************/
static XPlmi_Module XPlmi_Secure;
static XPlmi_ModuleCmd XSecure_Cmds[XSECURE_API_MAX];

/* Buffer holding access permissions of secure module commands */
static XPlmi_AccessPerm_t XSecure_AccessPermBuff[XSECURE_API_MAX] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_FEATURES),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_RSA_SIGN_VERIFY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_RSA_PUBLIC_ENCRYPT),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_RSA_PRIVATE_DECRYPT),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_SHA3_UPDATE),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_ELLIPTIC_GENERATE_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_ELLIPTIC_GENERATE_SIGN),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_ELLIPTIC_VALIDATE_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_ELLIPTIC_VERIFY_SIGN),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_INIT),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_OP_INIT),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_UPDATE_AAD),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_ENCRYPT_UPDATE),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_ENCRYPT_FINAL),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_DECRYPT_UPDATE),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_DECRYPT_FINAL),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_KEY_ZERO),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_WRITE_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_LOCK_USER_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_KEK_DECRYPT),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_SET_DPA_CM),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_KAT),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_TRNG_GENERATE),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_PERFORM_OPERATION),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_HNIC_CRYPTO_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_CPM5N_CRYPTO_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_PCIDE_CRYPTO_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_PKI_CRYPTO_STATUS),
	XPLMI_ALL_IPI_NO_ACCESS(XSECURE_API_UPDATE_DDR_KAT_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_HNIC_KAT_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_CPM5N_KAT_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_PCIDE_KAT_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_UPDATE_PKI_KAT_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_GEN_SHARED_SECRET),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_GET_KEY_WRAP_RSA_PUBLIC_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_KEY_UNWRAP),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY),
};

static XPlmi_Module XPlmi_Secure =
{
	XPLMI_MODULE_XILSECURE_ID,
	XSecure_Cmds,
	XSECURE_API(XSECURE_API_MAX),
	NULL,
	XSecure_AccessPermBuff,
	NULL,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
 * @brief	This function checks if a particular Secure API ID is supported
 * or not.
 *
 * @param	ApiId is API ID in the IPI request
 *
 * @return	XST_SUCCESS in case of success
 *		XST_INVALID_PARAM in case of unsupported API ID
 *
 *****************************************************************************/
static int XSecure_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
	case XSECURE_API(XSECURE_API_SHA3_UPDATE):
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
	case XSECURE_API(XSECURE_API_GET_KEY_WRAP_RSA_PUBLIC_KEY):
	case XSECURE_API(XSECURE_API_KEY_UNWRAP):
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
#endif
	case XSECURE_API(XSECURE_API_AES_INIT):
	case XSECURE_API(XSECURE_API_AES_OP_INIT):
	case XSECURE_API(XSECURE_API_AES_UPDATE_AAD):
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_UPDATE):
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_FINAL):
	case XSECURE_API(XSECURE_API_AES_DECRYPT_UPDATE):
	case XSECURE_API(XSECURE_API_AES_DECRYPT_FINAL):
	case XSECURE_API(XSECURE_API_AES_KEY_ZERO):
	case XSECURE_API(XSECURE_API_AES_WRITE_KEY):
	case XSECURE_API(XSECURE_API_AES_KEK_DECRYPT):
	case XSECURE_API(XSECURE_API_AES_SET_DPA_CM):
	case XSECURE_API(XSECURE_API_TRNG_GENERATE):
	case XSECURE_API(XSECURE_API_KAT):
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION):
	case XSECURE_API(XSECURE_API_UPDATE_DDR_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_HNIC_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_CPM5N_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PCIDE_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PKI_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_HNIC_CRYPTO_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_CPM5N_CRYPTO_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PCIDE_CRYPTO_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PKI_CRYPTO_STATUS):
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY):
#endif
		Status = XSecure_CryptoCheck();
		if (Status != XST_SUCCESS) {
			Status |= (int)XPLMI_WARNING_MINOR_MASK;
		}
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Cmd not supported\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;

}
/*****************************************************************************/
/**
 * @brief	This function processes XilSecure IPI commands
 *
 *****************************************************************************/
static int XSecure_ProcessCmd(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	const u32 *Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_FEATURES):
		Status = XSecure_FeaturesCmd(Pload[0]);
		break;
	case XSECURE_API(XSECURE_API_SHA3_UPDATE):
		Status = XSecure_Sha3IpiHandler(Cmd);
		break;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
		Status = XSecure_RsaIpiHandler(Cmd);
		break;
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
		Status = XSecure_EllipticIpiHandler(Cmd);
		break;
#endif
	case XSECURE_API(XSECURE_API_AES_INIT):
	case XSECURE_API(XSECURE_API_AES_OP_INIT):
	case XSECURE_API(XSECURE_API_AES_UPDATE_AAD):
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_UPDATE):
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_FINAL):
	case XSECURE_API(XSECURE_API_AES_DECRYPT_UPDATE):
	case XSECURE_API(XSECURE_API_AES_DECRYPT_FINAL):
	case XSECURE_API(XSECURE_API_AES_KEY_ZERO):
	case XSECURE_API(XSECURE_API_AES_WRITE_KEY):
	case XSECURE_API(XSECURE_API_AES_KEK_DECRYPT):
	case XSECURE_API(XSECURE_API_AES_SET_DPA_CM):
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION):
		Status = XSecure_AesIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_TRNG_GENERATE):
		Status = XSecure_TrngIpiHandler(Cmd);
		break;
#endif
	case XSECURE_API(XSECURE_API_KAT):
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XSECURE_KAT_MAJOR_ERROR, Status, StatusTmp, XSecure_KatPlatIpiHandler, Cmd)
		break;
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
		Status = XSecure_PlatEllipticIpiHandler(Cmd);
		break;
#endif
	case XSECURE_API(XSECURE_API_UPDATE_DDR_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_HNIC_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_CPM5N_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PCIDE_KAT_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PKI_KAT_STATUS):
		Status = XSecure_UpdateKatStatusIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_UPDATE_HNIC_CRYPTO_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_CPM5N_CRYPTO_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PCIDE_CRYPTO_STATUS):
	case XSECURE_API(XSECURE_API_UPDATE_PKI_CRYPTO_STATUS):
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_GET_KEY_WRAP_RSA_PUBLIC_KEY):
	case XSECURE_API(XSECURE_API_KEY_UNWRAP):
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
#endif
		Status = XSecure_PlatIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY):
		Status = XSecure_PlatAesIpiHandler(Cmd);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the XilSecure commands to the PLMI.
 *
 *****************************************************************************/
void XSecure_CmdsInit(void)
{
	u32 Idx;

	/* Register command handlers with XilPlmi */
	for (Idx = 0U; Idx < XPlmi_Secure.CmdCnt; Idx++) {
		XSecure_Cmds[Idx].Handler = XSecure_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Secure);

}
