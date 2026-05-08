/******************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/versal_2ve_2vm/xsecure_cmd.c
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
*       mb   04/17/26 Update XSecure_CryptoCheck API definition
* 5.7   tbk  02/05/26 Added support for getting crypto algorithm's version
*       tvp  04/29/26 Added IPI for HMAC
*       tvp  04/30/26 Routed XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN through
*                     XSecure_PlatEllipticIpiHandler
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
#include "xsecure_hmac_ipihandler.h"
#include "xsecure_cmd.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_cryptochk.h"
#include "xsecure_plat_aes_ipihandler.h"
#include "xil_cryptoalginfo.h"
#include "xsecure_server_aesalginfo.h"
#include "xsecure_server_rsaalginfo.h"
#include "xsecure_server_ellipticalginfo.h"
#include "xsecure_server_ecdhalginfo.h"
#include "xsecure_server_shaalginfo.h"
#include "xsecure_server_hmacalginfo.h"
#include "xsecure_server_lmsalginfo.h"
#include "xsecure_lms_ipihandler.h"
#include "xtrngpsx_alginfo.h"

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
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_LMS_SIGN_VERIFY),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_HMAC_OPERATION),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN),
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
 * 		or not and retrieves algorithm information.
 *
 * @param	Cmd	Pointer to the command structure containing API ID for
 *			algorithm information
 *
 * @return
 *		 - XST_SUCCESS - In case of success
 *		 - XST_INVALID_PARAM - In case of unsupported API ID or invalid parameters
 *		 - Error - On failure
 *
 *****************************************************************************/
static int XSecure_FeaturesCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_INVALID_PARAM;
	u32 ApiId;
	u32 AlgoVersion = 0U;
	u32 NistStatus = NIST_NON_COMPLIANT;

	/* Validate command pointer */
	if (Cmd == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Validate payload pointer */
	if (Cmd->Payload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	ApiId = Cmd->Payload[0U];

	switch (ApiId) {
	case XSECURE_API(XSECURE_API_SHA3_OPERATION):
	case XSECURE_API(XSECURE_API_SHA2_OPERATION):
		/* SHA2 and SHA3 share the same driver implementation as they are
		 * two hardware instances of the same SHA module */
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_SHA_MODULE_MAJOR_VERSION, XSECURE_SHA_MODULE_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
		break;
	case XSECURE_API(XSECURE_API_HMAC_OPERATION):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_HMAC_MODULE_MAJOR_VERSION, XSECURE_HMAC_MODULE_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
		break;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_RSA_MODULE_MAJOR_VERSION, XSECURE_RSA_MODULE_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
		break;
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_ELLIPTIC_MODULE_MAJOR_VERSION, XSECURE_ELLIPTIC_MODULE_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN):
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_ECDH_MODULE_MAJOR_VERSION, XSECURE_ECDH_MODULE_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
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
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_AES_MODULE_MAJOR_VERSION, XSECURE_AES_MODULE_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
		break;
	case XSECURE_API(XSECURE_API_TRNG_GENERATE):
		AlgoVersion = XIL_BUILD_VERSION(XTRNGPSX_MAJOR_VERSION, XTRNGPSX_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
		break;
	case XSECURE_API(XSECURE_API_KAT):
		break;
#endif
	case XSECURE_API(XSECURE_API_LMS_SIGN_VERIFY):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_LMS_MODULE_MAJOR_VERSION, XSECURE_LMS_MODULE_MINOR_VERSION);
		NistStatus = NIST_COMPLIANT;
		break;
	default:
		/* Skip CryptoCheck for unsupported APIs */
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Cmd not supported\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Run crypto environment check and preserve warning status if any */
	Status = XSecure_CryptoCheck(XSECURE_CORE_ALL);
	if (Status != XST_SUCCESS) {
		AlgoVersion = 0U;
		NistStatus = NIST_NON_COMPLIANT;
		Status |= (int)XPLMI_WARNING_MINOR_MASK;
		goto END;
	}

END:
	/* Fill the version in the response buffer */
	if (Cmd != NULL) {
		Cmd->Response[XSECURE_ALGO_VERSION_RESP_INDEX] = AlgoVersion;
		Cmd->Response[XSECURE_NIST_STATUS_RESP_INDEX] = NistStatus;
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
 *		 - XST_SUCCESS - Upon success
 *		 - XST_INVALID_PARAM - If any input parameter is invalid
 *		 - XSECURE_KAT_MAJOR_ERROR - If KAT fails
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
		Status = XSecure_FeaturesCmd(Cmd);
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
	case XSECURE_API(XSECURE_API_HMAC_OPERATION):
		Status = XSecure_HmacIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_KAT):
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK_FOR_INPRGRESS_STS(XSECURE_KAT_MAJOR_ERROR,
			                   Status, StatusTmp, XSecure_KatPlatIpiHandler, Cmd)
		break;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
	case XSECURE_API(XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN):
		Status = XSecure_PlatEllipticIpiHandler(Cmd);
		break;
#endif
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY):
		Status = XSecure_PlatAesIpiHandler(Cmd);
		break;
#endif
	case XSECURE_API(XSECURE_API_LMS_SIGN_VERIFY):
		Status = XSecure_LmsIpiHandler(Cmd);
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
 * @brief	This function registers the XilSecure commands to the PLMI.
 *
 *****************************************************************************/
void XSecure_CmdsInit(void)
{
	u32 Idx;

	/** - Register command handlers with XilPlmi */
	for (Idx = 0U; Idx < XPlmi_Secure.CmdCnt; Idx++) {
		XSecure_Cmds[Idx].Handler = XSecure_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Secure);

}
/** @} */
