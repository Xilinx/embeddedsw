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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xsecure_aes_ipihandler.h"
#include "xsecure_defs.h"
#include "xsecure_elliptic_ipihandler.h"
#include "xsecure_rsa_ipihandler.h"
#include "xsecure_sha_ipihandler.h"

/************************** Constant Definitions *****************************/
static XPlmi_Module XPlmi_Secure;
static XPlmi_ModuleCmd XSecure_Cmds[XSECURE_API_MAX];

static XPlmi_Module XPlmi_Secure =
{
	XPLMI_MODULE_XILSECURE_ID,
	XSecure_Cmds,
	XSECURE_API(XSECURE_API_MAX),
	NULL,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XSecure_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_SHA3_UPDATE):
	case XSECURE_API(XSECURE_API_SHA3_KAT):
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
		Status = XST_SUCCESS;
		break;
#endif
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

	switch (Cmd->CmdId & 0xFFU) {
	case XSECURE_API(XSECURE_API_FEATURES):
		Status = XSecure_FeaturesCmd(Pload[0]);
		break;
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_SHA3_UPDATE):
	case XSECURE_API(XSECURE_API_SHA3_KAT):
		Status = XSecure_Sha3IpiHandler(Cmd);
		break;
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
