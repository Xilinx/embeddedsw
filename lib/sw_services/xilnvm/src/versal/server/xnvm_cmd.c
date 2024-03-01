/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/server/xnvm_cmd.c
*
* This file contains the xilnvm IPI handler implementation.
*
* @cond xnvm_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/2021 Initial release
*       kal  07/25/2021 Registered eFUSE IPI handlers
*       kpt  08/27/2021 Added commands to support puf helper data efuse
*                       programming
* 2.4   bsv  09/09/2021 Added PLM_NVM macro
* 2.5   am   02/28/2022 Fixed MISRA C violation rule 4.5
* 3.1   skg  10/04/2022 Added invalid hidden handler for PLM to PLM communication
* 3.2   bm   06/23/2023 Added access permissions for IPI commands
* 3.3   kpt  02/21/2024 Added generic handler
*	vss  02/23/2024	Added IPI support for eFuse read and write
*
* </pre>
*
* @note
*
* @endcond
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xplmi_ssit.h"
#include "xnvm_defs.h"
#include "xnvm_bbram_common_cdohandler.h"
#include "xnvm_cmd.h"
#include "xnvm_efuse_ipihandler.h"

/************************** Function Prototypes ******************************/
static int XNvm_InvalidCmdHandler(u32 *Payload, u32 *RespBuf);
/************************** Constant Definitions *****************************/
static XPlmi_ModuleCmd XNvm_Cmds[XNVM_API_MAX];

/* Structure holding access permissions of nvm module commands */
static XPlmi_AccessPerm_t XNvm_AccessPermBuff[XNVM_API_MAX] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_FEATURES),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_WRITE_AES_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_ZEROIZE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_WRITE_USER_DATA),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_READ_USER_DATA),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_PUF),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_IV),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_REVOCATION_ID),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_USER_FUSES),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_PPK_HASH),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_DNA),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_PUF),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_CACHE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_IV),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC1),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_PUF_DATA),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_OFF_CHIP_ID),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_USER_EFUSE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_PPK_HASH),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_ANLG_TRIM),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_MISC_CTRL),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_SECURITY_CTRL),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC0_CTRL),
};

static XPlmi_Module XPlmi_Nvm =
{
	XPLMI_MODULE_XILNVM_ID,
	XNvm_Cmds,
	XNVM_API(XNVM_API_MAX),
	XNvm_InvalidCmdHandler,
	XNvm_AccessPermBuff,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function calls the handler for invalid commands
 *
 *
 * @param	Payload	   is pointer to payload data
 *
 * @param   RespBuf buffer to store response of slaves
 *
 * @return 	XST_SUCCESS		    on successful communication
 * 		    error code      	On failure
 *
 *****************************************************************************/
static int XNvm_InvalidCmdHandler(u32 *Payload, u32 *RespBuf){
	return XPlmi_SendIpiCmdToSlaveSlr(Payload, RespBuf);
}

/*****************************************************************************/
/**
 * @brief	This function checks for the supported features based on the
 * 		requested API ID
 *
 * @param	ApiId	ApiId to check the supported features
 *
 * @return 	XST_SUCCESS		if the requested API ID is supported
 * 		XST_INVALID_PARAM	On invalid command
 *
 *****************************************************************************/
static int XNvm_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
		case XNVM_API_ID_BBRAM_WRITE_AES_KEY:
		case XNVM_API_ID_BBRAM_ZEROIZE:
		case XNVM_API_ID_BBRAM_WRITE_USER_DATA:
		case XNVM_API_ID_BBRAM_READ_USER_DATA:
		case XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA:
		case XNVM_API_ID_EFUSE_WRITE:
		case XNVM_API_ID_EFUSE_READ_IV:
		case XNVM_API_ID_EFUSE_READ_REVOCATION_ID:
		case XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID:
		case XNVM_API_ID_EFUSE_READ_USER_FUSES:
		case XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS:
		case XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS:
		case XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS:
		case XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS:
		case XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS:
		case XNVM_API_ID_EFUSE_READ_PPK_HASH:
		case XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY:
		case XNVM_API_ID_EFUSE_READ_DNA:
		case XNVM_API_ID_EFUSE_READ_CACHE:
#ifdef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
		case XNVM_API_ID_EFUSE_WRITE_IV:
		case XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC1:
		case XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL:
		case XNVM_API_ID_EFUSE_WRITE_ANLG_TRIM:
		case XNVM_API_ID_EFUSE_WRITE_PUF_DATA:
		case XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC0_CTRL:
#endif
#ifdef XNVM_WRITE_KEY_MANAGEMENT_EFUSE
		case XNVM_API_ID_EFUSE_WRITE_OFF_CHIP_ID:
		case XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID:
		case XNVM_API_ID_EFUSE_WRITE_PPK_HASH:
#endif
#ifdef XNVM_WRITE_USER_EFUSE
		case XNVM_API_ID_EFUSE_WRITE_USER_EFUSE:
#endif
#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE))
		case XNVM_API_ID_EFUSE_WRITE_MISC_CTRL:
		case XNVM_API_ID_EFUSE_WRITE_SECURITY_CTRL:
#endif
#ifdef XNVM_ACCESS_PUF_USER_DATA
		case XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE:
		case XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE:
#else
		case XNVM_API_ID_EFUSE_READ_PUF:
		case XNVM_API_ID_EFUSE_WRITE_PUF:
#endif
			Status = XST_SUCCESS;
			break;
		default:
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Cmd not supported\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function processes XilNvm IPI commands
 *
 * @param	Cmd 	Pointer to the XPlmi_Cmd structure
 *
 * @return 	XST_SUCCESS		On successful IPI processing
 * 		XST_INVALID_PARAM	On invalid command
 * 		Error Code 		On Failure
 *****************************************************************************/
static int XNvm_ProcessCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & 0xFFU) {
		case XNVM_API(XNVM_API_FEATURES):
			Status = XNvm_FeaturesCmd(Pload[0]);
			break;
		case XNVM_API(XNVM_API_ID_BBRAM_WRITE_AES_KEY):
		case XNVM_API(XNVM_API_ID_BBRAM_ZEROIZE):
		case XNVM_API(XNVM_API_ID_BBRAM_WRITE_USER_DATA):
		case XNVM_API(XNVM_API_ID_BBRAM_READ_USER_DATA):
		case XNVM_API(XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA):
			Status = XNvm_BbramCommonCdoHandler(Cmd);
			break;
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_IV):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_REVOCATION_ID):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_USER_FUSES):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_PPK_HASH):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_DNA):
#ifdef XNVM_ACCESS_PUF_USER_DATA
		case XNVM_API(XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE):
#else
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF):
		case XNVM_API(XNVM_API_ID_EFUSE_READ_PUF):
#endif
			Status = XNvm_EfuseIpiHandler(Cmd);
			break;
		case XNVM_API(XNVM_API_ID_EFUSE_READ_CACHE):
			Status =  XNvm_EfuseRead(Pload[0U], Pload[1U],
						 Pload[2U], Pload[3U]);
			break;
#ifdef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC1):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_ANLG_TRIM):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF_DATA):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC0_CTRL):
#endif
#ifdef XNVM_WRITE_KEY_MANAGEMENT_EFUSE
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_OFF_CHIP_ID):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH):
#endif
#ifdef XNVM_WRITE_USER_EFUSE
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_USER_EFUSE):
#endif
#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE))
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_MISC_CTRL):
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_SECURITY_CTRL):
#endif
#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE)) || \
	(defined (XNVM_WRITE_USER_EFUSE))
			Status = XNvm_EfuseWriteAccess(Cmd, Pload[0U], Pload[1U], Pload[2U]);
#endif
			break;
		default:
			XNvm_Printf(XNVM_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the XilNvm commands to the PLMI.
 *
 * @param	GenericHandler Pointer to generic handler
 *
 *****************************************************************************/
void XNvm_CmdsInit(int (*GenericHandler)(void))
{
	u32 Idx;

	(void)GenericHandler;
	/**
     *	Register command handlers with XilPlmi
	 */
	for (Idx = 0U; Idx < XPlmi_Nvm.CmdCnt; Idx++) {
		XNvm_Cmds[Idx].Handler = XNvm_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Nvm);
}

#endif
