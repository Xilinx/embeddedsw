/***************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/versal_2vp/xsecure_cmd.c
*
* This file contains the xilsecure IPI handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.6   tvp  07/07/25 Initial release
* 5.7   tvp  11/18/25 Added support for generating shared secret
*       tvp  11/18/25 Added support for trng operations
*       mb   04/17/26 Update XSecure_CryptoCheck API definition
* 5.8   tbk  06/17/26 Added support for getting crypto algorithm's version
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
/*************************************** Include Files ********************************************/
#include "xplmi_config.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_error.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xsecure_aes_ipihandler.h"
#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_elliptic_ipihandler.h"
#include "xsecure_plat_elliptic_ipihandler.h"
#endif
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa_ipihandler.h"
#endif
#include "xsecure_sha_ipihandler.h"
#include "xsecure_plat_kat_ipihandler.h"
#include "xsecure_plat_ipihandler.h"
#include "xsecure_cmd.h"
#include "xsecure_cryptochk.h"
#include "xsecure_trng_ipihandler.h"
#include "xsecure_defs.h"
#include "xil_cryptoalginfo.h"
#include "xsecure_server_aesalginfo.h"
#include "xsecure_server_rsaalginfo.h"
#include "xsecure_server_ellipticalginfo.h"
#include "xsecure_server_shaalginfo.h"
#include "xsecure_server_trngalginfo.h"

#ifdef SDT
#include "xsecure_config.h"
#endif

/************************************ Function Prototypes *****************************************/
static int XSecure_InvalidCmdHandler(u32 *Payload, u32 *RespBuf);

/************************************ Constant Definitions ****************************************/
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
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_AES_PERFORM_OPERATION),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_GEN_SHARED_SECRET),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_TRNG_GENERATE),
	XPLMI_ALL_IPI_FULL_ACCESS(XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN),
};

static XPlmi_Module XPlmi_Secure =
{
	XPLMI_MODULE_XILSECURE_ID,
	XSecure_Cmds,
	XSECURE_API(XSECURE_API_MAX),
	XSecure_InvalidCmdHandler,
	XSecure_AccessPermBuff,
};

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function calls the handler for invalid commands.
 *
 * @param	Payload	is a pointer to IPI payload data.
 * @param	RespBuf	Buffer to store response of slaves.
 *
 * @return
 *		- XST_SUCCESS  On successful communication.
 *		- XPLMI_ERR_CMD_APIID  On unregistered command ID.
 *		- XPLMI_SSIT_INTR_NOT_ENABLED  If interrupts are not enabled.
 *
 **************************************************************************************************/
static int XSecure_InvalidCmdHandler(u32 *Payload, u32 *RespBuf)
{
	return XPlmi_SendIpiCmdToSlaveSlr(Payload, RespBuf);
}

/**************************************************************************************************/
/**
 * @brief	This function checks if a particular Secure API ID is supported and
 *		populates the response buffer with the corresponding crypto algorithm
 *		version and NIST compliance status.
 *
 * @param	Cmd	Pointer to the command structure.
 *
 * @return
 *		 - XST_SUCCESS - In case of success
 *		 - XST_INVALID_PARAM - In case of unsupported API ID or invalid parameters
 *		 - Error - On failure
 *
 **************************************************************************************************/
static int XSecure_FeaturesCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_INVALID_PARAM;
	u32 ApiId;
	u32 AlgoVersion = 0U;
	u32 NistStatus = (u32)NIST_NON_COMPLIANT;

	if (Cmd == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (Cmd->Payload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	ApiId = Cmd->Payload[0U];

	switch (ApiId) {
	case XSECURE_API(XSECURE_API_SHA3_UPDATE):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_SHA_MODULE_MAJOR_VERSION, XSECURE_SHA_MODULE_MINOR_VERSION);
		NistStatus = (u32)NIST_COMPLIANT;
		break;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_RSA_MODULE_MAJOR_VERSION, XSECURE_RSA_MODULE_MINOR_VERSION);
		NistStatus = (u32)NIST_COMPLIANT;
		break;
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN):
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
		/* ECDH shared secret generation uses the same ECC module */
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_ELLIPTIC_MODULE_MAJOR_VERSION, XSECURE_ELLIPTIC_MODULE_MINOR_VERSION);
		NistStatus = (u32)NIST_COMPLIANT;
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
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_AES_MODULE_MAJOR_VERSION, XSECURE_AES_MODULE_MINOR_VERSION);
		NistStatus = (u32)NIST_COMPLIANT;
		break;
	case XSECURE_API(XSECURE_API_TRNG_GENERATE):
		AlgoVersion = XIL_BUILD_VERSION(XSECURE_TRNG_MODULE_MAJOR_VERSION, XSECURE_TRNG_MODULE_MINOR_VERSION);
		NistStatus = (u32)NIST_COMPLIANT;
		break;
	case XSECURE_API(XSECURE_API_KAT):
		break;
#endif
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Cmd not supported\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Run crypto environment check and preserve warning status if any */
	Status = XSecure_CryptoCheck(XSECURE_CORE_ALL);
	if (Status != XST_SUCCESS) {
		AlgoVersion = 0U;
		NistStatus = (u32)NIST_NON_COMPLIANT;
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
/**************************************************************************************************/
/**
 * @brief	This function processes XilSecure IPI commands.
 *
 * @param	Cmd	Pointer to the command structure.
 *
 * @return
 *		- XST_SUCCESS  Upon success.
 *		- XST_INVALID_PARAM  If any input parameter is invalid.
 *		- XSECURE_KAT_MAJOR_ERROR  If KAT fails.
 *
 **************************************************************************************************/
static int XSecure_ProcessCmd(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;

	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Call the respective API handler according to API ID */
	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_FEATURES):
		/**   - XSecure_FeaturesCmd */
		Status = XSecure_FeaturesCmd(Cmd);
		break;
	case XSECURE_API(XSECURE_API_SHA3_UPDATE):
		/**    - XSecure_Sha3IpiHandler */
		Status = XSecure_Sha3IpiHandler(Cmd);
		break;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
		/**   - @ref XSecure_RsaIpiHandler */
		Status = XSecure_RsaIpiHandler(Cmd);
		break;
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
		/**   - @ref XSecure_EllipticIpiHandler */
		Status = XSecure_EllipticIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN):
	case XSECURE_API(XSECURE_API_GEN_SHARED_SECRET):
		Status = XSecure_PlatEllipticIpiHandler(Cmd);
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
		/**   - @ref XSecure_AesIpiHandler */
		Status = XSecure_AesIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_TRNG_GENERATE):
		Status = XSecure_TrngIpiHandler(Cmd);
		break;
#endif
	case XSECURE_API(XSECURE_API_KAT):
		/**   - @ref XSecure_KatIpiHandler */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK_FOR_INPRGRESS_STS(XSECURE_KAT_MAJOR_ERROR,
			Status, StatusTmp, XSecure_KatPlatIpiHandler, Cmd)
		break;
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
		/**   - @ref XSecure_PlatIpiHandler */
		Status = XSecure_PlatIpiHandler(Cmd);
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

/**************************************************************************************************/
/**
 * @brief	This function registers the XilSecure commands to the PLMI.
 *
 **************************************************************************************************/
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
