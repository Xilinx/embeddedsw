/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xsecure_error.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xsecure_aes_ipihandler.h"
#include "xsecure_defs.h"
#include "xsecure_elliptic_ipihandler.h"
#include "xsecure_rsa_ipihandler.h"
#include "xsecure_sha_ipihandler.h"
#include "xsecure_cmd.h"

/************************** Function Prototypes ******************************/
static int XSecure_CheckIpiAccess(u32 CmdId, u32 IpiReqType);

/************************** Constant Definitions *****************************/
static XPlmi_Module XPlmi_Secure;
static XPlmi_ModuleCmd XSecure_Cmds[XSECURE_API_MAX];

static XPlmi_Module XPlmi_Secure =
{
	XPLMI_MODULE_XILSECURE_ID,
	XSecure_Cmds,
	XSECURE_API(XSECURE_API_MAX),
	XSecure_CheckIpiAccess,
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
	case XSECURE_API(XSECURE_API_SHA3_KAT):
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
	case XSECURE_API(XSECURE_API_RSA_KAT):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_KAT):
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
	case XSECURE_API(XSECURE_API_AES_DECRYPT_KAT):
	case XSECURE_API(XSECURE_API_AES_DECRYPT_CM_KAT):
#endif
		Status = XST_SUCCESS;
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
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_FEATURES):
		Status = XSecure_FeaturesCmd(Pload[0]);
		break;
	case XSECURE_API(XSECURE_API_SHA3_UPDATE):
	case XSECURE_API(XSECURE_API_SHA3_KAT):
		Status = XSecure_Sha3IpiHandler(Cmd);
		break;
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
	case XSECURE_API(XSECURE_API_RSA_KAT):
		Status = XSecure_RsaIpiHandler(Cmd);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
	case XSECURE_API(XSECURE_API_ELLIPTIC_KAT):
		Status = XSecure_EllipticIpiHandler(Cmd);
		break;
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
	case XSECURE_API(XSECURE_API_AES_DECRYPT_KAT):
	case XSECURE_API(XSECURE_API_AES_DECRYPT_CM_KAT):
		Status = XSecure_AesIpiHandler(Cmd);
		break;
#endif
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

/*****************************************************************************/
/**
 * @brief	This function checks if the IPI command is accessible or not
 *
 * @param	CmdId - Not used in the function
 * @param	IpiReqType is the IPI command request type
 *
 * @return	XST_SUCCESS on success
 *              XSECURE_IPI_ACCESS_NOT_ALLOWED on failure
 *
 * @note	By default, only secure IPI requests are supported for Xilsecure
 *              client APIs. Non-secure IPI requests are supported only if
 *              it is enabled in BSP by user.
 *
 *****************************************************************************/
static int XSecure_CheckIpiAccess(u32 CmdId, u32 IpiReqType)
{
	int Status = XST_FAILURE;
	u8 NonSecureIpiAccess;
	(void)CmdId;

#ifndef XSECURE_NONSECURE_IPI_ACCESS
	NonSecureIpiAccess = FALSE;
#else
	NonSecureIpiAccess = TRUE;
#endif

	if ((NonSecureIpiAccess == FALSE) && (IpiReqType == XPLMI_CMD_NON_SECURE)) {
		Status = XSECURE_IPI_ACCESS_NOT_ALLOWED;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
