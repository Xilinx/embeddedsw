/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_cmd.c
*
* This file contains the xilocp IPI handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*       am   01/10/23 Added client side condition for dme ApiId
* 1.2   har  02/24/23 Updated the code to support modified command for GetUsrCfg
*       kal  05/28/23 Added SW PCR extend and logging functions
*       bm   06/23/23 Added access permissions for IPI commands
*       har  07/21/23 Add access permission for XOCP_API_GEN_SHARED_SECRET
* 1.3   tri  10/09/23 Added support to handle cdo chunk boundary
*       har  11/03/23 Moved handling of SW PCR Config CDO command from xocp_ipihandler.c to this file
*       am   01/31/24 Moved key Management operations under PLM_OCP_KEY_MNGMT macro
*       kpt  02/20/24 Added support to extend secure state into SWPCR
*       har  06/04/24 Added support to get Key Index as part of DevAkInput CDO command
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xocp_ipihandler.h"
#include "xocp_def.h"
#include "xocp_cmd.h"
#ifdef PLM_OCP_KEY_MNGMT
#include "xocp_keymgmt.h"
#include "xcert_genx509cert.h"
#endif
#include "xocp.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XOCP_CERT_USERIN_FIELD_MASK			(0x00FF0000U)
			/**< Mask for Field in Payload of Get Certificate User Cfg CDO command*/
#define XOCP_CERT_USERIN_FIELD_SHIFT			(16U)
			/**< Shift for Field in Payload of Get Certificate User Cfg CDO command*/
#define XOCP_CERT_USERIN_LEN_MASK			(0x0000FFFFU)
			/**< Mask for Length in Payload of Get Certificate User Cfg CDO command*/
#define XOCP_DEV_AK_INPUT_PLOAD_KEYIDX_INDEX			(1U + 12U)
		/**< Index of KeyIdx in Payload of DevAkInput comamnd
		 * Payload -> Subsystem ID (1 word) + Personlization string(12 words) + KeyIdx*/
#define XOCP_DEV_AK_INPUT_KEYIDX_MASK				(0x3U)
			/**< Mask for KeyIdx in Payload of DevAkInput CDO command */
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static int XOcp_SetSwPcrConfig(const XPlmi_Cmd *Cmd);
#ifdef PLM_OCP_KEY_MNGMT
static int XOcp_DevAkInput(const XPlmi_Cmd *Cmd);
static int XOcp_GetCertUserCfg(const XPlmi_Cmd *Cmd);
#endif

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
 * @brief   This function checks for the supported features based on the
 *          requested API ID
 *
 * @param   ApiId - ApiId to check the supported features
 *
 * @return
 *          -XST_SUCCESS - If the requested API ID is supported
 *          -XST_INVALID_PARAM - In case of unsupported API ID
 *
 *****************************************************************************/
static int XOcp_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
		case XOCP_API(XOCP_API_EXTEND_HWPCR):
		case XOCP_API(XOCP_API_GET_HWPCR):
		case XOCP_API(XOCP_API_GET_HWPCRLOG):
		case XOCP_API(XOCP_API_GENDMERESP):
#ifdef PLM_OCP_KEY_MNGMT
		case XOCP_API(XOCP_API_DEVAKINPUT):
		case XOCP_API(XOCP_API_GETCERTUSERCFG):
		case XOCP_API(XOCP_API_GETX509CERT):
		case XOCP_API(XOCP_API_ATTESTWITHDEVAK):
		case XOCP_API(XOCP_API_GEN_SHARED_SECRET):
#ifndef PLM_RSA_EXCLUDE
		case XOCP_API(XOCP_API_ATTEST_WITH_KEYWRAP_DEVAK):
#endif
#endif
		case XOCP_API(XOCP_API_SET_SWPCRCONFIG):
		case XOCP_API(XOCP_API_EXTEND_SWPCR):
		case XOCP_API(XOCP_API_GET_SWPCR):
		case XOCP_API(XOCP_API_GET_SWPCRLOG):
		case XOCP_API(XOCP_API_GET_SWPCRDATA):
			Status = XST_SUCCESS;
			break;
		default:
			XOcp_Printf(DEBUG_GENERAL, "Cmd not supported\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function processes XilOcp commands coming via CDO and IPI.
 *
 * @param   Cmd - Pointer to the XPlmi_Cmd structure
 *
 * @return
 *          - XST_SUCCESS - On successful processing of command
 *          - XST_INVALID_PARAM - On invalid command
 *          - Error Code - On Failure
 *
 *****************************************************************************/
static int XOcp_ProcessCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if (Cmd == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & XOCP_API_ID_MASK) {
		case XOCP_API(XOCP_API_FEATURES):
			Status = XOcp_FeaturesCmd(Pload[0]);
			break;
		case XOCP_API(XOCP_API_EXTEND_HWPCR):
		case XOCP_API(XOCP_API_GET_HWPCR):
		case XOCP_API(XOCP_API_GET_HWPCRLOG):
		case XOCP_API(XOCP_API_GENDMERESP):
#ifdef PLM_OCP_KEY_MNGMT
		case XOCP_API(XOCP_API_GETX509CERT):
		case XOCP_API(XOCP_API_ATTESTWITHDEVAK):
		case XOCP_API(XOCP_API_GEN_SHARED_SECRET):
#ifndef PLM_RSA_EXCLUDE
		case XOCP_API(XOCP_API_ATTEST_WITH_KEYWRAP_DEVAK):
#endif
#endif
		case XOCP_API(XOCP_API_EXTEND_SWPCR):
		case XOCP_API(XOCP_API_GET_SWPCR):
		case XOCP_API(XOCP_API_GET_SWPCRLOG):
		case XOCP_API(XOCP_API_GET_SWPCRDATA):
			Status = XOcp_IpiHandler(Cmd);
			break;
		case XOCP_API(XOCP_API_SET_SWPCRCONFIG):
			Status = XOcp_SetSwPcrConfig(Cmd);
			break;
#ifdef PLM_OCP_KEY_MNGMT
		case XOCP_API(XOCP_API_DEVAKINPUT):
			Status = XOcp_DevAkInput(Cmd);
			break;
		case XOCP_API(XOCP_API_GETCERTUSERCFG):
			Status = XOcp_GetCertUserCfg(Cmd);
			break;
#else
		case XOCP_API(XOCP_API_DEVAKINPUT):
		case XOCP_API(XOCP_API_GETCERTUSERCFG):
			XOcp_Printf(DEBUG_GENERAL, "CMD: Key Management Operaion is disabled\r\n");
			Status = XOCP_ERR_KEY_MANAGEMENT_NOT_ENABLED;
			break;
#endif
		default:
			XOcp_Printf(DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function registers the XilOcp commands to the PLMI.
 *
 *****************************************************************************/
void XOcp_CmdsInit(void)
{
	static XPlmi_AccessPerm_t XOcp_AccessPermBuff[XOCP_API_MAX] =
	{
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_FEATURES),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_EXTEND_HWPCR),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GET_HWPCR),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GET_HWPCRLOG),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GENDMERESP),
		XPLMI_ALL_IPI_NO_ACCESS(XOCP_API_DEVAKINPUT),
		XPLMI_ALL_IPI_NO_ACCESS(XOCP_API_GETCERTUSERCFG),
#ifdef PLM_OCP_KEY_MNGMT
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GETX509CERT),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_ATTESTWITHDEVAK),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GEN_SHARED_SECRET),
#ifndef PLM_RSA_EXCLUDE
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_ATTEST_WITH_KEYWRAP_DEVAK),
#endif
#endif
		XPLMI_ALL_IPI_NO_ACCESS(XOCP_API_SET_SWPCRCONFIG),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_EXTEND_SWPCR),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GET_SWPCR),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GET_SWPCRLOG),
		XPLMI_ALL_IPI_FULL_ACCESS(XOCP_API_GET_SWPCRDATA),
	};
	static XPlmi_ModuleCmd XOcp_Cmds[XOCP_API_MAX];
	static XPlmi_Module XPlmi_Ocp =
	{
		XPLMI_MODULE_XILOCP_ID,
		XOcp_Cmds,
		XOCP_API(XOCP_API_MAX),
		NULL,
		XOcp_AccessPermBuff,
#ifdef PLM_OCP_KEY_MNGMT
		XOcp_ShutdownHandler
#else
		NULL
#endif
	};
	u32 Idx;

	/** Register command handlers with XilPlmi */
	for (Idx = 0U; Idx < XPlmi_Ocp.CmdCnt; Idx++) {
		XOcp_Cmds[Idx].Handler = XOcp_ProcessCmd;
	}

	XPlmi_ModuleRegister(&XPlmi_Ocp);
}


/*****************************************************************************/
/**
 * @brief	This function gets the SW PCR configuration and stores it.
 *
 * @param	Cmd - Pointer to the XPlmi_Cmd structure
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_SetSwPcrConfig(const XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;
	u32 Len = Cmd->Len;

	if (Cmd->ProcessedLen != 0U) {
		Status = (int)XOCP_ERR_CHUNK_BOUNDARY_CROSSED;
	}
	else {
		Status = XOcp_StoreSwPcrConfigAndExtendSwPcr_0_1(Pload, Len);
	}

	return Status;
}

#ifdef PLM_OCP_KEY_MNGMT
/*****************************************************************************/
/**
 * @brief	This function processes XilOcp DEVAK input personalised string and
 *			corresponding subsystem IDs
 *
 * @param	Cmd - Pointer to the XPlmi_Cmd structure
 *
 * @return
 *			- XST_SUCCESS - On successful processing
 *          - Error Code - On Failure
 *
 *****************************************************************************/
static int XOcp_DevAkInput(const XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;
	u32 KeyIndex;

	if (Cmd->ProcessedLen != 0U) {
		Status = (int)XOCP_ERR_CHUNK_BOUNDARY_CROSSED;
	}
	else {
		KeyIndex = Pload[XOCP_DEV_AK_INPUT_PLOAD_KEYIDX_INDEX] & XOCP_DEV_AK_INPUT_KEYIDX_MASK;
		Status = XOcp_DevAkInputStore(Pload[0], (u8 *)(UINTPTR)&Pload[1],
			KeyIndex);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets values for user configurable fields for creating
 *		x.509 certificate.
 *
 * @param	Cmd - Pointer to the XPlmi_Cmd structure
 *
 * @return
 *		- XST_SUCCESS - On success
 *		- Error Code - On Failure
 *
 *****************************************************************************/
static int XOcp_GetCertUserCfg(const XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;
	u32 SubsystemId = Pload[0];
	u32 FieldType = (Pload[1] & XOCP_CERT_USERIN_FIELD_MASK) >>
				XOCP_CERT_USERIN_FIELD_SHIFT;
	u32 LenInBytes = Pload[1] & XOCP_CERT_USERIN_LEN_MASK;

	if (Cmd->ProcessedLen != 0U) {
		Status = (int)XOCP_ERR_CHUNK_BOUNDARY_CROSSED;
	}
	else {
		Status = XCert_StoreCertUserInput(SubsystemId, (XCert_UserCfgFields)FieldType,
			(u8 *)(UINTPTR)&Pload[2], LenInBytes);
	}

	return Status;
}
#endif	/* PLM_OCP_KEY_MNGMT */
#endif /* PLM_OCP */

