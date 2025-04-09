/******************************************************************************
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_cmd.c
*
* This file contains the Xilsecure IPI handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*       sk   08/22/24 Added support for key transfer to ASU
*       am   02/24/25 Moved key transfer command to generic xilplmi IPI command
*       pre  03/02/25 Disabled XSecure_PlatAesIpiHandler for SECURE_EXCLUDE case
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
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
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_GEN_SHARED_SECRET),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_SHA3_OPERATION),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_SHA2_OPERATION),
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
 * @param	ApiId	API ID in the IPI request
 *
 * @return
 *		 - XST_SUCCESS  In case of success
 *		 - XST_INVALID_PARAM  In case of unsupported API ID
 *
 *****************************************************************************/
static int XSecure_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
	case XSECURE_API(XSECURE_API_SHA3_OPERATION):
	case XSECURE_API(XSECURE_API_SHA2_OPERATION):
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
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
 * @param	Cmd	Pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  Upon success
 *		 - XST_INVALID_PARAM  If any input parameter is invalid
 *		 - XSECURE_KAT_MAJOR_ERROR  If KAT fails
 *
 *****************************************************************************/
static int XSecure_ProcessCmd(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	const u32 *Pload;


	if (Cmd == NULL || Cmd->Payload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_FEATURES):
		Status = XSecure_FeaturesCmd(Pload[0]);
		break;
	case XSECURE_API(XSECURE_API_SHA3_OPERATION):
	case XSECURE_API(XSECURE_API_SHA2_OPERATION):
		Status = XSecure_ShaIpiHandler(Cmd);
		break;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
		Status = XSecure_RsaIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
		Status = XSecure_PlatIpiHandler(Cmd);
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
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK_FOR_INPRGRESS_STS(XSECURE_KAT_MAJOR_ERROR,
			                   Status, StatusTmp, XSecure_KatPlatIpiHandler, Cmd)
		break;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
		Status = XSecure_PlatEllipticIpiHandler(Cmd);
		break;
#endif
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY):
		Status = XSecure_PlatAesIpiHandler(Cmd);
		break;
#endif
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
